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
    TP( ) : AppCamera(1024, 640) {}
    
    // creation des objets de l'application
    int init( )
    {        
        // charge un objet
        m_cube= read_mesh("data/cube.obj");
        
        // un autre objet
        m_objet= Mesh(GL_TRIANGLES);
        {
            // ajouter des triplets de sommet == des triangles dans objet...
        }
        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        return 0;   // pas d'erreur, sinon renvoyer -1
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        m_objet.release();
        return 0;   // pas d'erreur
    }
    
    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_Terrain.draw(camera().view(), camera().projection());
        
    // comment dessiner m_objet ?? 
    
    // et sans le superposer au cube deja dessine ?
        
        // continuer, afficher une nouvelle image
        // tant que la fenetre est ouverte...
        return 1;
    }

protected:
    Mesh m_objet;
    Mesh m_cube;
    Terrain m_Terrain;
};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
