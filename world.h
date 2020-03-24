#pragma once
#include "shader.h"
#include "texture.h"
#include "mesh.h"

namespace illusion {

	class world {
		bool m_fail;
		program object_prog;
		program light_prog;
		glm::vec3 camera_pos, camera_front, camera_up;
		std::vector<mesh> objects, lights;
		std::unordered_map<std::string, texture> texture_map;

		world(int screen_width, int screen_height)
			:m_fail(false), object_prog(), light_prog(), camera_pos(glm::vec3(-0.3f, 0.0f, 0.0f)),
			camera_front(glm::vec3(1.0f, 0.0f, 0.0f)), camera_up(glm::vec3(0.0f, 1.0f, 0.0f))
		{
			shader vertex_shader(GL_VERTEX_SHADER, "general.vs");
			shader fragment_shader(GL_FRAGMENT_SHADER, "object.fs");
			shader light_fragment_shader(GL_FRAGMENT_SHADER, "light.fs");
			m_fail = vertex_shader.fail() || fragment_shader.fail()
				|| light_fragment_shader.fail() || !object_prog.link(vertex_shader, fragment_shader)
				|| !light_prog.link(vertex_shader, light_fragment_shader);
			glm::mat4 model = glm::mat4(1.0f),
				projection = glm::perspective(glm::radians(45.0f), float(screen_width) / screen_height, 0.1f, 100.0f);
			object_prog.set("projection", projection);
			object_prog.set("material.shininess", 32.0f);
			object_prog.set("light.pos", glm::vec3(0.5f, 0.5f, 0.5f));
			object_prog.set("light.ambient", glm::vec3(0.4f, 0.4f, 0.4f));
			object_prog.set("light.diffuse", glm::vec3(1, 1, 1));
			object_prog.set("light.specular", glm::vec3(1, 1, 1));
			light_prog.set("projection", projection);
			light_prog.set("light_color", glm::vec3(1, 1, 1));
		}

		void process_input(GLFWwindow* window) {
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, true);
			float camera_speed = get_delta_time();
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				glm::vec3 temp = camera_speed * camera_front;
				temp.y = 0;
				camera_pos += temp;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				glm::vec3 temp = camera_speed * camera_front;
				temp.y = 0;
				camera_pos -= temp;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				glm::vec3 temp = glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
				temp.y = 0;
				camera_pos -= temp;
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				glm::vec3 temp = glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
				temp.y = 0;
				camera_pos += temp;
			}
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				camera_pos += camera_speed * camera_up;
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
				camera_pos -= camera_speed * camera_up;
		}

		static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
			glViewport(0, 0, width, height);
		}

		static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
			static float last_x = (float)xpos, last_y = (float)ypos;
			static float yaw = 0.0f, pitch = 0.0f;
			float xoffset = (float)xpos - last_x;
			float yoffset = last_y - (float)ypos;
			last_x = (float)xpos;
			last_y = (float)ypos;

			float sensitivity = 0.05f;
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			yaw += xoffset;
			pitch += yoffset;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			instance().camera_front = glm::normalize(glm::vec3(
				cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
				sin(glm::radians(pitch)),
				sin(glm::radians(yaw)) * cos(glm::radians(pitch))
			));
		}

		texture* build_texture(const char* name) {
			if (texture_map.count(name) == 0) texture_map[name].load(name);
			return &texture_map[name];
		}

	public:
		static constexpr int width = 800;
		static constexpr int height = 600;
		static world& instance() { static world w(width, height); return w; }
		void mainloop(GLFWwindow* window) {
			glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
			glfwSetCursorPosCallback(window, mouse_callback);
			while (!glfwWindowShouldClose(window)) {
				process_input(window);
				glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				object_prog.set("view", glm::lookAt(camera_pos, camera_pos + camera_front, camera_up));
				object_prog.set("view_pos", camera_pos);
				for (auto& it : objects) it.draw(object_prog);

				light_prog.set("view", glm::lookAt(camera_pos, camera_pos + camera_front, camera_up));
				for (auto& it : lights) it.draw(light_prog);

				glfwSwapBuffers(window);
				glfwPollEvents();
			}
		}

		template<typename T>
		void build_object(T&& builder) {
			builder.build();
			objects.emplace_back(mesh(builder.vertices, builder.indices,
									  build_texture(builder.diffuse), build_texture(builder.specular),
									  builder.model));
		}

		template<typename T>
		void build_light(T&& builder) {
			builder.build();
			lights.emplace_back(mesh(builder.vertices, builder.indices, nullptr, nullptr, builder.model));
		}

		bool fail() const { return m_fail; }
	};

}