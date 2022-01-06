#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

#include "shader.h"
#include "asset_manager.h"

using std::ifstream;
using std::ios;

#define BUFFER_OFFSET(i) ((char*)NULL + (i))
shader::shader(const char* computeShaderFile) {
	m_cs = computeShaderFile;
	m_type = shader_type::compute_shader;

	reload_shader();
	init_textures();
}

shader::shader(const char* vertexShaderFile, const char* fragmentShaderFile) {
	m_vs = vertexShaderFile; m_fs = fragmentShaderFile;
	m_type = shader_type::template_shader;

	reload_shader();
	init_textures();
}

shader::shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile) {
	m_vs = vertexShaderFile; m_gs = geometryShader; m_fs = fragmentShaderFile;
	m_type = shader_type::geometry_shader;

	reload_shader();
	init_textures();
}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile) {
	ifstream ifs(shaderFile, ios::in | ios::binary | ios::ate);
	if (ifs.is_open()) {
		unsigned int filesize = static_cast<unsigned int>(ifs.tellg());
		ifs.seekg(0, ios::beg);
		char* bytes = new char[filesize + 1];
		memset(bytes, 0, filesize + 1);
		ifs.read(bytes, filesize);
		ifs.close();
		return bytes;
	}
	return NULL;
}

void printShaderCompileError(GLuint shader) {
	GLint  logSize;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
	char* logMsg = new char[logSize];
	glGetShaderInfoLog(shader, logSize, NULL, logMsg);
	std::cerr << logMsg << std::endl;
	delete[] logMsg;
}

void printProgramLinkError(GLuint program) {
	GLint  logSize;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
	char* logMsg = new char[logSize];
	glGetProgramInfoLog(program, logSize, NULL, logMsg);
	std::cerr << logMsg << std::endl;
	delete[] logMsg;
}

bool shader::reload_shader() {
	if(m_program != -1)
		glDeleteShader(m_program);

	switch (m_type) {
	case shader_type::template_shader:
		m_program = init_template_shader();
		break;

	case shader_type::compute_shader:
		m_program = init_compute_shader();
		break;
	
	case shader_type::geometry_shader:
		m_program = init_geometry_shader();
		break;
	
	default:
		return false;
		break;
	}
	return true;
}

void shader::draw_mesh(const Mesh_Descriptor &descriptor, rendering_params& params){
	if (descriptor.m == nullptr) {
		WARN("Shader input mesh is nullptr");
		return;
	}

	for (int i = 0; i < descriptor.texs.size(); ++i) {
		if (descriptor.texs[i] == nullptr) {
			WARN("{} texture is nullptr", i);
			continue;
		}
		
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_texids[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, descriptor.texs[i]->width(), descriptor.texs[i]->height(), 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

    auto m = descriptor.m;
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
	if (uniform_loc != -1 && !params.lights.empty()) {
		//#todo_multiple_lights
		glUniform3f(uniform_loc, params.lights[0].x, params.lights[0].y, params.lights[0].z);
	}

	uniform_loc = glGetUniformLocation(m_program, "light_pv");
	if (uniform_loc != -1 && params.light_camera) {
		glm::mat4 pv = params.light_camera->GetP() * params.light_camera->GetV();
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(pv));
	}

	if (params.sm_texture != -1) {
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, params.sm_texture);
		auto uniform_loc = glGetUniformLocation(m_program, "shadow_map");
		if(uniform_loc != -1) {
			glUniform1i(uniform_loc, 0);
		}

		uniform_loc = glGetUniformLocation(m_program, "draw_shadow");
		if(uniform_loc != -1) {
			glUniform1i(uniform_loc, 1);
		}

	} else {
		auto uniform_loc = glGetUniformLocation(m_program, "draw_shadow");
		if(uniform_loc != -1) {
			glUniform1i(uniform_loc, 0);
		}
    }

	int ogl_draw_type = 0;
	if (params.dtype == draw_type::triangle) {
		ogl_draw_type = GL_TRIANGLES;
	} 

	if (params.dtype == draw_type::line_segments) {
		ogl_draw_type = GL_LINES;
	}

    if (params.dtype == draw_type::points) {
        ogl_draw_type = GL_POINTS;
    }

	glBindVertexArray(vao);
	glDrawArrays(ogl_draw_type, 0, (GLsizei)m->m_verts.size());
	glBindVertexArray(0);
}


GLuint shader::init_compute_shader() {
	bool error = false;
	struct Shader
	{
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[1] =
	{
	   { m_cs.c_str(), GL_COMPUTE_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for (int i = 0; i < 1; ++i) {
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);
		if (shaders[i].source == NULL) {
			std::cerr << "Failed to read " << s.filename << std::endl;
			error = true;
		}

		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader);

		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			std::cerr << s.filename << " failed to compile:" << std::endl;
			printShaderCompileError(shader);
			error = true;
		}

		delete[] s.source;

		glAttachShader(program, shader);
	}

	/* link  and error check */
	glLinkProgram(program);

	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "Shader program failed to link" << std::endl;
		printProgramLinkError(program);

		error = true;
	}

	return program;
}

GLuint shader::init_template_shader() {
	bool error = false;
	struct Shader
	{
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[2] =
	{
	   { m_vs.c_str(), GL_VERTEX_SHADER, NULL },
	   { m_fs.c_str(), GL_FRAGMENT_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for (int i = 0; i < 2; ++i) {
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);
		if (shaders[i].source == NULL) {
			std::cerr << "Failed to read " << s.filename << std::endl;
			error = true;
		}

		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader);

		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			std::cerr << s.filename << " failed to compile:" << std::endl;
			printShaderCompileError(shader);
			error = true;
		}

		delete[] s.source;

		glAttachShader(program, shader);
	}

	/* link  and error check */
	glLinkProgram(program);

	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "Shader program failed to link" << std::endl;
		printProgramLinkError(program);

		error = true;
	}

	return program; }

GLuint shader::init_geometry_shader() {
	bool error = false;
	struct Shader
	{
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[3] =
	{
	   { m_vs.c_str(), GL_VERTEX_SHADER, NULL },
	   { m_gs.c_str(), GL_GEOMETRY_SHADER, NULL },
	   { m_fs.c_str(), GL_FRAGMENT_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for (int i = 0; i < 3; ++i) {
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);
		if (shaders[i].source == NULL) {
			std::cerr << "Failed to read " << s.filename << std::endl;
			error = true;
		}

		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader);

		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			std::cerr << s.filename << " failed to compile:" << std::endl;
			printShaderCompileError(shader);
			error = true;
		}

		delete[] s.source;

		glAttachShader(program, shader);
	}

	/* link  and error check */
	glLinkProgram(program);

	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "Shader program failed to link" << std::endl;
		printProgramLinkError(program);

		error = true;
	}

	return program;
}

void shader::init_textures() {
    // https://learnopengl.com/Getting-started/Textures
    m_texids.resize(16);
	glGenTextures(m_texids.size(), m_texids.data());
	for(size_t i = 0; i < m_texids.size(); ++i) {
        FAIL(m_texids[i] == -1, "Texture({}:{}) Initialization failed", i, m_texids[i]);
	}
}

quad_shader::quad_shader(const char* computeShaderFile):shader(computeShaderFile) {
	init_vao();
}

quad_shader::quad_shader(const char* vertexShaderFile, const char* fragmentShaderFile):shader(vertexShaderFile, fragmentShaderFile) {
	init_vao();
}

quad_shader::quad_shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile):shader(vertexShaderFile, geometryShader, fragmentShaderFile) {	
	init_vao();
}

void quad_shader::init_vao() {
	const float quad_verts[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };

	glGenVertexArrays(1, &m_quad_vao);
	glBindVertexArray(m_quad_vao);

	glGenBuffers(1, &m_quad_vbo); 
	glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), &quad_verts[0], GL_STATIC_DRAW);

	const GLint pos_loc = 0;
	glEnableVertexAttribArray(pos_loc); 
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);
	glBindVertexArray(0); //unbind the vao
}

void quad_shader::draw_mesh(const Mesh_Descriptor &descriptor, rendering_params& params) {
	glUseProgram(m_program);

	/* Bind Texture If Have */
	for (int i = 0; i < descriptor.texs.size(); ++i) {
		if (descriptor.texs[i] == nullptr) {
			WARN("{} texture is nullptr", i);
			continue;
		}
		
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_texids[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, descriptor.texs[i]->width(), descriptor.texs[i]->height(), 0, GL_RGBA, GL_FLOAT, descriptor.texs[i]->data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindVertexArray(m_quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

