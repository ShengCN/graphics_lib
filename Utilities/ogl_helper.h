#pragma once
#include <glad/glad.h>

bool init_texutre(GLuint &texid);
void bind_texture(GLuint texid);

/* GPU memory object */
class ogl_texture {
public:
    ogl_texture();
    ~ogl_texture();

    /* Allocate depth memory */
    void new_depth_mem(int w, int h);

    /* Bind to host buffer or framebuffer */
    void new_img_mem(int w, int h,void *buffer);

    GLuint get_id();

private:
    GLuint m_tex; 
};

/* Frame Buffer Object */
class ogl_fbo {
public:
    ogl_fbo(int w, int h);
    ~ogl_fbo();
    
    void bind();
    void unbind();

    GLuint get_id();
    GLuint get_rgba_texture();
    GLuint get_depth_texture();

    static bool check_fbo();
private:
    GLuint m_fbo;
    ogl_texture m_rgba, m_depth;
};
