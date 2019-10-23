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
	void draw_scene(std::shared_ptr<ppc> cur_camera,int iter);
	void clean_up();
	vec3 scene_center();
	
	std::vector<std::shared_ptr<mesh>> get_inside_objects() { return m_meshes; }
	std::vector<std::shared_ptr<mesh>> get_meshes();
	std::shared_ptr<mesh> get_mesh(int mesh_id);

	//------- Public functions --------//
public:
	std::shared_ptr<triangle_mesh> load_stl(QString file_path, 
											vec3 color, 
											bool is_container = false, 
											bool is_normalize=true);
	AABB scene_aabb();
	bool save_scene(const QString filename);
	void add_mesh(std::shared_ptr<mesh> m);
	void add_vis_point(vec3 p, vec3 color);
	void add_vis_line_seg(vec3 t, vec3 h);
	std::shared_ptr<triangle_mesh> add_plane(vec3 p, vec3 n, vec3 c= vec3(0.9f),float size=1.0f);
	void clear_visualization() { m_vis_lines->clear_vertices(); m_vis_points->clear_vertices(); }
	void reset_camera(vec3 &look, vec3 &at);
	void reset_camera(std::shared_ptr<ppc> camera);

	/* Only Display Fract of total number segments */
	void set_vis_line_fract(float fract);
	void set_vis_line_animated(bool trigger);

	//------- Setter & Getter --------//
	void set_container(std::shared_ptr<mesh> m) { m_container = m; }
	void set_container_upper(std::shared_ptr<mesh> m);
	void set_container_bottm(std::shared_ptr<mesh> m);

	std::shared_ptr<mesh> get_container() { return m_container; }
	std::shared_ptr<mesh> get_upper() { return m_container_upper; }
	std::shared_ptr<mesh> get_bottom() { return m_container_bottm; }

	//------- Private Variables --------//
private:
	std::vector<std::shared_ptr<mesh>> m_meshes;
	std::shared_ptr<pc> m_vis_points;
	std::shared_ptr<line_segments> m_vis_lines;

	std::shared_ptr<mesh> m_container;
	std::shared_ptr<mesh> m_container_upper;
	std::shared_ptr<mesh> m_container_bottm;
};

