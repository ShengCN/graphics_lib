#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <limits>
#include "signed_distance_field.h"`
#include "graphics_lib/Utilities/cuda_helper.h"

__host__ __device__
float triangle_p_distance (vec3 p0, vec3 p1, vec3 p2, vec3 p) {
	// project p into the plane
	vec3 n = glm::normalize(glm::cross(p1 - p0, p2 - p0));
	float proj_length = glm::dot((p - p0), n);
	vec3 pp = p - proj_length * n;

	mat3 bary_mat(p0, p1, p2);
	if (glm::determinant(bary_mat) > 0) {
		vec3 bary_coord = glm::inverse(bary_mat) * pp;
		if (bary_coord.x >= 0 && bary_coord.x <= 1.0 &&
			bary_coord.y >= 0 && bary_coord.y <= 1.0 &&
			bary_coord.z >= 0.0 && bary_coord.z <= 1.0) {
			return std::abs(proj_length);
		}
	}
	
	float ret = FLT_MAX;
	float p0_dis = glm::distance(p0, p);
	float p1_dis = glm::distance(p1, p);
	float p2_dis = glm::distance(p2, p);
	if (p0_dis < ret) ret = p0_dis;
	if (p1_dis < ret) ret = p1_dis;
	if (p2_dis < ret) ret = p2_dis;

	return ret;
};

__host__ __device__
float signed_distance(vec3 *verts, int N, vec3 p) {
	float mini_dis = FLT_MAX;
	float winding_sum = 0.0f;

	for (int ti = 0; ti < N / 3; ++ti) {
		//printf("%d %d \n", ti, N / 3);
		vec3 p0 = verts[3 * ti + 0];
		vec3 p1 = verts[3 * ti + 1];
		vec3 p2 = verts[3 * ti + 2];
		float cur_dis = triangle_p_distance(p0, p1, p2, p);
		if (cur_dis < mini_dis) {
			mini_dis = cur_dis;
		}
	}
	// todo, winding sum to decide inside or outside
	return mini_dis;
}

__global__
void cuda_construct(glm::vec3 *verts, int N, float *sdf, int sdf_N, int w, int h, int z, AABB field_range) {
	vec3 diag = field_range.diagonal();
	vec3 p0 = field_range.p0;
	
	int ind = blockDim.x * blockIdx.x + threadIdx.x;
	int stride = blockDim.x * gridDim.x;
	
	for (int sdfi = ind; sdfi < sdf_N; sdfi += stride) {
		int k = sdfi / (w * h);
		int j = (sdfi - (w*h*k)) / w;
		int i = sdfi - (w * h * k) - j * w;
		vec3 p = p0 + diag * vec3((float)i / w, (float)j / h, (float)k / z);
		sdf[sdfi] = signed_distance(verts, N, p);
		//sdf[sdfi] = (float)i + (float)j + (float)k;
	}
}

signed_distance_field::signed_distance_field(int w, int h, int z):m_w(w), m_h(h), m_z(z){
	m_field_buffer.resize(w * h * z);
}

void signed_distance_field::construct(std::shared_ptr<mesh> m, AABB field_range) {
	m_field_range = field_range;
	
	cuda_timer clc;
	clc.tic();
	// memory
	container_cuda<vec3> d_verts(m->m_verts);
	container_cuda<float> d_sdf(m_field_buffer);

	int grid = 512, block = (d_sdf.get_n() + grid - 1) / grid;
	cuda_construct<<<grid, block >>>(d_verts.get_d(), d_verts.get_n(), d_sdf.get_d(), d_sdf.get_n(), m_w, m_h, m_z, m_field_range);
	GC(cudaDeviceSynchronize());

	// copy back
	d_sdf.mem_copy_back();
	clc.toc();
	INFO("constructing spent: " + std::to_string(clc.get_time()) + "ms");

	// CPU version
	//vec3 diag = m_field_range.diagonal();
	//for (int i = 0; i < m_w; ++i) {
	//	for (int j = 0; j < m_h; ++j) {
	//		for (int k = 0; k < m_z; ++k) {
	//			// compute closest point
	//			vec3 p = (vec3((float)i / m_w, (float)j / m_h, (float)k / m_z) + vec3(0.5f)) * diag + m_field_range.p0;
	//			float dis = close_distance(m, p);
	//			at(i, j, k) = dis;
	//		}
	//	}
	//}
}

void signed_distance_field::to_visualize_mesh(std::shared_ptr<mesh> m) {
	m->clear_vertices();
	float max_value = 0.0f;
	vec3 diag = m_field_range.diagonal();
	for (int i = 0; i < m_w; ++i) {
		for (int j = 0; j < m_h; ++j) {
			for (int k = 0; k < m_z; ++k) {
				float value = at(i, j, k);
				vec3 p = (vec3((float)i / m_w, (float)j / m_h, (float)k / m_z)) * diag;
				vec3 p2 = (vec3((float)(i+1) / m_w, (float)(j+1) / m_h, (float)(k+1) / m_z)) * diag;
				AABB cur_aabb(p, p2);
				auto cur_verts = cur_aabb.to_tri_mesh();
				m->m_verts.insert(m->m_verts.end(), cur_verts.begin(), cur_verts.end());
				for (int i = 0; i < cur_verts.size(); ++i) {
					m->m_colors.push_back(glm::vec3(value));
				}
				max_value = std::max(max_value, value);
			}
		}
	}

	for (auto &c : m->m_colors) {
		c = c / max_value;
	}
}

bool signed_distance_field::query(glm::vec3 p, float &ret) {
	vec3 relative = p - m_field_range.p0;
	vec3 diag = m_field_range.diagonal();
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

float& signed_distance_field::at(int u, int v, int w) {
	u = pd::clamp(u, 0, m_w-1);
	v = pd::clamp(v, 0, m_h-1);
	w = pd::clamp(w, 0, m_z-1);

	return m_field_buffer[u * m_h * m_z + v * m_z + w];
}

float signed_distance_field::close_distance(std::shared_ptr<mesh> m, glm::vec3 p) {
	float ret = std::numeric_limits<float>::max();
	if (!m) {
		return ret;
	}

	auto world_verts = m->compute_world_space_coords();
	for (int ti = 0; ti < world_verts.size()/3; ++ti) {
		vec3 p0 = world_verts[3 * ti + 0];
		vec3 p1 = world_verts[3 * ti + 1];
		vec3 p2 = world_verts[3 * ti + 2];
		
		float dis = triangle_p_distance(p0, p1, p2, p);
		ret = std::min(ret, dis);
	}
	return ret;
}
