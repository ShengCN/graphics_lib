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

	virtual void load_scene(QString scene_file);
	virtual bool reload_shaders();
	virtual void draw_scene(std::shared_ptr<ppc> cur_camera,int iter);
	virtual void clean_up();
	virtual vec3 scene_center();
	virtual std::vector<std::shared_ptr<mesh>> get_meshes();
	virtual std::shared_ptr<mesh> get_mesh(int mesh_id);
	virtual AABB scene_aabb();
	virtual bool save_scene(const QString filename);
	virtual void add_mesh(std::shared_ptr<mesh> m);
	virtual void reset_camera(vec3 &look, vec3 &at);
	virtual void reset_camera(std::shared_ptr<ppc> camera);

	//------- Public functions --------//
public:
	void add_vis_point(vec3 p, vec3 color);
	void add_vis_line_seg(vec3 t, vec3 h);
	void clear_visualization() { m_vis_lines->clear_vertices(); m_vis_points->clear_vertices(); }
	void draw_visualization(std::shared_ptr<ppc> cur_camera, int iter);

	/* Only Display Fraction of total number segments */
	void set_vis_line_fract(float fract);
	void set_vis_line_animated(bool trigger);

	//------- Getter & Setter --------//
public:
	void set_visualize_points(bool is_vis) { m_is_visualize_points = is_vis; }
	void set_visualize_lines(bool is_vis) { m_is_visualize_lines = is_vis; }
	void set_visualize_aabb(bool is_vis) { m_is_visualize_aabb = is_vis; }

	bool get_visualize_points() { return m_is_visualize_points; }
	bool get_visualize_lines() { return m_is_visualize_lines; }
	bool get_visualize_aabb() { return m_is_visualize_aabb; }

	//------- Protected Variables --------//
protected:
	std::vector<std::shared_ptr<mesh>> m_meshes;
	std::shared_ptr<pc> m_vis_points;
	std::shared_ptr<line_segments> m_vis_lines;
	bool m_is_visualize_points;
	bool m_is_visualize_lines;
	bool m_is_visualize_aabb;
};

