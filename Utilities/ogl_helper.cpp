#include "ogl_helper.h"
#include "Logger.h"

ogl_texture::ogl_texture():m_tex(-1) {
    glGenTextures(1, &m_tex);
    FAIL((m_tex==-1), "ogl texture initialize failed");
}

ogl_texture::~ogl_texture() {
    glDeleteTextures(1, &m_tex);
}

void ogl_texture::new_depth_mem(int w, int h) {
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ogl_texture::new_img_mem(int w, int h, void *buffer) {
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint ogl_texture::get_id() {
    return m_tex;
}

ogl_fbo::ogl_fbo(int w, int h) {
    glGenFramebuffers(1, &m_fbo);
    FAIL(m_fbo == -1, "framebuffer object initialization failed");

    /* Prepare GPU buffer */
    m_rgba.new_img_mem(w, h, NULL);
    m_depth.new_depth_mem(w, h);

    /* Bind related buffer member */
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rgba.get_id(), 0);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth.get_id(), 0);

    FAIL(!check_fbo(), "FBO initialization failed. {},{},{}", get_id(), m_rgba.get_id(), m_depth.get_id());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool ogl_fbo::check_fbo() {
    GLenum status;
    status = (GLenum)glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
        return true;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        ERROR("Framebuffer incomplete, incomplete attachment\n");
        return false;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        ERROR("Unsupported framebuffer format\n");
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        ERROR("Framebuffer incomplete, missing attachment\n");
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        ERROR("Framebuffer incomplete, missing draw buffer\n");
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        ERROR("Framebuffer incomplete, missing read buffer\n");
        return false;
    }
    return false;
}

ogl_fbo::~ogl_fbo() {
    glDeleteFramebuffers(1, &m_fbo);
}

GLuint ogl_fbo::get_id() {
    return m_fbo;
}

GLuint ogl_fbo::get_rgba_texture() {
    return m_rgba.get_id();
}

GLuint ogl_fbo::get_depth_texture() {
    return m_depth.get_id();
}

void ogl_fbo::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void ogl_fbo::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
