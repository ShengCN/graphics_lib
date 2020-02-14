#pragma once
#include "mesh.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_4_Core>
enum class shader_type {
	template_shader,
	compute_shader,
	geometry_shader
};

struct scene_shared_parameters{
	vec3 light_pos;
};

class shader: public QOpenGLFunctions_4_4_Core {
public:
	shader(const std::string vs_shader, const std::string fs_shader);
	
	bool reload_shader();
	virtual void draw_mesh(std::shared_ptr<ppc> cur_camera, 
						   std::shared_ptr<mesh> m, 
						   std::shared_ptr<scene_shared_parameters> params);
	~shader() {};

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
