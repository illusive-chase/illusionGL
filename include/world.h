#pragma once
#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "light.h"
#include <queue>

namespace illusion {

	//世界类，负责管理整个窗口的渲染和交互
	class World {
	public:
		enum class Mode { NO_SHADOW, NORMAL_SHADOW, REFLECTIVE_SHADOW };

	private:
		const Mode mode;
		bool m_fail; //是否正常
		Program object_prog; //绘制物体的program
		Program light_prog;
		Program help_prog;
		glm::vec3 camera_pos, camera_front, camera_up; //相机的位置、朝向和正上方向量
		std::vector<Mesh> objects, point_lights, spot_lights; //需要渲染的物体对象、点光源对象、聚光灯对象
		std::unordered_map<std::string, Texture> texture_map; //防止同名texture的重复加载

		FrameBuffer depth;
		FullFrameBuffer rsm_buf;

		World(int screen_width, int screen_height, Mode mode = Mode::NO_SHADOW)
			:mode(mode), m_fail(false), camera_pos(glm::vec3(-0.3f, 0.0f, 0.0f)),
			camera_front(glm::vec3(1.0f, 0.0f, 0.0f)), camera_up(glm::vec3(0.0f, 1.0f, 0.0f))
		{

			if (mode == Mode::NORMAL_SHADOW) {
				Shader vertex_shader(GL_VERTEX_SHADER, "./shader/general_shadow.vs");
				Shader fragment_shader(GL_FRAGMENT_SHADER, "./shader/object_shadow.fs");
				Shader light_vertex_shader(GL_VERTEX_SHADER, "./shader/general.vs");
				Shader light_fragment_shader(GL_FRAGMENT_SHADER, "./shader/light.fs");
				m_fail = vertex_shader.fail() || fragment_shader.fail() || light_vertex_shader.fail()
					|| light_fragment_shader.fail() || !object_prog.link(vertex_shader, fragment_shader)
					|| !light_prog.link(light_vertex_shader, light_fragment_shader);
				depth = FrameBuffer(shadow_width, shadow_height, nullptr, GL_FLOAT);
			} else if (mode == Mode::REFLECTIVE_SHADOW) {
				Shader vertex_shader(GL_VERTEX_SHADER, "./shader/general_shadow.vs");
				Shader fragment_shader(GL_FRAGMENT_SHADER, "./shader/object_rsm.fs");
				Shader light_vertex_shader(GL_VERTEX_SHADER, "./shader/general.vs");
				Shader light_fragment_shader(GL_FRAGMENT_SHADER, "./shader/light.fs");
				m_fail = vertex_shader.fail() || fragment_shader.fail() || light_vertex_shader.fail()
					|| light_fragment_shader.fail() || !object_prog.link(vertex_shader, fragment_shader)
					|| !light_prog.link(light_vertex_shader, light_fragment_shader);
				rsm_buf = FullFrameBuffer(shadow_width, shadow_height);
			} else {
				Shader vertex_shader(GL_VERTEX_SHADER, "./shader/general.vs");
				Shader fragment_shader(GL_FRAGMENT_SHADER, "./shader/object.fs");
				Shader light_vertex_shader(GL_VERTEX_SHADER, "./shader/general.vs");
				Shader light_fragment_shader(GL_FRAGMENT_SHADER, "./shader/light.fs");
				m_fail = vertex_shader.fail() || fragment_shader.fail() || light_vertex_shader.fail()
					|| light_fragment_shader.fail() || !object_prog.link(vertex_shader, fragment_shader)
					|| !light_prog.link(light_vertex_shader, light_fragment_shader);
			}
			
			
			glm::mat4 model = glm::mat4(1.0f),
				projection = glm::perspective(glm::radians(45.0f), float(screen_width) / screen_height, 0.1f, 100.0f);
			object_prog.set("projection", projection);
			object_prog.set("material.shininess", 32.0f);
			object_prog.set("dir_light_num", 0);
			object_prog.set("point_light_num", 0);
			object_prog.set("spot_light_num", 0);
			light_prog.set("projection", projection);
			light_prog.set("light_color", glm::vec3(1, 1, 1));
			
			if (mode == Mode::REFLECTIVE_SHADOW) {
				Shader rsm_vs(GL_VERTEX_SHADER, "./shader/rsm.vs");
				Shader rsm_fs(GL_FRAGMENT_SHADER, "./shader/rsm.fs");
				Shader rsm_gs(GL_GEOMETRY_SHADER, "./shader/rsm.gs");
				m_fail = m_fail || rsm_vs.fail() || rsm_gs.fail() || rsm_fs.fail() || !help_prog.link(rsm_vs, rsm_fs, rsm_gs);
				object_prog.set("indirect_map", 13);
				object_prog.set("normal_map", 12);
				object_prog.set("pos_map", 11);
				object_prog.set("depth_map", 14);
			} else if (mode == Mode::NORMAL_SHADOW) {
				Shader depth_vs(GL_VERTEX_SHADER, "./shader/shadow.vs");
				Shader depth_fs(GL_FRAGMENT_SHADER, "./shader/shadow.fs");
				Shader depth_gs(GL_GEOMETRY_SHADER, "./shader/shadow.gs");
				m_fail = m_fail || depth_fs.fail() || depth_vs.fail() || depth_gs.fail() || !help_prog.link(depth_vs, depth_fs, depth_gs);
				object_prog.set("depth_map", 14);
			}
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
			if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
				auto p = get_camera();
				for (int i = 0; i < 3; ++i) std::cout << p.first[i] << ' ';
				std::cout << std::endl;
				for (int i = 0; i < 3; ++i) std::cout << p.second[i] << ' ';
				std::cout << std::endl;
			}
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

		//根据路径构造texture
		Texture* build_texture(const char* name) {
			std::string str(name);
			if (texture_map.count(str) == 0) {
				Texture temp(GL_REPEAT);
				temp.load(str.c_str());
				texture_map[str] = std::move(temp);
			}
			return &texture_map[str];
		}

	public:
		static constexpr int width = 800, height = 600;
		static constexpr int shadow_width = 512, shadow_height = 512;
		static World& instance(Mode mode = Mode::NO_SHADOW) { static World w(width, height, mode); return w; }

		void set_camera(const glm::vec3& from, const glm::vec3& lookat) {
			camera_pos = from;
			camera_front = glm::normalize(lookat - from);
		}

		std::pair<glm::vec3, glm::vec3> get_camera() const {
			return std::make_pair(camera_pos, camera_pos + camera_front);
		}

		void mainloop(GLFWwindow* window, int times = -1) {
			glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
			if (times < 0) glfwSetCursorPosCallback(window, mouse_callback);
			
			if (mode == Mode::NORMAL_SHADOW || mode == Mode::REFLECTIVE_SHADOW) {
				glm::vec3 light_pos = object_prog.get<glm::vec3>("point_lights[0].pos");
				glm::mat4 light_projection;
				glm::mat4 shadow_matrices[6];
				constexpr float far_plane = 25.0f;
				light_projection = glm::perspective(glm::radians(90.0f), float(shadow_width) / shadow_height, 0.1f, far_plane);
				shadow_matrices[0] = light_projection * glm::lookAt(light_pos, light_pos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
				shadow_matrices[1] = light_projection * glm::lookAt(light_pos, light_pos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
				shadow_matrices[2] = light_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
				shadow_matrices[3] = light_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
				shadow_matrices[4] = light_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
				shadow_matrices[5] = light_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
				for (int i = 0; i < 6; ++i) {
					std::string str = ("shadow_matrices[" + std::to_string(i) + "]");
					help_prog.set(str.c_str(), shadow_matrices[i]);
				}
				help_prog.set("far_plane", far_plane);
				object_prog.set("far_plane", far_plane);
				help_prog.set("light.pos", light_pos);
			}
			
			while (!glfwWindowShouldClose(window)) {
				process_input(window);
				
				if (times) {
					if (mode == Mode::NORMAL_SHADOW) {
						glViewport(0, 0, shadow_width, shadow_height);
						depth.use(14);
						glClear(GL_DEPTH_BUFFER_BIT);
						for (auto& it : objects) it.draw(help_prog);
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						glViewport(0, 0, width, height);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					} else if (mode == Mode::REFLECTIVE_SHADOW) {
						glViewport(0, 0, shadow_width, shadow_height);
						rsm_buf.use(11);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						for (auto& it : objects) it.draw(help_prog);
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						glViewport(0, 0, width, height);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					} else {
						glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					}

					object_prog.set("view", glm::lookAt(camera_pos, camera_pos + camera_front, camera_up));
					object_prog.set("view_pos", camera_pos);
					for (auto& it : objects) it.draw(object_prog);

					light_prog.set("view", glm::lookAt(camera_pos, camera_pos + camera_front, camera_up));
					for (auto& it : point_lights) it.draw(light_prog);
					for (auto& it : spot_lights) it.draw(light_prog);

					glfwSwapBuffers(window);
					if (times > 0) --times;
				}
				glfwPollEvents();
			}
		}

		//根据builder构造物体对象
		template<typename T>
		void build_object(T&& builder) {
			builder.build();
			objects.emplace_back(Mesh(builder.vertices, builder.indices,
									  build_texture(builder.diffuse), build_texture(builder.specular),
									  builder.model));
		}

		//根据builder构造点光源
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
			if (mode == Mode::REFLECTIVE_SHADOW) {
				help_prog.set("light.diffuse", light.diffuse);
				help_prog.set("light.constant", light.constant);
				help_prog.set("light.linear", light.linear);
				help_prog.set("light.quadratic", light.quadratic);
			}
		}

		//根据builder构造聚光灯
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

		//构造平行光
		void build_dir_light(const DirLight& light) {
			object_prog.set("dir_light.dir", light.dir);
			object_prog.set("dir_light.ambient", light.ambient);
			object_prog.set("dir_light.diffuse", light.diffuse);
			object_prog.set("dir_light.specular", light.specular);
			object_prog.set("dir_light_num", 1);
		}

		//构造外部模型
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
					objects.emplace_back(Mesh(vertices, indices, 
											  diffuse ? build_texture(diffuse) : nullptr, 
											  specular ? build_texture(specular) : nullptr, model));
				}
				for (unsigned i = 0; i < node->mNumChildren; ++i) q.push(node->mChildren[i]);
				q.pop();
			}
		}

		bool fail() const { return m_fail; }
	};

}