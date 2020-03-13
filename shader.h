#pragma once
#include "basic.h"
#include "tmp.h"
#include <unordered_map>

namespace illusion {

	class shader {
	private:
		unsigned uid;
		bool valid;
	public:
		shader() :uid(~0), valid(false) {}
		shader(const shader& rhs) = delete;
		shader(GLenum type, const char* path) :uid(glCreateShader(type)), valid(false) {
			std::ifstream fr(path, std::ios::in);
			if (fr) {
				std::basic_string<GLchar> str = std::basic_string<GLchar>(std::istreambuf_iterator<GLchar>(fr),
																		  std::istreambuf_iterator<GLchar>());
				const GLchar* shader_source = str.c_str();
				glShaderSource(uid, 1, &shader_source, nullptr);
				fr.close();
				glCompileShader(uid);
				int success = 0;
				glGetShaderiv(uid, GL_COMPILE_STATUS, &success);
				valid = success;
				if (!valid) {
					char* info = new char[512];
					glGetShaderInfoLog(uid, 512, nullptr, info);
					std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << info << std::endl;
					delete[] info;
				}
			} else std::cerr << "ERROR::READER::FILE_READING_FAILED\n" << std::endl;
		}

		~shader() { if (valid) glDeleteShader(uid); }
		inline unsigned id() const { return uid; }
		bool fail() const { return !valid; }
	};

	class program {
	private:
		unsigned uid;
		bool valid;
		mutable std::unordered_map<const char*, GLint> mp;
		std::vector<float> vertices;
		std::vector<unsigned> indices;
		unsigned VAO, VBO, EBO;
		unsigned max_vertice_num;

		static unsigned& active() { static unsigned g_active = ~0; return g_active; }
		void load() {
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			vertices.clear();
			indices.clear();
		}
		inline void apply() const {
			if (active() != uid) {
				glUseProgram(uid);
				active() = uid;
			}
		}

	public:
		program() :uid(glCreateProgram()), valid(true), VAO(~0), VBO(~0), EBO(~0), max_vertice_num(0) {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);
		}
		program(const program&) = delete;
		~program() { 
			if (valid) glDeleteProgram(uid);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
			glDeleteVertexArrays(1, &VAO);
		}
		inline unsigned id() const { return uid; }
		bool fail() const { return !valid; }

		template<typename ...T>
		bool link(T&&... args) const {
			unsigned arg_list[sizeof...(args)];
			fill_with_return<unsigned, T&& ...>()(&shader::id, arg_list, args...);
			for (unsigned i = 0; i < sizeof...(args); ++i) glAttachShader(uid, arg_list[i]);
			int success = 0;
			glLinkProgram(uid);
			glGetProgramiv(uid, GL_LINK_STATUS, &success);
			if (!success) {
				char* info = new char[512];
				glGetProgramInfoLog(uid, 512, nullptr, info);
				std::cerr << "ERROR::PROGRAM::LINK_FAILED\n" << info << std::endl;
				delete[] info;
			}
			return success;
		}

		inline void paint() {
			apply();
			if (vertices.size()) load();
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, max_vertice_num, GL_UNSIGNED_INT, 0);
		}

		inline GLint get_uniform_location(const char* name) const {
			if (mp.count(name)) return mp[name];
			return mp[name] = glGetUniformLocation(uid, name);
		}

		inline void set(const char* name, int value) const {
			apply();
			glUniform1i(get_uniform_location(name), value);
		}
		inline void set(const char* name, float value) const {
			apply();
			glUniform1f(get_uniform_location(name), value);
		}
		inline void set(const char* name, const glm::vec3& value) const {
			apply();
			glUniform3fv(get_uniform_location(name), 1, glm::value_ptr(value));
		}
		inline void set(const char* name, const glm::mat4& value) const {
			apply();
			glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
		}

		void make_cube(float x, float y, float z, float length) {
			constexpr float cube_v[] = {
				0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
				0.5f, 0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
				0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
				0.5f, 0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
				0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
				0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
			   -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
				0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			   -0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
				0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
				0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			   -0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			   -0.5f, 0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
			   -0.5f, 0.5f, 0.5f,-1.0f, 0.0f, 0.0f,
			   -0.5f,-0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
			   -0.5f, 0.5f, 0.5f,-1.0f, 0.0f, 0.0f,
			   -0.5f,-0.5f, 0.5f,-1.0f, 0.0f, 0.0f,
			   -0.5f,-0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
				0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
			   -0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
				0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
			   -0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
			   -0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
				0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
				0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
				0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,
			   -0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
				0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,
			   -0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,
			   -0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
			   -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
			   -0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
			    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
			   -0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f
			};
			constexpr int cube_i[] = {
				0,1,2,3,4,5,
				6,7,8,9,10,11,
				12,13,14,15,16,17,
				18,19,20,21,22,23,
				24,25,26,27,28,29,
				30,31,32,33,34,35
			};
			vertices.insert(vertices.end(), cube_v, cube_v + 216);
			for (size_t i = vertices.size() - 216, len = vertices.size(); i < len; i += 6U) {
				vertices[i] = vertices[i] * length + x;
				vertices[i + 1U] = vertices[i + 1U] * length + y;
				vertices[i + 2U] = vertices[i + 2U] * length + z;
			}
			indices.insert(indices.end(), cube_i, cube_i + 36);
			max_vertice_num += 36;
		}
	};
}