#include <fstream>
#include <iostream>
#include <glm/matrix.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh.h"
#include "graphics_lib/Utilities/Utils.h" 
#include "graphics_lib/Utilities/Logger.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))
int mesh::id = 0;

mesh::mesh(const std::string vs, const std::string fs): m_vs(vs), m_fs(fs){
	cur_id = ++id;
}

mesh::~mesh() {
}

std::vector<vec3> mesh::compute_world_space_coords()
{
	std::vector<vec3> world_coords = m_verts;
	for (auto& v : world_coords) {
		v = m_world * v;
	}
	return world_coords;
}

std::vector<vec3> mesh::compute_world_space_normals() {
	std::vector<vec3> world_normals = m_norms;
	auto normal_transform = glm::inverse(glm::transpose(m_world));

	for (auto& n : world_normals) {
		n = glm::vec3(normal_transform * vec4(n,0.0f));
	}
	return world_normals;
}

vec3 mesh::compute_center() {
	vec3 center(0.0f, 0.0f, 0.0f);
	float weights = 1.0f/(float)m_verts.size();
	for (auto& v : m_verts) {
		center += v * weights;
	}
	return center;
}

vec3 mesh::compute_world_center() {
	vec3 center = compute_center();
	return m_world * center;
}

void mesh::add_world_transate(vec3 v) {
	m_world = glm::translate(v) * m_world;
}

void mesh::add_scale(vec3 s) {
	vec3 center = compute_center();
	mat4 scale_mat = glm::scale(s);
	m_world = m_world * scale_mat;
}

void mesh::add_rotate(pd::deg angle, vec3 axis) {
	mat4 rot_mat = glm::rotate(deg2rad(angle), axis);
	m_world = rot_mat * m_world;
}

void mesh::add_face(vec3 va, vec3 vb, vec3 vc) {
	m_verts.push_back(va);
	m_verts.push_back(vb);
	m_verts.push_back(vc);

	vec3 ba = vb - va, cb = vc - vb;
	vec3 normal = glm::normalize(glm::cross(ba, cb));
	m_norms.push_back(normal);
	m_norms.push_back(normal);
	m_norms.push_back(normal);

	m_colors.push_back(GGV.default_stl_color);
	m_colors.push_back(GGV.default_stl_color);
	m_colors.push_back(GGV.default_stl_color);
}

void mesh::add_vertex(vec3 v, vec3 n, vec3 c) {
	m_verts.push_back(v);
	m_norms.push_back(n);
	m_colors.push_back(c);
}

void mesh::add_vertex(vec3 v, vec3 n, vec3 c, vec2 uv) {
	add_vertex(v, n, c);
	m_uvs.push_back(uv);
}

AABB mesh::compute_aabb() const {
	assert(m_verts.size() > 0);

	AABB aabb(m_verts[0]);
	for (auto &v : m_verts) {
		aabb.add_point(v);
	}
	return aabb;
}

AABB mesh::compute_world_aabb() {
	auto world_space_points = compute_world_space_coords();
	if (world_space_points.empty())
		return AABB(vec3(0.0f));

	AABB aabb(world_space_points[0]);
	for (auto& v : world_space_points) {
		aabb.add_point(v);
	}

	return aabb;
}

void mesh::set_color(vec3 col)
{
	if (m_verts.empty())
		return;

	if(m_colors.empty()) {
		m_colors.clear();
		m_colors.resize(m_verts.size(), col);
	} else {
		for(auto &c:m_colors) {
			c = col;
		}
	}

}

bool mesh::reload_shaders() {
	clean_up();

	//this may change later, cautious
	return init_shaders();	
}

void mesh::normalize_position_orientation(vec3 scale/*=vec3(1.0f)*/, glm::quat rot_quant /*= glm::quat(0.0f,0.0f,0.0f,1.0f)*/) {
	// normalize, move to center and align
	vec3 center = compute_center();
	mat4 norm_transform = glm::toMat4(rot_quant) * 
		glm::rotate(deg2rad(90.0f), vec3(1.0f, 0.0f, 0.0f)) *
		glm::scale(scale) *
		glm::translate(-center);
	m_world = norm_transform;
}

void mesh::get_demose_matrix(vec3& scale, quat& rot, vec3& translate) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(m_world, // input
				   scale,
				   rot,
				   translate,
				   skew,
				   perspective);
}

void mesh::set_matrix(const vec3 scale, const quat rot, const vec3 translate) {
	m_world = glm::translate(translate) *
		glm::scale(scale) * glm::toMat4(rot);
}

void mesh::draw_aabb(std::shared_ptr<ppc> camera) {
	if(!m_aabb_draw)
		m_aabb_draw = std::make_shared<line_segments>(GGV.template_vs, GGV.template_fs);
	
	m_aabb_draw->init_as_aabb(compute_world_aabb());
	m_aabb_draw->update_ogl_buffers();
	m_aabb_draw->draw(camera, 0);
}

void mesh::recompute_normal() {
	m_norms.clear();
	int triangle_num = m_verts.size() / 3;
	for(int ti = 0; ti < triangle_num; ++ti) {
		vec3 a = m_verts[3 * ti + 0];
		vec3 b = m_verts[3 * ti + 1];
		vec3 c = m_verts[3 * ti + 2];

		vec3 n = glm::normalize(glm::cross(b - a, c - b));
		m_norms.push_back(n);
		m_norms.push_back(n);
		m_norms.push_back(n);
	}
}

void mesh::remove_duplicate_vertices() {
	std::vector<bool> vert_visited(m_verts.size(), false);
	for (int vi = 0; vi < m_verts.size(); ++vi) {
		if (vert_visited[vi])	continue;
		
		for (int ovi = vi + 1; ovi < m_verts.size(); ++ovi) {
			if(same_point(m_verts[vi], m_verts[ovi])) {
				vert_visited[ovi] = true;
				m_verts[ovi] = m_verts[vi];
			}
		}
	}
}

// merge with world space coordinates
void mesh::merge_mesh(std::shared_ptr<mesh> b) {
	if(!b) {
		LOG_FAIL("Merge mesh");
		assert(false);
	}

	auto b_world_coord = b->compute_world_space_coords();
	auto b_world_normal = b->compute_world_space_normals();

	std::copy(b_world_coord.begin(), b_world_coord.end(), std::back_inserter(m_verts));
	std::copy(b->m_colors.begin(), b->m_colors.end(), std::back_inserter(m_colors));
	std::copy(b_world_normal.begin(), b_world_normal.end(), std::back_inserter(m_norms));
	std::copy(b->m_uvs.begin(), b->m_uvs.end(), std::back_inserter(m_uvs));
}

void qt_mesh::init_ogl_shader() {
	if (!m_is_ogl_context_initialized) {
		initializeOpenGLFunctions();
		if (!init_shaders()) {
			std::string desc = std::to_string(id) + " init shader";
			LOG_FAIL(desc);
		}
		m_is_ogl_context_initialized = true;
	}
}


bool qt_mesh::init_shaders() {
	bool success = true;
	m_shader_program = new QOpenGLShaderProgram(nullptr);
	QString vs = QString::fromStdString(m_vs);
	QString fs = QString::fromStdString(m_fs);

	if (m_shader_program && file_exists(vs) && file_exists(fs)) {
		success &= m_shader_program->addShaderFromSourceFile(QOpenGLShader::Vertex, vs);
		success &= m_shader_program->addShaderFromSourceFile(QOpenGLShader::Fragment, fs);
		success &= m_shader_program->link();
	}
	else {
		success = false;
		LOG_FAIL("find shaders");
	}
	return success;
}

void qt_mesh::clean_up() {
	if(m_shader_program)
		m_shader_program->removeAllShaders();
}

bool qt_mesh::test_all_vertex_attrb(std::vector<GLuint>& vert_attrs) {
	bool success = true;
	for (auto& va : vert_attrs) {
		success &= (va != -1);
	}
	if (!success) {
		std::string desc = std::to_string(get_id()) + " some attr";
		LOG_FAIL(desc);
	}

	return success;
}

void qt_mesh::enable_vertex_attrib(std::vector<GLuint>& vert_attrs) {
	for (auto& va : vert_attrs) {
		if (va != -1) {
			m_shader_program->enableAttributeArray(va);
		}
	}
}

void qt_mesh::reset_vertex_attrib(std::vector<GLuint>& vert_attrs) {
	for (auto& va : vert_attrs) {
		va = -1;
	}
}

void pc::reset_ogl_state() {
	throw std::logic_error("The method or operation is not implemented.");
}

void pc::create_ogl_buffers() {
	if (m_verts.size() == 0) {
		return;
	}

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	//------- init vertex attr --------//
	m_vert_attr = m_shader_program->attributeLocation("pos_attr");
	m_col_attr = m_shader_program->attributeLocation("col_attr");

	//------- Transfer buffer into GPU --------//
	update_ogl_buffers();
}

void pc::update_ogl_buffers() {
	if (m_vao == -1)
		return;

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	//------- Vertex Attr Description --------//
	std::vector<GLuint> vert_attrbs{ m_vert_attr, m_col_attr };
	test_all_vertex_attrb(vert_attrbs);

	//------- Buffer update --------//
	unsigned int buffer_size = (m_verts.size() + m_colors.size()) * sizeof(vec3);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, 0, GL_STATIC_DRAW);
	if (m_verts.size() > 0 && m_vert_attr != -1) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_verts.size() * sizeof(vec3), &m_verts[0]);
		glVertexAttribPointer(m_vert_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(m_vert_attr);
	}

	if (m_colors.size() > 0 && m_col_attr != -1) {
		unsigned int offset = m_verts.size() * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m_colors.size() * sizeof(vec3), &m_colors[0]);
		glVertexAttribPointer(m_col_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(m_col_attr);
	}

}

void pc::draw(std::shared_ptr<ppc> ppc, int iter) {
	init_ogl_shader();

	if (m_shader_program && m_verts.size() > 0) {
		if (!m_is_initialized) {
			create_ogl_buffers();
			m_is_initialized = true;
		}

		m_shader_program->bind();
		mat4 p = ppc->GetP();
		mat4 v = ppc->GetV();

		mat4 pvm = p * v * m_world;
		auto program = m_shader_program->programId();
		auto uniform_loc = glGetUniformLocation(program, "PVM");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(pvm));

		uniform_loc = glGetUniformLocation(program, "M");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(m_world));

		glBindVertexArray(m_vao);
		glDrawArrays(GL_POINTS, 0, (GLsizei)m_verts.size());
		glBindVertexArray(0);
	}
	else {
		LOG_FAIL("Triangle shader program init or loading verts");
	}
}

void pc::clean_up() {
}

// use world space coordinates
bool triangle_mesh::to_eigen(MatrixXd& V, MatrixXi& F) {
	if (m_verts.empty())
		return false;

	unsigned int tri_num = m_verts.size() / 3;
	V.resize(m_verts.size(), 3);
	F.resize(tri_num, 3);

	auto world_space_vertice = compute_world_space_coords();

	for(size_t i = 0; i < tri_num; ++i) {
		size_t cur_ind = 3 * i + 0;
		V(cur_ind, 0) = world_space_vertice[cur_ind].x; V(cur_ind, 1) = world_space_vertice[cur_ind].y; V(cur_ind, 2) = world_space_vertice[cur_ind].z; cur_ind++;
		V(cur_ind, 0) = world_space_vertice[cur_ind].x; V(cur_ind, 1) = world_space_vertice[cur_ind].y; V(cur_ind, 2) = world_space_vertice[cur_ind].z; cur_ind++;
		V(cur_ind, 0) = world_space_vertice[cur_ind].x; V(cur_ind, 1) = world_space_vertice[cur_ind].y; V(cur_ind, 2) = world_space_vertice[cur_ind].z;
		
		F(i, 0) = 3 * i + 0;
		F(i, 1) = 3 * i + 1;
		F(i, 2) = 3 * i + 2;
	}

	return true;
}

void triangle_mesh::from_eigen(MatrixXd& V, MatrixXi& F) {
	assert(F.cols() == 3);
	assert(V.cols() == 3);

	clear_vertices();
	auto face_num = F.rows();
	for(int fi = 0; fi < face_num; ++fi) {
		size_t ind_a = F(fi, 0);
		size_t ind_b = F(fi, 1);
		size_t ind_c = F(fi, 2);

		vec3 va = vec3(V(ind_a, 0), V(ind_a, 1), V(ind_a, 2));
		vec3 vb = vec3(V(ind_b, 0), V(ind_b, 1), V(ind_b, 2));
		vec3 vc = vec3(V(ind_c, 0), V(ind_c, 1), V(ind_c, 2));

		add_face(va, vb, vc);
	}
}

void triangle_mesh::create_ogl_buffers() {
	if (m_verts.size() == 0) {
		qDebug() << "Forgot to load mesh?";
		return;
	}

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	//------- init vertex attr --------//
	m_vert_attr = m_shader_program->attributeLocation("pos_attr");
	m_norm_attr = m_shader_program->attributeLocation("norm_attr");
	m_col_attr = m_shader_program->attributeLocation("col_attr");
	m_uvs_attr = m_shader_program->attributeLocation("uv_attr");

	//------- Transfer buffer into GPU --------//
	update_ogl_buffers();
}

void triangle_mesh::update_ogl_buffers() {
	if (m_vao == -1)
		return;

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	//------- Vertex Attr Description --------//
	std::vector<GLuint> vert_attrbs{ m_vert_attr, m_norm_attr, m_col_attr, m_uvs_attr };
	test_all_vertex_attrb(vert_attrbs);

	//------- Buffer update --------//
	unsigned int buffer_size = (m_verts.size() + m_norms.size() + m_colors.size()) * sizeof(vec3) + m_uvs.size() * sizeof(vec2);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, 0, GL_DYNAMIC_DRAW);
	if (m_verts.size() > 0 && m_vert_attr != -1) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_verts.size() * sizeof(vec3), &m_verts[0]);
		glVertexAttribPointer(m_vert_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(m_vert_attr);
	}
	if (m_norms.size() > 0 && m_norm_attr != -1) {
		unsigned int offset = m_verts.size() * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m_norms.size() * sizeof(vec3), &m_norms[0]);
		glVertexAttribPointer(m_norm_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(m_norm_attr);
	}

	if (m_colors.size() > 0 && m_col_attr != -1) {
		unsigned int offset = (m_verts.size() + m_norms.size()) * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m_colors.size() * sizeof(vec3), &m_colors[0]);
		glVertexAttribPointer(m_col_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(m_col_attr);
	}

	if (m_uvs.size() > 0 && m_uvs_attr != -1) {
		unsigned int offset = (m_verts.size() + m_norms.size() + m_colors.size()) * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m_uvs.size() * sizeof(vec2), &m_uvs[0]);
		glVertexAttribPointer(m_uvs_attr, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(m_uvs_attr);
	}
}

void triangle_mesh::reset_ogl_state() {
	//std::vector<GLuint> vert_attrbs{ m_vert_attr, m_norm_attr, m_col_attr, m_uvs_attr };
	//reset_vertex_attrib(vert_attrbs);
	glBindVertexArray(0);
	m_shader_program->release();
}

void triangle_mesh::draw(std::shared_ptr<ppc> ppc, int iter) {
	init_ogl_shader();

	if (m_shader_program && m_verts.size() > 0) {
		if(!m_is_initialized){
			create_ogl_buffers();
			m_is_initialized = true;
		}
		
		m_shader_program->bind();
		mat4 p = ppc->GetP();
		mat4 v = ppc->GetV();

		mat4 pvm = p * v * m_world;
		auto program = m_shader_program->programId();
		auto uniform_loc = glGetUniformLocation(program, "PVM");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(pvm));

		uniform_loc = glGetUniformLocation(program, "M");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(m_world));

		uniform_loc = glGetUniformLocation(program, "slider");
		glUniform1f(uniform_loc, (GLfloat)global_variable::instance()->fract_slider);

		if (m_is_selected) {
			glUniform1i(uniform_loc, 1);

		} else {
			glUniform1i(uniform_loc, 0);
		}

		if(m_is_draw_normal){
			uniform_loc = glGetUniformLocation(program, "is_draw_normal");
			glUniform1i(uniform_loc, m_is_draw_normal);
		}

		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_verts.size());
		glBindVertexArray(0);
	}
	else {
		LOG_FAIL("Triangle shader program init or loading verts");
	}
}

void triangle_mesh::clean_up() {
	qt_mesh::clean_up();

	// clean up buffer
	reset_ogl_state();
}

void line_segments::reset_ogl_state() {
	throw std::logic_error("The method or operation is not implemented.");
}

void line_segments::init_as_aabb(AABB aabb) {
	m_verts.clear();
	m_colors.clear();

	vec3 p0 = aabb.p0, p6 = aabb.p1;
	vec3 diag = aabb.diagonal();
	vec3 diag_x = vec3(diag.x, 0.0f, 0.0f);
	vec3 diag_y = vec3(0.0f, diag.y, 0.0f);
	vec3 diag_z = vec3(0.0f, 0.0f, diag.z);

	vec3 p1 = p0 + diag_x;
	vec3 p2 = p0 + diag_x + diag_z;
	vec3 p3 = p0 + diag_z;
	
	vec3 p4 = p0 + diag_y;
	vec3 p5 = p1 + diag_y;
	vec3 p7 = p3 + diag_y;

	vec3 c = vec3(0.0f, 0.0f, 1.0f);

	add_line(p0, c, p1, c);
	add_line(p1, c, p2, c);
	add_line(p2, c, p3, c);
	add_line(p3, c, p0, c);

	add_line(p0, c, p4, c);
	add_line(p1, c, p5, c);
	add_line(p2, c, p6, c);
	add_line(p3, c, p7, c);

	add_line(p4, c, p5, c);
	add_line(p5, c, p6, c);
	add_line(p6, c, p7, c);
	add_line(p7, c, p4, c);
}

void line_segments::add_line(vec3 p0, vec3 c0, vec3 p1, vec3 c1) {
	m_verts.push_back(p0);
	m_verts.push_back(p1);

	m_colors.push_back(c0);
	m_colors.push_back(c1);
}

void line_segments::create_ogl_buffers()
{
	if (m_verts.size() == 0) {
		qDebug() << "Forgot to load mesh?";
		return;
	}

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	//------- init vertex attr --------//
	m_vert_attr = m_shader_program->attributeLocation("pos_attr");
	m_col_attr = m_shader_program->attributeLocation("col_attr");

	//------- Transfer buffer into GPU --------//
	update_ogl_buffers();
}

void line_segments::update_ogl_buffers()
{
	if (m_vao == -1)
		return;

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	//------- Vertex Attr Description --------//
	std::vector<GLuint> vert_attrbs{ m_vert_attr, m_col_attr};
	test_all_vertex_attrb(vert_attrbs);

	//------- Buffer update --------//
	unsigned int buffer_size = (m_verts.size() + m_colors.size()) * sizeof(vec3);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, 0, GL_DYNAMIC_DRAW);
	if (m_verts.size() > 0 && m_vert_attr != -1) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_verts.size() * sizeof(vec3), &m_verts[0]);
		glVertexAttribPointer(m_vert_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(m_vert_attr);
	}

	if (m_colors.size() > 0 && m_col_attr != -1) {
		unsigned int offset = m_verts.size() * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m_colors.size() * sizeof(vec3), &m_colors[0]);
		glVertexAttribPointer(m_col_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(m_col_attr);
	}
}

void line_segments::draw(std::shared_ptr<ppc> ppc, int iter) {
	init_ogl_shader();
	if (m_shader_program && m_verts.size() > 0) {
		if (!m_is_initialized) {
			create_ogl_buffers();
			m_is_initialized = true;
		}

		m_shader_program->bind();
		mat4 p = ppc->GetP();
		mat4 v = ppc->GetV();

		mat4 pvm = p * v * m_world;
		auto program = m_shader_program->programId();
		auto uniform_loc = glGetUniformLocation(program, "PVM");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(pvm));

		uniform_loc = glGetUniformLocation(program, "M");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(m_world));

		glBindVertexArray(m_vao);
		int speed = 5;
		GLsizei array_size = (GLsizei)(m_verts.size() * m_drawn_fract);
		if(m_animated) {
			array_size = (GLsizei)(iter * speed) % m_verts.size();
		}

		glDrawArrays(GL_LINES, 0, array_size);

		glBindVertexArray(0);
	}
	else {
		LOG_FAIL("Triangle shader program init or loading verts");
	}
}

void line_segments::clean_up()
{
	throw std::logic_error("The method or operation is not implemented.");
}

float AABB::diag_length() {
	return glm::length(p1 - p0);
}

