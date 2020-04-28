#pragma once
#include "texture.h"
#include "shader.h"
#include <vector>
#include <string>

namespace illusion {

    struct Vertex {
		glm::vec3 position, normal; //λ�á�������
		glm::vec2 coord; //��������
	};

	//mesh�࣬��һ����������Ⱦ����
	class Mesh {
		Texture* diffuse, *specular;
		unsigned VAO, VBO, EBO; //ÿ��meshά��һ�׻���
		glm::mat4 model; //ÿ��meshά��һ���任����
		unsigned indice_num; //���㣨��������

		Mesh(const Mesh&) = default; //��ֹ����

	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, Texture* diffuse, Texture* specular, const glm::mat4& model)
			:diffuse(diffuse), specular(specular), VAO(~0), VBO(~0), EBO(~0), model(model), indice_num(indices.size())
		{
			//�ǿ�
			if (indice_num) {
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);
				glGenBuffers(1, &EBO);

				glBindVertexArray(VAO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);

				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, coord));

				glBindVertexArray(0);
			}
		}

		Mesh(Mesh&& rhs) noexcept :Mesh(rhs) {
			rhs.VAO = ~0;
			rhs.VBO = ~0;
			rhs.EBO = ~0;
		}

		~Mesh() {
			if (~VAO) {
				glDeleteVertexArrays(1, &VAO);
				glDeleteBuffers(1, &VBO);
				glDeleteBuffers(1, &EBO);
			}
		}

		//ʵ�ʵĻ��ƺ���
		void draw(Program& prog) {
			if (indice_num) {
				if (diffuse) diffuse->bind(0), prog.set("material.diffuse", 0);
				if (specular) specular->bind(1), prog.set("material.specular", 1);
				prog.set("model", model);
				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, indice_num, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}
	};


}