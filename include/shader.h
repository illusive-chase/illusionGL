#pragma once
#include "basic.h"
#include "tmp.h"
#include <unordered_map>

namespace illusion {

	//封装了opengl shader
	class Shader {
	private:
		unsigned uid;
		bool valid;
	public:
		Shader() :uid(~0), valid(false) {}
		Shader(const Shader& rhs) = delete;
		Shader(GLenum type, const char* path) :uid(glCreateShader(type)), valid(false) {
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
					std::cerr << "ERROR::SHADER::COMPILATION_FAILED  " << path << std::endl << info << std::endl;
					delete[] info;
				}
			} else std::cerr << "ERROR::READER::FILE_READING_FAILED  " << path << std::endl;
		}

		~Shader() { if (valid) glDeleteShader(uid); }
		inline unsigned id() const { return uid; }
		bool fail() const { return !valid; }
	};


	//封装了opengl program
	class Program {
	private:
		unsigned uid;
		bool valid;
		mutable std::unordered_map<std::string, GLint> mp; //保存uniform变量的位置

		//保存当前在使用的program
		static unsigned& active() { static unsigned g_active = ~0; return g_active; }
		

	public:
		Program() :uid(glCreateProgram()), valid(true) {}
		Program(const Program&) = delete;
		~Program() { if (valid) glDeleteProgram(uid); }
		inline unsigned id() const { return uid; }
		bool fail() const { return !valid; }

		//对多个shader进行链接和编译
		template<typename ...T>
		bool link(T&&... args) const {
			unsigned arg_list[sizeof...(args)];
			fill_with_return<unsigned, T&& ...>()(&Shader::id, arg_list, args...);
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

		//使用该program
		inline void apply() const {
			if (active() != uid) {
				glUseProgram(uid);
				active() = uid;
			}
		}

		//获取变量位置
		inline GLint get_uniform_location(const char* name) const {
			if (mp.count(name)) return mp[name];
			int ret = glGetUniformLocation(uid, name);
			mp[name] = ret;
			if (ret < 0) std::cerr << "ERROR::PROGRAM::INVALID_NAME " << name << std::endl;
			return ret;
		}

		//设置和获取变量值

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
		template<typename T> T get(const char* name) const {
			T ret;
			glGetUniformfv(uid, get_uniform_location(name), glm::value_ptr(ret));
			return ret;
		}
		template<> int get<int>(const char* name) const {
			int ret;
			glGetUniformiv(uid, get_uniform_location(name), &ret);
			return ret;
		}
	
	};
}