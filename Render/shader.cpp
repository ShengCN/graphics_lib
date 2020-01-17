#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "graphics_lib/asset_manager.h"

using std::ifstream;
using std::ios;

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


shader::shader(const char* computeShaderFile) {
	m_cs = computeShaderFile;
	bool error = false;
	struct Shader
	{
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[1] =
	{
	   { computeShaderFile, GL_COMPUTE_SHADER, NULL }
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

	m_program = program;
}

shader::shader(const char* vertexShaderFile, const char* fragmentShaderFile) {
	m_vs = vertexShaderFile; m_fs = fragmentShaderFile;
	bool error = false;
	struct Shader
	{
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[2] =
	{
	   { vertexShaderFile, GL_VERTEX_SHADER, NULL },
	   { fragmentShaderFile, GL_FRAGMENT_SHADER, NULL }
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
	m_program = program;
}

shader::shader(const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile) {
	m_vs = vertexShaderFile; m_gs = geometryShader; m_fs = fragmentShaderFile;
	bool error = false;
	struct Shader
	{
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[3] =
	{
	   { vertexShaderFile, GL_VERTEX_SHADER, NULL },
	   { geometryShader, GL_GEOMETRY_SHADER, NULL },
	   { fragmentShaderFile, GL_FRAGMENT_SHADER, NULL }
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

	m_program = program;
}

bool shader::reload_shader() {
	//todo
	return false;
}

void shader::draw_mesh(std::shared_ptr<mesh> m) {
	GLuint vert_attr = glGetAttribLocation(m_program, "pos_attr");
	static GLuint vao = -1, vbo = -1;
	if (vao == -1) glGenVertexArrays(1, &vao);
	if (vbo == -1) glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//------- Buffer & init vertex attributes --------//
	std::vector<glm::vec3> &verts = m->m_verts;
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(vert_attr, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vert_attr);
	
	glUseProgram(m_program);

	auto &manager = asset_manager::instance();
	mat4 p = manager.cur_camera->GetP();
	mat4 v = manager.cur_camera->GetV();
	mat4 pvm = p * v * m->m_world;
	auto uniform_loc = glGetUniformLocation(m_program, "PVM");
	glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(pvm));

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
	glBindVertexArray(0);
}
