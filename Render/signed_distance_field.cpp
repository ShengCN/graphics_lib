#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include "signed_distance_field.h"`

signed_distance_field::signed_distance_field(int w, int h, int z):m_w(w), m_h(h), m_z(z){
	field_buffer.resize(w * h * z);
}

void signed_distance_field::construct(std::shared_ptr<mesh> m) {
	m_world_pos = m->compute_world_aabb();

	for (int i = 0; i < m_w; ++i) {
		for (int j = 0; j < m_h; ++j) {
			for (int k = 0; k < m_z; ++k) {
				// compute closest point
			}
		}
	}
}

bool signed_distance_field::query(glm::vec3 p, float &ret) {
	vec3 relative = p - m_world_pos.p0;
	vec3 diag = m_world_pos.diagonal();
	vec3 uvw = relative / diag;

	// outside of the volume 
	if (uvw.x < 0 || uvw.x >1.0 || uvw.y < 0 || uvw.y >1.0 || uvw.z < 0 || uvw.z >1.0) {
		return false;
	}

	float x = uvw.x * (m_w - 1);
	float y = uvw.y * (m_h - 1);
	float z = uvw.z * (m_z - 1);
	// Tri-linear interpolation
	float p0, p1, p2, p3, p4, p5, p6, p7, p8;
	int centerx = (int)x, centery = (int)y, centerz = (int)z;
	p0 = at(centerx, centery, centerz);
	p1 = at(centerx + 1, centery, centerz);
	p2 = at(centerx, centery, centerz + 1);
	p3 = at(centerx + 1, centery, centerz + 1);

	p4 = at(centerx, centery+1, centerz);
	p5 = at(centerx + 1, centery+1, centerz);
	p6 = at(centerx, centery+1, centerz + 1);
	p7 = at(centerx + 1, centery+1, centerz + 1);

	// a-b
	// c-d
	auto bilinear = [&](float a, float b, float c, float d, glm::vec2 t) {
		glm::mat2 w;
		w[0] = glm::vec2(a, b);
		w[1] = glm::vec2(c, d);
		w = glm::transpose(w);
		return glm::dot(vec2(1.0 - t.y, t.y), w * vec2(1.0 - t.x, t.x));
	};
	float d1 = bilinear(p0, p1, p2, p3, glm::vec2(uvw.x, uvw.z));
	float d2 = bilinear(p4, p5, p6, p7, glm::vec2(uvw.x, uvw.z));
	ret = pd::lerp(d1, d2, uvw.y);
}

float signed_distance_field::at(int u, int v, int w) {
	u = pd::clamp(u, 0, m_w-1);
	v = pd::clamp(v, 0, m_h-1);
	w = pd::clamp(w, 0, m_z-1);

	return field_buffer[u * m_h * m_z + v * m_z + w];
}
