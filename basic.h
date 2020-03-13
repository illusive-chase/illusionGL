#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace illusion {
	float get_delta_time() {
		static float last = 0.0f;
		float temp = last;
		last = (float)glfwGetTime();
		return last - temp;
	}
}