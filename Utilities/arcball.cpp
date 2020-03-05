#include "arcball.h"
#include "graphics_lib/Utilities/Utils.h"

void arcball::left_clicked(int x, int y) {
	m_left_has_clicked = true;
	m_last_x = x; m_last_y = y;
}

void arcball::left_released(int x, int y) {
	m_left_has_clicked = false;
}

bool arcball::left_mouse_move(int x, int y, int w, int h, vec3 &rotate_axis, pd::rad &angle){
	if (!m_left_has_clicked)
		return false;

	auto normalize_coord = [&](int x, int y, vec3 &out_coord) {
		float x_pos = std::clamp((float)x / w * 2.0f - 1.0f, 0.0f, 1.0f);
		float y_pos = -std::clamp((float)y / h * 2.0f - 1.0f, 0.0f, 1.0f) * (float)h/w;
		out_coord = vec3(x_pos, y_pos, std::sqrt(1.0 - x_pos * x_pos - y_pos * y_pos));
	};

	vec3 start_pos; normalize_coord(m_last_x, m_last_y, start_pos); 
	vec3 end_pos; normalize_coord(x, y, end_pos); 
	
	rotate_axis = glm::cross(start_pos, end_pos);

	if (glm::length(rotate_axis) < 1e-3)  {
		angle = 0.0f;
		return false;
	}
	rotate_axis = glm::normalize(rotate_axis);
	// INFO("rot axis: " + to_string(rotate_axis));

	// angle = std::clamp(std::acos(glm::dot(start_pos, end_pos)), deg2rad(-30.0f), deg2rad(30.0f));
	angle = std::acos(std::clamp(glm::dot(start_pos, end_pos),0.0f, 1.0f));

	return true;
}

void arcball::right_clicked(int x, int y) {
	m_right_has_clicked = true;
	m_last_x = x; m_last_y = y;
}

void arcball::right_released(int x, int y) {
	m_right_has_clicked = false;
}

bool arcball::right_mouse_move(int x, int y, int w, int h, float &camera_fov_diff) {
	if (!m_right_has_clicked)
		return false;

	float x_diff = x - m_last_x;
	camera_fov_diff = x_diff / w;
	return true;
}
