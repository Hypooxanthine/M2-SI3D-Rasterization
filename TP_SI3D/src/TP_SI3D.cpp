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
        float terrainSizeX = specs.chunkWidth * specs.chunkX * specs.cubeSize;
        float terrainSizeY = specs.chunkWidth * specs.chunkY * specs.cubeSize;
        float terrainSizeZ = specs.cubesHeight * specs.cubeSize;
        m_TerrainCenter = Point(terrainSizeX / 2.f, terrainSizeY / 2.f, terrainSizeZ / 2.f);
        m_camera.lookat(Point(0.f, 0.f, 0.f), Point(terrainSizeX, terrainSizeY, terrainSizeZ));
        
        Transform translation = Translation(-Vector(m_TerrainCenter));
        Transform rotation = RotationX(-90.f);
        m_LightPos = Point(terrainSizeX / 2.f, terrainSizeY / 2.f, terrainSizeZ);
        m_LightView = translation * rotation;
        m_LightProjection = Ortho(-terrainSizeX, terrainSizeX, -terrainSizeY, terrainSizeY, 0.f, terrainSizeZ * 2.f);

        m_DebugLightCube = make_frustum();

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

        /* PARTIE GESTION DATA */

        updateLightView();

        if (m_CullChunks)
            m_Terrain.cullChunks(m_camera.view(), m_camera.projection());

        /* PARTIE RENDU */

        // Première passe : shadow map

        m_ShadowFBO.bind();
        glViewport(0, 0, m_ShadowMap.getWidth(), m_ShadowMap.getHeight());
        glClear(GL_DEPTH_BUFFER_BIT);
        
        m_Terrain.draw(m_LightView, m_LightProjection);

        // Deuxième passe : rendu de la scène

        m_WindowFBO.bind();
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_Terrain.draw(m_camera.view(), m_camera.projection());

        // Debug : affichage d'une boite pour voir la lumiere
        draw(m_DebugLightCube, Inverse(m_LightProjection * m_LightView), m_camera);        
        
        return 1;
    }

    void updateLightView()
    {
        const auto& specs = m_Terrain.getSpecs();
        float terrainSizeX = specs.chunkWidth * specs.chunkX * specs.cubeSize;
        float terrainSizeY = specs.chunkWidth * specs.chunkY * specs.cubeSize;
        float terrainSizeZ = specs.cubesHeight * specs.cubeSize;
    }

protected:
    Orbiter m_camera;
    Terrain m_Terrain;

    FrameBuffer m_ShadowFBO, m_WindowFBO;
    Texture2D m_ShadowMap;

    Point m_LightPos, m_TerrainCenter;
    Transform m_LightView, m_LightProjection, m_LightViewport;

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
