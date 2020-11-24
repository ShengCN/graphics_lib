#pragma once
#include <vector>
#include "graphics_lib/Render/mesh.h"

class signed_distance_field {
public:
	signed_distance_field(int w=32, int h=32, int z=32);
	
	void construct(std::shared_ptr<mesh> m, AABB field_range);
	void to_visualize_mesh(std::shared_ptr<mesh> m);

	// query
	bool query(glm::vec3 p, float &ret);
	float& at(int u, int v, int w);

private:
	float close_distance(std::shared_ptr<mesh> m, glm::vec3 p);

private:
	AABB m_field_range;
	int m_w, m_h, m_z;
	std::vector<float> m_field_buffer;
};
