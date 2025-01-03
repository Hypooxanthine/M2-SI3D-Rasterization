
//! \file tuto_rayons.cpp

#include <vector>
#include <cfloat>
#include <chrono>
#include <random>

#include "vec.h"
#include "mat.h"
#include "color.h"
#include "image.h"
#include "image_io.h"
#include "image_hdr.h"
#include "orbiter.h"
#include "mesh.h"
#include "wavefront.h"

struct Ray
{
    Point o;                // origine
    Vector d;               // direction
    float tmax;             // position de l'extremite, si elle existe. le rayon est un intervalle [0 tmax]
    
    // le rayon est un segment, on connait origine et extremite, et tmax= 1
    Ray( const Point& origine, const Point& extremite ) : o(origine), d(Vector(origine, extremite)), tmax(1) {}
    
    // le rayon est une demi droite, on connait origine et direction, et tmax= \inf
    Ray( const Point& origine, const Vector& direction ) : o(origine), d(direction), tmax(FLT_MAX) {}
    
    // renvoie le point sur le rayon pour t
    Point point( const float t ) const { return o + t * d; }
};


struct Hit
{
    float t;            // p(t)= o + td, position du point d'intersection sur le rayon
    float u, v;         // p(u, v), position du point d'intersection sur le triangle
    int triangle_id;    // indice du triangle dans le mesh
    
    Hit( ) : t(FLT_MAX), u(), v(), triangle_id(-1) {}
    Hit( const float _t, const float _u, const float _v, const int _id ) : t(_t), u(_u), v(_v), triangle_id(_id) {}
    
    // renvoie vrai si intersection
    operator bool ( ) { return (triangle_id != -1); }
};

struct Triangle
{
    Point p;            // sommet a du triangle
    Vector e1, e2;      // aretes ab, ac du triangle
    int id;             // indice du triangle
    
    Triangle( const TriangleData& data, const int _id ) : p(data.a), e1(Vector(data.a, data.b)), e2(Vector(data.a, data.c)), id(_id) {}
    
    /* calcule l'intersection ray/triangle
        cf "fast, minimum storage ray-triangle intersection" 
        
        renvoie faux s'il n'y a pas d'intersection valide (une intersection peut exister mais peut ne pas se trouver dans l'intervalle [0 tmax] du rayon.)
        renvoie vrai + les coordonnees barycentriques (u, v) du point d'intersection + sa position le long du rayon (t).
        convention barycentrique : p(u, v)= (1 - u - v) * a + u * b + v * c
    */
    Hit intersect( const Ray &ray, const float tmax ) const
    {
        Vector pvec= cross(ray.d, e2);
        float det= dot(e1, pvec);
        
        float inv_det= 1 / det;
        Vector tvec(p, ray.o);
        
        float u= dot(tvec, pvec) * inv_det;
        if(u < 0 || u > 1) return Hit();        // pas d'intersection
        
        Vector qvec= cross(tvec, e1);
        float v= dot(ray.d, qvec) * inv_det;
        if(v < 0 || u + v > 1) return Hit();    // pas d'intersection
        
        float t= dot(e2, qvec) * inv_det;
        if(t > tmax || t < 0) return Hit();     // pas d'intersection
        
        return Hit(t, u, v, id);                // p(u, v)= (1 - u - v) * a + u * b + v * c
    }
};

std::vector<Triangle> triangles;
Mesh mesh;

// renvoie la normale au point d'intersection
Vector normal( const Mesh& mesh, const Hit& hit )
{
    // recuperer le triangle du mesh
    const TriangleData& data= mesh.triangle(hit.triangle_id);
    
    // interpoler la normale avec les coordonn�es barycentriques du point d'intersection
    float w= 1 - hit.u - hit.v;
    Vector n= w * Vector(data.na) + hit.u * Vector(data.nb) + hit.v * Vector(data.nc);
    return normalize(n);
}

Hit intersect(const Point& o, const Point& e)
{
    const Ray ray(o, e);
    Hit hit;
    hit.t = ray.tmax;
    for (std::size_t i = 0; i < triangles.size(); i++)
    {
        if (Hit h = triangles[i].intersect(ray, hit.t))
        {
            assert(h.t > 0);
            hit = h;
        }
    }

    return hit;
}

Hit intersect(const Point& o, const Vector& d, const float tmax)
{
    return intersect(o, o + d * tmax);
}

Vector Reflect(const Vector& v, const Vector& n)
{
    return v - 2.f * dot(v, n) * n;
}

const Color L_sun = Color{ 1.f, 1.f, 1.f };

float V_Sun(const Point& p, const Vector& l)
{
    const Vector Y = Vector{ 0.f, 1.f, 0.f };
    if (dot(l, Y) < 0) return 0.f;

    Hit hit = intersect(p, p + l);
    
    return !hit;
}

Color Li(const Point& p, const Vector& l)
{
    if (Hit hit = intersect(p, p + l)) // Intersection avec un triangle ?
    {
        const Material& mat = mesh.triangle_material(hit.triangle_id);
        return mat.emission; // On renvoie son emission : s'il n'est pas emissif, on renverra du noir
    }
    
    // Sinon : ciel ?
    const Vector Y = Vector{ 0.f, 1.f, 0.f };
    if (dot(l, Y) > 0) // Si le rayon l pointe vers le ciel
        return L_sun;

    return Black();
}

constexpr float pdf(const Vector& l)
{
    return 1.f / (2.f * M_PIf);
}

template <typename RNG>
Color Lr(const Point& p, const Vector& o, const Vector& n, RNG& rng, unsigned int samples, int triangleID)
{
    std::uniform_real_distribution<float> uniform(0.f, 1.f);

    Color emissiveTrianglesContrib = Color(0.f, 0.f, 0.f);

    for (size_t i = 0; i < triangles.size(); i++)
    {
        const auto& emission = mesh.triangle_material(i).emission;
        if (emission.max() < 0.00001f)
            continue;

        const Triangle& t = triangles.at(i);

        for (unsigned int s = 0; s < samples; s++)
        {
            const float u1 = uniform(rng);
            const float u2 = uniform(rng);
            
            const float alpha = 1.f - std::sqrt(u1);
            const float beta = (1.f - u2) * std::sqrt(u1);
            const float gamma = u2 * std::sqrt(u1);

            const Point q = alpha * t.p + beta * (t.p + t.e1) + gamma * (t.p + t.e2);

            if (Hit h = intersect(p, q)) // Il faut intersecter le triangle emissif
                if (static_cast<size_t>(h.triangle_id) != i) // Mais un autre triangle peut se trouver au milieu
                    continue;
                
            // Le triangle emissif est bien visible ici

            float pdf = 1.f / (length(cross(t.e1, t.e2)) / 2.f);
            float cos_theta = std::max(0.f, dot(n, normalize(q - p)));

            emissiveTrianglesContrib = emissiveTrianglesContrib + emission * cos_theta / pdf / static_cast<float>(samples) / M_PIf;
        }
    }

    Color fragColor = mesh.triangle_material(triangleID).diffuse * emissiveTrianglesContrib; 
    fragColor.a = 1.f;
    return fragColor;

    /*
    for (unsigned int i = 0; i < samples; i++)
    {
        const float u1 = uniform(rng);
        const float u2 = uniform(rng);

        Vector l;
        {
            const float cos_theta = u1;
            const float sin_theta = std::sqrt(1.f - cos_theta * cos_theta);
            const float phi = 2.f * M_PIf * u2;
            l = normalize(Vector{ std::cos(phi) * sin_theta, sin_theta * std::sin(phi), cos_theta });
        }

        float cos_theta = std::max(0.f, dot(n, l));
        out = out + Li(p, l) * cos_theta / pdf(l);
    }

    Color col = mesh.triangle_material(triangleID).diffuse * L_sun * out / static_cast<float>(samples) / M_PIf;
    col.a = 1.f;
    return col;
    */
}

int main( const int argc, const char **argv )
{
    const char *mesh_filename= "data/cornell.obj";
    if(argc > 1)
        mesh_filename= argv[1];
        
    const char *orbiter_filename= "data/cornell_orbiter.txt";
    if(argc > 2)
        orbiter_filename= argv[2];
    
    Orbiter camera;
    if(camera.read_orbiter(orbiter_filename) < 0)
        return 1;

    mesh= read_mesh(mesh_filename);

    std::random_device hwseed;
    std::default_random_engine rng(hwseed());
    
    {
        int n= mesh.triangle_count();
        for(int i= 0; i < n; i++)
            triangles.emplace_back(mesh.triangle(i), i);
    }

    //
    Image image(1024, 768);

    // recupere les transformations pour generer les rayons
    camera.projection(image.width(), image.height(), 45);
    Transform model= Identity();
    Transform view= camera.view();
    Transform projection= camera.projection();
    Transform viewport= camera.viewport();
    Transform inv= Inverse(viewport * projection * view * model);
    
auto start= std::chrono::high_resolution_clock::now();
    
    // parcours tous les pixels de l'image
    for(int y= 0; y < image.height(); y++)
    for(int x= 0; x < image.width(); x++)
    {
        // generer le rayon au centre du pixel
        Point origine= inv(Point(x + float(0.5), y + float(0.5), 0));
        Point extremite= inv(Point(x + float(0.5), y + float(0.5), 1));
        Ray ray(origine, extremite);
        
        // calculer les intersections avec tous les triangles
        Hit hit = intersect(origine, extremite);

        if(hit)
        {
            Vector n = normal(mesh, hit);
            // Couleur par l'estimateur de Monte-Carlo
            const Point p = ray.o + hit.t * ray.d;
            image(x, y) = Lr(p + 0.0001f * n, -normalize(ray.d), n, rng, 512, hit.triangle_id);
        }
    }

auto stop= std::chrono::high_resolution_clock::now();
    int cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    printf("%dms\n", cpu);
    
    write_image(image, "render.png");
    write_image_hdr(image, "render.hdr");
    return 0;
}
