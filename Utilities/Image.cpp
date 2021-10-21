#include <cfloat>
#include <common.h>
#include <cstddef>
#include <stdexcept>
#include "Image.h"
#include "Utils.h"

Image::Image(int w, int h) {
    init(w, h);
    init_buffer();
}

Image::Image(const std::string fname) {
    load(fname);
}

glm::vec3 Image::get_rgb(int i, int j) const {
    size_t ind = get_ind(i, j, width(), height());
    if (ind > m_h * m_w - 1) {
        WARN(fmt::format("Image get_rgb input {}/ i: {} j: {} w: {} h: {} error", ind, i, j, width(), height()));
        exit(0);
        return glm::vec3(0.0f);
    }

    return glm::vec3(m_buffer[ind]);
}

float Image::get_a(int i, int j) const {
    size_t ind = get_ind(i, j, width(), height());
    if (ind > m_h * m_w - 1) {
        WARN("Image get_rgb input error");
        return 0.0f;
    }

    return m_buffer[ind].a;
}

bool Image::set_rgb(int i, int j, glm::vec3 v) {
    if (!ind_check(i, j)) {
        return false;
    }

    at(i,j) = vec4(v, 1.0);
    return true;
}

bool Image::set_rgba(int i, int j, glm::vec4 v) {
    if (!ind_check(i, j)) {
            return false;
    }
    at(i,j) = v;
    return true;
}

bool Image::set_a(int i, int j, float v) {
    if (!ind_check(i, j)) {
            return false;
    }
    at(i,j).a = v;
    return true;
}

glm::vec4& Image::at(int i, int j) {
    size_t ind = get_ind(i, j, width(), height());
    if (!(ind < height() * width() && ind >=0)) {
        throw std::invalid_argument(fmt::format("Index out of range. Ind: {}, ({},{}),h: {}, w: {}", ind, i, j, height(), width()));
    }

    return m_buffer[ind];
}

glm::vec4 Image::get(int i, int j) const  {
    size_t ind = get_ind(i, j, width(), height());
    if (!(ind < height() * width() && ind >=0)) {
        throw std::invalid_argument(fmt::format("Index out of range. Ind: {}, ({},{}),h: {}, w: {}", ind, i, j, height(), width()));
        return glm::vec4(0.0f);
    }
    return m_buffer[ind];
}

bool Image::ind_check(int i, int j) {
    return j * m_w + i < m_h * m_w;
}

void Image::init_buffer() {
    m_buffer.resize(m_h * m_w, glm::vec4(0.0f,0.0f,0.0f,1.0f));
}

void Image::padding(int i, int j, int w, int h, int &outi, int &outj) const {
	outi = i;
	outj = j;

	if (j < 0) {
		outj = 0;
	} 
	if (j > h -1) {
		// outj = 2 * (h - 1) - j; 
		outj = h -1;
	}
	
	if (i < 0) {
		// outi = -i;
		outi = 0;
	}
	if (i > w - 1) {
		// outi = 2 * (w-1) - i;
		outi = w - 1;
	}
}

glm::vec4 Image::uint_vec4(unsigned int v) {
    glm::vec4 ret(0.0f);
    ret[0] = ((unsigned char*)&v)[0]/255.0;
    ret[1] = ((unsigned char*)&v)[1]/255.0;
    ret[2] = ((unsigned char*)&v)[2]/255.0;
    ret[3] = ((unsigned char*)&v)[3]/255.0;
    return ret;
}

glm::vec3 Image::uint_vec3(unsigned int v) {
    glm::vec3 ret(0.0f);
    ret[0] = ((unsigned char*)&v)[0]/255.0;
    ret[1] = ((unsigned char*)&v)[1]/255.0;
    ret[2] = ((unsigned char*)&v)[2]/255.0;
    return ret;
}

unsigned int Image::vec3_uint(glm::vec3 v) {
    unsigned int ret = 0xff000000;
    ((unsigned char*)&ret)[0] = (unsigned char)(v[0] * 255.0);
    ((unsigned char*)&ret)[1] = (unsigned char)(v[1] * 255.0);
    ((unsigned char*)&ret)[2] = (unsigned char)(v[2] * 255.0);
    return ret;
}

unsigned int Image::vec4_uint(glm::vec4 v) {
    unsigned int ret = 0x00000000;
    ((unsigned char*)&ret)[0] = (unsigned char)(v[0] * 255.0);
    ((unsigned char*)&ret)[1] = (unsigned char)(v[1] * 255.0);
    ((unsigned char*)&ret)[2] = (unsigned char)(v[2] * 255.0);
    ((unsigned char*)&ret)[3] = (unsigned char)(v[3] * 255.0);
    return ret;
}

size_t Image::get_ind(size_t i, size_t j, size_t w,size_t h) {
    if (i < 0 || i >= w || j < 0 || j >=h) {
        throw std::invalid_argument(fmt::format("Index error: {} {} ({},{})", i, j, w, h));
        return 0;
    }
    // return (h-1-j) * w + i;
    return j * w + i;
}

std::vector<unsigned int> Image::to_unsigned_data() {
    std::vector<unsigned int> ret(width() * height());

    for(int i = 0; i < width(); ++i) for (int j = 0; j < height(); ++j) {
        int ind = get_ind(i, j, width(), height());
        ret[ind] = vec4_uint(glm::clamp(at(i, j), vec4(0.0f), vec4(1.0f)));
    }

    return ret;
}

void Image::clear(glm::vec4 c) {
    std::fill(m_buffer.begin(), m_buffer.end(), c);
}

void Image::from_unsigned_data(unsigned char *data, int w, int h) {
    if (!data) {
        throw std::invalid_argument("Reading Image failed!");
        return;
    }

    set_dim(w, h);
    m_buffer.resize(w * h);
    for(int i = 0; i < w; ++i) for(int j = 0; j < h; ++j) {
        long long ind = get_ind(i, j, w, h);

        m_buffer[ind] = vec4(data[4 * ind + 0]/255.0f,
            data[4 * ind + 1]/255.0f,
            data[4 * ind + 2]/255.0f,
            data[4 * ind + 3]/255.0f);
    }
}

void Image::from_unsigned_data(unsigned int *data, int w, int h) {
    if (!data) {
        throw std::invalid_argument("Reading Image failed!");
        return;
    }

    size_t num = w * h;
    set_dim(w, h);
    m_buffer.resize(num);
    for(int i = 0; i < num; ++i) {
        m_buffer[i] = uint_vec4(data[i]);
    }
}

Image Image::normalize() {
    return norm_minmax();
}

bool Image::save(const std::string fname, bool normalize) {
    if (normalize) {
        Image tmp = this->normalize();
        return tmp.save(fname);
    }

    std::vector<unsigned int> tmp = to_unsigned_data();

    /* Note, only save to png currently */
    return purdue::save_image(fname.c_str(), tmp.data(), m_w, m_h);
}

bool Image::load(const std::string fname) {
    int c=0;
    std::vector<unsigned char> buf;
    if (purdue::read_image(fname, m_w, m_h, c, buf)) {
        INFO("Loading " + fname);
        INFO(fmt::format("W: {}, H: {}, C: {} \n", m_w, m_h, c));
        from_unsigned_data(buf.data(), m_w, m_h);    
        INFO("Loading Success");
        return true;
    }

    return false;
}

Image::Image() {
    init(0,0); 
}

void Image::init(int w, int h) {
    m_h = h;
    m_w = w;
}

void Image::set_color(glm::vec4 c) {
    for(int i = 0; i < m_w; ++i) for(int j = 0; j < m_h; ++j) {
        at(i,j) = c;
    }
}

Image Image::operator+(const Image &rhs) const {
    if (width() != rhs.width() || height() != rhs.height()) {
        throw std::invalid_argument(fmt::format("Image operator -, dim does not match! {},{} but rhs {},{}", width(), height(), rhs.width(), rhs.height()));
    }

    Image ret(width(), height());
    for(int i = 0; i < m_w; ++i) for(int j = 0; j < m_h; ++j) {
        vec4 c = get(i,j);
        ret.at(i,j) = c + rhs.get(i,j);
        ret.at(i,j).a = c.a;
    }
    return ret;
}

Image Image::operator-(const Image &rhs) const {
    FAIL(width() != rhs.width() || height() != rhs.height(), "Image operator -, dim does not match! {},{} but rhs {},{}", width(), height(), rhs.width(), rhs.height());

    Image ret(width(), height());
#pragma omp parallel for collapse(2)
    for(int i = 0; i < m_w; ++i) for(int j = 0; j < m_h; ++j) {
        vec4 c = get(i,j);
        ret.at(i,j) = c - rhs.get(i,j);
        ret.at(i,j).a = c.a;
    }
    return ret;
}

Image Image::operator*(const Image &rhs) const {
    Image ret(width(), height());
#pragma omp parallel for collapse(2)
    for(int i =0; i < m_w; ++i) for (int j = 0; j < m_h; ++j) {
        ret.at(i,j) = get(i,j) * rhs.get(i,j);
    }
    return ret;
}

Image Image::operator*(float v) {
    Image ret(width(), height());
#pragma omp parallel for collapse(2)
    for(int i =0; i < m_w; ++i) for (int j = 0; j < m_h; ++j) {
        ret.at(i,j) = at(i,j) * v;
    }
    return ret;
}

Image Image::operator/(float v) {
    Image ret(width(), height());
#pragma omp parallel for collapse(2)
    for(int i =0; i < m_w; ++i) for (int j = 0; j < m_h; ++j) {
        ret.at(i,j) = at(i,j)/v;
    }
    return ret;
}

float Image::sum() {
    vec3 ret(0.0f);
    for(int i =0; i < m_w; ++i) for (int j = 0; j < m_h; ++j) {
        ret += vec3(at(i,j));
    }
    return ret.r + ret.g + ret.b;
}

vec3 Image::min() {
    if (m_h == 0 || m_w == 0) {
        WARN("Image has not been initialized yet");
        return vec3(0.0f);
    }

    vec3 ret(FLT_MAX);
    for(int i =0; i < m_w; ++i) for (int j = 0; j < m_h; ++j) {
        vec3 cp = get(i,j);
        ret.x = std::min(cp.x, ret.x);
        ret.y = std::min(cp.y, ret.y);
        ret.z = std::min(cp.z, ret.z);
    }

    return ret;
}

vec3 Image::max() {
    if (m_h == 0 || m_w == 0) {
        WARN("Image has not been initialized yet");
        return vec3(0.0f);
    }

    vec3 ret(-FLT_MAX);
    for(int i =0; i < m_w; ++i) for (int j = 0; j < m_h; ++j) {
        vec3 cp = get(i,j);
        ret.x = std::max(cp.x, ret.x);
        ret.y = std::max(cp.y, ret.y);
        ret.z = std::max(cp.z, ret.z);
    }

    return ret;
}

Image Image::norm_minmax() {
    Image ret(width(), height());
    vec3 min_v(FLT_MAX, FLT_MAX, FLT_MAX), max_v(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for(int i = 0; i < m_w; ++i) for(int j = 0; j < m_h; ++j) {
        vec3 p = vec3(at(i,j));
        min_v.x = std::min(min_v.x, p.x);
        min_v.y = std::min(min_v.y, p.y);
        min_v.z = std::min(min_v.z, p.z);

        max_v.x = std::max(max_v.x, p.x);
        max_v.y = std::max(max_v.y, p.y);
        max_v.z = std::max(max_v.z, p.z);
    }
     
    float min_ = 0.0f,max_ = 0.0f;
    vec3 grad = max_v - min_v;
    if (grad.x >= grad.y && grad.x >= grad.z) {
        min_ = min_v.x;
        max_ = max_v.x;
    }
    if (grad.y >= grad.x && grad.y >= grad.z) {
        min_ = min_v.y;
        max_ = max_v.y;
    }
    if (grad.z >= grad.y && grad.z >= grad.x) {
        min_ = min_v.z;
        max_ = max_v.z;
    }

    for(int i = 0; i < m_w; ++i) for(int j = 0; j < m_h; ++j) {
        vec4 p = at(i,j);
        ret.at(i,j) = vec4((p.x-min_)/(max_-min_),(p.y-min_)/(max_-min_), (p.z-min_)/(max_-min_), p.a);
        ret.at(i,j) = glm::clamp(ret.at(i,j), vec4(0.0f), vec4(1.0f));
    }
    return ret;
}

Image Image::resize(int size) const {
    int h = height(), w = width(), nh, nw;
    float fact, aspect = (float)h/w;
    if (h > w) {
        fact = (float)size/h;
    } else {
        fact = (float)size/w;
    }
    nh = int(h * fact);
    nw = int(w * fact);

    Image ret(nw, nh);

    /* Our current interpolation method is INTER_AREA */
    for(int i = 0; i < nw; ++i) for(int j = 0; j < nh; ++j) {
        int sample_i = std::floor(i / fact), sample_ii = std::ceil((i + 1)/fact);
        int sample_j = std::floor(j / fact), sample_jj = std::ceil((j+1)/fact);

        float area = (sample_jj - sample_j) * (sample_ii - sample_i);
        vec4 sample(0.0f);
        /* INTER_AREA can only be applied to decimation */
        if (fact < 1.0) {
            for(int ii = sample_i; ii <= sample_ii; ++ii) for (int jj = sample_j; jj <= sample_jj; ++jj) {
                int padi, padj;
                padding(ii, jj, width(), height(), padi, padj);
                sample += vec4(get_rgb(padi, padj), 1.0);
            }
            sample = sample/sample.a;
        } else {
            /* Nearest Neighbors */
            sample = vec4(get_rgb(sample_i, sample_j), 1.0f);
        }


        if (std::isnan(sample.x)) {
            INFO(fmt::format("({},{}): ({},{},{}). fact:{}, aspect: {}", i, j, sample.x, sample.y, sample.z, fact, aspect));
            INFO(fmt::format("Area: {}, {},{}/{},{}", area, sample_i, sample_j, sample_ii, sample_jj));
        } 
        ret.set_rgba(i, j, sample);
    }
    return ret;
}

void Image::copy_buffer(int w, int h, std::vector<glm::vec4> &buffer) {
    FAIL(w * h !=buffer.size(), "Copy Buffer size not match.");

    m_w = w;
    m_h = h;
    m_buffer = buffer;
}
