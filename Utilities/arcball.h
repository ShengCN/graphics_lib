#pragma once
#include "graphics_lib/Render/ppc.h"

class arcball {
public:
	void clicked(int x, int y);
	void released(int x, int y);
	void mouse_move(int x, int y, std::shared_ptr<ppc> camera);

private:
	bool m_has_clicked=false;
	int m_last_x, m_last_y;
};

