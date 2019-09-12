#include <fstream>
#include <iostream>

#include "mesh.h"
#include "Utils.h"
#include "Logger.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))
int mesh::id = 0;

// base class clean up shaders, children class clean up buffer
void mesh::clean_up() {
	m_shader_program->removeAllShaders();
}

bool mesh::save_stl(const QString file) {
	bool success = true;

	// loading 
	if (check_file_extension(file, "stl") || check_file_extension(file, "STL")) {
		std::ofstream mfile(file.toStdString(), std::ios::out | std::ios::binary);
		if(mfile.is_open()){
			char header_info[80];
			mfile.write(header_info, 80);
			auto world_verts = compute_world_space_coords();
			unsigned int triangle_num = (unsigned int)world_verts.size() / 3;
			mfile.write((char*)&triangle_num, 4);

			for(unsigned int ti = 0; ti < triangle_num; ++ti){
				mfile.write((char*)&m_norms[3 * ti + 0], 12);
				mfile.write((char*)&world_verts[3 * ti + 0], 12);
				mfile.write((char*)&world_verts[3 * ti + 1], 12);
				mfile.write((char*)&world_verts[3 * ti + 2], 12);

				unsigned short attrib_byte_count = 0;
				mfile.write((char*)&attrib_byte_count, 2);
			}

			qDebug() << "File " << file << " saved. \n";
		} else {
			qDebug() << "File " << file << " cannot be saved! \n";
		}
	}
	else {
		success = false;
		qDebug() << "It's not a stl file!";
	}

	return success;
}

mesh::mesh(QString vs, QString fs): m_vs(vs), m_fs(fs), m_shader_program(nullptr){
	cur_id = ++id;
	initializeOpenGLFunctions();
	if (!init_shaders(vs, fs)) {
		std::string desc = std::to_string(id) + " init shader";
		LOG_FAIL(desc);
	}
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

void mesh::scale(vec3 s) {
	vec3 center = compute_center();
	mat4 scale_mat = glm::scale(s);
	for (auto&v : m_verts) {
		vec3 offset = v - center;
		offset = scale_mat * offset;
		v = center + offset;
	}
}

AABB mesh::compute_aabb() const {
	assert(m_verts.size() > 0);

	AABB aabb(m_verts[0]);
	for (auto &v : m_verts) {
		aabb.add_point(v);
	}
	return aabb;
}

AABB mesh::compute_world_aabb() const {
	auto aabb = compute_aabb();
	aabb.p0 = m_world*aabb.p0;
	aabb.p1 = m_world*aabb.p1;
	return aabb;
}

void mesh::set_color(vec3 col)
{
	if (m_verts.empty())
		return;

	m_colors.clear();
	m_colors.resize(m_verts.size(), col);
}

bool mesh::init_shaders(const QString vs, const QString fs) {
	bool success = true;
	m_shader_program = new QOpenGLShaderProgram(nullptr);

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

bool mesh::reload_shaders() {
	clean_up();

	//this may change later, cautious
	return init_shaders(m_vs, m_fs);	
}

bool mesh::test_all_vertex_attrb(std::vector<GLuint>& vert_attrs) {
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

void mesh::enable_vertex_attrib(std::vector<GLuint>& vert_attrs) {
	for(auto& va:vert_attrs){
		if (va != -1) {
			m_shader_program->enableAttributeArray(va);
		}
	}
}

void mesh::reset_vertex_attrib(std::vector<GLuint>& vert_attrs) {
	for (auto& va : vert_attrs) {
		va = -1;
	}
}

void mesh::normalize_to_origin(glm::vec3 scale /*= glm::vec3(1.0f)*/) {
	vec3 center = compute_world_center();
	for(auto &v:m_verts){
		v = v - center;
	}
}

void pc::create_ogl_buffers() {
}

void pc::update_ogl_buffers() {
}

void pc::draw(std::shared_ptr<ppc> ppc, int iter)
{
}

void pc::clean_up() {
}

void triangle_mesh::write_xml_element(QXmlStreamWriter& xml) {
	xml.writeStartElement("shape");
	xml.writeAttribute("type", "obj");

	mitsuba_xml::write_map_element(xml,
								   "string",
								   { {"filename", file_path.toStdString()} });

	if(m_is_emitter){
		xml.writeStartElement("emitter");
		xml.writeAttribute("type", "area");
		mitsuba_xml::write_map_element(xml,
									   "spectrum",
									   { {"radiance", "1"} }); //#todo_change_default value
		xml.writeEndElement();	//emitter
	}

	xml.writeEndElement();  // shape
}

//// load stl file
//bool triangle_mesh::load(const QString file) {
//	bool success = true;
//
//	m_verts.clear();
//	m_norms.clear();
//
//	// loading 
//	if (check_file_extension(file, "stl") || check_file_extension(file, "STL"))
//	{
//		std::ifstream mfile(file.toStdString(), std::ios::in | std::ios::binary);
//
//		char header_info[81] = "";
//		char nTri[4];
//		unsigned int nTriLong;
//		if (mfile) {
//			qDebug() << "Successfully open file: " << file;
//
//			// read 80 byte header
//			if (mfile.read(header_info, 80))
//				qDebug() << "header: " << header_info;
//			else
//				qDebug() << "Cannot read header info\n";
//
//			// read number of triangles
//			if (mfile.read(nTri, 4))
//			{
//				nTriLong = *((unsigned int*)nTri);
//				std::cout << "Triangle numbers: " << static_cast<int>(nTriLong) << std::endl;
//			}
//			else
//				qDebug() << "Cannot read number of triangles\n";
//
//			// Read all triangles
//			for (int trii = 0; trii < static_cast<int>(nTriLong); ++trii)
//			{
//				char facet[50];
//				if (mfile.read(facet, 50))
//				{
//					auto readv3 = [&](char *sr, char *ds, int startIndex)
//					{
//						for (int si = 0; si < 12; ++si)
//						{
//							ds[si] = sr[startIndex + si];
//						}
//					};
//
//					vec3 normal;
//					vec3 p0, p1, p2;
//
//					readv3(facet, (char*)&normal[0], 0);
//					readv3(facet, (char*)&p0[0], 0 + 12);
//					readv3(facet, (char*)&p1[0], 0 + 24);
//					readv3(facet, (char*)&p2[0], 0 + 36);
//
//					m_verts.push_back(p0);
//					m_verts.push_back(p1);
//					m_verts.push_back(p2);
//					
//					//// compute normals by ourself
//					normal = glm::cross(p1 - p0, p2 - p1);
//					m_norms.push_back(normal);
//					m_norms.push_back(normal);
//					m_norms.push_back(normal);
//				}
//				else {
//					success = false;
//					qDebug() << "Read triangle error\n";
//				}
//			}
//		}
//		else {
//			success = false;
//			qDebug() << "Cannot open file: " << file;
//		}
//	}
//	else {
//		success = false;
//		qDebug() << "It's not a stl file!";
//	}
//
//	qDebug() << "There are " << m_verts.size() << " points \n";
//
//	// normalize, move to center and align
//	vec3 center = compute_center();
//	mat4 norm_transform = glm::rotate(deg2rad(90.0f), vec3(1.0f, 0.0f, 0.0f)) *
//		glm::translate(-center);
//	mat4 normal_norm_transfrom = glm::inverse(glm::transpose(norm_transform));
//
//	for (int i = 0; i < m_verts.size(); ++i) {
//		m_verts[i] = norm_transform * m_verts[i];
//		m_norms[i] = normal_norm_transfrom * m_norms[i];
//	}
//
//	return success;
//}

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
	if (m_shader_program && m_verts.size() > 0) {
		if(!m_ogl_initialized){
			create_ogl_buffers();
			m_ogl_initialized = true;
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

		uniform_loc = glGetUniformLocation(program, "cur_selected_id");
		if (m_is_selected) {
			glUniform1i(uniform_loc, 1);
		} else {
			glUniform1i(uniform_loc, 0);
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
	mesh::clean_up();

	// clean up buffer
	reset_ogl_state();
}

void line_segment::reset_ogl_state() {
	throw std::logic_error("The method or operation is not implemented.");
}

void line_segment::create_ogl_buffers()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void line_segment::update_ogl_buffers()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void line_segment::draw(std::shared_ptr<ppc> ppc, int iter)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void line_segment::clean_up()
{
	throw std::logic_error("The method or operation is not implemented.");
}

float AABB::diag_length() {
	return glm::length(p1 - p0);
}
