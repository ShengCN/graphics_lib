#pragma once
#include "graphics_lib/Render/shader.h"
#include "graphics_lib/Render/scene.h"
#include "graphics_lib/Render/ppc.h"

// rendering related assets
class asset_manager {
	//------- Variables --------//
public:
	std::shared_ptr<scene> cur_scene;
	std::vector<std::shared_ptr<mesh>> m_lights;
	std::vector<std::shared_ptr<shader>> shaders;
	std::unordered_map<int, std::shared_ptr<shader>> m_rendering_mappings; // mesh_id -> shader
	std::shared_ptr<ppc> cur_camera=nullptr;
	
	float shadow_ppc_fov = 30.0f;
	int w = 256, h = 256;
	int threads=1, id=0;

	float a=0.0, b=0.0;

public:
	static asset_manager& instance() {
		static asset_manager ins;
		return ins;
	}

	static std::shared_ptr<mesh> add_point_light(vec3 p);
private:
	asset_manager();

public:
	asset_manager(asset_manager const&) = delete;
	void operator=(asset_manager const&) = delete;
};