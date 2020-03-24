#pragma once
#include "basic.h"

namespace illusion {
	namespace stb_extension {
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
	}

	class texture {
	private:
		unsigned uid;
		bool valid;
		texture(const texture&) = default;

	public:
		texture(unsigned wrap_method = GL_REPEAT,
				unsigned min_filter = GL_LINEAR_MIPMAP_LINEAR,
				unsigned max_filter = GL_LINEAR) 
			:uid(~0), valid(false) 
		{
			glGenTextures(1, &uid);
			if (~uid) {
				valid = true;
				glActiveTexture(GL_TEXTURE15);
				glBindTexture(GL_TEXTURE_2D, uid);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		}
		texture(texture&& rhs) noexcept :texture(rhs) { rhs.valid = false; }
		bool fail() const { return !valid; }
		inline unsigned id() const { return uid; }
		~texture() { if (valid) glDeleteTextures(1, &uid); }

		bool load(const char* path, bool minmap = true) const {
			int width, height, nr_channels;
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D, uid);
			stb_extension::stbi_set_flip_vertically_on_load(true);
			unsigned char* data = stb_extension::stbi_load(path, &width, &height, &nr_channels, 3);
			if (data) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				if (minmap) glGenerateMipmap(GL_TEXTURE_2D);
				return true;
			}
			std::cerr << "ERROR::TEXTURE::LOAD_FAILED" << std::endl;
			return false;
		}

		inline void bind(unsigned index) const { 
			if (index >= 15) std::cerr << "ERROR::TEXTURE::INVALID_BIND_INDEX" << std::endl;
			glActiveTexture(GL_TEXTURE0 + index);
			glBindTexture(GL_TEXTURE_2D, uid);
		}
	};

}