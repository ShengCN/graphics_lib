#pragma once

#include <vector>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <algorithm>
#include <memory>
#include <QOpenGLFunctions_4_2_Core>
#include <glm/ext/matrix_transform.hpp>

#include "mitsuba_xml.h"
#include "ppc.h"
#include "global_variable.h"	

using pd::operator *;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

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

	float diag_length();
};

/*!
 * \class Base class for mesh
 *
 * \brief To initialize a mesh
 * Do this:
 * Initialize with shader files
 * Load mesh 
 * create ogl buffers
 *
 * \author YichenSheng
 * \date August 2019
 */
class mesh: public QOpenGLFunctions_4_2_Core, public mitsuba_xml_interface
{
public:
	mesh(QString vs, QString fs);
	~mesh();

	//------- shared functions --------//
public:
	std::vector<vec3> compute_world_space_coords();
	vec3 compute_center();	// center in model space
	vec3 compute_world_center();  // center in world space
	void world_transate(mat4 m) { m_world = m * m_world; }
	void scale(vec3 s);	// scale in model space
	AABB compute_aabb() const;
	AABB compute_world_aabb() const;
	void set_color(vec3 col);
	bool init_shaders(const QString vs, const QString fs);
	bool reload_shaders();
	bool test_all_vertex_attrb(std::vector<GLuint>& vert_attrs);
	void enable_vertex_attrib(std::vector<GLuint>& vert_attrs);
	void reset_vertex_attrib(std::vector<GLuint>& vert_attrs);
	unsigned int triangle_num() { return m_verts.size() / 3;}
	void normalize_to_origin(glm::vec3 scale = glm::vec3(1.0f));
	void set_select(bool is_selected) { m_is_selected = is_selected; }

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
	virtual void clean_up();	
	virtual bool save_stl(const QString name);

	//------- member variables --------//
public:
	mat4 m_world = glm::identity<glm::mat4>(); // model space -> world space
	std::vector<vec3> m_verts;
	std::vector<vec3> m_norms;
	std::vector<vec3> m_colors;
	std::vector<vec2> m_uvs;
	QOpenGLShaderProgram* m_shader_program;
	QString m_vs, m_fs;
	int cur_id = -1;
	static int id;
	bool m_ogl_initialized = false;
	bool m_is_selected = false;
	QString file_path;
	bool m_is_emitter = false;
};

/*!
 * \class point cloud mesh
 *
 * \brief Concrete class for point clouds
 *
 * \author YichenSheng
 * \date August 2019
 */
class pc :public mesh {
public:
	pc(QString vs, QString fs) :mesh(vs, fs) {};

	//------- interface --------//
public:

	virtual void create_ogl_buffers() override;
	virtual void update_ogl_buffers() override;
	virtual void draw(std::shared_ptr<ppc> ppc, int iter) override;
	virtual void clean_up() override;
};

/*!
 * \class triangle mesh
 *
 * \brief Concrete class for triangle mesh
 *
 * \author YichenSheng
 * \date August 2019
 */
class triangle_mesh :public mesh {
public:
	triangle_mesh(QString vs, QString fs) :
		mesh(vs, fs),
		m_vao(-1),
		m_vbo(-1),
		m_vert_attr(-1),
		m_norm_attr(-1), 
		m_col_attr(-1), 
		m_uvs_attr(-1) {
	};

public:

	//------- interface --------//
public:
	virtual void create_ogl_buffers() override;
	virtual void update_ogl_buffers() override;
	virtual void reset_ogl_state() override;
	virtual void draw(std::shared_ptr<ppc> ppc, int iter) override;
	virtual void clean_up() override; 
	virtual void write_xml_element(QXmlStreamWriter& xml) override;

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
class line_segment :public mesh {
public:
	line_segment(QString vs, QString fs) :mesh(vs, fs) {};

	//------- interface --------//
public:
	virtual void create_ogl_buffers() override;
	virtual void update_ogl_buffers() override;
	virtual void reset_ogl_state() override;
	virtual void draw(std::shared_ptr<ppc> ppc, int iter) override;
	virtual void clean_up() override;
};