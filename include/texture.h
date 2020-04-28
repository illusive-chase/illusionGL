#pragma once
#include <stdio.h>
#include "basic.h"

namespace illusion {
	namespace stb_extension {
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
	}

	//封装了opengl texture
	class Texture {
	private:
		unsigned uid;
		bool valid;
		Texture(const Texture&) = default;

	public:
		Texture(unsigned wrap_method,
				unsigned min_filter = GL_LINEAR_MIPMAP_LINEAR,
				unsigned max_filter = GL_LINEAR) 
			:uid(~0), valid(false) 
		{
			glGenTextures(1, &uid);
			if (~uid) {
				valid = true;
				glActiveTexture(GL_TEXTURE15);
				glBindTexture(GL_TEXTURE_2D, uid);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_method);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_method);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, max_filter);
			}
		}
		Texture() :uid(~0), valid(false) {}
		Texture(Texture&& rhs) noexcept :Texture(rhs) { rhs.valid = false; }
		Texture& operator=(Texture&& rhs) noexcept {
			uid = rhs.uid;
			valid = rhs.valid;
			rhs.uid = ~0;
			rhs.valid = false;
			return *this;
		}
		bool fail() const { return !valid; }
		inline unsigned id() const { return uid; }
		~Texture() { if (valid) glDeleteTextures(1, &uid); }

		//读取实际数据
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
			std::cerr << "ERROR::TEXTURE::LOAD_FAILED " << path << std::endl;
			return false;
		}

		//绑定该texture
		inline void bind(unsigned index) const { 
			if (index >= 15) std::cerr << "ERROR::TEXTURE::INVALID_BIND_INDEX" << std::endl;
			glActiveTexture(GL_TEXTURE0 + index);
			glBindTexture(GL_TEXTURE_2D, uid);
		}
	};


	class CubeTexture {
	private:
		unsigned uid;
		bool valid;
		CubeTexture(const CubeTexture&) = default;

	public:
		CubeTexture() :uid(~0), valid(false) {}
		CubeTexture(unsigned wrap_method,
					unsigned min_filter = GL_LINEAR_MIPMAP_LINEAR,
					unsigned max_filter = GL_LINEAR) :uid(~0), valid(false) {
			glGenTextures(1, &uid);
			if (~uid) {
				valid = true;
				glActiveTexture(GL_TEXTURE15);
				glBindTexture(GL_TEXTURE_CUBE_MAP, uid);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_method);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_method);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_method);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, max_filter);
			}
		}
		CubeTexture(CubeTexture&& rhs) noexcept :CubeTexture(rhs) { rhs.valid = false; }
		CubeTexture& operator=(CubeTexture&& rhs) noexcept {
			uid = rhs.uid;
			valid = rhs.valid;
			rhs.uid = ~0;
			rhs.valid = false;
			return *this;
		}
		bool fail() const { return !valid; }
		inline unsigned id() const { return uid; }
		~CubeTexture() { if (valid) glDeleteTextures(1, &uid); }
		
		inline void bind(unsigned index) const {
			if (index >= 15) std::cerr << "ERROR::TEXTURE::INVALID_BIND_INDEX" << std::endl;
			glActiveTexture(GL_TEXTURE0 + index);
			glBindTexture(GL_TEXTURE_CUBE_MAP, uid);
		}
	};


	class FrameBuffer {
		CubeTexture tex;
		unsigned FBO;
	public:
		FrameBuffer() :FBO(~0) {}
		FrameBuffer(FrameBuffer&& rhs) noexcept : tex(std::move(rhs.tex)), FBO(rhs.FBO) {
			rhs.FBO = ~0;
		}
		FrameBuffer(int width, int height, FrameBuffer* depth, GLenum type) :tex(GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST), FBO(~0){
			for (int i = 0; i < 6; ++i)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, !depth ? GL_DEPTH_COMPONENT : GL_RGB,
							 width, height, 0, !depth ? GL_DEPTH_COMPONENT : GL_RGB, type, NULL);
			glGenFramebuffers(1, &FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			glFramebufferTexture(GL_FRAMEBUFFER, !depth ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, tex.id(), 0);
			if (!depth) glDrawBuffer(GL_NONE), glReadBuffer(GL_NONE);
			else glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth->tex.id(), 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		FrameBuffer& operator=(FrameBuffer&& rhs) noexcept {
			if (~FBO) glDeleteFramebuffers(1, &FBO);
			tex = std::move(rhs.tex);
			FBO = rhs.FBO;
			rhs.FBO = ~0;
			return *this;
		}
		~FrameBuffer() { if (~FBO) glDeleteFramebuffers(1, &FBO); }
		void use(int id) { glBindFramebuffer(GL_FRAMEBUFFER, FBO); tex.bind(id); }
	};

	class FullFrameBuffer {
		CubeTexture normal, depth, color, pos;
		unsigned FBO;
	public:
		FullFrameBuffer() :FBO(~0) {}
		FullFrameBuffer(FullFrameBuffer&& rhs) noexcept :
			normal(std::move(rhs.normal)),
			depth(std::move(rhs.depth)),
			color(std::move(rhs.color)),
			pos(std::move(rhs.pos)),
			FBO(rhs.FBO)
		{
			rhs.FBO = ~0;
		}
		FullFrameBuffer(int width, int height) :
			normal(GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST),
			depth(GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST),
			color(GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST),
			pos(GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST),
			FBO(~0)
		{
			depth.bind(0);
			for (int i = 0; i < 6; ++i)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			normal.bind(0);
			for (int i = 0; i < 6; ++i)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
			pos.bind(0);
			for (int i = 0; i < 6; ++i)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
			color.bind(0);
			for (int i = 0; i < 6; ++i)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glGenFramebuffers(1, &FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pos.id(), 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal.id(), 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, color.id(), 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth.id(), 0);
			unsigned attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, attachments);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		FullFrameBuffer& operator=(FullFrameBuffer&& rhs) noexcept {
			if (~FBO) glDeleteFramebuffers(1, &FBO);
			normal = std::move(rhs.normal);
			color = std::move(rhs.color);
			pos = std::move(rhs.pos);
			depth = std::move(rhs.depth);
			FBO = rhs.FBO;
			rhs.FBO = ~0;
			return *this;
		}
		~FullFrameBuffer() { if (~FBO) glDeleteFramebuffers(1, &FBO); }
		void use(int id_pos_normal_color_depth) {
			int id = id_pos_normal_color_depth;
			glBindFramebuffer(GL_FRAMEBUFFER, FBO); pos.bind(id); normal.bind(id + 1); color.bind(id + 2); depth.bind(id + 3);
		}
	};

}