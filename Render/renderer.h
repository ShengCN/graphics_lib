#pragma once
#include <common.h>
#include <memory>
#include "scene.h"
#include "ppc.h"
#include "shader.h"

class renderer {
private:
    std::unordered_map<std::string, std::shared_ptr<shader>> m_shaders;
    bool m_transparent_OIT = true;
    GLuint m_quad_vao=-1;

public:
    renderer();
    void render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc);

    /*
     * Trigger if we render order independent transparency
     */
    void set_OIT(bool trigger);

private:
    template<typename T>
    std::shared_ptr<shader> init_shaders(const std::string vs, const std::string fs);

    /* Different rendering style */
    void default_render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc, std::string shader_name=default_shader_name);
    void oit_render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc);

    void init_quad();
    void draw_quad();
};
