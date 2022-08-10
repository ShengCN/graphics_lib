#pragma once
#include "common.h"
#include "Render/renderer.h"
#include "Render/geo.h"
#include "Render/mesh.h"
#include "Render/shader.h"
#include "Render/scene.h"
#include "Render/ppc.h"
#include "Utilities/voxelization.h"
#include "asset_manager.h"

class render_engine{
private: /* Private Variables */
    int m_h, m_w; /* current render frame size */
    std::string m_title;
    size_t m_cur_scene;
    std::vector<std::shared_ptr<scene>> m_scenes;

public: /* API */
    //------- Initialize --------//
    render_engine(std::string config_file);
    bool init_ogl_states();

    int width()  { return m_w; };
    int height() { return m_h; };
    std::string title() {return m_title;}


private: /* Private functions */
    bool init_variables(std::string config_file);
    bool init_shaders();
};
