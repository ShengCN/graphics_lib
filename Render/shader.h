#pragma once
#include "mesh.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_2_Core>

enum class shader_type {
	template_shader,
	compute_shader,
	geometry_shader
};

class shader:QOpenGLFunctions_4_2_Core {
public:
	shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	
	bool reload_shader();
	virtual void draw_mesh(std::shared_ptr<mesh> m);

private:
	//GLuint init_compute_shader();
	//GLuint init_template_shader();
	//GLuint init_geometry_shader();

protected:
	//GLuint m_program;
	QOpenGLShaderProgram m_program;
	std::string m_vs, m_fs, m_gs, m_cs;
	shader_type m_type;
};
