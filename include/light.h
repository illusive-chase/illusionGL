#pragma once
#include <glm/glm.hpp>

namespace illusion {

	//���Դ
	struct PointLight {
		glm::vec3 pos;
		float constant; //˥���ĳ�����
		float linear; //˥����������
		float quadratic; //˥���Ķ�����
		glm::vec3 ambient; //��������ɫ
		glm::vec3 diffuse; //���������ɫ
		glm::vec3 specular; //���淴�����ɫ

		PointLight(const glm::vec3& pos) :pos(pos), constant(1.0f), linear(0.09f), quadratic(0.032f),
			ambient(glm::vec3(0.3f, 0.3f, 0.3f)), diffuse(glm::vec3(1, 1, 1)), specular(glm::vec3(1, 1, 1)) {}
	};

	//ƽ�й�
	struct DirLight {
		glm::vec3 dir;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		DirLight(const glm::vec3& dir) :dir(dir),
			ambient(glm::vec3(0.3f, 0.3f, 0.3f)), diffuse(glm::vec3(1, 1, 1)), specular(glm::vec3(1, 1, 1)) {}
	};

	//�۹��
	struct SpotLight {
		glm::vec3 pos;
		glm::vec3 dir;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		//�۹�Ƶ������нǵ�����ֵ
		float cut_off;
		float outer_cut_off;
		SpotLight(const glm::vec3& pos, const glm::vec3& dir) :pos(pos), dir(dir), cut_off(0.976296f), outer_cut_off(0.953717f),
			ambient(glm::vec3(0.3f, 0.3f, 0.3f)), diffuse(glm::vec3(1, 1, 1)), specular(glm::vec3(1, 1, 1)) {}
	};
}