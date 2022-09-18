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

    const std::string OIT_vs = "Shaders/transparent_vs.glsl";
    const std::string OIT_fs = "Shaders/transparent_fs.glsl";

    m_shaders[default_shader_name] = init_shaders<shader>(template_vs, template_fs);
    m_shaders[quad_shader_name]    = init_shaders<quad_shader>(quad_vs, quad_fs);
    m_shaders[plane_shader_name]   = init_shaders<shader>(ground_vs, ground_fs);
    m_shaders[mask_shader_name]    = init_shaders<shader>(mask_vs, mask_fs);
    m_shaders[sm_shader_name]      = init_shaders<shader>(shmap_vs, shmap_fs);
    m_shaders[shadow_caster_name]  = init_shaders<shader>(shadow_vs, shadow_fs);
    m_shaders[OIT_shader_name]     = init_shaders<shader>(OIT_vs, OIT_fs);

    init_quad();
}


void renderer::render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc) {
    if (m_transparent_OIT) {
        oit_render(cur_scene, cur_ppc);
    } else {
        default_render(cur_scene, cur_ppc);
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

void renderer::set_OIT(bool trigger) {
    m_transparent_OIT = trigger;
}

void renderer::default_render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc, std::string shader_name) {
    rendering_params cur_render_params;
    cur_render_params.cur_camera = cur_ppc;

    auto rendered_meshes = cur_scene->get_mesh_descriptors();
    for (auto mdesc:rendered_meshes) {
        auto cur_desc = mdesc.second;
        m_shaders[shader_name]->draw_mesh(*cur_desc, cur_render_params);
    }
}

void renderer::oit_render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc) {
    static int old_fbo = -1;
    static unsigned int framebuffer = -1;
    static unsigned int accum_texture = -1;
    static unsigned int reveal_texture = -1;
    static unsigned int rbo = -1;
    static int cam_width = cur_ppc->width();
    static int cam_height = cur_ppc->height();

    bool update = false;
    update = (cam_width != cur_ppc->width()) || (cam_height != cur_ppc->height());

    // initialize
    if (framebuffer == -1 || update) {
        cam_width = cur_ppc->width();
        cam_height = cur_ppc->height();

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glGenTextures(1, &accum_texture);
        glBindTexture(GL_TEXTURE_2D, accum_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, cam_width, cam_height, 0, GL_RGBA, GL_FLOAT, NULL);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &reveal_texture);
        glBindTexture(GL_TEXTURE_2D, reveal_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, cam_width, cam_height, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accum_texture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, reveal_texture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            INFO("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // ------------------------ pass 1, render opaque ------------------------ //
    // todo

    // ------------------------ pass 2, render transparent ------------------------ //
    // clear initial values
    // render transparent

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDepthMask(GL_FALSE);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, DrawBuffers);
    glEnable(GL_BLEND);
    glBlendFunci(0, GL_ONE, GL_ONE);
    //glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

    // render_scene(cur_scene, params);
    default_render(cur_scene, cur_ppc, OIT_shader_name);
    glDepthMask(GL_TRUE);

    // ------------------------ pass 3, merge results ------------------------ //
    glBindFramebuffer(GL_FRAMEBUFFER, old_fbo); /* TODO, check */
    glDrawBuffer(GL_BACK);

    std::shared_ptr<shader> quad_shader = m_shaders.at(quad_shader_name);
    quad_shader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, accum_texture);
    glUniform1i(glGetUniformLocation(quad_shader->get_shader_program(), "accum_tex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, reveal_texture);
    glUniform1i(glGetUniformLocation(quad_shader->get_shader_program(), "weight_tex"), 1);

    glViewport(0, 0, cur_ppc->width(), cur_ppc->height());
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    draw_quad();
}

void renderer::init_quad() {
    GLuint vao, vbo;
    //quad is in z=0 plane, and goes from -1.0 to +1.0 in x,y directions.
    const float quad_verts[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };

    //generate vao id to hold the mapping from attrib variables in shader to memory locations in vbo
    glGenVertexArrays(1, &vao);
    //binding vao means that bindbuffer, enablevertexattribarray and vertexattribpointer
    // state will be remembered by vao
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo); // Generate vbo to hold vertex attributes for triangle
    glBindBuffer(GL_ARRAY_BUFFER, vbo); //specify the buffer where vertex attribute data is stored
    //upload from main memory to gpu memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), &quad_verts[0], GL_STATIC_DRAW);

    //get a reference to an attrib variable name in a shader
    const GLint pos_loc = 0;
    glEnableVertexAttribArray(pos_loc); //enable this attribute

    //tell opengl how to get the attribute values out of the vbo (stride and offset)
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);
    glBindVertexArray(0); //unbind the vao

    m_quad_vao = vao;
}

void renderer::draw_quad() {
    glBindVertexArray(m_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
