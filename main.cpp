#include <fstream>
#include "world.h"
#include "builder.h"
using namespace illusion;

void error_callback(int, const char* msg) {
	std::cerr << msg << std::endl;
}

int main() {
	// 错误信息输出到log里
	std::ofstream fout("log.txt");
	std::cerr.rdbuf(fout.rdbuf());
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	//创建窗口
	constexpr int screen_width = World::width, screen_height = World::height;
	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "RSM", NULL, NULL);
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

#ifdef _DEBUG

	glfwSetErrorCallback(error_callback);

#endif

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glViewport(0, 0, screen_width, screen_height);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//创建世界
	World& w = World::instance(World::Mode::REFLECTIVE_SHADOW);
	if (w.fail()) return -1;

	//w.set_camera(glm::vec3(0.955841, 0.52701, 0.284357), glm::vec3(-0.0140141, 0.326787, 0.145462));

	//在原点创建模型
	w.build_model("./assets/robot/nanosuit.obj", 0.04f);

	//创建平面对象
	//w.build_object(PlaneBuilder(glm::vec3(0, 0, 0), 1.0f, "./assets/container2.png", "./assets/container2_specular.png"));
	//w.build_object(RoomBuilder(glm::vec3(0.0f, 0.5f, 0.0f), 1.0f, "./assets/container2.png", "./assets/container2_specular.png"));
	//w.build_object(RoomBuilder(glm::vec3(0.0f, 0.5f, 0.0f), 1.0f, "./assets/green.png"));
	PlaneBuilder bd(glm::vec3(-0.5f, 0.5f, 0.0f), 1.1f, "./assets/green.png");
	bd.rotate(-90, glm::vec3(0, 0, 1));
	w.build_object(std::move(bd));
	bd = PlaneBuilder(glm::vec3(0.0f, 0.5f, -0.5f), 1.1f, "./assets/blue.png");
	bd.rotate(90, glm::vec3(1, 0, 0));
	w.build_object(std::move(bd));
	w.build_object(PlaneBuilder(glm::vec3(0, 0, 0), 1.1f, "./assets/orange.png"));

	bd = PlaneBuilder(glm::vec3(0.5f, 0.5f, 0.0f), 1.1f, "./assets/green.png");
	bd.rotate(90, glm::vec3(0, 0, 1));
	w.build_object(std::move(bd));
	bd = PlaneBuilder(glm::vec3(0.0f, 0.5f, 0.5f), 1.1f, "./assets/blue.png");
	bd.rotate(-90, glm::vec3(1, 0, 0));
	w.build_object(std::move(bd));
	bd = PlaneBuilder(glm::vec3(0.0f, 1.0f, 0.0f), 1.1f, "./assets/orange.png");
	bd.rotate(180, glm::vec3(1, 0, 0));
	w.build_object(std::move(bd));

	//创建点光源
	w.build_point_light(NoneBuilder(), PointLight(glm::vec3(0.4f, 0.4f, 0.4f)));

	//循环
	w.mainloop(window, -1);
	glfwTerminate();

	return 0;
}