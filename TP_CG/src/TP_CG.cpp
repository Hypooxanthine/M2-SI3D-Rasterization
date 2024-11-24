
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

struct PointLight
{
    Point position;
    Color color;
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

// utilise AppTime pour les screenshots...
class TP : public AppTime
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : AppTime(1024, 640) {}
    
    int init( )
    {
        m_grid= make_grid();
        m_objet= read_mesh("data/cube.obj");
        
        Point pmin, pmax;
        m_grid.bounds(pmin, pmax);
        m_camera.lookat(pmin, pmax);

        // etape 1 : creer le shader program
        m_GShader= read_program(s_GShaderPath.data());
        m_ShadersCompileOK = m_ShadersCompileOK && !program_print_errors(m_GShader);
        m_ColorsComputeShader = read_program(s_ColorsComputeShaderPath.data());
        m_ShadersCompileOK = m_ShadersCompileOK && !program_print_errors(m_ColorsComputeShader);

        if (m_ShadersCompileOK)
            std::cout << "Shaders compiled successfully" << std::endl;
        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        // Initialisation des textures
        zbufferTexture.generateForDepth(s_TexturesWidth, s_TexturesHeight);
        position_matid_Texture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);
        normal_Shininess_Texture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);
        ambientTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, false);
        diffuseTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, false);
        specularTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, false);
        colorTexture.generateForColor(s_TexturesWidth, s_TexturesHeight, true);

        // Initialisation du framebuffer de la fenêtre.
        // onScreen = true va faire que le framebuffer aura un renderID de 0.
        // Donc quand on va le bind, on passera sur le framebuffer par défaut
        // permettant de dessiner dans la surface de rendu de la fenêtre SDL.
        windowFrameBuffer.generate(true);

        // Initialisation du frambuffer pour le deffered rendering (GBuffer)
        gFrameBuffer.generate();
        gFrameBuffer.attachTexture(zbufferTexture, FrameBuffer::TextureAttachment::Depth, 0);
        gFrameBuffer.attachTexture(position_matid_Texture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(normal_Shininess_Texture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(ambientTexture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(diffuseTexture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.attachTexture(specularTexture, FrameBuffer::TextureAttachment::Color, 0);
        gFrameBuffer.setupBindings();

        // Initialisation du frambuffer intermédiaire qui contient la sortie du compute shader
        intermediateFrameBuffer.generate();
        intermediateFrameBuffer.attachTexture(colorTexture, FrameBuffer::TextureAttachment::Color, 0);
        intermediateFrameBuffer.setupBindings();

        // Initialisation du shader storage buffer object pour les lumières
        lights.first = 2;
        lights.second = {
            PointLight{Point(5, 5, 0), Color(1, 0, 0), 10.f, 100.f},
            PointLight{Point(0, 5, 5), Color(0, 0, 1), 10.f, 100.f}
        };
        lightsSSBO.generate();
        lightsSSBO.setBindingPoint(0);
        lightsSSBO.setData(&lights.first, sizeof(unsigned int) + lights.second.size() * sizeof(PointLight));

        return 0;   // ras, pas d'erreur
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        // etape 3 : detruire le shader program
        if (m_GShader)
            release_program(m_GShader);
        if (m_ColorsComputeShader)
            release_program(m_ColorsComputeShader);
        // et les objets
        m_grid.release();
        m_objet.release();
        return 0;
    }
    
    // dessiner une nouvelle image
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
            m_ShadersCompileOK = true;
            reload_program(m_GShader, s_GShaderPath.data());
            m_ShadersCompileOK = m_ShadersCompileOK && !program_print_errors(m_GShader);
            reload_program(m_ColorsComputeShader, s_ColorsComputeShaderPath.data());
            m_ShadersCompileOK = m_ShadersCompileOK && !program_print_errors(m_ColorsComputeShader);

            if (m_ShadersCompileOK)
                std::cout << "Shaders compiled successfully" << std::endl;
        }

        if (!m_ShadersCompileOK)
            return 1;
        
        // etape 2 : dessiner m_objet avec le shader program
        // configurer le pipeline 
        gFrameBuffer.bind();
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(m_GShader);

        // configurer le shader program
        // . recuperer les transformations
        Transform model= RotationX(global_time() / 20);
        Transform normalMatrix = model.normal();
        Transform view= m_camera.view();
        Transform projection= m_camera.projection(window_width(), window_height(), 45);
        
        // . composer les transformations : model, view et projection
        Transform mv= view * model;
        Transform mvp= projection * mv;
        
        // . parametrer le shader program :
        //   . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
        program_uniform(m_GShader, "modelMatrix", model);
        program_uniform(m_GShader, "mvpMatrix", mvp);
        program_uniform(m_GShader, "normalMatrix", normalMatrix);
        
        // . parametres "supplementaires" :
        
        // go !
        // mesh associe les donnees positions, texcoords, normals et colors aux attributs declares dans le vertex shader
        m_objet.draw(m_GShader, /* use position */ true, /* use texcoord */ false, /* use normal */ true, /* use color */ false, /* use material index*/ false);

        // dessine aussi le repere et la grille pour le meme point de vue
        draw(m_grid, Identity(), m_camera);

        /* Deuxième passe : on envoie les textures au compute shader */

        // D'abord, on nettoie le framebuffer intermédiaire
        intermediateFrameBuffer.bind();
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT);

        // On utilise le compute shader
        glUseProgram(m_ColorsComputeShader);

        // On envoie les données du gbuffer au compute shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, position_matid_Texture.getRenderId());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal_Shininess_Texture.getRenderId());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ambientTexture.getRenderId());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture.getRenderId());
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, specularTexture.getRenderId());

        // Paramètres des threads du compute shader
        int localSizeX = 16;
        int localSizeY = 16;

        int groupCountX = (window_width() + localSizeX - 1) / localSizeX;
        int groupCountY = (window_height() + localSizeY - 1) / localSizeY;

        // Pour que le compute shader sache quelle est la vraie taille de la frame
        // (les dimensions de la frame ne sont pas forcément un multiple de 16)
        program_uniform(m_ColorsComputeShader, "frameWidth", window_width());
        program_uniform(m_ColorsComputeShader, "frameHeight", window_height());

        // On envoie les lumières au compute shader
        lightsSSBO.bind();

        // Texture sur laquelle on va écrire les couleurs finales depuis le compute shader
        glBindImageTexture(0, colorTexture.getRenderId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(groupCountX, groupCountY, 1);

        // On attend que le compute shader ait fini
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // On dessine la texture contenant les couleurs finales
        windowFrameBuffer.blitFrom(intermediateFrameBuffer, window_width(), window_height());
        windowFrameBuffer.bind();
        
        return 1;
    }

protected:
    Mesh m_objet;
    Mesh m_grid;
    Orbiter m_camera;
    GLuint m_GShader, m_ColorsComputeShader;
    bool m_ShadersCompileOK = true;

    Texture2D zbufferTexture;
    Texture2D position_matid_Texture, normal_Shininess_Texture, ambientTexture, diffuseTexture, specularTexture;
    Texture2D colorTexture; // Cette texture contiendra la couleur finale de la frame
    FrameBuffer windowFrameBuffer, gFrameBuffer, intermediateFrameBuffer;
    ShaderStorageBufferObject lightsSSBO;

    std::pair<unsigned int, std::array<PointLight, 2>> lights;

    static constexpr size_t s_TexturesWidth = 4096, s_TexturesHeight = 4096;
    static constexpr std::string_view s_GShaderPath = "TP_CG/shaders/gshader.glsl";
    static constexpr std::string_view s_ColorsComputeShaderPath = "TP_CG/shaders/compute_colors.glsl";
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}

