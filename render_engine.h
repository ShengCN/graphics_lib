#pragma once
#include "graphics_lib/Render/shader.h"
#include "graphics_lib/Render/scene.h"
#include "graphics_lib/Render/ppc.h"
#include "asset_manager.h"

// rendering related assets
class render_engine {
	//-------  APIs --------//
public:
	render_engine();

	std::vector<std::shared_ptr<mesh>> get_rendering_meshes();
	bool load_mesh(const std::string model_fname);
	bool save_mesh(std::shared_ptr<mesh> m , const std::string model_fname);
	void draw_line(glm::vec3 t, glm::vec3 h);
	void set_mesh_color(std::shared_ptr<mesh> m, vec3 c);
	void mesh_add_transform(glm::mat4 m);
	void mesh_set_transform(glm::mat4 m);
	glm::mat4 get_mesh_world(std::shared_ptr<mesh> m);
	void cut_mesh(std::shared_ptr<mesh> m, vec3 p, vec3 n);
	void add_point_light(glm::vec3 lp);
	std::shared_ptr<ppc> get_render_ppc();

	void draw_visualize_line(glm::vec3 t, glm::vec3 h);
	void draw_visualize_voxels(AABB voxel);


private:
	asset_manager cur_manager;
};