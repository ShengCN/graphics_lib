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
		return glm::vec3(m * glm::vec4(p, 0.0f));
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

