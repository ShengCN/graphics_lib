#include "arcball.h"
#include "graphics_lib/Utilities/Utils.h"

void arcball::clicked(int x, int y) {
	m_has_clicked = true;
	m_last_x = x; m_last_y = y;
}

void arcball::released(int x, int y) {
	m_has_clicked = false;
}

bool arcball::mouse_move(int x, int y, int w, int h, vec3 &rotate_axis, pd::rad &angle){
	if (!m_has_clicked)
		return false;

	auto normalize_coord = [&](int x, int y, vec3 &out_coord) {
		float x_pos = std::clamp((float)x / w * 2.0f - 1.0f, 0.0f, 1.0f);
		float y_pos = -std::clamp((float)y / h * 2.0f - 1.0f, 0.0f, 1.0f) * (float)h/w;
		out_coord = vec3(x_pos, y_pos, std::sqrt(1.0 - x_pos * x_pos - y_pos * y_pos));
	};

	vec3 start_pos; normalize_coord(m_last_x, m_last_y, start_pos);
	vec3 end_pos; normalize_coord(x, y, end_pos);
	
	rotate_axis = glm::cross(start_pos, end_pos);
	if (glm::length(rotate_axis) < 1e-3) return false;
	rotate_axis = glm::normalize(rotate_axis);
	angle = std::clamp(std::acos(glm::dot(start_pos, end_pos)), deg2rad(-30.0f), deg2rad(30.0f));

	return true;
}
