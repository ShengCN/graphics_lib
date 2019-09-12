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

	void load_scene(QString scene_file);
	bool reload_shaders();
	void draw_scene(int iter);
	void clean_up();
	vec3 scene_center();

	//------- Public functions --------//
public:
	void load_mesh(QString file_path);
	// void load_stl(QString file_path, vec3 color, bool is_container = false);
	AABB scene_aabb();
	bool save_scene(const QString filename);
	void mesh_selected(int id);

	//------- Getter & Setter --------//
public:
	void set_mesh_transform(int mesh_id, glm::mat4 new_transform);
	std::shared_ptr<ppc> get_camera() { return m_camera; }
	std::vector<std::shared_ptr<mesh>> get_meshes() { return m_meshes; }

	//------- Mouse controls --------//
public:
	void mouse_pressed(int x, int y);
	void mouse_released(int x, int y);
	void mouse_movement(int x, int y);

	//------- Keyboard controls --------//
	void key_pressed(int k, bool is_shift = false);

private:
	bool m_is_pressed = false;
	int m_last_x, m_last_y;
	std::shared_ptr<ppc> m_last_ppc;
	std::shared_ptr<ppc> m_new_ppc;

	//------- Private Variables --------//
private:
	std::vector<std::shared_ptr<mesh>> m_meshes;
	std::shared_ptr<ppc> m_camera;
};

