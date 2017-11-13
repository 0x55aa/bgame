#include "textures.hpp"
#include "stb_image.h"
#include <tuple>

namespace bengine {
    std::tuple<unsigned int, int, int, int> load_texture(const std::string &filename) {
        int width, height, bpp;
        GLuint texture_id;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *image_data = stbi_load(filename.c_str(), &width, &height, &bpp, STBI_rgb_alpha);
        if (image_data == nullptr) throw std::runtime_error(std::string("Cannot open: ") + std::string(filename));

        glGenTextures(1, &texture_id);
        if (texture_id == 0) throw std::runtime_error(std::string("Cannot allocate: ") + std::string(filename));
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(image_data);

        return std::make_tuple(texture_id, width, height, bpp);
    }

    texture_t::texture_t(const std::string &filename) {
		std::tie(texture_id, width, height, bpp) = load_texture(filename);
    }

    texture_t::~texture_t() {
        if (texture_id != 0) {
            glDeleteTextures(1, &texture_id);
            texture_id = 0;
        }
    }
};