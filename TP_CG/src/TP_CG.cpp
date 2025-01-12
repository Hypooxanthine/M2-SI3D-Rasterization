
//! \file tuto9.cpp utilisation d'un shader 'utilisateur' pour afficher un objet Mesh

#include <cmath>

#include "mat.h"
#include "mesh.h"
#include "wavefront.h"

#include "orbiter.h"
#include "program.h"
#include "uniforms.h"
#include "draw.h"
#include "texture.h"

#include "app_time.h"        // classe Application a deriver

#include "FrameBuffer.h"
#include "ShaderStorageBufferObject.h"
#include "Shader.h"
#include "ComputeShader.h"

struct PointLight
{
    alignas(16) Point position;
    alignas(16) Point color;
    float intensity;
    float radius;
};


// utilitaire. creation d'une grille / repere.
Mesh make_grid( const int n= 10 )
{
    Mesh grid= Mesh(GL_LINES);
    
    // grille
    grid.color(White());
    for(int x= 0; x < n; x++)
    {
        float px= float(x) - float(n)/2 + .5f;
        grid.vertex(Point(px, 0, - float(n)/2 + .5f));
        grid.vertex(Point(px, 0, float(n)/2 - .5f));
    }

    for(int z= 0; z < n; z++)
    {
        float pz= float(z) - float(n)/2 + .5f;
        grid.vertex(Point(- float(n)/2 + .5f, 0, pz)); 
        grid.vertex(Point(float(n)/2 - .5f, 0, pz)); 
    }
    
    // axes XYZ
    grid.color(Red());
    grid.vertex(Point(0, 0.1, 0));
    grid.vertex(Point(1, 0.1, 0));
    
    grid.color(Green());
    grid.vertex(Point(0, 0.1, 0));
    grid.vertex(Point(0, 1.1, 0));
    
    grid.color(Blue());
    grid.vertex(Point(0, 0.1, 0));
    grid.vertex(Point(0, 0, 1));
    
    return grid;
}

class TP : public AppTime
{
public:
    TP( ) : AppTime(1024, 640) {}
    
    int init( )
    {
        m_objet= read_mesh("data/dragon.obj");
        
        Point min, max;
        m_objet.bounds(min, max);
        m_camera.lookat(min, max);

        // etape 1 : creer le shader program
        m_GShader.generate();
        m_FirstPassColorsShader.generate();
        m_SecondPassColorShader.generate();
        m_FullColorsShader.generate();

        m_ShadersCompileOK = true;
        m_ShadersCompileOK = m_ShadersCompileOK && m_GShader.reload(s_GShaderPath);
        m_ShadersCompileOK = m_ShadersCompileOK && m_FirstPassColorsShader.reload(s_FirstPassColors);
        m_ShadersCompileOK = m_ShadersCompileOK && m_SecondPassColorShader.reload(s_SecondPassColors);
        m_ShadersCompileOK = m_ShadersCompileOK && m_FullColorsShader.reload(s_FullColors);

        if (m_ShadersCompileOK)
            std::cout << "Shaders compiled successfully" << std::endl;

        // Initialisation des textures
        zbufferTexture.generateForDepth(s_TexturesWidth, s_TexturesHeight);
        position_matid_Texture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);
        normalTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);
        albedoTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);
        metallic_diffuse_shininessTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);
        colorTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);

        // Initialisation du framebuffer de la fenêtre.
        // onScreen = true va faire que le framebuffer aura un renderID de 0.
        // Donc quand on va le bind, on passera sur le framebuffer par défaut
        // permettant de dessiner dans la surface de rendu de la fenêtre SDL.
        windowFrameBuffer.generate(true);

        // Initialisation du frambuffer pour le deffered rendering (GBuffer)
        gFrameBuffer.generate();
        gFrameBuffer.attachTexture(position_matid_Texture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(normalTexture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(albedoTexture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(metallic_diffuse_shininessTexture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(zbufferTexture, FrameBuffer::TextureAttachment::Depth, 0);
        gFrameBuffer.setupBindings();

        // Initialisation du frambuffer intermédiaire qui contient la sortie du compute shader
        intermediateFrameBuffer.generate();
        intermediateFrameBuffer.attachTexture(colorTexture, FrameBuffer::TextureAttachment::Color, 0);
        intermediateFrameBuffer.setupBindings();

        // Initialisation du shader storage buffer object pour les lumières
        lights.first = 2;
        lights.second = {
            PointLight{{0, 5, 5}, {1, 1, 1}, 1.f, 100.f},
            PointLight{{5, 5, 0}, {1, 1, 1}, 1.f, 100.f}
        };
        lightsSSBO.generate();
        lightsSSBO.setBindingPoint(0);
        lightsSSBO.setData(&lights.first, sizeof(unsigned int) + lights.second.size() * sizeof(PointLight));

        // Face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);


        // Initialisation des vecteurs pour la prise en compte des pixels pour l'interpolation
        dAdiag = { -1,  1 }; dAdiag2 = { -2,  2 };
        dBdiag = {  1,  1 }; dBdiag2 = {  2,  2 };
        dCdiag = {  1, -1 }; dCdiag2 = {  2, -2 };
        dDdiag = { -1, -1 }; dDdiag2 = { -2, -2 };

        dAortho = { -1,  0 }; dAortho2 = { -2,  0 };
        dBortho = {  0,  1 }; dBortho2 = {  0,  2 };
        dCortho = {  1,  0 }; dCortho2 = {  2,  0 };
        dDortho = {  0, -1 }; dDortho2 = {  0, -2 };

        return 0;   // ras, pas d'erreur
    }
    
    int quit( )
    {
        m_objet.release();
        return 0;
    }
    
    int render( )
    {
        // recupere l'etat de la souris
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        
        // deplace la camera
        if(mb & SDL_BUTTON(1))
            m_camera.rotation(mx, my);      // tourne autour de l'objet
        else if(mb & SDL_BUTTON(3))
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height()); // deplace le point de rotation
        else if(mb & SDL_BUTTON(2))
            m_camera.move(mx);           // approche / eloigne l'objet
        
        // recupere l'etat de la molette / touch
        SDL_MouseWheelEvent wheel= wheel_event();
        if(wheel.y != 0)
        {
            clear_wheel_event();
            m_camera.move(8.f * wheel.y);  // approche / eloigne l'objet
        }
        
        // recharge le shader program
        if(key_state('r'))
        {
            clear_key_state('r');        // une seule fois...
            if (!m_GShader.reload(s_GShaderPath))
            {
                std::cerr << "Couldn't reload " << s_GShaderPath << "\n";
                return 1;
            }
            if (!m_FirstPassColorsShader.reload(s_FirstPassColors))
            {
                std::cerr << "Couldn't reload " << s_FirstPassColors << "\n";
                return 1;
            }
            if (!m_SecondPassColorShader.reload(s_SecondPassColors))
            {
                std::cerr << "Couldn't reload " << s_SecondPassColors << "\n";
                return 1;
            }
            if (!m_FullColorsShader.reload(s_FullColors))
            {
                std::cerr << "Couldn't reload " << s_FullColors << "\n";
                return 1;
            }

            std::cout << "Shaders compiled successfully" << std::endl;
        }

        if (key_state('t'))
        {
            clear_key_state('t');
            m_UseInterpolation = !m_UseInterpolation;
        }
        
        // etape 2 : dessiner m_objet avec le shader program
        // configurer le pipeline 
        gFrameBuffer.bind();
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, window_width(), window_height());
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClearDepth(1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_GShader.bind();

        // Transformations MVP
        Transform model = Identity();// RotationX(global_time() / 20);
        Transform normalMatrix = model.normal();
        Transform view = m_camera.view();
        Transform projection = m_camera.projection(window_width(), window_height(), 45);
        Transform mv = view * model;
        Transform mvp = projection * mv;
        
        // Uniforms
        m_GShader.setUniform("modelMatrix", model);
        m_GShader.setUniform("mvpMatrix", mvp);
        m_GShader.setUniform("normalMatrix", normalMatrix);
        
        // Draw calls
        m_objet.draw(m_GShader.getRenderId(), /* use position */ true, /* use texcoord */ false, /* use normal */ true, /* use color */ false, /* use material index*/ false);

        /* Deuxième passe : on envoie les textures au compute shader */

        // D'abord, on nettoie le framebuffer intermédiaire
        intermediateFrameBuffer.bind();
        glViewport(0, 0, window_width(), window_height());
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (m_UseInterpolation)
        {

            /* Calculer un pixel sur 16 */

            // On utilise le premier compute shader pour calculer un pixel sur 16
            m_FirstPassColorsShader.bind();

            // On envoie les données du gbuffer (entre autres) au compute shader
            setComputeShaderData(m_FirstPassColorsShader, GL_WRITE_ONLY);

            m_FirstPassColorsShader.dispatch(window_width() / 16, window_height() / 16, 1);

            // On attend que le compute shader ait fini
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            /* Calculer ou interpoler les autres pixels */

            // On utilise le second compute shader
            m_SecondPassColorShader.bind();

            // On envoie les données du gbuffer (entre autres) au compute shader
            // En read/write ici car on a besoin de récupérer les valeurs des pixels
            // déjà calculés, pour les interpoler justement
            setComputeShaderData(m_SecondPassColorShader, GL_READ_WRITE);
            
            m_SecondPassColorShader.setUniform("fillMask", 0b0000010000000000);
            m_SecondPassColorShader.setUniform("dA", dAdiag2);
            m_SecondPassColorShader.setUniform("dB", dBdiag2);
            m_SecondPassColorShader.setUniform("dC", dCdiag2);
            m_SecondPassColorShader.setUniform("dD", dDdiag2);

            m_SecondPassColorShader.dispatch(window_width() / 16, window_height() / 16, 1);

            m_SecondPassColorShader.setUniform("fillMask", 0b0000000100000100);
            m_SecondPassColorShader.setUniform("dA", dAortho2);
            m_SecondPassColorShader.setUniform("dB", dBortho2);
            m_SecondPassColorShader.setUniform("dC", dCortho2);
            m_SecondPassColorShader.setUniform("dD", dDortho2);

            m_SecondPassColorShader.dispatch(window_width() / 16, window_height() / 16, 1);

            m_SecondPassColorShader.setUniform("fillMask", 0b1010000010100000);
            m_SecondPassColorShader.setUniform("dA", dAdiag);
            m_SecondPassColorShader.setUniform("dB", dBdiag);
            m_SecondPassColorShader.setUniform("dC", dCdiag);
            m_SecondPassColorShader.setUniform("dD", dDdiag);

            m_SecondPassColorShader.dispatch(window_width() / 16, window_height() / 16, 1);

            m_SecondPassColorShader.setUniform("fillMask", 0b0101101001011010);
            m_SecondPassColorShader.setUniform("dA", dAortho);
            m_SecondPassColorShader.setUniform("dB", dBortho);
            m_SecondPassColorShader.setUniform("dC", dCortho);
            m_SecondPassColorShader.setUniform("dD", dDortho);

            m_SecondPassColorShader.dispatch(window_width() / 16, window_height() / 16, 1);

            // On attend que le compute shader ait fini
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        else
        {

            /* Calculer tous les pixels */

            m_FullColorsShader.bind();

            // On envoie les données du gbuffer (entre autres) au compute shader
            setComputeShaderData(m_FullColorsShader, GL_WRITE_ONLY);

            m_FullColorsShader.dispatch(window_width() / 16, window_height() / 16, 1);

            // On attend que le compute shader ait fini
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        /* Récupérer la texture calculée par les compute shaders pour l'afficher */

        // On place la texture depuis le framebuffer intermédiaire
        // vers le framebuffer de la fenêtre (renderid = 0)
        windowFrameBuffer.blitFrom(intermediateFrameBuffer, window_width(), window_height());

        // On utilise maintenant le framebuffer par défaut pour le swap buffers
        windowFrameBuffer.bind();
        return 1;
    }

    void setComputeShaderData(ComputeShader& shader, GLenum readwrite)
    {
        GLuint slot = 0;
        shader.setTextureUniform(position_matid_Texture, slot++, "g_position_matid");
        shader.setTextureUniform(normalTexture, slot++, "g_normal");
        shader.setTextureUniform(albedoTexture, slot++, "g_albedo");
        shader.setTextureUniform(metallic_diffuse_shininessTexture, slot++, "g_metallic_diffuse_shininess");
        shader.setUniform("cameraPos", m_camera.position());

        // Pour que le compute shader sache quelle est la vraie taille de la frame
        // (les dimensions de la frame ne sont pas forcément un multiple de 16)
        shader.setUniform("frameWidth", window_width());
        shader.setUniform("frameHeight", window_height());

        // On envoie les lumières au compute shader
        lightsSSBO.bind();

        // Texture sur laquelle on va écrire les couleurs finales depuis le compute shader
        glBindImageTexture(0, colorTexture.getRenderId(), 0, GL_FALSE, 0, readwrite, GL_RGBA32F);
    }

protected:
    Mesh m_objet;
    Orbiter m_camera;
    Shader m_GShader;
    ComputeShader m_FirstPassColorsShader, m_SecondPassColorShader, m_FullColorsShader;
    bool m_ShadersCompileOK = true;

    Texture2D zbufferTexture;
    Texture2D position_matid_Texture, normalTexture, albedoTexture, metallic_diffuse_shininessTexture;
    Texture2D colorTexture; // Cette texture contiendra la couleur finale de la frame
    FrameBuffer windowFrameBuffer, gFrameBuffer, intermediateFrameBuffer;
    ShaderStorageBufferObject lightsSSBO;

    std::pair<unsigned int, std::array<PointLight, 2>> lights;
    std::array<int, 2> dAdiag, dBdiag, dCdiag, dDdiag, dAdiag2, dBdiag2, dCdiag2, dDdiag2;
    std::array<int, 2> dAortho, dBortho, dCortho, dDortho, dAortho2, dBortho2, dCortho2, dDortho2;

    bool m_UseInterpolation = true;

    static constexpr size_t s_TexturesWidth = 4096, s_TexturesHeight = 4096;
    static constexpr std::string_view s_GShaderPath = "TP_CG/shaders/gshader.glsl";
    static constexpr std::string_view s_FirstPassColors = "TP_CG/shaders/first_pass_colors.glsl";
    static constexpr std::string_view s_SecondPassColors = "TP_CG/shaders/second_pass_colors.glsl";
    static constexpr std::string_view s_FullColors = "TP_CG/shaders/full_colors.glsl";
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}

