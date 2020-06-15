#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "graphics_lib/asset_manager.h"

using std::ifstream;
using std::ios;

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

shader::shader(const char* computeShaderFile) {
	m_cs = computeShaderFile;
	m_type = shader_type::compute_shader;

	reload_shader();
}

shader::shader(const char* vertexShaderFile, const char* fragmentShaderFile) {
	m_vs = vertexShaderFile; m_fs = fragmentShaderFile;
	m_type = shader_type::template_shader;

	reload_shader();
}

shader::shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile) {
	m_vs = vertexShaderFile; m_gs = geometryShader; m_fs = fragmentShaderFile;
	m_type = shader_type::geometry_shader;

	reload_shader();
}

bool shader::reload_shader() {
	init_ogl();

	if (m_shader_program != nullptr) {
		m_shader_program->removeAllShaders();
	}

	return init_template_shader();
}

void shader::draw_mesh(std::shared_ptr<mesh> m, rendering_params& params) {
	GLuint vert_attr = glGetAttribLocation(m_program, "pos_attr");
	GLuint norm_attr = glGetAttribLocation(m_program, "norm_attr");
	GLuint col_attr = glGetAttribLocation(m_program, "col_attr");
	GLuint uv_attr = glGetAttribLocation(m_program, "uv_attr");

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

	glUseProgram(m_program);

	mat4 p = params.cur_camera->GetP();
	mat4 v = params.cur_camera->GetV();
	mat4 pvm = p * v * m->m_world;
	auto uniform_loc = glGetUniformLocation(m_program, "PVM");
	glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(pvm));

	uniform_loc = glGetUniformLocation(m_program, "P");
	if(uniform_loc!=-1)
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(p));

	uniform_loc = glGetUniformLocation(m_program, "V");
	if (uniform_loc != -1)
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(v));

	uniform_loc = glGetUniformLocation(m_program, "M");
	if (uniform_loc != -1)
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(m->m_world));

	uniform_loc = glGetUniformLocation(m_program, "light_pos");
	if (uniform_loc != -1) {
		//#todo_multiple_lights
		glUniform3f(uniform_loc, params.p_lights[0].x, params.p_lights[0].y, params.p_lights[0].z);
	}
	int ogl_draw_type = 0;
	if (params.dtype == draw_type::triangle) {
		ogl_draw_type = GL_TRIANGLES;
	} 

	if (params.dtype == draw_type::line_segments) {
		ogl_draw_type = GL_LINES;
	}

	glBindVertexArray(vao);
	glDrawArrays(ogl_draw_type, 0, (GLsizei)m->m_verts.size());
	glBindVertexArray(0);
}

bool shader::init_template_shader() {
	bool success = true;
	
	if (pd::file_exists(m_vs) && pd::file_exists(m_fs)) {
		success &= m_shader_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString::fromStdString(m_vs));
		success &= m_shader_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString::fromStdString(m_fs));
		success &= m_shader_program->link();
	}
	else {
		WARN("Cannot find shader source files");
		return false;
	}
	return success;
}

void shader::init_ogl() {
	initializeOpenGLFunctions();

	if (m_shader_program == nullptr) 
		m_shader_program = new QOpenGLShaderProgram(nullptr);
	
	m_program = m_shader_program->programId();
}
