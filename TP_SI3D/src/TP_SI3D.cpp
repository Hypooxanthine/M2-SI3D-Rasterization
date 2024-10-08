#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_camera.h"

#include "Terrain.h"


class TP : public AppCamera
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : AppCamera(1024, 640), m_Terrain() {}
    
    // creation des objets de l'application
    int init( ) override
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        return 0;   // pas d'erreur, sinon renvoyer -1
    }
    
    // destruction des objets de l'application
    int quit( ) override
    {
        return 0;
    }
    
    // dessiner une nouvelle image
    int render( ) override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_Terrain.draw(camera().view(), camera().projection());
        
        return 1;
    }

protected:
    Terrain m_Terrain;
};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
