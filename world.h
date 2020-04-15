#pragma once
#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "light.h"
#include <queue>

namespace illusion {

	class World {
		bool m_fail;
		Program object_prog;
		Program light_prog;
		glm::vec3 camera_pos, camera_front, camera_up;
		std::vector<Mesh> objects, point_lights, spot_lights;
		std::unordered_map<std::string, Texture> texture_map;

		World(int screen_width, int screen_height)
			:m_fail(false), object_prog(), light_prog(), camera_pos(glm::vec3(-0.3f, 0.0f, 0.0f)),
			camera_front(glm::vec3(1.0f, 0.0f, 0.0f)), camera_up(glm::vec3(0.0f, 1.0f, 0.0f))
		{
			Shader vertex_shader(GL_VERTEX_SHADER, "general.vs");
			Shader fragment_shader(GL_FRAGMENT_SHADER, "object.fs");
			Shader light_fragment_shader(GL_FRAGMENT_SHADER, "light.fs");
			m_fail = vertex_shader.fail() || fragment_shader.fail()
				|| light_fragment_shader.fail() || !object_prog.link(vertex_shader, fragment_shader)
				|| !light_prog.link(vertex_shader, light_fragment_shader);
			glm::mat4 model = glm::mat4(1.0f),
				projection = glm::perspective(glm::radians(45.0f), float(screen_width) / screen_height, 0.1f, 100.0f);
			object_prog.set("projection", projection);
			object_prog.set("material.shininess", 32.0f);
			object_prog.set("dir_light_num", 0);
			object_prog.set("point_light_num", 0);
			object_prog.set("spot_light_num", 0);
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

		Texture* build_texture(const char* name) {
			if (texture_map.count(name) == 0) texture_map[name].load(name);
			return &texture_map[name];
		}

	public:
		static constexpr int width = 800;
		static constexpr int height = 600;
		static World& instance() { static World w(width, height); return w; }
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
				for (auto& it : point_lights) it.draw(light_prog);
				for (auto& it : spot_lights) it.draw(light_prog);

				glfwSwapBuffers(window);
				glfwPollEvents();
			}
		}

		template<typename T>
		void build_object(T&& builder) {
			builder.build();
			objects.emplace_back(Mesh(builder.vertices, builder.indices,
									  build_texture(builder.diffuse), build_texture(builder.specular),
									  builder.model));
		}

		template<typename T>
		void build_point_light(T&& builder, const PointLight& light) {
			builder.build();
			std::string prefix = "point_lights[" + std::to_string(point_lights.size()) + "].";
			point_lights.emplace_back(Mesh(builder.vertices, builder.indices, nullptr, nullptr, builder.model));
			object_prog.set((prefix + "pos").c_str(), light.pos);
			object_prog.set((prefix + "ambient").c_str(), light.ambient);
			object_prog.set((prefix + "diffuse").c_str(), light.diffuse);
			object_prog.set((prefix + "specular").c_str(), light.specular);
			object_prog.set((prefix + "constant").c_str(), light.constant);
			object_prog.set((prefix + "linear").c_str(), light.linear);
			object_prog.set((prefix + "quadratic").c_str(), light.quadratic);
			object_prog.set("point_light_num", (int)point_lights.size());
		}

		template<typename T>
		void build_spot_light(T&& builder, const SpotLight& light) {
			builder.build();
			std::string prefix = "spot_lights[" + std::to_string(spot_lights.size()) + "].";
			spot_lights.emplace_back(Mesh(builder.vertices, builder.indices, nullptr, nullptr, builder.model));
			object_prog.set((prefix + "pos").c_str(), light.pos);
			object_prog.set((prefix + "dir").c_str(), light.dir);
			object_prog.set((prefix + "ambient").c_str(), light.ambient);
			object_prog.set((prefix + "diffuse").c_str(), light.diffuse);
			object_prog.set((prefix + "specular").c_str(), light.specular);
			object_prog.set((prefix + "cut_off").c_str(), light.cut_off);
			object_prog.set((prefix + "outer_cut_off").c_str(), light.outer_cut_off);
			object_prog.set("spot_light_num", (int)spot_lights.size());
		}

		void build_dir_light(const DirLight& light) {
			object_prog.set("dir_light.dir", light.dir);
			object_prog.set("dir_light.ambient", light.ambient);
			object_prog.set("dir_light.diffuse", light.diffuse);
			object_prog.set("dir_light.specular", light.specular);
			object_prog.set("dir_light_num", 1);
		}

		void build_model(const std::string& path, float scale = 1.0f) {
			glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

			std::string directory = path.substr(0, path.find_last_of('/'));
			Assimp::Importer importer;
			std::vector<Mesh> ret;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
				return;
			}
			std::queue<aiNode*> q;
			q.push(scene->mRootNode);
			while (!q.empty()) {
				aiNode* node = q.front();
				for (unsigned i = 0; i < node->mNumMeshes; ++i) {
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					std::vector<Vertex> vertices;
					std::vector<unsigned> indices;
					for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
						vertices.push_back(Vertex{
							glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
							glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
							(mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0]->x, mesh->mTextureCoords[0]->x) : glm::vec2(0, 0))
						});
					}
					for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
						aiFace face = mesh->mFaces[i];
						for (unsigned int j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
					}
					
					aiString diff, spec;
					const char* diffuse = nullptr;
					const char* specular = nullptr;
					if (mesh->mMaterialIndex >= 0) {
						aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
						if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
							aiString temp;
							material->GetTexture(aiTextureType_DIFFUSE, 0, &temp);
							diff = aiString(directory + '/' + temp.C_Str());
							diffuse = diff.C_Str();
						}
						if (material->GetTextureCount(aiTextureType_SPECULAR)) {
							aiString temp;
							material->GetTexture(aiTextureType_SPECULAR, 0, &temp);
							spec = aiString(directory + '/' + temp.C_Str());
							specular = spec.C_Str();
						}
					}
					objects.emplace_back(Mesh(vertices, indices, build_texture(diff.C_Str()), build_texture(spec.C_Str()), model));
				}
				for (unsigned i = 0; i < node->mNumChildren; ++i) q.push(node->mChildren[i]);
				q.pop();
			}
		}

		bool fail() const { return m_fail; }
	};

}