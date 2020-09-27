#include "ray_intersect.h"
#include "graphics_lib/Utilities/Utils.h"
#include "graphics_lib/Utilities/Logger.h"

bool ray_plane(ray r, vec3 p, vec3 n, vec3 &intersect_point) {
	float t = glm::dot(p - r.ro, n) / glm::dot(r.rd, n);
	intersect_point = r.ro + r.rd * t;
	return t > 0.0f;
}

bool ray_square(ray r, vec3 p, vec3 n, vec3 pp, vec3 &intersect_point) {
	//if (!ray_plane(r, p, n, intersect_point))
	//	return false;

	ray_plane(r, p, n, intersect_point);
	INFO(pd::to_string(p));
	INFO(pd::to_string(pp));
	INFO(pd::to_string(intersect_point));

	float eps = 1e-3;
	return (intersect_point.x > p.x - eps && intersect_point.y > p.y - eps && intersect_point.z > p.z - eps
		&& intersect_point.x < pp.x + eps && intersect_point.y < pp.y + eps && intersect_point.z < pp.z + eps);
}
