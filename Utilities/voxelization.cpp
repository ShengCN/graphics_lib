#include "voxelization.h"

void voxelizater::voxelize(std::shared_ptr<mesh> m, int steps, std::shared_ptr<mesh>& out_mesh) {
	if(out_mesh == nullptr) {
		WARN("Input null pointer");
		return;
	}

	out_mesh->clear_vertices();
	AABB mesh_aabb = m->compute_world_aabb();
	vec3 diag = mesh_aabb.diagonal();
	vec3 unit_diag = diag / (float)steps;

	vec3 x = vec3(diag.x, 0.0f,0.0f), y = vec3(0.0f, diag.y, 0.0f), z = vec3(0.0f, 0.0f, diag.z);
	mat3 basis = mat3(x, y, z);

	for (int xi = 0; xi < steps; ++xi) {
		float x_fract = (float)xi / steps;
		for (int yi = 0; yi < steps; ++yi) {
			float y_fract = (float)yi / steps;
			for (int zi = 0; zi < steps; ++zi) {
				float z_fract = (float)zi / steps;
				vec3 cell_p0 = mesh_aabb.p0 + basis * vec3(x_fract, y_fract, z_fract);
				AABB cell_aabb(cell_p0, cell_p0 + unit_diag);
				out_mesh->add_vertices(cell_aabb.to_line_mesh());
			}
		}
		
	}
}
