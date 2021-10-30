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
    //for(auto &mpair:m_meshes) {
    for(mesh_id id = 0; id < mesh::id; ++id ) { 
        if (m_meshes.find(id) == m_meshes.end()) {
            continue;
        }

        writer.Key(fmt::format("{}", id).c_str());
        std::string scene_json = m_meshes.at(id)->to_json();
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

    for (auto &member:document.GetObject()) {
        INFO("Find mesh {}", member.name.GetString());
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
            WARN("Cannot find the mesh file. Use plane instead");
            mesh_ptr = get_plane_mesh(vec3(0.f), vec3(0.0f,1.0f,0.0f));
            m_meshes[mesh_ptr->get_id()] = mesh_ptr;
        }

        mesh_ptr->set_world_mat(world_mat);
        mesh_ptr->set_caster(caster);

        /* Keep m_meshes key - id consistent */
        if (mesh_ptr->get_id() != id) {
            m_meshes.erase(mesh_ptr->get_id());
            mesh_ptr->cur_id = id;
            m_meshes[mesh_ptr->cur_id] = mesh_ptr;
        }
    }

    return ret;
}

std::shared_ptr<mesh> scene::add_mesh(const std::string mesh_file, vec3 color) {
	std::shared_ptr<mesh> new_mesh = std::make_shared<mesh>();

    FAIL(new_mesh == nullptr || !load_model(mesh_file, new_mesh), "Mesh {} cannot be loaded.", mesh_file);

	new_mesh->set_color(color);
	int id = new_mesh->get_id();
	m_meshes[id] = new_mesh;
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
			center += m.second->compute_world_center() * weight;
		} else {
			WARN("There is a nullptr in the scene meshes");
		}
	}

	return center;
}

std::shared_ptr<mesh> scene::get_mesh(mesh_id id) {
    if (m_meshes.find(id) == m_meshes.end()) {
        for(auto mptr:m_meshes) {
            ERROR("Current meshes: {}", mptr.second->get_id());
        }
        FAIL(true, "Cannot find mesh(ID: {})", id);
    }
	return m_meshes.at(id);
}

std::unordered_map<mesh_id, std::shared_ptr<mesh>> scene::get_meshes(){
	return m_meshes; 
}

AABB scene::scene_aabb() {
	AABB scene_aabb(vec3(0.0f));

	for (auto m : m_meshes) {
		AABB cur_aabb = m.second->compute_world_aabb();
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

void scene::add_mesh(std::shared_ptr<mesh> m)  {
	if (m == nullptr) {
		throw std::invalid_argument(fmt::format("Try to insert a nullptr to scene"));
		return;
	}
	m_meshes[m->get_id()] = m;	
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
	INFO("center: " + pd::to_string(m->compute_center()));
	INFO("world matrix: \n" + pd::to_string(m->m_world));

	AABB world_aabb = m->compute_world_aabb();
	vec3 meshes_center = world_aabb.center();
	float mesh_length = m->compute_world_aabb().diag_length();
	if (mesh_length < 0.1f)
		mesh_length = 5.0f;
	new_pos = meshes_center + mesh_length * relative_vec;
	new_at = meshes_center;

	camera->_position = new_pos;
	camera->_front = glm::normalize(new_at - new_pos);
}

void scene::stand_on_plane(std::shared_ptr<mesh> m) {
	if(m_meshes.size() < 1) {
		WARN("There is no ground yet");
		return;
	}

	vec3 lowest_point = m->compute_world_aabb().p0;

	vec3 ground_height = m_meshes[0]->compute_world_center();
	float offset = ground_height.y - lowest_point.y;
	m->m_world = glm::translate(vec3(0.0, offset, 0.0)) * m->m_world;
}

