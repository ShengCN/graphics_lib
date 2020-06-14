#pragma once
#include <QOpenGLFunctions_4_2_Core>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include "mesh.h"

enum class shader_type {
	template_shader,
	compute_shader,
	geometry_shader
};
class shader : public QOpenGLFunctions_4_2_Core {
public:
	shader(const char* computeShaderFile);
	shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile);
	
	bool reload_shader();
	virtual void draw_mesh(std::shared_ptr<mesh> m);
	QOpenGLShaderProgram* get_program() { return m_shader_program; }

private:
	bool init_template_shader();
	void init_ogl();

protected:
	QOpenGLShaderProgram* m_shader_program;
	GLuint m_program;
	std::string m_vs, m_fs, m_gs, m_cs;
	shader_type m_type;
};
