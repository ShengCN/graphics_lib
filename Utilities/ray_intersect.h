#pragma once
#include "graphics_lib/Render/ppc.h"

// ray_plane(r, m_xyz_selected_point, plane_normal, intersect_point);
bool ray_plane(ray r, vec3 p, vec3 n, vec3 &intersect_point);
bool ray_square(ray r, vec3 p, vec3 n, vec3 pp, vec3 &intersect_point);
