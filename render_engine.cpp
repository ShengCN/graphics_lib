#include <common.h>
#include "Utilities/Utils.h"
#include "render_engine.h"
#include "yaml-cpp/yaml.h"

render_engine::render_engine(std::string config_file) {
    /*
     *  Init variables
     * */
    FAIL(!init_variables(config_file),"Input variables init failed");
}


bool render_engine::init_variables(std::string config_file) {
    if (!pd::file_exists(config_file)) {
        ERROR("File {} not exist!", config_file);
        return false;
    }

    YAML::Node config = YAML::LoadFile(config_file);


    m_h = config["h"].as<int>();
    m_w = config["w"].as<int>();
    m_title = config["title"].as<std::string>();

    INFO("H, W: {}, {}", m_h, m_w);
    return true;
}


bool render_engine::init_ogl_states() {
    bool ret = true;

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glPointSize(3.0);

    return ret;
}
