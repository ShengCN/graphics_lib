#pragma once
#include <glad/glad.h>
#include "mesh.h"

enum class shader_type {
	template_shader,
	compute_shader,
	geometry_shader
};
class shader {
public:
	shader(const char* computeShaderFile);
	shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile);
	
	bool reload_shader();
	virtual void draw_mesh(std::shared_ptr<mesh> m);
	GLuint get_program() { return m_program; }

private:
	GLuint init_compute_shader();
	GLuint init_template_shader();
	GLuint init_geometry_shader();

private:
	GLuint m_program;
	std::string m_vs, m_fs, m_gs, m_cs;
	shader_type m_type;
};


