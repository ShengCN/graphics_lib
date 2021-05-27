#include "image.h"
#include <cassert>
#include "Utilities/Utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Dep/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Dep/stb/stb_image_write.h"


image::image(int h, int w, int c): m_h(h), m_w(w), m_c(c) {
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

unsigned int& image::at(int i, int j) {
    size_t ind = j * m_w + i;
    assert(ind < m_h * m_w - 1);

    return m_buffer[j * m_w + i];
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
    return stbi_write_png(fname.c_str(), m_w, m_h, m_c, m_buffer.data(), m_w * m_c);
}

bool image::load(const std::string fname) {
    unsigned char *img = stbi_load(fname.c_str(), &m_w, &m_h, &m_c, 0);
    if (img == nullptr) {
        return false;
    }

    m_buffer = std::vector<unsigned int>(img, img + m_w * m_h);
    return true;
}
