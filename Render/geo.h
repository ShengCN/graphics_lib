#pragma once
#include <common.h>
#include <Render/mesh.h>

/** 
 * Geometry Processing Libarray 
 *  1. Basic geometry datastructure
 *      a. geo_mesh
 *      b. geo_face
 *      c. geo_edge
 *      d. geo_point
 * 
 *  2. Basic geometry processing functions
 *      a. construct a geo mesh
 * 
 **/
struct geo_point {
public:
    glm::vec3 p;

    geo_point(vec3 p):p(p) {}
    ~geo_point();
};

struct geo_face;
struct geo_edge {
    std::shared_ptr<geo_point> h, t; // head, tail
    std::shared_ptr<geo_edge> twin;
    std::shared_ptr<geo_edge> next; 
    std::shared_ptr<geo_face> face;

    geo_edge();
    geo_edge(std::shared_ptr<geo_point> h, std::shared_ptr<geo_point> t); 
    ~geo_edge();
};

struct geo_face {
    std::shared_ptr<geo_edge> edge_head;

    geo_face(std::shared_ptr<geo_edge> e):edge_head(e) {};
    ~geo_face();
    vec3 normal();
};

struct geo_mesh {
    std::vector<std::shared_ptr<geo_face>> faces; 
    std::vector<std::shared_ptr<geo_point>> points;
    std::vector<std::shared_ptr<geo_edge>> edges;

    geo_mesh();
    geo_mesh(std::shared_ptr<mesh> m); 
    ~geo_mesh();

private:
    /* Avoid duplicates */
    std::shared_ptr<geo_face> add_face(vec3 p0,vec3 p1, vec3 p2);
    std::shared_ptr<geo_point> add_point(vec3 p);
    std::shared_ptr<geo_edge> add_edge(vec3 p0, vec3 p1);
};

std::vector<std::shared_ptr<geo_edge>> compute_sihouette(std::shared_ptr<mesh> mesh_ptr, vec3 p);