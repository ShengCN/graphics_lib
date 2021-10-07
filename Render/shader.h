/*
 * OpenGL Related Drawing Functions should be put here 
*/
#pragma once

#include "mesh.h"
#include "ppc.h"
#include <memory>

enum class draw_type {
	triangle,
	line_segments
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
	std::vector<vec3> p_lights;
	draw_type dtype;

	vec3 sm_target_center;
	GLuint sm_texture;
	std::shared_ptr<ppc> light_camera;

	rendering_params():frame(0), dtype(draw_type::triangle), sm_texture(0), sm_target_center(0.0f) {
	}
};

/* For complex object with different materials, Use this descriptor */
struct Mesh_Descriptor {
	std::shared_ptr<mesh> m;
	std::vector<std::shared_ptr<Image>> texs;

    Mesh_Descriptor(std::shared_ptr<mesh> m):m(m) {}
    Mesh_Descriptor(std::shared_ptr<mesh> m, std::vector<std::shared_ptr<Image>> texs):m(m), texs(texs) {}
}; 

class shader  {
public:
	shader(const char* computeShaderFile);
	shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile);
	
	bool reload_shader();
	//virtual void draw_mesh(std::shared_ptr<mesh> m, rendering_params& params);
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
	static GLuint m_texids[16]; // https://learnopengl.com/Getting-started/Textures
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

class shadow_shader: public shader {
public:
	shadow_shader(const char* computeShaderFile);
	shadow_shader(const char* vertexShaderFile, const char* fragmentShaderFile);
	shadow_shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile);
	virtual void draw_mesh(const Mesh_Descriptor &descriptor, rendering_params& params) override;
	GLuint get_sm_rgb_texture();
	GLuint get_sm_texture();

	static float m_shadow_fov;
private:
	void init();

private:
	GLuint m_depth_fbo=0, m_depth_texture_id=0, m_rgb_texture=0;
	int light_w = 2048, light_h = 2048;
};
