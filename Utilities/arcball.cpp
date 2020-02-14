#include "arcball.h"
#include "graphics_lib/Utilities/Utils.h"

void arcball::clicked(int x, int y) {
	m_has_clicked = true;
	m_last_x = x; m_last_y = y;
}

void arcball::released(int x, int y) {
	m_has_clicked = false;
}

void arcball::mouse_move(int x, int y, std::shared_ptr<ppc> camera) {
	if (!m_has_clicked)
		return;

	int w = camera->_width, h = camera->_height;
	auto normalize_coord = [&](int x, int y, vec3 &out_coord) {
		float x_pos = std::clamp((float)x / w * 2.0f - 1.0f, 0.0f, 1.0f);
		float y_pos = std::clamp((float)y / h * 2.0f - 1.0f, 0.0f, 1.0f);
		out_coord = vec3(x_pos, y_pos, std::sqrt(1.0 - x_pos * x_pos - y_pos * y_pos));
	};

	vec3 start_pos; normalize_coord(m_last_x, m_last_y, start_pos);
	vec3 end_pos; normalize_coord(x, y, end_pos);
	
	vec3 rotate_axis = glm::cross(start_pos, end_pos);
	if (glm::length(rotate_axis) < 1e-3) return;

	pd::rad rotate_angle = std::acos(glm::dot(start_pos, end_pos));
	INFO(rad2deg(rotate_angle));
	camera->Rotate_Axis(vec3(0.0f), rotate_axis, rad2deg(rotate_angle));
	camera->PositionAndOrient(camera->_position, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
}
