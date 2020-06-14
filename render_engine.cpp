#include "render_engine.h"

render_engine::render_engine() {
	/* Initialize Shaders */	
}

std::vector<std::shared_ptr<mesh>> render_engine::get_rendering_meshes() {
	return {};
}

bool render_engine::load_mesh(const std::string model_fname) {
	return false;
}

bool render_engine::save_mesh(std::shared_ptr<mesh> m, const std::string model_fname) {
	return false;
}

void render_engine::draw_line(glm::vec3 t, glm::vec3 h) {

}

void render_engine::set_mesh_color(std::shared_ptr<mesh> m, vec3 c) {

}

void render_engine::mesh_add_transform(glm::mat4 m) {

}

void render_engine::mesh_set_transform(glm::mat4 m) {

}

glm::mat4 render_engine::get_mesh_world(std::shared_ptr<mesh> m) {

}

void render_engine::cut_mesh(std::shared_ptr<mesh> m, vec3 p, vec3 n) {

}

void render_engine::add_point_light(glm::vec3 lp) {

}

std::shared_ptr<ppc> render_engine::get_render_ppc() {
	
}

void render_engine::draw_visualize_line(glm::vec3 t, glm::vec3 h) {

}

void render_engine::draw_visualize_voxels(AABB voxel) {

}
