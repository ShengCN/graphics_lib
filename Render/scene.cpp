#include <common.h>
#include "scene.h"
#include "Utilities/model_loader.h"

scene::scene() {
}

scene::~scene() {
}


std::string scene::to_json() {
    using namespace rapidjson;
    StringBuffer s;
    Writer<StringBuffer> writer(s);

    /* Scene Level */
    writer.StartObject();
    for(auto &mpair:m_meshes) {
        int id = mpair.first;
        writer.Key(fmt::format("{}", id).c_str());
        std::string scene_json = m_meshes.at(id)->m->to_json();
        writer.RawValue(scene_json.c_str(), scene_json.size(), rapidjson::Type::kStringType);
    }
    writer.EndObject();

    return s.GetString();
}

std::shared_ptr<mesh> scene::get_plane_mesh(vec3 p, vec3 n) {
    vec3 x(1.0f,0.0f,0.0f), y(0.0f,1.0f,0.0f);

    std::shared_ptr<mesh> plane = std::make_shared<mesh>();
    FAIL(!plane, "Cannot make memory");

    // avoid same line
    if (glm::dot(x, n) > glm::dot(y, n)) {
        x = glm::cross(n, y);
        y = glm::cross(n, x);
    } else {
        y = glm::cross(n, x);
        x = glm::cross(n, y);
    }

    float big_scale = 10000.0f;
    plane->add_vertex(p + big_scale * (-x + y), n, vec3(1.0f));
    plane->add_vertex(p + big_scale * (-x - y), n, vec3(1.0f));
    plane->add_vertex(p + big_scale * (x - y), n, vec3(1.0f));

    plane->add_vertex(p + big_scale * (x - y), n, vec3(1.0f));
    plane->add_vertex(p + big_scale * (x + y), n, vec3(1.0f));
    plane->add_vertex(p + big_scale * (-x + y), n, vec3(1.0f));
    return plane;
}

int scene::from_json(const std::string json_str) {
    using namespace rapidjson;
    Document document;
    document.Parse(json_str.c_str()); 
    clean_up();
    bool ret = true;

    const std::string mesh_key = "meshes";
    if (document.HasMember("meshes")) {
        auto cur_node = document[mesh_key.c_str()].GetObject();
        for (auto &member:cur_node) {
            //INFO("Find mesh {}", member.name.GetString());
            mesh_id id = std::stoi(member.name.GetString());
            if (mesh::id < id - 1) {
                mesh::id = id - 1;
            }

            auto mesh_obj = member.value.GetObject();

            std::string mesh_path;
            mat4 world_mat;
            bool caster;

            ret = ret & rapidjson_get_string(mesh_obj, "path", mesh_path);
            ret = ret & rapidjson_get_mat4(mesh_obj, "World Matrix", world_mat);
            ret = ret & rapidjson_get_bool(mesh_obj, "Caster", caster);

            /* Initialize the mesh */
            std::shared_ptr<mesh> mesh_ptr;
            if (purdue::file_exists(mesh_path)) {
                mesh_ptr = add_mesh(mesh_path, vec3(0.7f));
            } else {
                WARN("Cannot find the mesh file({}). Use plane instead", mesh_path);
                mesh_ptr = get_plane_mesh(vec3(0.f), vec3(0.0f,1.0f,0.0f));
                mesh_ptr->set_caster(false);
                m_meshes[mesh_ptr->get_id()] = std::make_shared<Mesh_Descriptor>(mesh_ptr);
            }

            mesh_ptr->set_world_mat(world_mat);
            mesh_ptr->set_caster(caster);

            /* Keep m_meshes key - id consistent */
            if (mesh_ptr->get_id() != id) {
                m_meshes.erase(mesh_ptr->get_id());
                mesh_ptr->cur_id = id;
                m_meshes[mesh_ptr->cur_id] = std::make_shared<Mesh_Descriptor>(mesh_ptr);
            }

            for(auto m:m_meshes) {
                DBG("Current meshes: {}, {}", m.first, m.second->m->get_id());
            }
        }

        if (document.HasMember("lights")) {
            auto light_array  = document["lights"].GetArray();
            for (auto &l:light_array) {
                m_lights.push_back(pd::string_vec3(l.GetString()));
            }
        }
    }

    return ret;
}

std::shared_ptr<mesh> scene::add_mesh(const std::string mesh_file, vec3 color) {
    std::shared_ptr<mesh> new_mesh = std::make_shared<mesh>();

    FAIL(new_mesh == nullptr || !load_model(mesh_file, new_mesh), "Mesh {} cannot be loaded.", mesh_file);

    if (new_mesh->m_verts.size() != new_mesh->m_norms.size()) {
        new_mesh->recompute_normal();
    }

    new_mesh->set_color(color);
    int id = new_mesh->get_id();
    m_meshes[id] = std::make_shared<Mesh_Descriptor>(new_mesh);

    return new_mesh;
}

bool scene::load_scene(std::string scene_file) {
    //#todo_parse_scene
    //#todo_parse_ppc

    return false;
}

bool scene::save_scene(const std::string filename) {
    //TODO
    /* Save Camera */
    

    /* Save Environments */

    /* Save Meshes */

    return false;
}


void scene::clean_up() {
    m_meshes.clear();
    mesh::id = 0;
}

vec3 scene::scene_center() {
    vec3 center(0.0f,0.0f,0.0f);
    float weight = (float)m_meshes.size();
    for (auto&m : m_meshes) {
        if (m.second != nullptr) {
            center += m.second->m->compute_world_center() * weight;
        } else {
            WARN("There is a nullptr in the scene meshes");
        }
    }

    return center;
}


void scene::scale(glm::vec3 s) {
    for(auto m:m_meshes) {
        m.second->m->add_scale(s);
    }
}


std::shared_ptr<mesh> scene::get_mesh(mesh_id id) {
    if (m_meshes.find(id) == m_meshes.end()) {
        for(auto mptr:m_meshes) {
            ERROR("Current meshes: {}", mptr.second->m->get_id());
        }
        FAIL(true, "Cannot find mesh(ID: {})", id);
    }
    return m_meshes.at(id)->m;
}

AABB scene::scene_aabb() {
    AABB scene_aabb(vec3(0.0f));

    for (auto m : m_meshes) {
        AABB cur_aabb = m.second->m->compute_world_aabb();
        scene_aabb.add_point(cur_aabb.p0);
        scene_aabb.add_point(cur_aabb.p1);
    }

    return scene_aabb;
}

bool scene::remove_mesh(mesh_id id) {
    if (m_meshes.find(id) == m_meshes.end()) {
        WARN("Try to remove a not existed mesh ID[{}]", id);
        return false;
    }

    m_meshes.erase(id);
    return true;
}

// compute default ppc position
void scene::reset_camera(vec3 &look, vec3 &at) {
    vec3 meshes_center = scene_center();
    float mesh_length = scene_aabb().diag_length();
    if (mesh_length < 0.1f)
        mesh_length = 5.0f;
    look = meshes_center + vec3(0.0f, mesh_length * 0.3f, mesh_length);
    at = meshes_center;
}

void scene::reset_camera(std::shared_ptr<ppc> camera) {
    if(!camera) {
        WARN("input pointer nullptr");
        return;
    }

    vec3 new_pos, new_at;
    reset_camera(new_pos, new_at);
    camera->_position = new_pos;
    camera->_front = glm::normalize(new_at - new_pos);
}

void scene::focus_at(std::shared_ptr<ppc> camera, std::shared_ptr<mesh> m, glm::vec3 relative_vec) {
    if (!camera || !m) {
        WARN("input pointer nullptr");
        return;
    }

    vec3 new_pos, new_at;

    //vec3 meshes_center = m->compute_world_center();

    AABB world_aabb = m->compute_world_aabb();
    vec3 meshes_center = m->compute_world_center();
    float mesh_length = m->compute_world_aabb().diag_length();
    if (mesh_length < 0.0001f)
        mesh_length = 1.0f;
    new_pos = meshes_center + mesh_length * relative_vec;
    new_at = meshes_center;

    INFO("Mesh center: {}, Camera new Pos: {}, at: {}", 
            purdue::to_string(meshes_center),
            purdue::to_string(new_pos),
            purdue::to_string(new_at));

    camera->PositionAndOrient(new_pos, meshes_center, vec3(0.0f,1.0f,0.0f));
}

std::unordered_map<mesh_id, std::shared_ptr<Mesh_Descriptor>> scene::get_mesh_descriptors() {
    return m_meshes;
}

bool scene::set_draw_type(mesh_id id, draw_type type) {
    if (m_meshes.find(id) == m_meshes.end()) { /* Cannot find the mesh */
        ERROR("Cannot find the mesh");
        return false;
    }

    m_meshes.at(id)->type = type;
}

std::shared_ptr<mesh> scene::add_mesh(std::shared_ptr<mesh> m, draw_type type) {
    auto cur_desc = std::make_shared<Mesh_Descriptor>(m);
    cur_desc->type = type;

    m_meshes[m->get_id()] = cur_desc;
    return m;
}


