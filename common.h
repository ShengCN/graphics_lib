#pragma once

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <cmath>
#include <sstream>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include "glm/ext/matrix_float3x3.hpp"
#include "Utilities/Utils.h"
#include "Utilities/Logger.h"
#include "Utilities/Image.h"

namespace purdue {
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

};

namespace pd = purdue;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;

typedef int mesh_id;
const std::string default_shader_name = "template";
const std::string quad_shader_name = "quad";
const std::string plane_shader_name = "plane";
const std::string mask_shader_name = "mask";
const std::string sm_shader_name = "shadow_map";
const std::string shadow_caster_name = "shadow_cast";
