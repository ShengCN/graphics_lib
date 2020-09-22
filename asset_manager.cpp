#include "asset_manager.h"

asset_manager::asset_manager() {
	plane = std::make_shared<mesh>();
	plane->add_face(vec3(-1.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, -1.0f));
	plane->add_face(vec3(1.0f, 0.0f, -1.0f), vec3(-1.0f, 0.0f, -1.0f), vec3(-1.0f, 0.0f, 1.0f));
	plane->add_scale(vec3(3.0f));
}