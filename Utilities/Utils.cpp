#include "Utils.h"
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "Dep/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Dep/stb/stb_image_write.h"
#include "Logger.h"

namespace purdue
{
    bool read_image(const std::string fname, int &w, int &h, int &c, std::vector<unsigned char> &buffer)  {
        unsigned char *img = stbi_load(fname.c_str(), &w, &h, &c, STBI_rgb_alpha);
        if (img == nullptr) {
            return false;
        }

        stbi_set_flip_vertically_on_load(true);

        /* Make a copy */
        // INFO("Image dimension: {}, {}, {}", w, h, c);
        buffer = std::vector<unsigned char>(img, img + w*h*4);
        free(img);
        return true;
    }

    bool save_image(const std::string fname, unsigned int *pixels, int w, int h, int c /*= 4*/) {
        return stbi_write_png(fname.c_str(), w, h, c, pixels, w*c);
    }

    std::default_random_engine generator;
    float normal_random(float mean, float sig) {
        std::normal_distribution<float> distribution(mean, sig);
        return distribution(generator);
    }

    void timer::print_elapsed(const std::string log) {
        if (!_is_ticed) {
            std::cerr << "timer has not been ticed \n";
            return;
        }

        auto elapsed = get_elapse();
        //std::cout << fmt::format("{:30} [Time: {}s]\n", log, elapsed * 1e-9);
        INFO("{:30} [Time: {}s]\n", log, elapsed * 1e-9);
    }
}
