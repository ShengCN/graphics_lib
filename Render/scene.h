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
	std::shared_ptr<mesh> add_mesh(const std::string mesh_file, vec3 color);
	bool remove_mesh(int mesh_id);
	void add_mesh(std::shared_ptr<mesh> m);
	void load_scene(std::string scene_file);
	void clean_up();
	vec3 scene_center();
	std::shared_ptr<mesh> get_mesh(mesh_id id);
	std::unordered_map<mesh_id, std::shared_ptr<mesh>> get_meshes();
	AABB scene_aabb();
	bool save_scene(const std::string filename);
	void reset_camera(vec3 &look, vec3 &at);
	void reset_camera(std::shared_ptr<ppc> camera);
	void focus_at(std::shared_ptr<ppc> camera, std::shared_ptr<mesh> m, glm::vec3 relative_vec);
	void stand_on_plane(std::shared_ptr<mesh> m);

	//------- Protected Variables --------//
protected:
	// std::vector<std::shared_ptr<mesh>> m_meshes;
	std::unordered_map<mesh_id, std::shared_ptr<mesh>> m_meshes;
	std::unordered_map<mesh_id, std::shared_ptr<mesh>> m_lights;
};

