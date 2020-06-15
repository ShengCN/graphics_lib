#pragma once
#include "graphics_lib/Render/shader.h"
#include "graphics_lib/Render/scene.h"
#include "graphics_lib/Render/ppc.h"
#include "asset_manager.h"

// rendering related assets
class render_engine {
	//-------  APIs --------//
public:
	//------- Initialize --------//
	render_engine();
	void init();

	//------- Render --------//
	void render(int frame);
	void render_visualize(int frame);

	//------- Queries --------//
	std::vector<std::shared_ptr<mesh>> get_rendering_meshes();
	std::shared_ptr<mesh> get_mesh(int id);
	std::shared_ptr<ppc> get_render_ppc();

	//------- IO --------//
	int load_mesh(const std::string model_fname);
	bool save_mesh(std::shared_ptr<mesh> m, const std::string model_fname);
	bool load_render_scene(const std::string scene_file);
	bool reload_shaders();

	//------- UI --------//
	void camera_press(int x, int y);
	void camera_release(int x, int y);
	void camera_move(int x, int y);

	//------- Modify --------//
	void look_at(int mesh_id);
	void norm_render_scene();
	void draw_line(glm::vec3 t, glm::vec3 h);
	void set_mesh_color(std::shared_ptr<mesh> m, vec3 c);
	void mesh_add_transform(std::shared_ptr<mesh> m, glm::mat4 mat);
	void mesh_set_transform(std::shared_ptr<mesh> m, glm::mat4 mat);
	glm::mat4 get_mesh_world(std::shared_ptr<mesh> m);
	void cut_mesh(std::shared_ptr<mesh> m, vec3 p, vec3 n);
	void add_point_light(glm::vec3 lp);
	void set_render_camera(int w, int h, float fov);

	void draw_visualize_line(glm::vec3 t, glm::vec3 h);
	void draw_visualize_voxels(AABB voxel);

	void clear_visualize();

private:
	void render_scene(std::shared_ptr<scene> cur_scene);

private:
	asset_manager cur_manager;
};