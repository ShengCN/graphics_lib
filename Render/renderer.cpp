#include "renderer.h"
#include "Utilities/Utils.h"
#include "common.h"

renderer::renderer() {
    const std::string template_vs = "Shaders/template_vs.glsl";
    const std::string template_fs = "Shaders/template_fs.glsl";

    const std::string quad_vs = "Shaders/quad_vs.glsl";
    const std::string quad_fs = "Shaders/quad_fs.glsl";

    const std::string ground_vs = "Shaders/ground_vs.glsl";
    const std::string ground_fs = "Shaders/ground_fs.glsl";

    const std::string mask_vs = "Shaders/mask_vs.glsl";
    const std::string mask_fs = "Shaders/mask_fs.glsl";

    const std::string shmap_vs = "Shaders/shadow_map_vs.glsl";
    const std::string shmap_fs = "Shaders/shadow_map_fs.glsl";

    const std::string shadow_vs = "Shaders/draw_shadow_vs.glsl";
    const std::string shadow_fs = "Shaders/draw_shadow_fs.glsl";

    m_shaders[default_shader_name] = init_shaders<shader>(template_vs, template_fs);
    m_shaders[quad_shader_name]    = init_shaders<quad_shader>(quad_vs, quad_fs);
    m_shaders[plane_shader_name]   = init_shaders<shader>(ground_vs, ground_fs);
    m_shaders[mask_shader_name]    = init_shaders<shader>(mask_vs, mask_fs);
    m_shaders[sm_shader_name]      = init_shaders<shader>(shmap_vs, shmap_fs);
    m_shaders[shadow_caster_name]  = init_shaders<shader>(shadow_vs, shadow_fs);
}


void renderer::render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc) {
    rendering_params cur_render_params;
    cur_render_params.cur_camera = cur_ppc;

    std::unordered_map<mesh_id, std::shared_ptr<mesh>> meshes = cur_scene->get_meshes();
    for (auto m:meshes) {
        Mesh_Descriptor cur_descriptor(m.second);

        m_shaders[default_shader_name]->draw_mesh(cur_descriptor, cur_render_params);
    }
}

template<typename T>
std::shared_ptr<shader> renderer::init_shaders(const std::string vs, const std::string fs) {
    if (!pd::file_exists(vs) || !pd::file_exists(fs)) {
        ERROR("Cannot find file {} or {}", vs, fs);
        exit(1);
    }

    return std::make_shared<T>(vs.c_str(), fs.c_str());
}

