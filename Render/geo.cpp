#include "geo.h"
#include "Utilities/Utils.h"
#include <string>
#include <omp.h>

geo_mesh::geo_mesh() {}

geo_mesh::geo_mesh(std::shared_ptr<mesh> m) {
    if (m == nullptr) {
        INFO("Input null");
        return;
    }

    auto world_verts = m->compute_world_space_coords();

    for (int i = 0; i <  world_verts.size() / 3; ++i) {
        add_face(world_verts[3 * i + 0], world_verts[3 * i + 1],
            world_verts[3 * i + 2]);
    }
}

geo_mesh::~geo_mesh() {}

std::shared_ptr<geo_face> geo_mesh::add_face(vec3 p0, vec3 p1, vec3 p2) {
    /* counter clockwise order */
    auto edge_head = add_edge(p0, p1);
    auto e1 = add_edge(p1, p2);
    auto e2 = add_edge(p2, p0);

    edge_head->next = e1;
    e1->next = e2;
    e2->next = edge_head;
    
    auto new_face = std::make_shared<geo_face>(edge_head);
    faces.push_back(new_face);
    
    edge_head->face = new_face;
    e1->face = new_face;
    e2->face = new_face;

    return new_face;
}

std::shared_ptr<geo_point> geo_mesh::add_point(vec3 p) {
    int found_point_id = -1;
    for(int i = 0; i < points.size(); ++i) {
        std::shared_ptr<geo_point> point = points[i];
        if (pd::same_point(point->p, p)) {
            found_point_id = i;
        }
    }    
    
    if (found_point_id != -1) {
        return points[found_point_id];
    }
    auto new_point = std::make_shared<geo_point>(p);
    points.push_back(new_point);

    return new_point;
}

std::shared_ptr<geo_edge> geo_mesh::add_edge(vec3 p0, vec3 p1) {
    auto head = add_point(p0);
    auto tail = add_point(p1);

    auto new_edge = std::make_shared<geo_edge>(head, tail);

    /* find twin edge, avoid duplicate */
    std::shared_ptr<geo_edge> twin = nullptr;

    for(int i = 0; i < edges.size(); ++i) {
        auto e = edges[i];
        if (e->h == head && e->t == tail) {
            new_edge = e;
        }

        if (e->t == head && e->h == tail) {            
            twin = e;
        }
    }    
    
    if (twin != nullptr) {
        new_edge->twin  = twin;
        twin->twin = new_edge;
    }
    
    edges.push_back(new_edge);
    return new_edge;
}

std::vector<std::shared_ptr<geo_edge>>
compute_sihouette(std::shared_ptr<geo_mesh> mesh_ptr, vec3 p) {
    if (mesh_ptr == nullptr) {
        INFO("Cannot find the mesh");
        return {};
    }

    /* Iterate over all edges, sihouette edges are those have different dot
     * products relative to p */
    auto &cur_mesh = *mesh_ptr;
    std::vector<std::shared_ptr<geo_edge>> ret;

    for (int i = 0; i < cur_mesh.edges.size(); ++i ) {
        auto edge = cur_mesh.edges[i];
        if (edge->twin == nullptr) {
            // INFO("Check mesh, one edge does not have twin");
            continue;
        }

        vec3 cur_normal = edge->face->normal();
        // back face culling
        vec3 d = p - edge->h->p;
        if (glm::dot(d, cur_normal) <= 0.0f) {
            continue;
        }

        vec3 neighbor_normal = edge->twin->face->normal();
        if (glm::dot(d, cur_normal) * glm::dot(d, neighbor_normal) < 0.0f) {
            /** check if it is in ret to avoid duplicates **/
            bool found = false;

            for (auto ret_edge : ret) {
                if (ret_edge == edge || ret_edge == edge->twin) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                ret.push_back(edge);
            }
        }
    }

    return ret;
}

std::vector<vec3> compute_shadow_volume(std::shared_ptr<geo_mesh> mesh_ptr, vec3 p) {
	std::vector<std::shared_ptr<geo_edge>> sihouettes = compute_sihouette(mesh_ptr, p);
    std::vector<vec3> ret(sihouettes.size() * 6);
    int counter = 0;
	for(auto e:sihouettes) {
		vec3 h = e->h->p, t = e->t->p;
		vec3 lh_vec = glm::normalize(h-p), lt_vec = glm::normalize(t - p);
		
        ret[counter++] = h;
        ret[counter++] = (h + lh_vec * 1e9f);
        ret[counter++] = (t + lt_vec * 1e9f);

        ret[counter++] = (h);
        ret[counter++] = (t + lt_vec * 1e9f);
        ret[counter++] = (t);
	}
    
    return ret;
}

geo_face::~geo_face() {}

vec3 geo_face::normal() {
    vec3 p0 = edge_head->h->p;
    vec3 p1 = edge_head->t->p;
    vec3 p2 = edge_head->next->t->p;
    return glm::cross(p1-p0, p2-p1);
}

geo_edge::geo_edge() { h = t = nullptr; }

geo_edge::geo_edge(std::shared_ptr<geo_point> h, std::shared_ptr<geo_point> t)
    : h(h), t(t) {}

geo_edge::~geo_edge() {}

geo_point::~geo_point() {}
