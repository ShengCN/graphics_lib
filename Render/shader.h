/*
 * OpenGL Related Drawing Functions should be put here 
*/
#pragma once

#include "mesh.h"
#include "ppc.h"
#include <memory>

enum class draw_type {
	triangle,
	line_segments,
    points
};

enum class shader_type {
	template_shader,
	compute_shader,
	geometry_shader
};

struct rendering_params {
	int frame;
    // uint64_t time;
	std::shared_ptr<ppc> cur_camera;
	std::shared_ptr<ppc> light_camera;
    std::vector<glm::vec3> lights;
    GLuint sm_texture;

	rendering_params():frame(0) {
	}
};

/* For complex object with different materials, Use this descriptor */
struct Mesh_Descriptor {
	std::shared_ptr<mesh> m;
	std::vector<std::shared_ptr<Image>> texs;
    draw_type type = draw_type::triangle;

    Mesh_Descriptor(std::shared_ptr<mesh> m):m(m) {}

    Mesh_Descriptor(std::shared_ptr<mesh> m,
                    std::vector<std::shared_ptr<Image>> texs):m(m), texs(texs) {}

    Mesh_Descriptor(std::shared_ptr<mesh> m,
                    std::vector<std::shared_ptr<Image>> texs,
                    draw_type type):m(m), texs(texs), type(type) {}
};

class shader  {
public:
	shader(const char* computeShaderFile);
	shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile);
	
	bool reload_shader();
	virtual void draw_mesh(const Mesh_Descriptor &descriptor, rendering_params& params);

	GLuint get_program() { return m_program; }
	GLuint get_shader_program() { return m_program; }
	void bind() {	glUseProgram(m_program);	}
	
private:
	GLuint init_template_shader();
	GLuint init_compute_shader();
	GLuint init_geometry_shader();
	void init_textures();

protected:
	GLuint m_program;
	std::string m_vs, m_fs, m_gs, m_cs;
	shader_type m_type;
    std::vector<GLuint> m_texids; 
};

class quad_shader: public shader {
public:
	quad_shader(const char* computeShaderFile);
	quad_shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	quad_shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile);

	virtual void draw_mesh(const Mesh_Descriptor &descriptor, rendering_params& params) override;
private:
	GLuint m_quad_vao, m_quad_vbo;
	void init_vao();
};

