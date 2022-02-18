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
    cur_camera = std::make_shared<ppc>(512, 512, 90.0f, 0.05f, 10.0f);

	/* Lightings */
	lights = { glm::vec3(1.0f) * 3.0f };
    light_camera = std::make_shared<ppc>(512, 512, 60.0f, 0.5f, 1000.0f);
}

int asset_manager::from_json(const std::string json_fname) {
    FAIL(!check_assets(), "Asset manager failed to initialize");

    std::ifstream iss(json_fname);
    if (!purdue::file_exists(json_fname) || !iss.is_open()) {
        WARN("{} is not exists or able to open!", json_fname);
        iss.close();
        exit(1);
        return -1;
    } 

    std::stringstream buffer;
    buffer << iss.rdbuf();
    iss.close();
    std::string json_str = buffer.str(), cur_key;
    bool ret = true;

    using namespace rapidjson;
    Document document;
    document.Parse(json_str.c_str());

    /* Camera */
    cur_key = "camera";
    if (document.HasMember(cur_key.c_str()) && document[cur_key.c_str()].IsObject()) {
        ret = ret & (bool)cur_camera->from_json(
                get_obj_string(
                    document[cur_key.c_str()]));
    } else {
        ERROR("Cannot find {} or {} type is wrong", cur_key, cur_key);
        ret = ret & false;
    }

    /* Light */
    cur_key = "lights";
    if (document.HasMember(cur_key.c_str()) && document[cur_key.c_str()].IsArray()) {
        /* Parse Light Arrays */
        const Value& a = document[cur_key.c_str()];
        for(SizeType i = 0; i < a.Size(); ++i) {
            lights.push_back(purdue::string_vec3(a[i].GetString()));
        }
    } else {
        ERROR("Cannot find {} or {} type is wrong",cur_key, cur_key);
        ret = ret & false;
    }

    /* Render Scene */
    cur_key = "render_scene";
    if (document.HasMember(cur_key.c_str()) && document[cur_key.c_str()].IsObject()) {
        ret = ret & render_scene->from_json(
                get_obj_string(
                    document[cur_key.c_str()]));
    } else {
        ERROR("Cannot find {} or {} type is wrong",cur_key, cur_key);
        ret = ret & false;
    }

    return (int)ret;
}

int asset_manager::to_json(const std::string json_fname) {
    /* Serielize Orders 
        * Camera
        * Lights
        * Render scene 
     **/
    check_assets();
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
    std::string camera_json = std::dynamic_pointer_cast<ISerialize>(cur_camera)->to_json();
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
    std::string scene_json = std::dynamic_pointer_cast<ISerialize>(render_scene)->to_json();
    writer.RawValue(scene_json.c_str(), scene_json.size(), rapidjson::Type::kStringType);
    writer.EndObject();

    out << s.GetString() << std::endl;
    return 1;
}
