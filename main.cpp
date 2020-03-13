#include <fstream>
#include "shader.h"
#include "texture.h"
using namespace illusion;

glm::vec3 camera_pos(0.0f, 0.0f, 0.3f),
camera_front(0.0f, 0.0f, -1.0f),
camera_up(0.0f, 1.0f, 0.0f);


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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

	camera_front = glm::normalize(glm::vec3(
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	));
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

int main() {
	std::ofstream fout("log.txt");
	std::cerr.rdbuf(fout.rdbuf());
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	constexpr int screen_width = 800, screen_height = 600;
	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 800, 600); 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	shader vertex_shader(GL_VERTEX_SHADER, "general.vs");
	shader fragment_shader(GL_FRAGMENT_SHADER, "object.fs");
	shader light_fragment_shader(GL_FRAGMENT_SHADER, "light.fs");
	
	if (vertex_shader.fail() || fragment_shader.fail() || light_fragment_shader.fail()) return -1;

	glm::mat4 model = glm::mat4(1.0f),
		projection = glm::perspective(glm::radians(45.0f), float(screen_width) / screen_height, 0.1f, 100.0f);

	program shader_program;
	if (!shader_program.link(vertex_shader, fragment_shader)) return -1;
	shader_program.make_cube(0, 0, 0, 0.5f);
	shader_program.set("projection", projection);
	shader_program.set("model", model);
	shader_program.set("material.ambient", glm::vec3(1.0f, 0.5f, 0.31f));
	shader_program.set("material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
	shader_program.set("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
	shader_program.set("material.shininess", 32.0f);
	shader_program.set("light.pos", glm::vec3(0.5f, 0.5f, 0.5f));
	shader_program.set("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader_program.set("light.diffuse", glm::vec3(0.2f, 0.2f, 0.2f));
	shader_program.set("light.specular", glm::vec3(1, 1, 1));
	

	program light_program;
	if (!light_program.link(vertex_shader, light_fragment_shader)) return -1;
	light_program.make_cube(0.5f, 0.5f, 0.5f, 0.2f);
	light_program.set("light_color", glm::vec3(1, 1, 1));
	light_program.set("projection", projection);
	light_program.set("model", model);


	/*
	texture tex;
	if (tex.fail() || !tex.load("texture.bmp")) return -1;
	tex.bind(0);
	shader_program.set("tex", 0);
	*/

	while (!glfwWindowShouldClose(window)) {
		process_input(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_program.set("view", glm::lookAt(camera_pos, camera_pos + camera_front, camera_up));
		shader_program.set("view_pos", camera_pos);
		shader_program.paint();

		light_program.set("view", glm::lookAt(camera_pos, camera_pos + camera_front, camera_up));
		light_program.paint();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}