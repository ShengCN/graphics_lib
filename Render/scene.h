#pragma once
#include <vector>
#include <memory>
#include <unordered_map>

#include "ppc.h"
#include "mesh.h"
#include "shader.h"

struct scene_shared_parameters;
struct visualize_direction {
	vec3 position;
	vec3 direction;
	vec3 color;
	float scale;

	visualize_direction(vec3 p, vec3 d, vec3 c, float s):
		position(p), direction(d), color(c), scale(s){}

	void arcball_rotate(pd::rad degree, vec3 &rot_axis);
	mat4 compute_to_world();
};

class scene {
public:
	scene();
	~scene();

	//------- Shared Functions --------//
	std::shared_ptr<mesh> load_mesh(const std::string mesh_file, std::shared_ptr<shader> render_shader, bool is_added_in_scene=true);
	virtual void add_mesh(std::shared_ptr<mesh> m_);
	void erase_mesh(std::shared_ptr<mesh> m);
	virtual void load_scene(std::string scene_file);
	virtual void draw_scene(std::shared_ptr<ppc> cur_camera, int iter=0);
	virtual void clean_up();
	virtual vec3 scene_center();
	virtual std::vector<std::shared_ptr<mesh>> get_meshes();
	virtual std::shared_ptr<mesh> get_mesh(int mesh_id);
	virtual AABB scene_aabb();
	virtual bool save_scene(const std::string filename);
	virtual void reset_camera(vec3 &look, vec3 &at);
	virtual void reset_camera(std::shared_ptr<ppc> camera);
	void focus_at(std::shared_ptr<ppc> camera, 
				  std::shared_ptr<mesh> m,
				  float distance_fract=1.0f);
	std::shared_ptr<mesh> get_target_mesh() { return m_target_mesh; }
	void stand_on_plane(std::shared_ptr<mesh> ground_mesh, std::shared_ptr<mesh> m);
	void add_light(std::shared_ptr<mesh> l) {
		m_lights.push_back(l);
	}

	std::vector<std::shared_ptr<mesh>> get_lights() { return m_lights; }

	//------- Visualization Meshes --------//
	std::shared_ptr<mesh> add_visualize_sphere(vec3 p, float radius=1, vec3 col=vec3(1.0f,0.0f,0.0f));
	void clear_visualize_point() { m_visualize_spheres.clear(); }
	std::vector<std::shared_ptr<mesh>> get_visualize_meshes() { return m_visualize_spheres; }
	
	void add_visualize_lines(const std::vector<std::vector<glm::vec3>> &position_list);
	void clear_visualize_lines();

	void initialize_direction_mesh(const std::string mesh_file);
	void clear_visualize_direction() { m_visualize_direction_stacks.clear(); }
	void add_visualize_direction(visualize_direction &vd);
	void draw_visualize_direction();
	std::vector<visualize_direction>& get_vis_direction_stacks() { return m_visualize_direction_stacks; };
	void draw_axis();

	//------- Protected Functions --------//
protected:
	void draw_triangle_mesh(std::shared_ptr<mesh> &m);
	void draw_lines(std::shared_ptr<mesh> &m);

	//------- Protected Variables --------//
protected:
	std::vector<std::shared_ptr<mesh>> m_meshes;
	std::vector<std::shared_ptr<mesh>> m_lights;
	std::vector<std::shared_ptr<mesh>> m_visualize_spheres;
	std::vector<visualize_direction> m_visualize_direction_stacks;
	std::shared_ptr<mesh> m_visualize_direction;
	std::shared_ptr<mesh> m_visualize_lines;
	std::shared_ptr<scene_shared_parameters> m_scene_rendering_shared;
	std::shared_ptr<mesh> m_axis;
	std::shared_ptr<mesh> m_target_mesh = nullptr;
};

