#include "render_engine.h"
#include "Utilities/voxelization.h"
#include <glm/gtx/transform.hpp>

render_engine::render_engine() {
	m_draw_render = true;
	m_draw_visualize = false;
}

void render_engine::render(int frame) {
	rendering_params params = { cur_manager.cur_camera, cur_manager.lights, frame, draw_type::triangle};

	if (m_draw_render)
		render_scene(cur_manager.render_scene, params);
	if (m_draw_visualize) {
		params = { cur_manager.cur_camera, cur_manager.lights, frame, draw_type::line_segments };
		if (m_vis_frame_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		render_scene(cur_manager.visualize_scene, params);
		if (m_vis_frame_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void render_engine::init() {
	//#todo_init_scene
		//------- initialize rendering states --------//
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(5.0f);

	//------- initialize Shaders --------//
	const std::string template_vs = "Shaders/template_vs.glsl";
	const std::string template_fs = "Shaders/template_fs.glsl";
	cur_manager.shaders["template"] = std::make_shared<shader>(template_vs.c_str(), template_fs.c_str());

	//------- initialize scene --------//
	cur_manager.render_scene = std::make_shared<scene>();
	cur_manager.visualize_scene = std::make_shared<scene>();
}

void render_engine::render_scene(std::shared_ptr<scene> cur_scene, rendering_params params) {
	// shader render mesh
	auto& meshes = cur_scene->get_meshes();

	for(auto &m:meshes) {
		cur_manager.rendering_mappings.at(m)->draw_mesh(m, params);
	}
}

std::shared_ptr<mesh> render_engine::vis_new_mesh() {
	auto ret_mesh = cur_manager.visualize_scene->new_mesh();
	cur_manager.rendering_mappings[ret_mesh] = cur_manager.shaders.at("template");
	return ret_mesh;
}

std::vector<std::shared_ptr<mesh>> render_engine::get_rendering_meshes() {
	return cur_manager.render_scene->get_meshes();
}

std::shared_ptr<mesh> render_engine::get_mesh(int id) {
	return cur_manager.render_scene->get_mesh(id);
}

std::vector<std::shared_ptr<mesh>> render_engine::get_meshes(std::vector<int> ids) {
	std::vector<std::shared_ptr<mesh>> ret;
	for(int id : ids) {
		ret.push_back(get_mesh(id));
	}
	return ret;
}

int render_engine::load_mesh(const std::string model_fname) {
	auto cur_mesh = cur_manager.render_scene->load_mesh(model_fname, cur_manager.shaders["template"]);
	if (cur_mesh) {
		cur_manager.rendering_mappings[cur_mesh] = cur_manager.shaders["template"];
		return cur_mesh->get_id();
	}

	return -1;
}

int render_engine::load_mesh(const std::string model_fname, vec3 c) {
	auto cur_mesh = cur_manager.render_scene->load_mesh(model_fname, cur_manager.shaders["template"]);
	if (cur_mesh) {
		cur_manager.rendering_mappings[cur_mesh] = cur_manager.shaders["template"];
		cur_mesh->set_color(c);
		return cur_mesh->get_id();
	}

	return -1;
}

bool render_engine::save_mesh(std::shared_ptr<mesh> m, const std::string model_fname) {
	return false;
}

bool render_engine::load_render_scene(const std::string scene_file) {
	return false;
}

bool render_engine::reload_shaders() {
	bool success = true;
	for(auto &s:cur_manager.shaders) {
		success = success && s.second->reload_shader();
	}
	return success;
}

void render_engine::camera_press(int x, int y) {
	cur_manager.cur_camera->mouse_press(x, y);
}

void render_engine::camera_release(int x, int y) {
	cur_manager.cur_camera->mouse_release(x, y);
}

void render_engine::camera_move(int x, int y) {
	cur_manager.cur_camera->mouse_move(x, y);
}

void render_engine::look_at(int mesh_id) {
	auto& at_mesh = get_mesh(mesh_id);
	if(at_mesh) {
		cur_manager.render_scene->focus_at(cur_manager.cur_camera, at_mesh);
	} else {
		WARN("Cannot find mesh " + std::to_string(mesh_id));
	}
}

void render_engine::norm_render_scene() {
	//#todo_normalize_render_scene
}

void render_engine::draw_line(glm::vec3 t, glm::vec3 h) {
}

void render_engine::set_mesh_color(std::shared_ptr<mesh> m, vec3 c) {
	if(m)
		m->set_color(c);
}

void render_engine::mesh_add_transform(std::shared_ptr<mesh> m, glm::mat4 mat) {
	m->m_world = mat * m->m_world;
}

void render_engine::mesh_set_transform(std::shared_ptr<mesh> m, glm::mat4 mat) {
	m->set_world_mat(mat);
}

void render_engine::mesh_apply_transform(std::shared_ptr<mesh> m, glm::mat3 mat) {
	if(m == nullptr) {
		return;
	}

	vec3 trans = mat[0];
	vec3 rotate = mat[1];
	vec3 scale = mat[2];

	glm::mat4 new_scale = glm::scale(scale);
	glm::mat4 new_rotation = glm::rotate(pd::deg2rad(rotate[0]), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::rotate(pd::deg2rad(rotate[1]), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(pd::deg2rad(rotate[2]), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 new_translate = glm::translate(trans);

	glm::mat4 new_transform = new_translate * new_rotation * new_scale;
	m->set_world_mat(new_transform);
}

glm::mat4 render_engine::get_mesh_world(std::shared_ptr<mesh> m) {
	return m->get_world_mat();
}

void render_engine::cut_mesh(std::shared_ptr<mesh> m, vec3 p, vec3 n) {
}

void render_engine::add_point_light(glm::vec3 lp) {
	cur_manager.lights.push_back(lp);
}

void render_engine::set_render_camera(int w, int h, float fov) {
	if (cur_manager.cur_camera == nullptr) {
		cur_manager.cur_camera = std::make_shared<ppc>(w, h, fov);
		return;
	} 
	cur_manager.cur_camera->_width = w;
	cur_manager.cur_camera->_height = h;
	cur_manager.cur_camera->_fov = fov;
}

void render_engine::draw_visualize_voxels(std::vector<AABB> voxels) {
	auto& vis_mesh = vis_new_mesh();
	if (vis_mesh) {
		for(auto& cur_bb : voxels) {
			vis_mesh->add_vertices(cur_bb.to_tri_mesh());
		}
	}
	vis_mesh->set_color(vec3(0.0f, 0.0f, 0.8f));
}

void render_engine::voxel_vis(int mesh_id) {
	auto& mesh_ptr = get_mesh(mesh_id);
	if (mesh_ptr) {
		std::vector<AABB> voxels; voxelizater::voxelize(mesh_ptr, 10, voxels);
		draw_visualize_voxels(voxels);
	}
}

std::shared_ptr<ppc> render_engine::get_render_ppc() {
	return cur_manager.cur_camera;
}

void render_engine::draw_visualize_line(glm::vec3 t, glm::vec3 h) {
}


void render_engine::clear_visualize() {

}
