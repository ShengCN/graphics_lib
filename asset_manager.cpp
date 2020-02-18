#include "asset_manager.h"

asset_manager::asset_manager() {
	if(cur_camera == nullptr) {
		cur_camera = std::make_shared<ppc>(w, h, 50.0f);
	}
}

void asset_manager::reload_shaders() {
	for(auto s:shaders) {
		s.second->reload_shader();
	}
}

std::shared_ptr<mesh> asset_manager::add_point_light(vec3 p) {
	auto &manager = instance();
	
	std::shared_ptr<mesh> light = std::make_shared<mesh>();
	light->m_verts.push_back(p);

	manager.m_lights.push_back(light);
	return light;
}

void asset_manager::set_rendering_shader(std::shared_ptr<mesh> m, const std::string shader_name) {
	m_rendering_mappings[m] = shaders.at(shader_name);
}
