/* General Image Class
*/
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

class image {
private:
    int m_h, m_w, m_c;
    std::vector<unsigned int> m_buffer;

public:
    image(int h, int w, int c);
    ~image()=default;

    /* IO */
    bool save(const std::string fname);
    bool load(const std::string fname);

    /* Low level IO */
    glm::vec3 get_rgb(int i, int j);
    float get_a(int i, int j);
    unsigned int* data() { return m_buffer.data();}
    unsigned int& at(int i, int j);

private:
    void init_buffer();
    glm::vec4 uint_vec4(unsigned int v);
    unsigned int vec3_uint(glm::vec3 v);
    unsigned int vec4_uint(glm::vec4 v);
};
