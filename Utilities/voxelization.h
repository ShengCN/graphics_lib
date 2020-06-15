#pragma once
#include <memory>
#include "graphics_lib/Render/mesh.h"

class voxelizater {
public:
	voxelizater() = default;
	static void voxelize(std::shared_ptr<mesh> m,
				  int steps,
				  std::shared_ptr<mesh>& out_mesh);

private:

};