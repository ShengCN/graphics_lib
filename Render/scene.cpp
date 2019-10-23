#include <memory>
#include <glm/gtx/quaternion.hpp>

#include "scene.h"
#include "graphics_lib/common.h"
#include "graphics_lib/Utilities/Logger.h"
#include "graphics_lib/Utilities/Utils.h"

scene::scene() {
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

	if(m_container)
		success &= m_container->reload_shaders();

	if (m_container_upper)
		success &= m_container_upper->reload_shaders();

	if (m_container_bottm)
		success &= m_container_bottm->reload_shaders();

	return success;
}

void scene::draw_scene(std::shared_ptr<ppc> cur_camera, int iter) {
	if (cur_camera == nullptr) {
		LOG_FAIL("Camera initialized");
		// assert(false);
		return;
	}

	auto gv = global_variable::instance();
	auto scale_compute = [](float x) {
		return (40.0f * x) + 1.0f;
	};
	
	if (gv->is_frame_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(gv->is_transparent)	glDepthMask(GL_FALSE);
	
	if(m_container)
		m_container->draw(cur_camera, iter);

	if (m_container_upper)
		m_container_upper->draw(cur_camera, iter);

	if (m_container_bottm)
		m_container_bottm->draw(cur_camera, iter);

	// scene meshes
	for (auto m : m_meshes) {
		m->draw(cur_camera, iter);
	}

	// visualization points
	if(gv->is_visualize){
		if(m_vis_lines) m_vis_lines->draw(cur_camera, iter);
		if(m_vis_points) m_vis_points->draw(cur_camera, iter);

		auto all_meshes = get_meshes();
		if(gv->is_draw_aabb) {
			for (auto m : all_meshes) {
				m->draw_aabb(cur_camera);
			}
		}
	}

	if (gv->is_transparent) glDepthMask(GL_TRUE);
	if (gv->is_frame_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	
	if(m_container)
		ret.push_back(m_container);
	
	if (m_container_upper)
		ret.push_back(m_container_upper);

	if (m_container_bottm)
		ret.push_back(m_container_bottm);

	auto inside_objs = get_inside_objects();
	for(auto i:inside_objs){
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

std::shared_ptr<triangle_mesh> scene::load_stl(QString file_path, vec3 color, bool is_container, bool is_normalize) {
	auto gv = global_variable::instance();

	std::shared_ptr<triangle_mesh> mesh = std::make_shared<triangle_mesh>(gv->template_vs, gv->template_fs);
	mesh->load(file_path);
	mesh->set_color(color);
	
	if (is_normalize)
		mesh->normalize_position_orientation();

	if (is_container) {
		m_container = mesh;
		m_container->m_is_container = true;
	} else 
		m_meshes.push_back(mesh);

	return mesh;
}

AABB scene::scene_aabb() {
	AABB scene_aabb(vec3(0.0f));

	if(m_container){
		scene_aabb = m_container->compute_world_aabb();
	}
	else {
		if (!m_meshes.empty()) {
			scene_aabb = m_meshes[0]->compute_world_aabb();
		}

		for (auto m : m_meshes) {
			AABB cur_aabb = m->compute_world_aabb();
			scene_aabb.add_point(cur_aabb.p0);
			scene_aabb.add_point(cur_aabb.p1);
		}
	}

	return scene_aabb;
}

bool scene::save_scene(const QString filename) {
	//#TODO_Save_Scene
	// merge 

	// save
	return m_container->save_stl(filename);
}

void scene::add_mesh(std::shared_ptr<mesh> m) {
	if (!m)
		LOG_FAIL("Add mesh");

	m_meshes.push_back(m);
}

void scene::add_vis_point(vec3 p, vec3 color) {
	auto gv = global_variable::instance();
	if(!m_vis_points) {
		m_vis_points = std::make_shared<pc>(gv->template_vs, gv->template_fs);
	}
	m_vis_points->add_point(p, color);
}

void scene::add_vis_line_seg(vec3 t, vec3 h) {
	auto gv = global_variable::instance();
	if(!m_vis_lines) {
		m_vis_lines = std::make_shared<line_segments>(gv->template_vs, gv->template_fs);
	}

	vec3 red = vec3(1.0f, 0.0f, 0.0f);
	m_vis_lines->add_line(t, red, h, red);
}

std::shared_ptr<triangle_mesh> scene::add_plane(vec3 p, vec3 n, vec3 c, float size) {
	auto gv = global_variable::instance();
	std::shared_ptr<triangle_mesh> plane = std::make_shared<triangle_mesh>(gv->template_vs, gv->template_fs);
	plane->m_verts.push_back(vec3(-1.0f, 0.0f, -1.0f));
	plane->m_verts.push_back(vec3(-1.0f, 0.0f, 1.0f));
	plane->m_verts.push_back(vec3(1.0f, 0.0f, -1.0f));

	plane->m_verts.push_back(vec3(1.0f, 0.0f, -1.0f));
	plane->m_verts.push_back(vec3(-1.0f, 0.0f, 1.0f));
	plane->m_verts.push_back(vec3(1.0f, 0.0f, 1.0f));

	plane->m_colors.push_back(c);
	plane->m_colors.push_back(c);
	plane->m_colors.push_back(c);
	plane->m_colors.push_back(c);
	plane->m_colors.push_back(c);
	plane->m_colors.push_back(c);

	plane->add_scale(vec3(size));
	m_meshes.push_back(plane);

	return plane;
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

void scene::set_vis_line_fract(float fract) {
	if(m_vis_lines)
		m_vis_lines->set_drawing_fract(fract);
}

void scene::set_vis_line_animated(bool trigger) {
	if (m_vis_lines)
		m_vis_lines->set_animated(trigger);
}

void scene::set_container_upper(std::shared_ptr<mesh> m) {
	m_container_upper = m;
}

void scene::set_container_bottm(std::shared_ptr<mesh> m) {
	m_container_bottm = m;;
}