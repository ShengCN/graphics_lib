#include <fstream>
#include <iostream>
#include <glm/matrix.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "mesh.h"
#include "graphics_lib/Utilities/Utils.h" 
#include "graphics_lib/Utilities/Logger.h"

int mesh::id = 0;

mesh::mesh(const std::string vs, const std::string fs): m_vs(vs), m_fs(fs){
	cur_id = ++id;
}

mesh::~mesh() {
}

std::vector<vec3> mesh::compute_world_space_coords()
{
	std::vector<vec3> world_coords = m_verts;
	for (auto& v : world_coords) {
		v = m_world * v;
	}
	return world_coords;
}

std::vector<vec3> mesh::compute_world_space_normals() {
	std::vector<vec3> world_normals = m_norms;
	auto normal_transform = glm::inverse(glm::transpose(m_world));

	for (auto& n : world_normals) {
		n = glm::vec3(normal_transform * vec4(n,0.0f));
	}
	return world_normals;
}

vec3 mesh::compute_center() {
	vec3 center(0.0f, 0.0f, 0.0f);
	float weights = 1.0f/(float)m_verts.size();
	for (auto& v : m_verts) {
		center += v * weights;
	}
	return center;
}

vec3 mesh::compute_world_center() {
	vec3 center = compute_center();
	return m_world * center;
}

void mesh::add_world_transate(vec3 v) {
	m_world = glm::translate(v) * m_world;
}

void mesh::add_scale(vec3 s) {
	vec3 center = compute_center();
	mat4 scale_mat = glm::scale(s);
	m_world = m_world * scale_mat;
}

void mesh::add_rotate(pd::deg angle, vec3 axis) {
	mat4 rot_mat = glm::rotate(deg2rad(angle), axis);
	m_world = rot_mat * m_world;
}

void mesh::add_face(vec3 va, vec3 vb, vec3 vc) {
	m_verts.push_back(va);
	m_verts.push_back(vb);
	m_verts.push_back(vc);

	vec3 ba = vb - va, cb = vc - vb;
	vec3 normal = glm::normalize(glm::cross(ba, cb));
	m_norms.push_back(normal);
	m_norms.push_back(normal);
	m_norms.push_back(normal);

	m_colors.push_back(GGV.default_stl_color);
	m_colors.push_back(GGV.default_stl_color);
	m_colors.push_back(GGV.default_stl_color);
}

void mesh::add_vertex(vec3 v, vec3 n, vec3 c) {
	m_verts.push_back(v);
	m_norms.push_back(n);
	m_colors.push_back(c);
}

void mesh::add_vertex(vec3 v, vec3 n, vec3 c, vec2 uv) {
	add_vertex(v, n, c);
	m_uvs.push_back(uv);
}

AABB mesh::compute_aabb() const {
	assert(m_verts.size() > 0);

	AABB aabb(m_verts[0]);
	for (auto &v : m_verts) {
		aabb.add_point(v);
	}
	return aabb;
}

AABB mesh::compute_world_aabb() {
	auto world_space_points = compute_world_space_coords();
	if (world_space_points.empty())
		return AABB(vec3(0.0f));

	AABB aabb(world_space_points[0]);
	for (auto& v : world_space_points) {
		aabb.add_point(v);
	}

	return aabb;
}

void mesh::set_color(vec3 col)
{
	if (m_verts.empty())
		return;

	if(m_colors.empty()) {
		m_colors.clear();
		m_colors.resize(m_verts.size(), col);
	} else {
		for(auto &c:m_colors) {
			c = col;
		}
	}

}

void mesh::set_color(unsigned triangle_id, vec3 col) {
	// ensure there exist colors
	if (m_colors.empty()) {
		set_color(glm::vec3(0.3f));
	}

	unsigned int vi = 3 * triangle_id + 0, vj = 3 * triangle_id + 1, vk = 3 * triangle_id + 2;
	m_colors[vi] = m_colors[vj] = m_colors[vk] = col;
}

bool mesh::reload_shaders() {
	clean_up();

	//this may change later, cautious
	return init_shaders();	
}

void mesh::normalize_position_orientation(vec3 scale/*=vec3(1.0f)*/, glm::quat rot_quant /*= glm::quat(0.0f,0.0f,0.0f,1.0f)*/) {
	// normalize, move to center and align
	vec3 center = compute_center();
	float diagnoal = compute_world_aabb().diag_length();
	mat4 norm_transform = glm::toMat4(rot_quant) * 
		glm::rotate(deg2rad(90.0f), vec3(1.0f, 0.0f, 0.0f)) *
		glm::scale(scale/ diagnoal) *
		glm::translate(-center);
	m_world = norm_transform;
}

void mesh::get_demose_matrix(vec3& scale, quat& rot, vec3& translate) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(m_world, // input
				   scale,
				   rot,
				   translate,
				   skew,
				   perspective);
}

void mesh::set_matrix(const vec3 scale, const quat rot, const vec3 translate) {
	m_world = glm::translate(translate) *
		glm::scale(scale) * glm::toMat4(rot);
}

void mesh::recompute_normal() {
	m_norms.clear();
	int triangle_num = m_verts.size() / 3;
	for(int ti = 0; ti < triangle_num; ++ti) {
		vec3 a = m_verts[3 * ti + 0];
		vec3 b = m_verts[3 * ti + 1];
		vec3 c = m_verts[3 * ti + 2];

		vec3 n = glm::normalize(glm::cross(b - a, c - b));
		m_norms.push_back(n);
		m_norms.push_back(n);
		m_norms.push_back(n);
	}
}

void mesh::remove_duplicate_vertices() {
	std::vector<bool> vert_visited(m_verts.size(), false);
	for (int vi = 0; vi < m_verts.size(); ++vi) {
		if (vert_visited[vi])	continue;
		
		for (int ovi = vi + 1; ovi < m_verts.size(); ++ovi) {
			if(same_point(m_verts[vi], m_verts[ovi])) {
				vert_visited[ovi] = true;
				m_verts[ovi] = m_verts[vi];
			}
		}
	}
}

// merge with world space coordinates
void mesh::merge_mesh(std::shared_ptr<mesh> b) {
	if(!b) {
		LOG_FAIL("Merge mesh");
		assert(false);
	}

	auto b_world_coord = b->compute_world_space_coords();
	auto b_world_normal = b->compute_world_space_normals();

	std::copy(b_world_coord.begin(), b_world_coord.end(), std::back_inserter(m_verts));
	std::copy(b->m_colors.begin(), b->m_colors.end(), std::back_inserter(m_colors));
	std::copy(b_world_normal.begin(), b_world_normal.end(), std::back_inserter(m_norms));
	std::copy(b->m_uvs.begin(), b->m_uvs.end(), std::back_inserter(m_uvs));
}