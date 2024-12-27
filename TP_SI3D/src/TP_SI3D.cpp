#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_time.h"

#include "Terrain.h"
#include "FrustumCulling.h"

#include "FrameBuffer.h"

Mesh make_frustum( )
{
    glLineWidth(2);    
    Mesh camera= Mesh(GL_LINES);
    
    camera.color(Yellow());
    // face avant
    camera.vertex(-1, -1, -1);
    camera.vertex(-1, 1, -1);
    camera.vertex(-1, 1, -1);
    camera.vertex(1, 1, -1);
    camera.vertex(1, 1, -1);
    camera.vertex(1, -1, -1);
    camera.vertex(1, -1, -1);
    camera.vertex(-1, -1, -1);
    
    // face arriere
    camera.vertex(-1, -1, 1);
    camera.vertex(-1, 1, 1);
    camera.vertex(-1, 1, 1);
    camera.vertex(1, 1, 1);
    camera.vertex(1, 1, 1);
    camera.vertex(1, -1, 1);
    camera.vertex(1, -1, 1);
    camera.vertex(-1, -1, 1);
    
    // aretes
    camera.vertex(-1, -1, -1);
    camera.vertex(-1, -1, 1);
    camera.vertex(-1, 1, -1);
    camera.vertex(-1, 1, 1);
    camera.vertex(1, 1, -1);
    camera.vertex(1, 1, 1);
    camera.vertex(1, -1, -1);
    camera.vertex(1, -1, 1);
    
    return camera;
}

class TP : public AppTime
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : AppTime(1024, 640), m_Terrain() {}
    
    // creation des objets de l'application
    int init( ) override
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        m_WindowFBO.generate(true);

        m_ShadowMap.generateForDepth(4096, 4096);
        m_ShadowFBO.generate();
        m_ShadowFBO.attachTexture(m_ShadowMap, FrameBuffer::TextureAttachment::Depth, 0);

        const auto& specs = m_Terrain.getSpecs();
        m_TerrainSizeX = specs.chunkWidth * specs.chunkX * specs.cubeSize;
        m_TerrainSizeY = specs.chunkWidth * specs.chunkY * specs.cubeSize;
        m_TerrainSizeZ = specs.cubesHeight * specs.cubeSize;
        m_TerrainCenter = Point(m_TerrainSizeX / 2.f, m_TerrainSizeZ / 2.f, m_TerrainSizeY / 2.f);
        float maxVal = std::max(m_TerrainSizeX, std::max(m_TerrainSizeY, m_TerrainSizeZ)) / 2.f;
        m_LightProjection = Ortho(-maxVal, maxVal, -maxVal, maxVal, -maxVal, maxVal);
        
        m_camera.lookat(Point(0.f, 0.f, 0.f), Point(m_TerrainSizeX, m_TerrainSizeY, m_TerrainSizeZ));

        updateLightView(0.f);

        m_DebugLightCube = make_frustum();

        auto imageData = read_image_data("data/CubeWorld/Blocks_PixelArt.png");
        ASSERT(imageData.size > 0, "Could not load data/CubeWorld/Blocks_PixelArt.png");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT(m_SpriteSheetTexture.loadFromImage(imageData), "Could not load spritesheet from image data");
    
        ASSERT(m_CubeShader.load("data/shaders/TP_SI3D/Cube.glsl"), "Could not load data/shaders/TP_SI3D/Cube.glsl");
        ASSERT(m_CubeShadowBuilder.load("data/shaders/TP_SI3D/CubeShadowBuilder.glsl"), "Could not load data/shaders/TP_SI3D/CubeShadowBuilder.glsl");

        return 0;   // pas d'erreur, sinon renvoyer -1
    }
    
    // destruction des objets de l'application
    int quit( ) override
    {
        m_DebugLightCube.release();

        return 0;
    }
    
    // dessiner une nouvelle image
    int render( ) override
    {
        /* PARTIE EVENEMENTS */

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

        if(key_state('c'))
        {
            clear_key_state('c');
            m_CullChunks = !m_CullChunks;
            if (m_CullChunks == false)
            {
                std::cout << "Frustum culling disabled\n";
                m_Terrain.stopCulling();
            }
            else
            {
                std::cout << "Frustum culling enabled\n";
            }
        }

        if(key_state('a'))
        {
            clear_key_state('a');
            updateLightView(-5.f);
        }

        if(key_state('e'))
        {
            clear_key_state('e');
            updateLightView(5.f);
        }

        /* PARTIE GESTION DATA */

        if (m_CullChunks)
            m_Terrain.cullChunks(m_camera.view(), m_camera.projection());

        /* PARTIE RENDU */

        // Première passe : shadow map
        
        updateShadowMap();

        // Deuxième passe : rendu de la scène

        m_WindowFBO.bind();
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
        m_CubeShader.bind();
        m_CubeShader.setUniform("viewMatrix", m_camera.view());
        m_CubeShader.setUniform("projectionMatrix", m_camera.projection());
        m_CubeShader.setTextureUniform(m_ShadowMap, 1, "shadowMap");
        m_CubeShader.setUniform("lightViewMatrix", m_LightView);
        m_CubeShader.setUniform("lightProjectionMatrix", m_LightProjection);
        m_CubeShader.setUniform("lightDir", m_LightDir);
        m_CubeShader.setUniform("lightColor", White());
        m_CubeShader.setTextureUniform(m_SpriteSheetTexture, 0, "spriteSheet");
        
        m_Terrain.draw();

        // Debug : affichage d'une boite pour voir la lumiere
        draw(m_DebugLightCube, Inverse(m_LightProjection * m_LightView), m_camera);

        return 1;
    }

    void updateLightView(float rot)
    {
        static float rotAcc = 0.f;
        rotAcc += rot;
        Transform translation = Translation(Vector(m_TerrainCenter));
        Transform rotation = RotationX(-90.f + rotAcc);
        m_LightView = Inverse(translation * rotation);
        m_LightDir = m_LightView(Vector(0.f, 0.f, -1.f));

        m_ShadowMapDirty = true;
    }

    void updateShadowMap()
    {
        if (m_ShadowMapDirty == false)
            return;

        m_ShadowFBO.bind();
        glViewport(0, 0, m_ShadowMap.getWidth(), m_ShadowMap.getHeight());
        glClear(GL_DEPTH_BUFFER_BIT);

        m_CubeShadowBuilder.bind();
        m_CubeShadowBuilder.setUniform("viewMatrix", m_LightView);
        m_CubeShadowBuilder.setUniform("projectionMatrix", m_LightProjection);
        
        m_Terrain.draw();

        m_ShadowMapDirty = false;
    }

protected:
    Orbiter m_camera;
    Terrain m_Terrain;

    Texture2D m_SpriteSheetTexture;

    Shader m_CubeShader, m_CubeShadowBuilder;

    FrameBuffer m_ShadowFBO, m_WindowFBO;
    Texture2D m_ShadowMap;
    bool m_ShadowMapDirty = true;

    Point m_TerrainCenter;
    float m_TerrainSizeX, m_TerrainSizeY, m_TerrainSizeZ;
    Transform m_LightView, m_LightProjection;
    Vector m_LightDir;

    Mesh m_DebugLightCube;

    bool m_CullChunks = true;
};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
