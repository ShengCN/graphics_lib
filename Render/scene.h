#pragma once
#include <common.h> 

#include "ppc.h"
#include "mesh.h"
#include "shader.h"

class scene : public ISerialize {
protected:
    std::unordered_map<mesh_id, std::shared_ptr<Mesh_Descriptor>> m_meshes;
    std::vector<glm::vec2> m_lights;

public:
	scene();
	~scene();

    /* Interface */
    virtual std::string to_json() override;  
    virtual int from_json(const std::string jsonstr) override;  


	//------- Shared Functions --------//
    bool load_scene(std::string scene_file);
    bool save_scene(std::string scene_file);
    void clean_up();

	std::shared_ptr<mesh> add_mesh(const std::string mesh_file, vec3 color=vec3(0.7f));
    std::shared_ptr<mesh> add_mesh(std::shared_ptr<mesh> m, draw_type type);
    bool remove_mesh(mesh_id id);

    bool set_draw_type(mesh_id id, draw_type type);

    /* Queries */
    std::shared_ptr<mesh> get_mesh(mesh_id id);
    std::unordered_map<mesh_id, std::shared_ptr<Mesh_Descriptor>> get_mesh_descriptors();

	static std::shared_ptr<mesh> get_plane_mesh(vec3 p=vec3(0.0f), vec3 n=vec3(0.0f,1.0f,0.0f));
	vec3 scene_center();
	AABB scene_aabb();
	void reset_camera(vec3 &look, vec3 &at);
	void reset_camera(std::shared_ptr<ppc> camera);
	void focus_at(std::shared_ptr<ppc> camera, std::shared_ptr<mesh> m, glm::vec3 relative_vec);
	void scale(glm::vec3 s);
};
