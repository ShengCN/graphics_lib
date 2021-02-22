#pragma once

#include <Dep/glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <random>
#include "Utilities/Logger.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;

namespace purdue {
	typedef float deg;
	typedef float rad;
	constexpr float pi = 3.14159265f;

	inline std::string to_string(glm::vec3 v) {
		std::stringstream out;
		out << v.x << "," << v.y << "," << v.z;
		return out.str();
	}

	inline std::string to_string(glm::vec4 v) {
		std::stringstream out;
		out << v.x << "," << v.y << "," << v.z << "," << v.w;
		return out.str();
	}

	inline std::string to_string(glm::mat4 m) {
		std::stringstream out;
		out << to_string(m[0]) << std::endl;
		out << to_string(m[1]) << std::endl;
		out << to_string(m[2]) << std::endl;
		out << to_string(m[3]);
		return out.str();
	}

	/*!
	 *
	 * \brief Math lab
	 *
	 * \author YichenSheng
	 * \date August 2019
	 */
	inline rad deg2rad(deg d) {
		return d / 180.0f * pi;
	}

	inline deg rad2deg(rad r) {
		return r / pi * 180.0f;
	}

	inline float deg2quat(deg d) {
		return std::cos(deg2rad(d / 2));
	}

	template<typename T>
	inline T clamp(T v, T m, T mm) {
		v = std::max(v, m);
		v = std::min(v, mm);
		return v;
	}
	
	inline float random_float(float fmin = 0.0f, float fmax = 1.0f) {
		// engine
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_real_distribution<float> u(fmin, fmax);
		return u(engine);
	}

	inline int random_int(int mi = 0, int ma = 10) {
		// engine
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_int_distribution<> dis(mi, ma);

		return dis(engine);
	}

	float normal_random(float mean, float sig);

	template<typename T>
	T lerp(T a, T b, float fract) {
		return (1.0f - fract) * a + fract * b;
	}

	inline bool same_point(const glm::vec3 &a, const glm::vec3 &b) {
		return glm::distance(a, b) < 1e-3;
	}
};

namespace pd = purdue;