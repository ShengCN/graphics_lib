#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QSurface>

#include "shader.h"

using std::ifstream;
using std::ios;

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

shader::shader(const std::string vs_shader, const std::string fs_shader) :
	m_vs(vs_shader), m_fs(fs_shader) {
	initializeOpenGLFunctions();
	reload_shader();
}

bool shader::reload_shader() {
	// m_program.link();

	m_program.removeAllShaders();
	m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, QString::fromStdString(m_vs));
	m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, QString::fromStdString(m_fs));
	
	if (m_program.link()) {
		return true;
	}else {
		WARN(m_program.log().toStdString());
		return false;
	}
}

void shader::draw_mesh(std::shared_ptr<ppc> cur_camera, std::shared_ptr<mesh> m, std::shared_ptr<scene_shared_parameters> params) {
	GLuint vert_attr = m_program.attributeLocation("pos_attr");
	GLuint norm_attr = m_program.attributeLocation("norm_attr");
	GLuint col_attr = m_program.attributeLocation("col_attr");
	GLuint uv_attr = m_program.attributeLocation("uv_attr");

	static GLuint vao = -1, vbo = -1;
	if (vao == -1) glGenVertexArrays(1, &vao);
	if (vbo == -1) glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//------- Buffer update --------//
	size_t buffer_size = (m->m_verts.size() + m->m_norms.size() + m->m_colors.size()) * sizeof(vec3) + m->m_uvs.size() * sizeof(vec2);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, 0, GL_DYNAMIC_DRAW);
	if (m->m_verts.size() > 0 && vert_attr != -1) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, m->m_verts.size() * sizeof(vec3), &m->m_verts[0]);
		glVertexAttribPointer(vert_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(vert_attr);
	}
	if (m->m_norms.size() > 0 && norm_attr != -1) {
		size_t offset = m->m_verts.size() * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m->m_norms.size() * sizeof(vec3), &m->m_norms[0]);
		glVertexAttribPointer(norm_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(norm_attr);
	}

	if (m->m_colors.size() > 0 && col_attr != -1) {
		size_t offset = (m->m_verts.size() + m->m_norms.size()) * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m->m_colors.size() * sizeof(vec3), &m->m_colors[0]);
		glVertexAttribPointer(col_attr, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(col_attr);
	}

	if (m->m_uvs.size() > 0 && uv_attr != -1) {
		size_t offset = (m->m_verts.size() + m->m_norms.size() + m->m_colors.size()) * sizeof(vec3);
		glBufferSubData(GL_ARRAY_BUFFER, offset, m->m_uvs.size() * sizeof(vec2), &m->m_uvs[0]);
		glVertexAttribPointer(uv_attr, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
		glEnableVertexAttribArray(uv_attr);
	}

	m_program.bind();

	mat4 p = cur_camera->GetP();
	mat4 v = cur_camera->GetV();
	mat4 pvm = p * v * m->m_world;
	auto uniform_loc = glGetUniformLocation(m_program.programId(), "PVM");
	glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(pvm));

	uniform_loc = glGetUniformLocation(m_program.programId(), "P");
	if (uniform_loc != -1)
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(p));

	uniform_loc = glGetUniformLocation(m_program.programId(), "V");
	if (uniform_loc != -1)
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(v));

	uniform_loc = glGetUniformLocation(m_program.programId(), "M");
	if (uniform_loc != -1)
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(m->m_world));

	uniform_loc = glGetUniformLocation(m_program.programId(), "iter");
	if (uniform_loc != -1)
		glUniform1i(uniform_loc, params->iter);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m->m_verts.size());
	glBindVertexArray(0);
}
