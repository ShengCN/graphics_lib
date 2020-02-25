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
			return nullptr;
		}
	}
	catch (std::exception& e) {
		WARN(e.what());
	}

	new_mesh->set_to_center();
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

void scene::focus_at(std::shared_ptr<ppc> camera, std::shared_ptr<mesh> m) {
	if (!camera || !m) {
		WARN("input pointer nullptr");
		return;
	}

	vec3 new_pos, new_at;
	
	vec3 meshes_center = m->compute_world_center();
	float mesh_length = m->compute_world_aabb().diag_length();
	if (mesh_length < 0.1f)
		mesh_length = 0.5f;
	new_pos = meshes_center + vec3(0.0f, mesh_length * 0.3f, mesh_length);
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

	sphere->add_face(-a, b, c);
	sphere->add_face(-a, c, -b);
	sphere->add_face(-a, -c, b);

	const int tessellation_times = 4;
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
