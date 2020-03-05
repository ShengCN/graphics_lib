#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "mesh.h"
#include "graphics_lib/Utilities/Utils.h" 
#include "graphics_lib/Utilities/Logger.h"

int mesh::id = 0;


mesh::mesh(bool increase_id/*= true*/) {
	if (increase_id)
		cur_id = ++id;

	m_translation = m_scale = m_rotation = m_to_center = glm::identity<mat4>();
}

mesh::~mesh() {
}

std::vector<vec3> mesh::compute_world_space_coords()
{
	std::vector<vec3> world_coords = m_verts;
	for (auto& v : world_coords) {
		v = get_world_mat() * v;
	}
	return world_coords;
}

std::vector<vec3> mesh::compute_world_space_normals() {
	std::vector<vec3> world_normals = m_norms;
	auto normal_transform = glm::inverse(glm::transpose(get_world_mat()));

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
	return get_world_mat() * center;
}

void mesh::add_world_transate(vec3 v) {
	m_translation = glm::translate(v) * m_translation;
}

void mesh::add_scale(vec3 s) {
	mat4 scale_mat = glm::scale(s);
	m_scale = scale_mat * m_scale;
}

void mesh::add_rotate(pd::rad angle, vec3 axis) {
	mat4 rot_mat = glm::rotate(angle, axis);
	m_rotation = rot_mat * m_rotation;
}

void mesh::add_face(vec3 va, vec3 vb, vec3 vc, vec3 na, vec3 nb, vec3 nc) {
	m_verts.push_back(va);
	m_verts.push_back(vb);
	m_verts.push_back(vc);

	m_norms.push_back(na);
	m_norms.push_back(nb);
	m_norms.push_back(nc);

	m_colors.push_back(vec3(0.8f));
	m_colors.push_back(vec3(0.8f));
	m_colors.push_back(vec3(0.8f));
}

void mesh::add_face(vec3 va, vec3 vb, vec3 vc) {
	vec3 ba = vb - va, cb = vc - vb;
	vec3 normal = glm::normalize(glm::cross(ba, cb));
	add_face(va, vb, vc, normal, normal, normal);
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

void mesh::set_color(vec3 col) {
	if (m_verts.empty())
		return;

	if (m_colors.empty()) {
		m_colors.clear();
		m_colors.resize(m_verts.size(), col);
	}
	else {
		for (auto &c : m_colors) {
			c = col;
		}
	}

}

void mesh::normalize_position_orientation(vec3 scale/*=vec3(1.0f)*/, glm::quat rot_quant /*= glm::quat(0.0f,0.0f,0.0f,1.0f)*/) {
	if (m_verts.empty()) {
		WARN("vertices empty!");
		return;
	}

	// normalize, move to center and align
	vec3 center = compute_center();
	float diag_lenght = compute_aabb().diag_length();
	
	// add_world_transate(-center);
	set_to_center();
	add_scale(scale / diag_lenght);
	m_rotation = glm::toMat4(rot_quant) * m_rotation;
}

void mesh::set_matrix(const vec3 scale, const quat rot, const vec3 translate) {
	m_scale = glm::scale(scale);
	m_rotation = glm::toMat4(rot);
	m_translation = glm::translate(translate);
}

void mesh::reset_matrix(bool is_update_to_center) {
	m_translation = m_scale = m_rotation = glm::identity<mat4>();
	if(is_update_to_center) {
		m_to_center = glm::identity<mat4>();
	}
}

void mesh::clear_vertices() { 
	reset_matrix();
	m_verts.clear(); m_norms.clear(); m_colors.clear(); m_uvs.clear(); 
}

void mesh::recompute_normal() {
	m_norms.clear();
	size_t triangle_num = m_verts.size() / 3;
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

void mesh::set_to_center() {
	set_to_center(-compute_center());
}

void mesh::set_to_center(vec3 position) {
	m_to_center = glm::translate(position);
}
