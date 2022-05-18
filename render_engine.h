#pragma once
#include "common.h"
#include "Render/geo.h"
#include "Render/mesh.h"
#include "Render/shader.h"
#include "Render/scene.h"
#include "Render/ppc.h"
#include "Utilities/voxelization.h"
#include "asset_manager.h"
//#include <RT/RT_engine.h>

class render_engine{
    //-------  APIs --------//
public:
    //------- Initialize --------//
    render_engine();
    void init();
    void test_scene(int w, int h);
    void init_camera(int w, int h, float fov);
    void set_camera_nearfar(float near, float far);

    //------- Render --------//
    void render(int frame);
    void rt_render(int frame);

    //------- Query --------//
    std::shared_ptr<ppc> get_render_ppc();
    std::shared_ptr<ppc> get_light_ppc();
    AABB get_mesh_size(mesh_id id); 
    void get_casters(std::vector<glm::vec3> &verts, AABB &aabb);
    void get_receiver(std::vector<glm::vec3> &verts, AABB &aabb);
    glm::vec3 get_mesh_center(mesh_id id);
    AABB get_mesh_aabb(mesh_id id);

    //------- Modify Scene --------//
    mesh_id add_mesh(const std::string model_fname, bool norm=false, vec3 color=vec3(0.8f));
    mesh_id add_plane_mesh(vec3 p, vec3 n);
    void remove_mesh(mesh_id id);
    void stand_on_plane(mesh_id id, vec3 p, vec3 n);
    void set_point_light(glm::vec3 lp);
    vec3 get_light_pos();
    void set_mesh_verts(mesh_id id, std::vector<glm::vec3> &verts);
    bool reload_mesh(mesh_id id, std::string fname);
    void clear_scene();
    void scaling_space(glm::vec3 s);

    //------- IO --------//
    bool save_mesh(mesh_id id, const std::string model_fname);
    bool save_framebuffer(const std::string ofname);
    int to_json(const std::string json_fname);
    int from_json(const std::string json_fname);

    //------- UI --------//
    void camera_press(int x, int y);
    void camera_release(int x, int y);
    void camera_move(int x, int y);
    void camera_scroll(int offset);
    void camera_keyboard(char m, bool shift);

    //------- Modify --------//
    void norm_render_scene();
    void norm_mesh(mesh_id id, glm::vec3 scale=vec3(1.0f));
    void draw_line(glm::vec3 t, glm::vec3 h, vec3 tc, vec3 hc);
    void set_mesh_color(mesh_id id, vec3 c);
    void mesh_add_transform(mesh_id id, glm::mat4 mat);
    void mesh_set_transform(mesh_id id, glm::mat4 mat);
    void add_rotate(mesh_id id, purdue::deg angle, vec3 axis);
    glm::mat4 get_mesh_world(mesh_id m);
    void set_render_camera(int w, int h);
    void set_render_camera(int w, int h, float fov);
    void set_render_camera_fov(float fov);
    void set_shader(mesh_id m, const std::string shader_name);
    void recompute_normal(int mesh_id);
    void update_time(double t);
    double get_time();
    void set_caster(mesh_id id, bool is_caster);
    void ppc_event_listen(bool islisten);

    //------- Rendering --------//
    void look_at(int mesh_id, vec3 relative=vec3(0.0f,0.0f,1.0f));
    // void draw_render(bool trigger) { m_draw_render = trigger; }
    // void draw_visualize(bool trigger) { m_draw_visualize = trigger; }
    void draw_visualize_voxels(std::vector<AABB> voxels);
    void set_vis_frame(bool trigger) { m_vis_frame_mode = trigger; }
    void voxel_vis(int mesh_id);
    void draw_visualize_line(glm::vec3 t, glm::vec3 h);
    void draw_quad();
    void draw_image(std::shared_ptr<Image> img);
    void draw_mesh(mesh_id id);
    void draw_shadow(mesh_id shadow_receiver);
    void draw_sihouette(int mesh_id, vec3 light_pos);
    void draw_shadow_volume(int mesh_id, vec3 light_pos);
    bool reload_shaders();

    void prepare_shadow_map_states();
    void prepare_default_shading();
    void reset_shadow_map_states();
    void render_shadow_maps();
    void default_shading();

    //------- Visualization --------//
    mesh_id add_visualize_line(vec3 h, vec3 t);
    bool remove_visualize_line(mesh_id id);
    void clear_visualize();

    //------- Others --------//
    std::shared_ptr<Image> get_frame_buffer();
    GLuint to_GPU_texture(Image &img);
    std::shared_ptr<Image> from_GPU_texture(GLuint texid, int w, int h);
    std::shared_ptr<Image> composite(const Image &bg, const Image &fg);
    std::shared_ptr<mesh> get_mesh(int id);
    void set_draw_types(draw_type dt);
    void dbg_scene();

private:
    bool init_shaders();
    bool init_ogl_states();
    void render_scene(std::shared_ptr<scene> cur_scene, rendering_params params);
    void render_weighted_OIT(std::shared_ptr<scene> cur_scene, rendering_params params);

protected:
    std::shared_ptr<mesh> vis_new_mesh();
    std::shared_ptr<mesh> add_empty_mesh();
    asset_manager m_manager;
    //std::shared_ptr<dynamic_renderer> m_rt_renderer;
    bool m_vis_frame_mode;
    double m_curtime;

    /* OpenGL States */
    std::shared_ptr<ogl_fbo> m_fbo;
    draw_type m_cur_draw_type;
};

