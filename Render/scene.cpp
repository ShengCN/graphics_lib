#include <memory>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <exception>

#include "scene.h"
#include "graphics_lib/common.h"
#include "graphics_lib/Utilities/Logger.h"
#include "graphics_lib/Utilities/Utils.h"
#include "graphics_lib/Utilities/model_loader.h"
#include "graphics_lib/asset_manager.h"

scene::scene() {
}


scene::~scene() {
}


void scene::load_scene(std::string scene_file) {
	//#todo_parse_scene
	//#todo_parse_ppc
}

void scene::draw_scene(std::shared_ptr<ppc> cur_camera, int iter) {
	if (cur_camera == nullptr) {
		WARN("Camera initialized failed");
		// assert(false);
		return;
	}

	// scene meshes
	for (auto m : m_meshes) {
		// m->draw(cur_camera, iter);
		asset_manager::instance().m_rendering_mappings.at(m->get_id())->draw_mesh(m);
	}
}

void scene::clean_up() {
	m_meshes.clear();
}

vec3 scene::scene_center() {
	vec3 center(0.0f,0.0f,0.0f);
	float weight = (float)m_meshes.size();
	for (auto&m : m_meshes) {
		center += m->compute_world_center() * weight;
	}

	return center;
}

std::vector<std::shared_ptr<mesh>> scene::get_meshes() {
	std::vector<std::shared_ptr<mesh>> ret;
	
	for(auto i:m_meshes){
		ret.push_back(i);
	}

	return ret;
}

std::shared_ptr<mesh> scene::get_mesh(int mesh_id) {
	auto meshes = get_meshes();
	for(auto m:meshes){
		if(m->get_id() == mesh_id) {
			return m;
		}
	}

	std::cerr << "Cannot find the mesh \n";
	return nullptr;
}

AABB scene::scene_aabb() {
	AABB scene_aabb(vec3(0.0f));

	for (auto m : m_meshes) {
		AABB cur_aabb = m->compute_world_aabb();
		scene_aabb.add_point(cur_aabb.p0);
		scene_aabb.add_point(cur_aabb.p1);
	}

	return scene_aabb;
}

bool scene::save_scene(const std::string filename) {
	//#TODO_Save_Scene
	// merge 

	// save
	return false;
}

std::shared_ptr<mesh> scene::load_mesh(const std::string mesh_file, std::shared_ptr<shader> render_shader) {
	std::shared_ptr<mesh> new_mesh = std::make_shared<mesh>();
	auto loader = model_loader::create(mesh_file);
	try {
		if (loader->load_model(mesh_file, new_mesh)) {
			INFO("Loading file " + mesh_file + " success");
		}
		else {
			WARN("Loading file " + mesh_file + " failed");
		}
	}
	catch (std::exception& e) {
		WARN(e.what());
	}

	m_meshes.push_back(new_mesh);
	asset_manager::instance().m_rendering_mappings[new_mesh->get_id()] = render_shader;

	return new_mesh;
}

void scene::add_mesh(std::shared_ptr<mesh> m) {
	if (!m)
		WARN("Add mesh failed");

	m_meshes.push_back(m);
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

void scene::focus_at(std::shared_ptr<ppc> camera, std::shared_ptr<mesh> m) {
	if (!camera || !m) {
		WARN("input pointer nullptr");
		return;
	}

	vec3 new_pos, new_at;
	
	vec3 meshes_center = m->compute_world_center();
	float mesh_length = m->compute_world_aabb().diag_length();
	if (mesh_length < 0.1f)
		mesh_length = 5.0f;
	new_pos = meshes_center + 2.0f * vec3(0.0f, mesh_length * 0.3f, mesh_length);
	new_at = meshes_center;

	camera->_position = new_pos;
	camera->_front = glm::normalize(new_at - new_pos);
}

void scene::stand_on_plane(std::shared_ptr<mesh> m) {
	if(m_meshes.size() < 1) {
		WARN("There is no ground yet");
		return;
	}

	vec3 m_center = m->compute_world_center();
	vec3 m_aabb_diag = m->compute_world_aabb().diagonal();
	float half_obj_height = m_aabb_diag.y * 0.5f;

	vec3 ground_height = m_meshes[0]->compute_world_center();
	float offset = ground_height.y + half_obj_height - m_center.y;
	m->m_world = glm::translate(vec3(0.0, offset, 0.0)) * m->m_world;
}
