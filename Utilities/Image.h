/* General Image Class
*/
#pragma once
#include <common.h>
#include <cstddef>

class Image {
private:
    int m_h, m_w;
    // std::vector<unsigned int> m_buffer;
    std::vector<glm::vec4> m_buffer;

public:
    Image();
    Image(const std::string fname);
    Image(int w, int h);
    ~Image()=default;

    /* Properties */
    int width() const { return m_w; }
    int height() const{ return m_h; }
    void set_dim(int w, int h) {m_w = w; m_h = h;}
    void set_color(glm::vec4 c);
    void clear(glm::vec4 c=glm::vec4(1.0f));

    /* Processing */
    Image normalize();

    /* IO */
    bool save(const std::string fname, bool normalize=false);
    bool load(const std::string fname);

    /* Low level IO */
    glm::vec3 get_rgb(int i, int j) const;
    float get_a(int i, int j) const;
    bool set_rgb(int i, int j, glm::vec3 v);
    bool set_rgba(int i, int j, glm::vec4 v);
    bool set_a(int i, int j, float v);

    glm::vec4* data() { return m_buffer.data();}
    glm::vec4& at(int i, int j);
    glm::vec4 get(int i, int j) const;
    std::vector<glm::vec4>& get_buffer() { return m_buffer; }
    std::vector<unsigned int> to_unsigned_data();
    void from_unsigned_data(const std::vector<unsigned int> &data, int w, int h);
    void from_unsigned_data(unsigned char *data, int w, int h);
    void from_unsigned_data(unsigned int *data, int w, int h);

    void padding(int i, int j, int w, int h, int &outi, int &outj) const;
    static unsigned int vec3_uint(glm::vec3 v);
    static unsigned int vec4_uint(glm::vec4 v);
    static glm::vec4 uint_vec4(unsigned int v);
    static glm::vec3 uint_vec3(unsigned int v);
    static size_t get_ind(size_t i, size_t j, size_t w, size_t h);
    float sum();
    glm::vec3 min();
    glm::vec3 max();

    Image operator-(const Image &rhs) const;
    Image operator+(const Image &rhs) const;
    Image operator/(float v);
    Image operator*(float v);
    Image operator*(const Image &rhs) const;

    Image resize(int size) const;

private:
    void init(int w, int h);
    bool ind_check(int i, int j);
    void init_buffer();
    Image norm_minmax();
};
