#include <memory>
#include <glm/gtx/quaternion.hpp>

#include "scene.h"
#include "graphics_lib/common.h"
#include "graphics_lib/Utilities/Logger.h"
#include "graphics_lib/Utilities/Utils.h"

scene::scene() : 
	m_is_visualize_points(false), 
	m_is_visualize_lines(false), 
	m_is_visualize_aabb(false) {
}


scene::~scene() {
}

void scene::load_scene(QString scene_file) {
	//#todo_parse_scene
	//#todo_parse_ppc
}

bool scene::reload_shaders() {
	bool success = true;
	for (auto& m : m_meshes) {
		success &= m->reload_shaders();
	}

	return success;
}

void scene::draw_scene(std::shared_ptr<ppc> cur_camera, int iter) {
	if (cur_camera == nullptr) {
		LOG_FAIL("Camera initialized");
		// assert(false);
		return;
	}

	// scene meshes
	for (auto m : m_meshes) {
		m->draw(cur_camera, iter);
	}

	draw_visualization(cur_camera, iter);
}

void scene::clean_up() {
}

vec3 scene::scene_center() {
	vec3 center(0.0f,0.0f,0.0f);
	float weight = (float)m_meshes.size();
	for (auto&m : m_meshes) {
		center += m->compute_world_center() * weight;
	}

	if(m_vis_lines)
		center += 0.5f * center + 0.5f * m_vis_lines->compute_world_center();

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

bool scene::save_scene(const QString filename) {
	//#TODO_Save_Scene
	// merge 

	// save
	return false;
}

void scene::add_mesh(std::shared_ptr<mesh> m) {
	if (!m)
		LOG_FAIL("Add mesh");

	m_meshes.push_back(m);
}

void scene::add_vis_point(vec3 p, vec3 color) {
	if(!m_vis_points) {
		m_vis_points = std::make_shared<pc>(GGV.template_vs, GGV.template_fs);
	}
	m_vis_points->add_point(p, color);
}

void scene::add_vis_line_seg(vec3 t, vec3 h) {
	if(!m_vis_lines) {
		m_vis_lines = std::make_shared<line_segments>(GGV.template_vs, GGV.template_fs);
	}

	vec3 red = vec3(1.0f, 0.0f, 0.0f);
	m_vis_lines->add_line(t, red, h, red);
}

// compute default ppc position
void scene::reset_camera(vec3 &look, vec3 &at) {
	vec3 meshes_center = scene_center();
	float mesh_length = scene_aabb().diag_length();
	if (mesh_length < 0.1f)
		mesh_length = 5.0f;
	look = meshes_center + vec3(0.0f, mesh_length * 0.3f, mesh_length * 1.0f);
	at = meshes_center;
}

void scene::reset_camera(std::shared_ptr<ppc> camera) {
	if(!camera) {
		LOG_FAIL("input pointer");
		return;
	}

	vec3 new_pos, new_at;
	reset_camera(new_pos, new_at);
	camera->_position = new_pos;
	camera->_front = glm::normalize(new_at - new_pos);
}

void scene::draw_visualization(std::shared_ptr<ppc> cur_camera, int iter) {
	// visualize points
	if (m_is_visualize_points) {
		if (m_vis_points) m_vis_points->draw(cur_camera, iter);
	}

	// visualize lines
	if (m_is_visualize_lines) {
		if (m_vis_lines) m_vis_lines->draw(cur_camera, iter);
	}

	if (m_is_visualize_aabb) {
		auto all_meshes = get_meshes();
		for (auto m : all_meshes) {
			m->draw_aabb(cur_camera);
		}
	}
}

void scene::set_vis_line_fract(float fract) {
	if(m_vis_lines)
		m_vis_lines->set_drawing_fract(fract);
}

void scene::set_vis_line_animated(bool trigger) {
	if (m_vis_lines)
		m_vis_lines->set_animated(trigger);
}