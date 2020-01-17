#pragma once
#include <vector>
#include <memory>
#include <unordered_map>

#include "ppc.h"
#include "mesh.h"
#include "shader.h"

class scene {
public:
	scene();
	~scene();

	//------- Shared Functions --------//
	std::shared_ptr<mesh> load_mesh(const std::string mesh_file, std::shared_ptr<shader> render_shader);
	virtual void add_mesh(std::shared_ptr<mesh> m);

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
	void stand_on_plane(std::shared_ptr<mesh> m);

	//------- Protected Variables --------//
protected:
	std::vector<std::shared_ptr<mesh>> m_meshes;
};

