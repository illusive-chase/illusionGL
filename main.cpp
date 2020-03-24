#include <fstream>
#include "world.h"
#include "builder.h"
using namespace illusion;

int main() {
	std::ofstream fout("log.txt");
	std::cerr.rdbuf(fout.rdbuf());
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	constexpr int screen_width = world::width, screen_height = world::height;
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
	glViewport(0, 0, screen_width, screen_height);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	world& w = world::instance();
	if (w.fail()) return -1;
	w.build_object(cube_builder(glm::vec3(0, 0, 0), 0.2f, "container2.png", "container2_specular.png"));
	w.build_light(cube_builder(glm::vec3(0.5f, 0.5f, 0.5f), 0.2f));
	w.mainloop(window);
	glfwTerminate();

	return 0;
}