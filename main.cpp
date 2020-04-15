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
	
	constexpr int screen_width = World::width, screen_height = World::height;
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
	
	World& w = World::instance();
	if (w.fail()) return -1;
	//w.build_object(CubeBuilder(glm::vec3(0, 0, 0), 0.2f, "container2.png", "container2_specular.png"));
	w.build_model("C:/Users/illusion/Desktop/3dmax/robot/nanosuit.obj", 0.04f);
	//w.build_spot_light(CubeBuilder(glm::vec3(0.5f, 0.5f, 0.5f), 0.2f), SpotLight(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-1, -1, -1)));
	w.build_dir_light(DirLight(glm::vec3(-1, -1, -1)));
	w.mainloop(window);
	glfwTerminate();

	return 0;
}