#pragma once
#include "common.h"
#include "Render/renderer.h"
#include "Render/geo.h"
#include "Render/mesh.h"
#include "Render/shader.h"
#include "Render/scene.h"
#include "Render/ppc.h"
#include "Utilities/voxelization.h"

class render_engine {
private: /* Private Variables */
    int m_h, m_w; /* current render frame size */
    std::string m_title;
    size_t m_cur_scene_ind;
    std::shared_ptr<ppc> m_cur_ppc;

    std::shared_ptr<renderer> m_renderer;
    std::vector<std::shared_ptr<scene>> m_scenes;

public: /* API */
    //------- Initialize --------//
    render_engine(std::string config_file);
    bool init_variables();
    bool init_ogl_states();
    void clear_scene();

    int width()  { return m_w; };
    int height() { return m_h; };
    std::string title() {return m_title;}

    void render(int iter);

    /* IO */
    std::shared_ptr<scene> get_cur_scene();
    mesh_id add_mesh(const std::string mesh_file, glm::vec3 color= default_mesh_color);
    mesh_id add_mesh(std::shared_ptr<mesh> mesh_ptr, draw_type type);
    bool remove_mesh(mesh_id id);
    std::shared_ptr<mesh> get_mesh(mesh_id id);
    glm::mat4 get_obj_toworld(mesh_id id);
    void set_obj_toworld(mesh_id id, glm::mat4 toworld);

    /* Render */
    void set_render_type(mesh_id id, draw_type type);
    void set_OIT(bool trigger);

    /* Camera */
    void camera_press(int x, int y);
    void camera_release(int x, int y);
    void camera_move(int x, int y);
    void camera_scroll(int offset);
    void camera_keyboard(char m, bool shift);
    void camera_focus(mesh_id id);

    glm::mat4 get_camera_view();
    glm::mat4 get_camera_perspective();

    bool to_json(std::string);
    bool from_json(const std::string json_str);

private: /* Private functions */
    bool init_shaders();
};
