#pragma once
#include "graphics_lib/Render/ppc.h"

class arcball {
public:
	void left_clicked(int x, int y);
	void left_released(int x, int y);
	bool left_mouse_move(int x, int y, int w, int h,vec3 &rotate_axis, pd::rad &angle);
	bool is_left_clicked() {
		return m_left_has_clicked;
	}
	
	void right_clicked(int x, int y);
	void right_released(int x, int y);
	bool right_mouse_move(int x, int y, int w, int h, float &camera_fov_diff);
	bool is_right_clicked() {
		return m_right_has_clicked;
	}

private:
	bool m_left_has_clicked=false;
	bool m_right_has_clicked = false;

	int m_last_x, m_last_y;
};

