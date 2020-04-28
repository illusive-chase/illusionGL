#pragma once
#include <glm/glm.hpp>

namespace illusion {

	//点光源
	struct PointLight {
		glm::vec3 pos;
		float constant; //衰减的常数项
		float linear; //衰减的线性项
		float quadratic; //衰减的二次项
		glm::vec3 ambient; //环境光颜色
		glm::vec3 diffuse; //漫反射光颜色
		glm::vec3 specular; //镜面反射光颜色

		PointLight(const glm::vec3& pos) :pos(pos), constant(1.0f), linear(0.09f), quadratic(0.032f),
			ambient(glm::vec3(0.3f, 0.3f, 0.3f)), diffuse(glm::vec3(1, 1, 1)), specular(glm::vec3(1, 1, 1)) {}
	};

	//平行光
	struct DirLight {
		glm::vec3 dir;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		DirLight(const glm::vec3& dir) :dir(dir),
			ambient(glm::vec3(0.3f, 0.3f, 0.3f)), diffuse(glm::vec3(1, 1, 1)), specular(glm::vec3(1, 1, 1)) {}
	};

	//聚光灯
	struct SpotLight {
		glm::vec3 pos;
		glm::vec3 dir;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		//聚光灯的内外切角的余弦值
		float cut_off;
		float outer_cut_off;
		SpotLight(const glm::vec3& pos, const glm::vec3& dir) :pos(pos), dir(dir), cut_off(0.976296f), outer_cut_off(0.953717f),
			ambient(glm::vec3(0.3f, 0.3f, 0.3f)), diffuse(glm::vec3(1, 1, 1)), specular(glm::vec3(1, 1, 1)) {}
	};
}