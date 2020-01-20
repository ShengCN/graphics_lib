#include "asset_manager.h"

asset_manager::asset_manager() {
	if(cur_camera == nullptr) {
		cur_camera = std::make_shared<ppc>(w, h, 50.0f);
	}
}

void asset_manager::add_point_light(vec3 p) {
	auto &manager = instance();
	
	std::shared_ptr<mesh> light = std::make_shared<mesh>();
	light->m_verts.push_back(p);

	manager.m_lights.push_back(light);
}