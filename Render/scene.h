#pragma once
#include <vector>
#include <memory>
#include <unordered_map>

#include "ppc.h"
#include "mesh.h"
#include "shader.h"

struct scene_shared_parameters;
class scene {
public:
	scene();
	~scene();

	//------- Shared Functions --------//
	std::shared_ptr<mesh> load_mesh(const std::string mesh_file, std::shared_ptr<shader> render_shader);
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
	void focus_at(std::shared_ptr<ppc> camera, std::shared_ptr<mesh> m);
	std::shared_ptr<mesh> get_target_mesh() { return m_target_mesh; }
	void stand_on_plane(std::shared_ptr<mesh> m);
	void add_light(std::shared_ptr<mesh> l) {
		m_lights.push_back(l);
	}

	std::vector<std::shared_ptr<mesh>> get_lights() { return m_lights; }

	void add_visualize_sphere(vec3 p, float radius=1, vec3 col=vec3(1.0f,0.0f,0.0f));
	void clear_visualize_point() { m_visualize_objs.clear(); }
	std::vector<std::shared_ptr<mesh>> get_visualize_meshes() { return m_visualize_objs; }
	//------- Protected Variables --------//
protected:
	std::vector<std::shared_ptr<mesh>> m_meshes;
	std::vector<std::shared_ptr<mesh>> m_lights;
	std::vector<std::shared_ptr<mesh>> m_visualize_objs;
	std::shared_ptr<scene_shared_parameters> m_scene_rendering_shared;

	std::shared_ptr<mesh> m_target_mesh = nullptr;
};

