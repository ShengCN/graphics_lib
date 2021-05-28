#include "image.h"
#include "common.h"
#include <cassert>
#include "Utilities/Utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Dep/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Dep/stb/stb_image_write.h"


image::image(int h, int w, int c) {
    init(w, h, c);
    init_buffer();
}

glm::vec3 image::get_rgb(int i, int j) {
    size_t ind = j * m_w + i;
    if (ind > m_h * m_w - 1) {
        WARN("Image get_rgb input error");
        return glm::vec3(0.0f);
    }

    return glm::vec3(uint_vec4(m_buffer[ind]));
}

float image::get_a(int i, int j) {
    size_t ind = j * m_w + i;
    if (ind > m_h * m_w - 1) {
        WARN("Image get_rgb input error");
        return 0.0f;
    }

    return uint_vec4(m_buffer[ind]).a;
}

bool image::set_rgb(int i, int j, glm::vec3 v) {
    if (!ind_check(i, j)) {
        return false;
    }

    at(i,j) = vec3_uint(v);
}

bool image::set_rgba(int i, int j, glm::vec4 v) {
    if (!ind_check(i, j)) {
            return false;
    }
    at(i,j) = vec4_uint(v);
}

bool image::set_a(int i, int j, float v) {
    if (!ind_check(i, j)) {
            return false;
    }
    ((unsigned char*)&at(i,j))[3] = (unsigned char)(v * 255.0);   
}

unsigned int& image::at(int i, int j) {
    size_t ind = j * m_w + i;
    assert(ind < m_h * m_w - 1);

    return m_buffer[j * m_w + i];
}

bool image::ind_check(int i, int j) {
    return j * m_w + i < m_h * m_w - 1;
}

void image::init_buffer() {
    m_buffer.resize(m_h * m_w);
}

glm::vec4 image::uint_vec4(unsigned int v) {
    glm::vec4 ret(0.0f);
    ret[0] = ((unsigned char*)&v)[0]/255.0;
    ret[1] = ((unsigned char*)&v)[1]/255.0;
    ret[2] = ((unsigned char*)&v)[2]/255.0;
    ret[3] = ((unsigned char*)&v)[3]/255.0;
    return ret;
}

unsigned int image::vec3_uint(glm::vec3 v) {
    unsigned int ret = 0x00000000f;
    ((unsigned char*)&ret)[0] = (unsigned char)(v[0] * 255.0);
    ((unsigned char*)&ret)[1] = (unsigned char)(v[1] * 255.0);
    ((unsigned char*)&ret)[2] = (unsigned char)(v[2] * 255.0);
    return ret;
}

unsigned int image::vec4_uint(glm::vec4 v) {
    unsigned int ret = 0x00000000f;
    ((unsigned char*)&ret)[0] = (unsigned char)(v[0] * 255.0);
    ((unsigned char*)&ret)[1] = (unsigned char)(v[1] * 255.0);
    ((unsigned char*)&ret)[2] = (unsigned char)(v[2] * 255.0);
    ((unsigned char*)&ret)[3] = (unsigned char)(v[3] * 255.0);
    return ret;
}

bool image::save(const std::string fname) {
    /* Note, only save to png currently */
    return stbi_write_png(fname.c_str(), m_w, m_h, 4, m_buffer.data(), m_w * 4);
}

bool image::load(const std::string fname) {
    unsigned char *img = stbi_load(fname.c_str(), &m_w, &m_h, &m_c, STBI_rgb_alpha);
    if (img == nullptr) {
        return false;
    }

    INFO("Loading " + fname);
    printf("W: %d, H: %d, C: %d \n", m_w, m_h, m_c);
    m_buffer.resize(m_w * m_h);
    memcpy(m_buffer.data(), img, m_w * m_h * 4);
    free(img);
    return true;
}

image::image() {
    init(0,0,0); 
}

uint image::ogl_texid() {
    if (m_ogl_texid == -1) {
        glGenTextures(1, &m_ogl_texid);
        glBindTexture(GL_TEXTURE_2D, m_ogl_texid);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

   return m_ogl_texid;
}

void image::init(int h, int w, int c) {
    m_h = h;
    m_w = w;
    m_c = c;
    m_ogl_texid = -1;
}