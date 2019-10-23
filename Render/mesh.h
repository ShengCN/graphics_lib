#pragma once

#include <vector>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <algorithm>
#include <memory>
#include <QOpenGLFunctions_4_2_Core>

#include <Eigen/Core>
using namespace Eigen;

#include "graphics_lib/Utilities/Utils.h"
#include "ppc.h"
#include "graphics_lib/Utilities/Utils.h"

/*!
 * \class Axis aligned bounding box
 *
 * \brief Simplified geometry for fast pruning 
 *
 * \author YichenSheng
 * \date August 2019
 */
struct AABB
{
	vec3 p0, p1;
	AABB(vec3 p) :p0(p), p1(p) {}
	AABB(vec3 sm, vec3 lg) :p0(sm), p1(lg) {} // small, large

	void add_point(vec3 p) {
		p0.x = std::min(p.x, p0.x);
		p0.y = std::min(p.y, p0.y);
		p0.z = std::min(p.z, p0.z);

		p1.x = std::max(p.x, p1.x);
		p1.y = std::max(p.y, p1.y);
		p1.z = std::max(p.z, p1.z);
	}

	void add_aabb(const AABB &new_aabb) {
		add_point(new_aabb.p0);
		add_point(new_aabb.p1);
	}

	float diag_length();
	
	vec3 diagonal() {
		return p1 - p0;
	}

	vec3 center() {
		return 0.5f * (p1 + p0);
	}
};

class line_segments;
/*!
 * \class Base class for mesh
 *
 * \brief 
 *
 * \author YichenSheng
 * \date August 2019
 */
class mesh
{
public:
	mesh(const std::string vs, const std::string fs);
	~mesh();

	//------- shared functions --------//
public:
	std::vector<vec3> compute_world_space_coords();
	std::vector<vec3> compute_world_space_normals();
	vec3 compute_center();	// center in model space
	vec3 compute_world_center();  // center in world space
	
	void add_world_transate(vec3 v);
	void add_scale(vec3 s);	// scale in model space
	void add_rotate(pd::deg angle, vec3 axis);

	void add_face(vec3 va, vec3 vb, vec3 vc);	// estimate normal
	void add_vertex(vec3 v, vec3 n, vec3 c);
	void add_vertex(vec3 v, vec3 n, vec3 c, vec2 uv);
	AABB compute_aabb() const;
	AABB compute_world_aabb();
	void set_color(vec3 col);
	virtual bool init_shaders()=0;
	bool reload_shaders();

	void normalize_position_orientation(vec3 scale=vec3(1.0f), 
										glm::quat rot_quant = glm::quat(0.0f,0.0f,0.0f,0.0f));
	
	void get_demose_matrix(vec3& scale, quat& rot, vec3& translate);
	void set_matrix(const vec3 scale, const quat rot, const vec3 translate);
	void draw_aabb(std::shared_ptr<ppc> camera);
	void clear_vertices() { m_world = glm::identity<mat4>(); m_verts.clear(); m_norms.clear(); m_colors.clear(); m_uvs.clear(); }
	void recompute_normal();
	void remove_duplicate_vertices();
	void merge_mesh(std::shared_ptr<mesh> b);
	std::string to_string() {
		return std::to_string(get_id());
	}

	//------- getter & setter --------//
public:
	mat4 get_world_mat() { return m_world; }
	void set_world_mat(const mat4 m) { m_world = m; }
	int get_id() { return cur_id; }

	//------- interface --------//
public:
	virtual void create_ogl_buffers() = 0;
	virtual void update_ogl_buffers() = 0;
	virtual void reset_ogl_state() = 0;
	virtual void draw(std::shared_ptr<ppc> ppc, int iter) = 0;
	virtual void clean_up()=0;	
	virtual bool test_all_vertex_attrb(std::vector<GLuint>& vert_attrs)=0;
	virtual void enable_vertex_attrib(std::vector<GLuint>& vert_attrs)=0;
	virtual void reset_vertex_attrib(std::vector<GLuint>& vert_attrs)=0;

	//------- member variables --------//
public:
	mat4 m_world = glm::mat4(1.0f); // model space -> world space
	std::vector<vec3> m_verts;
	std::vector<vec3> m_norms;
	std::vector<vec3> m_colors;
	std::vector<vec2> m_uvs;
	std::string file_path;
	
	std::string m_vs, m_fs;
	int cur_id = -1;
	static int id;
	bool m_is_ogl_context_initialized = false;
	bool m_is_initialized = false;
	bool m_is_axis = false;
	bool m_is_container = false;

	//------- Private variables --------//
private:
	std::shared_ptr<line_segments> m_aabb_draw;
};

class qt_mesh:public mesh, public QOpenGLFunctions_4_2_Core {
public:
	qt_mesh(const std::string vs, const std::string fs) :
		mesh(vs, fs), m_shader_program(nullptr), m_is_selected(false), m_is_draw_normal(false) {};
	void init_ogl_shader();

public:
	//------- Virtual Functions --------//
	virtual bool init_shaders() override;
	virtual void clean_up() override;
	virtual bool test_all_vertex_attrb(std::vector<GLuint>& vert_attrs) override;
	virtual void enable_vertex_attrib(std::vector<GLuint>& vert_attrs) override;
	virtual void reset_vertex_attrib(std::vector<GLuint>& vert_attrs) override;

protected:
	QOpenGLShaderProgram* m_shader_program;
	bool m_is_selected;
	bool m_is_draw_normal;
};

/*!
 * \class point cloud mesh
 *
 * \brief Concrete class for point clouds
 *
 * \author YichenSheng
 * \date August 2019
 */
class pc :public qt_mesh {
public:
	pc(const std::string vs, const std::string fs) :
		qt_mesh(vs, fs),
		m_vao(-1),
		m_vbo(-1),
		m_vert_attr(-1),
		m_col_attr(-1) {};

	//------- Public functions --------//
public:
	void add_point(vec3 p, vec3 c) {
		m_verts.push_back(p);
		m_colors.push_back(c);
	}

	void clear_points() {
		m_verts.clear();
		m_colors.clear();
	}

	//------- interface --------//
public:
	virtual void create_ogl_buffers() override;
	virtual void update_ogl_buffers() override;
	virtual void draw(std::shared_ptr<ppc> ppc, int iter) override;
	virtual void clean_up() override;
	virtual void reset_ogl_state() override;

private:
	GLuint m_vao, m_vbo;
	GLuint m_vert_attr, m_col_attr;
};

/*!
 * \class triangle mesh
 *
 * \brief Concrete class for triangle mesh
 *
 * \author YichenSheng
 * \date August 2019
 */
class triangle_mesh :public qt_mesh {
public:
	triangle_mesh(const std::string vs, const std::string fs) :
		qt_mesh(vs, fs),
		m_vao(-1),
		m_vbo(-1),
		m_vert_attr(-1),
		m_norm_attr(-1), 
		m_col_attr(-1), 
		m_uvs_attr(-1) {
	};

public:
	//#todo_remove_eigen_from_this_file
	bool to_eigen(MatrixXd& V, MatrixXi& F);
	void from_eigen(MatrixXd& V, MatrixXi& F);

	//------- interface --------//
public:
	virtual void create_ogl_buffers() override;
	virtual void update_ogl_buffers() override;
	virtual void reset_ogl_state() override;
	virtual void draw(std::shared_ptr<ppc> ppc, int iter) override;
	virtual void clean_up() override; 

	//------- variables --------//
protected:
	GLuint m_vao, m_vbo;
	GLuint m_vert_attr, m_norm_attr, m_col_attr, m_uvs_attr;
};

/*!
 * \class line segments
 *
 * \brief 
 *
 * \author YichenSheng
 * \date August 2019
 */
class line_segments :public qt_mesh {
public:
	line_segments(const std::string vs, const std::string fs) :qt_mesh(vs, fs), m_drawn_fract(1.0f) {};

	void init_as_aabb(AABB aabb);
	void add_line(vec3 p0, vec3 c0, vec3 p1, vec3 c1);
	void set_drawing_fract(float fract) {
		m_drawn_fract = fract;
		m_drawn_fract = glm::clamp(m_drawn_fract, 0.0f, 1.0f);
	}

	void set_animated(bool trigger) { m_animated = trigger; }
	//------- interface --------//
public:
	virtual void create_ogl_buffers() override;
	virtual void update_ogl_buffers() override;
	virtual void reset_ogl_state() override;
	virtual void draw(std::shared_ptr<ppc> ppc, int iter) override;
	virtual void clean_up() override;

private:
	GLuint m_vao, m_vbo;
	GLuint m_vert_attr, m_col_attr;
	float m_drawn_fract;
	bool m_animated = false;
};