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
#include "graphics_lib/Render/shader.h"

scene::scene() {
	m_scene_rendering_shared = std::make_shared<scene_shared_parameters>();

	// create axis mesh
	m_axis = std::make_shared<mesh>(false);
	float axis_length = 3.0f;
	m_axis->add_vertex(vec3(0.0f, 0.0f, 0.0f) * axis_length, vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
	m_axis->add_vertex(vec3(1.0f, 0.0f, 0.0f) * axis_length, vec3(0.0f,1.0f,0.0f), vec3(1.0f,0.0f,0.0f));

	m_axis->add_vertex(vec3(0.0f, 0.0f, 0.0f) * axis_length, vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	m_axis->add_vertex(vec3(0.0f, 1.0f, 0.0f) * axis_length, vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));

	m_axis->add_vertex(vec3(0.0f, 0.0f, 0.0f) * axis_length, vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	m_axis->add_vertex(vec3(0.0f, 0.0f, 1.0f) * axis_length, vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	asset_manager::instance().m_rendering_mappings[m_axis] = asset_manager::instance().shaders.at("line_segment");
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

	m_scene_rendering_shared->iter = iter;
	// scene meshes
	for (auto m : m_meshes) {
		asset_manager::instance().m_rendering_mappings.at(m)->draw_mesh(cur_camera, m, m_scene_rendering_shared);
	}

	// visualization meshes
	if(asset_manager::instance().m_is_visualize) {
		for (auto m : m_visualize_objs) {
			asset_manager::instance().m_rendering_mappings.at(m)->draw_mesh(cur_camera, m, m_scene_rendering_shared);
		}

		if(m_visualize_direction) {
			draw_visualize_direction();
		}
	}

	draw_axis();
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

std::shared_ptr<mesh> scene::load_mesh(const std::string mesh_file, std::shared_ptr<shader> render_shader, bool is_added_in_scene) {
	std::shared_ptr<mesh> new_mesh = std::make_shared<mesh>();
	auto loader = model_loader::create(mesh_file);
	try {
		if (loader->load_model(mesh_file, new_mesh)) {
			INFO("Loading file " + mesh_file + " success");
		}
		else {
			WARN("Loading file " + mesh_file + " failed");
			return nullptr;
		}
	}
	catch (std::exception& e) {
		WARN(e.what());
	}

	new_mesh->set_to_center();

	if(is_added_in_scene)
		m_meshes.push_back(new_mesh);

	asset_manager::instance().m_rendering_mappings[new_mesh] = render_shader;

	return new_mesh;
}

void scene::add_mesh(std::shared_ptr<mesh> m) {
	if (!m)
		WARN("Add mesh failed");

	m_meshes.push_back(m);
}

void scene::erase_mesh(std::shared_ptr<mesh> m) {
	for(auto mesh_iter = m_meshes.begin(); mesh_iter != m_meshes.end(); ++mesh_iter) {
		if (*mesh_iter == m) {
			m_meshes.erase(mesh_iter);
			break;
		}
	}
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

void scene::focus_at(std::shared_ptr<ppc> camera, std::shared_ptr<mesh> m, float distance_fract) {
	if (!camera || !m) {
		WARN("input pointer nullptr");
		return;
	}

	vec3 new_pos, new_at;
	
	vec3 meshes_center = m->compute_world_center();
	float mesh_length = m->compute_world_aabb().diag_length();
	if (mesh_length < 0.1f)
		mesh_length = 0.5f;
	new_pos = meshes_center + vec3(0.0f, mesh_length * 0.3f, mesh_length) * distance_fract;
	new_at = meshes_center;

	camera->_position = new_pos;
	camera->_front = glm::normalize(new_at - new_pos);
	m_target_mesh = m;
}

void scene::stand_on_plane(std::shared_ptr<mesh> ground_mesh, std::shared_ptr<mesh> m) {
	if(m_meshes.size() < 1) {
		WARN("There is no ground yet");
		return;
	}

	vec3 lowest_point = m->compute_world_aabb().p0;

	vec3 ground_height = ground_mesh->compute_world_center();
	float offset = ground_height.y - lowest_point.y;
	m->add_world_transate(vec3(0.0, offset, 0.0));
}

std::shared_ptr<mesh> scene::add_visualize_sphere(vec3 p, float radius, vec3 col) {
	std::shared_ptr<mesh> sphere = std::make_shared<mesh>(false);
	// tessellation of a unit sphere
	vec3 a(0.0f, 1.0f, 0.0f), b(1.0f, 0.0f, 0.0f), c(0.0f, 0.0f, 1.0f);
	sphere->add_face(a, c, b);
	sphere->add_face(a, -b, c);
	sphere->add_face(a, b, -c);
	sphere->add_face(a, -c, -b);

	sphere->add_face(-a, b, c);
	sphere->add_face(-a, c, -b);
	sphere->add_face(-a, -c, b);
	sphere->add_face(-a, -b, -c);

	const int tessellation_times = 3;
	for(int i = 0; i < tessellation_times; ++i) {
		std::vector<vec3> triangles = sphere->m_verts;
		sphere->m_verts.clear();
		for(int ti = 0; ti < triangles.size() / 3 ; ++ti) {
			//     A
			//    / \
			//   D---F
			//  / \ / \
			// B --E-- C
			vec3 &a = triangles[3 * ti + 0];
			vec3 &b = triangles[3 * ti + 1];
			vec3 &c = triangles[3 * ti + 2];
			vec3 d = glm::normalize(a + b);
			vec3 e = glm::normalize(b + c);
			vec3 f = glm::normalize(a + c);

			sphere->add_face(a, d, f, glm::normalize(a), glm::normalize(d), glm::normalize(f));
			sphere->add_face(d, b, e, glm::normalize(d), glm::normalize(b), glm::normalize(e));
			sphere->add_face(d, e, f, glm::normalize(d), glm::normalize(e), glm::normalize(f));
			sphere->add_face(f, e, c, glm::normalize(f), glm::normalize(e), glm::normalize(c));
		}
	}

	// translation and scale
	sphere->add_scale(vec3(radius));
	sphere->add_world_transate(p);

	sphere->set_color(col);

	auto &manager = asset_manager::instance();
	manager.set_rendering_shader(sphere, "template");
	m_visualize_objs.push_back(sphere);

	return sphere;
}

void scene::initialize_direction_mesh(const std::string mesh_file) {
	m_visualize_direction = load_mesh(mesh_file, asset_manager::instance().shaders.at("template"), false);
	m_visualize_direction->set_color(vec3(0.0f, 0.8f, 0.0f));
	AABB aabb = m_visualize_direction->compute_aabb();
	vec3 center = aabb.center();
	for (auto &v:m_visualize_direction->m_verts) {
		v = v - center;
	}

	float scale_fact = 1.0f / m_visualize_direction->compute_world_aabb().diag_length();
	m_visualize_direction->add_scale(vec3(scale_fact));
	vec3 move_distance = vec3(0.0f, 0.5f * (aabb.p1.y - aabb.p0.y), 0.0f);
	m_visualize_direction->set_to_center(move_distance);
}

void scene::add_visualize_direction(visualize_direction &vd) {
	m_visualize_direction_stacks.push_back(vd);
}

void scene::draw_visualize_direction() {
	auto &manager = asset_manager::instance();
	if (!m_visualize_direction)
		return;
	
	for(auto &dv:m_visualize_direction_stacks) {
		m_visualize_direction->set_color(dv.color);
		m_visualize_direction->add_world_transate(dv.position);
		
		// default direction is y direction
		// compute axis and angle
		vec3 y = vec3(0.0f, 1.0f, 0.0f);
		vec3 rot_axs = vec3(1.0f, 0.0f, 0.0f);
		vec3 normalized_target = glm::normalize(dv.direction);
		bool is_close_parallel = std::abs(1.0f- std::abs(glm::dot(y, normalized_target))) < 1e-3;
		if(!is_close_parallel) {
			rot_axs = glm::cross(y, normalized_target);
		}

		pd::rad rot_angle = std::acos(glm::dot(y, normalized_target));
		m_visualize_direction->add_rotate(rot_angle, rot_axs);
		m_visualize_direction->add_scale(vec3(1.0f,1.0f,1.0f) * dv.scale);

		manager.m_rendering_mappings.at(m_visualize_direction)->draw_mesh(
			manager.cur_camera, 
			m_visualize_direction, 
			m_scene_rendering_shared);
		m_visualize_direction->reset_matrix(false);
	}

}

void scene::draw_axis() {
	if (m_axis == nullptr)
		return;

	auto &manager = asset_manager::instance();
	manager.m_rendering_mappings.at(m_axis)->draw_mesh(
		manager.cur_camera,
		m_axis,
		m_scene_rendering_shared,
		mesh_type::line_mesh);
}

void visualize_direction::arcball_rotate(pd::rad degree, vec3 &rot_axis) {
	mat4 to_camera = asset_manager::instance().cur_camera->GetV();
	mat4 camera_to_obj = glm::inverse(to_camera);

	vec3 obj_space_rot_axis = glm::normalize(rot_axis);
	direction = glm::rotate(degree, obj_space_rot_axis) * direction;
	INFO("rot deg: " + std::to_string(degree));
	INFO("rot axis: " + to_string(obj_space_rot_axis));

	INFO(to_string(direction));
}

mat4 visualize_direction::compute_to_world() {

	mat4 translation_mat = glm::translate(position);

	// default direction is y direction
	// compute axis and angle
	vec3 y = vec3(0.0f, 1.0f, 0.0f);
	vec3 rot_axs = vec3(1.0f, 0.0f, 0.0f);
	vec3 normalized_target = glm::normalize(direction);
	bool is_close_parallel = std::abs(1.0f - std::abs(glm::dot(y, normalized_target))) < 1e-3;
	if (!is_close_parallel) {
		rot_axs = glm::cross(y, normalized_target);
	}

	pd::rad rot_angle = std::acos(glm::dot(y, normalized_target));
	mat4 rotation_mat = glm::rotate(rot_angle, rot_axs);
	mat4 scale_mat = glm::scale(vec3(scale));
	return translation_mat * rotation_mat * scale_mat;
}
