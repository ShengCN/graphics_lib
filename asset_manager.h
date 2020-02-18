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
	std::unordered_map<std::string, std::shared_ptr<shader>> shaders;
	std::unordered_map<std::shared_ptr<mesh>, std::shared_ptr<shader>> m_rendering_mappings; // mesh_id -> shader
	std::shared_ptr<ppc> cur_camera=nullptr;
	int w = 512, h = 512;
	bool m_is_visualize = false;

public:
	static asset_manager& instance() {
		static asset_manager ins;
		return ins;
	}

	void reload_shaders();
	static std::shared_ptr<mesh> add_point_light(vec3 p);
	void set_rendering_shader(std::shared_ptr<mesh> m, const std::string shader_name);
	void visualize(bool trigger) { m_is_visualize = trigger; }

private:
	asset_manager();

public:
	asset_manager(asset_manager const&) = delete;
	void operator=(asset_manager const&) = delete;
};