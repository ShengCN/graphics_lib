#include <common.h>
#include <memory>
#include "render_engine.h"
#include "Utilities/Utils.h"

render_engine::render_engine(std::string config_file) {
    /*   Init variables  */
    FAIL(!init_variables(), "Init variables");
    FAIL(!from_json(config_file),"Read from config failed");
}

bool render_engine::init_variables() {
    m_h = m_w = 500;
    m_cur_scene_ind = 0;
    m_cur_ppc       = std::make_shared<ppc>(m_w, m_h, 50.0f);

    m_scenes.clear();
    m_scenes.push_back(std::make_shared<scene>());

    return true;
}


bool render_engine::init_ogl_states() {
    bool ret = true;

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glPointSize(3.0);

    m_renderer      = std::make_shared<renderer>();

    return ret;
}

void render_engine::render(int iter) {
    auto cur_scene = m_scenes[m_cur_scene_ind];

    m_renderer->render(cur_scene, m_cur_ppc);
}


std::shared_ptr<scene> render_engine::get_cur_scene() {
    if (m_cur_scene_ind >= m_scenes.size()) {
        ERROR("Current scene index({}/{}) is out of range", m_cur_scene_ind, m_scenes.size());
        exit(1);
    }

    return m_scenes[m_cur_scene_ind];
}

mesh_id render_engine::add_mesh(const std::string mesh_file, glm::vec3 color) {
    std::shared_ptr<mesh> new_mesh = std::make_shared<mesh>();

    if (!load_model(mesh_file, new_mesh)) {
        return -1;
    }

    return get_cur_scene()->add_mesh(new_mesh);
}

bool render_engine::to_json(std::string) {
    // TODO
    return true;
}


bool render_engine::from_json(const std::string json_fname) {
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
        ret = ret & (bool)m_cur_ppc->from_json(
                get_obj_string(
                    document[cur_key.c_str()]));

        m_h = m_cur_ppc->height();
        m_w = m_cur_ppc->width();
        m_cur_ppc->set_event_listen(true);
        m_cur_ppc->set_trackball(false);
    } else {
        ERROR("Cannot find {} or {} type is wrong", cur_key, cur_key);
        ret = ret & false;
    }

    /* Render Scene */
    cur_key = "render_scene";
    if (document.HasMember(cur_key.c_str()) && document[cur_key.c_str()].IsObject()) {
        ret = ret & get_cur_scene()->from_json(
                get_obj_string(
                    document[cur_key.c_str()]));
    } else {
        ERROR("Cannot find {} or {} type is wrong",cur_key, cur_key);
        ret = ret & false;
    }

    return (int)ret;
}


void render_engine::camera_press(int x, int y) {
    m_cur_ppc->mouse_press(x, y);
}

void render_engine::camera_release(int x, int y) {
    m_cur_ppc->mouse_release(x, y);
}

void render_engine::camera_move(int x, int y) {
    m_cur_ppc->mouse_move(x, y);
}

void render_engine::camera_scroll(int offset) {
    m_cur_ppc->scroll((double)offset);
}

void render_engine::camera_keyboard(char m, bool shift) {
    float speed = 1.0f * 0.1f;

    if(shift)
        speed *= 10.0f;

    switch (m)
    {
        case 'w':
            m_cur_ppc->Keyboard(CameraMovement::forward, speed);
            break;
        case 'a':
            m_cur_ppc->Keyboard(CameraMovement::left, speed);
            break;
        case 's':
            m_cur_ppc->Keyboard(CameraMovement::backward, speed);
            break;
        case 'd':
            m_cur_ppc->Keyboard(CameraMovement::right, speed);
            break;
        case 'q':
            m_cur_ppc->Keyboard(CameraMovement::up, speed);
            break;
        case 'e':
            m_cur_ppc->Keyboard(CameraMovement::down, speed);
            break;
        default:
            break;
    }
}


glm::mat4 render_engine::get_camera_view() {
    return m_cur_ppc->GetV();
}


glm::mat4 render_engine::get_camera_perspective() {
    return m_cur_ppc->GetP();
}

glm::mat4 render_engine::get_obj_toworld(mesh_id id) {
    auto mesh = get_cur_scene()->get_mesh(id);
    return mesh->get_world_mat();
}


void render_engine::set_obj_toworld(mesh_id id, glm::mat4 toworld) {
    auto mesh = get_cur_scene()->get_mesh(id);
    return mesh->set_world_mat(toworld);
}
