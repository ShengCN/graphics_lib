#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

namespace purdue {
	typedef float deg;
	typedef float rad;
	constexpr float pi = 3.14159265f;

	// only for vector p
	inline glm::vec3 operator*(glm::mat4 m, glm::vec3 p) {
		return glm::vec3(m * glm::vec4(p, 1.0f));
	}
};

namespace pd = purdue;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;
using pd::operator *;

//------- Singleton Class for Shared Variables --------//
// src: https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class shared_variables {
//------- variables --------//
public:
	const std::string shader_folder = "Shaders/";
	const std::string template_vs = shader_folder + "template_vs.glsl";
	const std::string template_fs = shader_folder + "template_fs.glsl";
	vec3 default_stl_color = glm::vec3(0.3f);

public:
	static shared_variables& instance() {
		static shared_variables instance;
		return instance;
	}

private:
	shared_variables() = default;

//------- constraints on copy and move --------//
public:
	shared_variables(shared_variables const&) = delete;
	void operator=(shared_variables const&) = delete;
};


#ifndef GGV	// graphics lib global variables
#define GGV shared_variables::instance()
#endif // !GGV

