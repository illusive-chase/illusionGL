#pragma once
#include "texture.h"
#include "shader.h"
#include <vector>
#include <string>

namespace illusion {

    struct vertex {
		glm::vec3 position, normal;
		glm::vec2 coord;
	};

	class mesh {
		texture* diffuse, *specular;
		unsigned VAO, VBO, EBO;
		glm::mat4 model;
		unsigned indice_num;

		mesh(const mesh&) = default;

	public:
		mesh(const std::vector<vertex>& vertices, const std::vector<unsigned>& indices, texture* diffuse, texture* specular, const glm::mat4& model)
			:diffuse(diffuse), specular(specular), VAO(~0), VBO(~0), EBO(~0), model(model), indice_num(indices.size()) {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
			
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
			
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
			
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, coord));

			glBindVertexArray(0);
		}

		mesh(mesh&& rhs) noexcept :mesh(rhs) {
			rhs.VAO = ~0;
			rhs.VBO = ~0;
			rhs.EBO = ~0;
		}

		~mesh() {
			if (~VAO) {
				glDeleteVertexArrays(1, &VAO);
				glDeleteBuffers(1, &VBO);
				glDeleteBuffers(1, &EBO);
			}
		}

		void draw(program& prog) {
			if (diffuse) diffuse->bind(0), prog.set("material.diffuse", 0);
			if (specular) specular->bind(1), prog.set("material.specular", 1);
			prog.set("model", model);
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indice_num, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	};


}