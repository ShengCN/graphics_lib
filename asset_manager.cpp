#include "asset_manager.h"

asset_manager::asset_manager() {
	render_scene = std::make_shared<scene>();
	visualize_scene = std::make_shared<scene>();
    cur_camera = std::make_shared<ppc>(512, 512, 90.0f);

	/* Lightings */
	lights = { glm::vec3(0.75f) * 5.0f };
    light_camera = std::make_shared<ppc>(1024, 1024, 30.0f);

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
    std::fstream out(json_fname, std::ios::out);
    if (!out.is_open()) {
        out.close();
        ERROR("{} cannot be open for writing", json_fname);
        return -1;
    }

    using namespace rapidjson;
    StringBuffer s;
    Writer<StringBuffer> writer(s);

    writer.StartObject();
    /* Camera */
    writer.Key("camera");
    FAIL(!m_serializers.at("camera"), "When serializing, camera is nullptr");
    std::string camera_json = m_serializers.at("camera")->to_json();
    writer.RawValue(camera_json.c_str(), camera_json.size(), rapidjson::Type::kStringType);

    /* Lights */
    writer.Key("lights");
    writer.StartArray();
    for(auto &l:lights) {
        writer.String(purdue::to_string(l).c_str());
    }
    writer.EndArray();

    /* Render Scene */
    writer.Key("render_scene");
    FAIL(!m_serializers.at("render_scene"), "When serializing, render_scene is nullptr");
    std::string scene_json = m_serializers.at("render_scene")->to_json();
    writer.RawValue(scene_json.c_str(), scene_json.size(), rapidjson::Type::kStringType);
    writer.EndObject();

    out << s.GetString() << std::endl;
    return -1;
}

int asset_manager::from_json(const std::string json_fname) {
    std::ifstream iss(json_fname);

    if (!purdue::file_exists(json_fname) || !iss.is_open()) {
        WARN("{} is not exists or able to open!", json_fname);
        iss.close();
        return -1;
    }
    
    std::stringstream buffer;
    buffer << iss.rdbuf();
    iss.close();
    std::string json_str = buffer.str();
    INFO("{}", json_str);

    return -1;
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
