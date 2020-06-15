#include "render_engine.h"

render_engine::render_engine() {
}

void render_engine::render(int frame) {
	render_scene(cur_manager.render_scene);
}

void render_engine::render_visualize(int frame) {
	render_scene(cur_manager.visualize_scene);
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

void render_engine::render_scene(std::shared_ptr<scene> cur_scene) {
	// shader render mesh
	auto& meshes = cur_scene->get_meshes();
	rendering_params params = { cur_manager.cur_camera, cur_manager.lights };

	for(auto &m:meshes) {
		cur_manager.rendering_mappings.at(m)->draw_mesh(m, params);
	}
}

std::vector<std::shared_ptr<mesh>> render_engine::get_rendering_meshes() {
	return cur_manager.render_scene->get_meshes();
}

std::shared_ptr<mesh> render_engine::get_mesh(int id) {
	return cur_manager.render_scene->get_mesh(id);
}

int render_engine::load_mesh(const std::string model_fname) {
	auto cur_mesh = cur_manager.render_scene->load_mesh(model_fname, cur_manager.shaders["template"]);
	if (cur_mesh) {
		cur_manager.rendering_mappings[cur_mesh] = cur_manager.shaders["template"];
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
	m->set_color(c);
}

void render_engine::mesh_add_transform(std::shared_ptr<mesh> m, glm::mat4 mat) {
	m->m_world = mat * m->m_world;
}

void render_engine::mesh_set_transform(std::shared_ptr<mesh> m, glm::mat4 mat) {
	m->set_world_mat(mat);
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

std::shared_ptr<ppc> render_engine::get_render_ppc() {
	return cur_manager.cur_camera;
}

void render_engine::draw_visualize_line(glm::vec3 t, glm::vec3 h) {
}

void render_engine::draw_visualize_voxels(AABB voxel) {

}

void render_engine::clear_visualize() {

}
