#pragma once
#include "graphics_lib/Render/ppc.h"

class arcball {
public:
	void clicked(int x, int y);
	void released(int x, int y);
	bool mouse_move(int x, int y, int w, int h,vec3 &rotate_axis, pd::rad &angle);

private:
	bool m_has_clicked=false;
	int m_last_x, m_last_y;
};

