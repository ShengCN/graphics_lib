#include "asset_manager.h"

asset_manager::asset_manager() {
	render_scene = std::make_shared<scene>();
	visualize_scene = std::make_shared<scene>();

	/* Lightings */
	lights = { glm::vec3(0.75f) * 1.0f };

    /* Serielizer */
    m_serializers["render_scene"] = std::dynamic_pointer_cast<ISerialize>(render_scene);
    m_serializers["camera"] = std::dynamic_pointer_cast<ISerialize>(cur_camera);
}


int asset_manager::to_json(const std::string json_fname) {
    /* Serielize Orders 
     * Camera
     * Lights
     * Render scene 
     * */
    std::string json_str;
    json_str += m_serializers.at("camera")->to_json();
    json_str += m_serializers.at("render_scene")->to_json();
}

int asset_manager::from_json(const std::string json_str) {
}
