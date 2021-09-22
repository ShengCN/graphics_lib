#pragma once
#include <common.h>
#include <memory>
#include "Render/geo.h"
#include "Render/mesh.h"
#include "Render/shader.h"
#include "Render/scene.h"
#include "Render/ppc.h"
#include "Utilities/voxelization.h"
#include "asset_manager.h"

class render_engine {
	//-------  APIs --------//
public:
	//------- Initialize --------//
	render_engine();
	void init();
	void test_scene(int w, int h);
	void init_camera(int w, int h, float fov);

	//------- Render --------//
	void render(int frame);

	//------- Query --------//
	std::shared_ptr<ppc> get_render_ppc();

	//------- Modify Scene --------//
	mesh_id add_mesh(const std::string model_fname, vec3 color=vec3(0.8f));
	mesh_id add_plane_mesh(vec3 p, vec3 n); 
	void remove_mesh(mesh_id id);
	void stand_on_plane(mesh_id id, vec3 p, vec3 n);
	void set_point_light(glm::vec3 lp);

	//------- IO --------//
	bool save_mesh(mesh_id id, const std::string model_fname);
	bool load_render_scene(const std::string scene_file);
	bool save_framebuffer(const std::string ofname);

	//------- UI --------//
	void camera_press(int x, int y);
	void camera_release(int x, int y);
	void camera_move(int x, int y);
	void camera_scroll(int offset);
	void camera_keyboard(char m, bool shift);

	//------- Modify --------//
	void norm_render_scene();
	void draw_line(glm::vec3 t, glm::vec3 h, vec3 tc, vec3 hc);
	void set_mesh_color(mesh_id id, vec3 c);
	void mesh_add_transform(mesh_id id, glm::mat4 mat);
	void mesh_set_transform(mesh_id id, glm::mat4 mat);
	void add_rotate(mesh_id id, purdue::deg angle, vec3 axis);
	glm::mat4 get_mesh_world(mesh_id m);
	void set_render_camera(int w, int h);
	void set_render_camera(int w, int h, float fov);
	void set_shader(mesh_id m, const std::string shader_name);
	void recompute_normal(int mesh_id);

	//------- Rendering --------//
	void look_at(int mesh_id, vec3 relative=vec3(0.0f,0.0f,1.0f));
	// void draw_render(bool trigger) { m_draw_render = trigger; }
	// void draw_visualize(bool trigger) { m_draw_visualize = trigger; }
	void draw_visualize_voxels(std::vector<AABB> voxels);
	void set_vis_frame(bool trigger) { m_vis_frame_mode = trigger; }
	void voxel_vis(int mesh_id);
	void draw_visualize_line(glm::vec3 t, glm::vec3 h);
	void draw_quad();
	void draw_image(std::shared_ptr<Image> img); 
	void draw_mesh(mesh_id id);
	void draw_shadow(mesh_id shadow_receiver);
	void draw_sihouette(int mesh_id, vec3 light_pos);
	void draw_shadow_volume(int mesh_id, vec3 light_pos);
	bool reload_shaders();

	//------- Visualization --------//
	mesh_id add_visualize_line(vec3 h, vec3 t);
	bool remove_visualize_line(mesh_id id);
	void clear_visualize();

	//------- Others --------//
	std::shared_ptr<Image> get_frame_buffer();

private:
	bool init_shaders();
	bool init_scenes();
	std::shared_ptr<mesh> get_mesh(int id);
	std::shared_ptr<mesh> vis_new_mesh();
	std::shared_ptr<mesh> add_empty_mesh();
	void render_scene(std::shared_ptr<scene> cur_scene, rendering_params params);
	void render_weighted_OIT(std::shared_ptr<scene> cur_scene, rendering_params params);

private:
	asset_manager m_manager;
	bool m_vis_frame_mode;
};