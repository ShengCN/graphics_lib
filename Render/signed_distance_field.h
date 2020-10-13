#pragma once
#include <vector>
#include "graphics_lib/Render/mesh.h"

class signed_distance_field {
public:
	signed_distance_field(int w, int h, int z);
	
	void construct(std::shared_ptr<mesh> m);

	// query
	bool query(glm::vec3 p, float &ret);
	float at(int u, int v, int w);
private:
	AABB m_world_pos;
	int m_w, m_h, m_z;
	std::vector<float> field_buffer;
};
