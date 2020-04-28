#pragma once
#include "mesh.h"

namespace illusion {

	//������builder�࣬builder�����ڹ���ģ�͵Ķ��㡢����������������
	struct BasicBuilder {
		std::vector<Vertex> vertices; //����
		std::vector<unsigned> indices; //��������
		const char* diffuse, * specular; //��������·��
		glm::mat4 model; //�任����
		BasicBuilder(const char* diffuse, const char* specular)
			:diffuse(diffuse), specular(specular), model(glm::mat4(1.0f)) {}
		virtual void build() = 0;
		void rotate(int degree, const glm::vec3& axis) {
			model = glm::rotate(model, degree * glm::pi<float>() / 180.0f, axis);
		}
	};

	//��builder���������κ�ģ��
	struct NoneBuilder : public BasicBuilder {
		NoneBuilder() :BasicBuilder(nullptr, nullptr) {}
		void build() {}
	};

	//ƽ��builder
	struct PlaneBuilder : public BasicBuilder {
		PlaneBuilder(const glm::vec3& center, float size, const char* diffuse = nullptr) :BasicBuilder(diffuse, diffuse) {
			model = glm::translate(glm::scale(model, glm::vec3(size)), center / size);
		}
		PlaneBuilder(const glm::vec3& center, float size, const char* diffuse, const char* specular) :BasicBuilder(diffuse, specular) {
			model = glm::translate(glm::scale(model, glm::vec3(size)), center / size);
		}

		void build() {
			indices = { 0,1,2,0,2,3 };
			constexpr float cube_v[] = {
				0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.0f,-0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			   -0.5f, 0.0f,-0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			   -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			};
			vertices = std::vector<Vertex>(reinterpret_cast<const Vertex*>(cube_v),
										   reinterpret_cast<const Vertex*>(cube_v + (sizeof(cube_v) / sizeof(float))));
		}
	};

	//������builder
	struct CubeBuilder : public BasicBuilder {
		CubeBuilder(const glm::vec3& pos, float size, const char* diffuse = nullptr) :BasicBuilder(diffuse, diffuse) {
			model = glm::translate(glm::scale(model, glm::vec3(size)), pos / size);
		}
		CubeBuilder(const glm::vec3& pos, float size, const char* diffuse, const char* specular) : BasicBuilder(diffuse, specular) {
			model = glm::translate(glm::scale(model, glm::vec3(size)), pos / size);
		}

		void build() {
			indices.resize(36U);
			for (unsigned i = 0; i < 36U; ++i) indices[i] = i;
			constexpr float cube_v[] = {
				// positions          // normals           // texture coords
				// Back face
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
				 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // bottom-right         
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // top-left
				// Front face
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom-left
				 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // bottom-right
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // top-right
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // top-right
				-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // top-left
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom-left
				// Left face
				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
				-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-left
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
				-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-right
				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
				// Right face
				 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
				 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
				 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-right         
				 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
				 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
				 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-left     
				// Bottom face
				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
				 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // top-left
				 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
				 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
				-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // bottom-right
				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
				// Top face
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
				 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // top-right     
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f  // bottom-left        
			};
			vertices = std::vector<Vertex>(reinterpret_cast<const Vertex*>(cube_v), 
										   reinterpret_cast<const Vertex*>(cube_v + (sizeof(cube_v) / sizeof(float))));
		}
	};

	struct RoomBuilder : public BasicBuilder {
		RoomBuilder(const glm::vec3& pos, float size, const char* diffuse = nullptr) :BasicBuilder(diffuse, diffuse) {
			model = glm::translate(glm::scale(model, glm::vec3(size)), pos / size);
		}
		RoomBuilder(const glm::vec3& pos, float size, const char* diffuse, const char* specular) : BasicBuilder(diffuse, specular) {
			model = glm::translate(glm::scale(model, glm::vec3(size)), pos / size);
		}

		void build() {
			indices.resize(36U);
			for (unsigned i = 0; i < 36U; ++i) indices[i] = i;
			constexpr float cube_v[] = {
				// positions          // normals           // texture coords
				// Back face
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // Bottom-left
				 0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // bottom-right         
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // top-right
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // top-right
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // top-left
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom-left
				// Front face
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
				 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // bottom-right
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
				-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // top-left
				// Left face
				-0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
				-0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
				-0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-left
				-0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
				-0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
				-0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-right
				// Right face
				 0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
				 0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-right         
				 0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
				 0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
				 0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-left     
				 0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
				// Bottom face
				-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-right
				 0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
				 0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // top-left
				 0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
				-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-right
				-0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, // bottom-right
				// Top face
				-0.5f,  0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-left
				 0.5f,  0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // top-right     
				 0.5f,  0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
				 0.5f,  0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
				-0.5f,  0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  // bottom-left        
				-0.5f,  0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f // top-left
			};
			vertices = std::vector<Vertex>(reinterpret_cast<const Vertex*>(cube_v),
										   reinterpret_cast<const Vertex*>(cube_v + (sizeof(cube_v) / sizeof(float))));
		}
	};

	
}