#include "asset_manager.h"


std::unordered_map<std::string, std::shared_ptr<shader>> asset_manager::shaders;
asset_manager::asset_manager() {
    init();
}

int asset_manager::check_assets() {
    /* Make Sure Every Pointer Exists */
    if(!render_scene) {
        FAIL(true, "Assets Check Failed. [render_scene]");
        return -1;
    }

    if(!visualize_scene) {
        FAIL(true, "Assets Check Failed. [visualize_scene]");
        return -1;
    }

    auto meshes = render_scene->get_meshes();
    for (auto mpair:meshes) {
        if (!mpair.second) {
            FAIL(true, "Assets Check Failed. [mesh: {}]", mpair.first);
            return -1;
        }
    }
    return 1;
}

void asset_manager::init() {
	render_scene = std::make_shared<scene>();
	visualize_scene = std::make_shared<scene>();
    cur_camera = std::make_shared<ppc>(512, 512, 90.0f);

	/* Lightings */
	lights = { glm::vec3(0.75f) * 5.0f };
    light_camera = std::make_shared<ppc>(1024, 1024, 30.0f);
}
