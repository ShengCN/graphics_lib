#pragma once
#include "graphics_lib/Render/shader.h"
#include "graphics_lib/Render/scene.h"
#include "graphics_lib/Render/ppc.h"
#include "asset_manager.h"

#include <QOpenGLFunctions_4_0_Core>

// rendering related assets
class render_engine : public QOpenGLFunctions_4_0_Core {
	//-------  APIs --------//
public:
	//------- Initialize --------//
	render_engine();
	void init();

	//------- Render --------//
	void render(int frame);

	//------- Queries --------//
	std::vector<std::shared_ptr<mesh>> get_rendering_meshes();
	std::shared_ptr<mesh> get_mesh(int id);
	std::vector<std::shared_ptr<mesh>> get_meshes(std::vector<int> ids);
	std::shared_ptr<ppc> get_render_ppc();

	//------- IO --------//
	int load_mesh(const std::string model_fname, std::string shader_name="template");
	int load_mesh(const std::string model_fname, vec3 c, std::string shader_name = "template");
	bool save_mesh(std::shared_ptr<mesh> m, const std::string model_fname);
	bool load_render_scene(const std::string scene_file);
	bool reload_shaders();
	void add_mesh(std::shared_ptr<mesh> m);

	//------- UI --------//
	void camera_press(int x, int y);
	void camera_release(int x, int y);
	void camera_move(int x, int y);

	//------- Modify --------//
	void look_at(int mesh_id, vec3 relative=vec3(0.0f,0.0f,1.0f));
	void norm_render_scene();
	void draw_line(glm::vec3 t, glm::vec3 h, vec3 tc, vec3 hc);
	void set_mesh_color(std::shared_ptr<mesh> m, vec3 c);
	void mesh_add_transform(std::shared_ptr<mesh> m, glm::mat4 mat);
	void mesh_add_transform(int id, glm::mat4 mat);
	void mesh_set_transform(std::shared_ptr<mesh> m, glm::mat4 mat);
	void mesh_apply_transform(std::shared_ptr<mesh> m, glm::mat3 mat);
	glm::mat4 get_mesh_world(std::shared_ptr<mesh> m);
	void cut_mesh(std::shared_ptr<mesh> m, vec3 p, vec3 n);
	void add_point_light(glm::vec3 lp);
	void set_render_camera(int w, int h, float fov);
	void set_shader(std::shared_ptr<mesh> m, const std::string shader_name);
	void remove_mesh(int mesh_id);
	void clear_all();
	void set_draw_type(draw_type t) { m_cur_draw_type = t; }

	//------- Rendering --------//
	void draw_render(bool trigger) { m_draw_render = trigger; }
	void draw_visualize(bool trigger) { m_draw_visualize = trigger; }
	void draw_visualize_voxels(std::vector<AABB> voxels);
	void set_vis_frame(bool trigger) { m_vis_frame_mode = trigger; }
	void voxel_vis(int mesh_id);
	void draw_visualize_line(glm::vec3 t, glm::vec3 h);
	void draw_quad();
	void draw_aabb_axis(glm::vec3 p);

	void clear_visualize();

private:
	void render_scene(std::shared_ptr<scene> cur_scene, rendering_params params);
	void render_weighted_OIT(std::shared_ptr<scene> cur_scene, rendering_params params);
	std::shared_ptr<mesh> vis_new_mesh();

	GLuint create_quad();
private:
	asset_manager cur_manager;
	bool m_draw_render, m_draw_visualize;
	bool m_vis_frame_mode;
	GLuint m_quad_vao;
	draw_type m_cur_draw_type;
};