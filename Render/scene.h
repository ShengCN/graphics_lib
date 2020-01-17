#pragma once
#include <vector>
#include <memory>
#include "ppc.h"
#include "mesh.h"

class scene
{
public:
	scene();
	~scene();

	virtual void load_scene(std::string scene_file);
	virtual bool reload_shaders();
	virtual void draw_scene(std::shared_ptr<ppc> cur_camera,int iter);
	virtual void clean_up();
	virtual vec3 scene_center();
	virtual std::vector<std::shared_ptr<mesh>> get_meshes();
	virtual std::shared_ptr<mesh> get_mesh(int mesh_id);
	virtual AABB scene_aabb();
	virtual bool save_scene(const std::string filename);
	virtual void add_mesh(std::shared_ptr<mesh> m);
	virtual void reset_camera(vec3 &look, vec3 &at);
	virtual void reset_camera(std::shared_ptr<ppc> camera);

	//------- Protected Variables --------//
protected:
	std::vector<std::shared_ptr<mesh>> m_meshes;
};

