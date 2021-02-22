#include <fstream>
#include <memory>
#include <glm/ext/matrix_float4x4.hpp>
#include "otb_window.h"
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <string>

#include "Render/shader.h"
#include "Utilities/Utils.h"
#include "Utilities/model_loader.h"

using namespace purdue;

otb_window::otb_window() {
}

otb_window::~otb_window() {

}

render_engine otb_window::m_engine;;

void otb_window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	char m;
	bool shift = false;
	if(key == GLFW_KEY_W && action == GLFW_PRESS) {
		m = 'w';
	}

	if(key == GLFW_KEY_A && action == GLFW_PRESS) {
		m = 'a';
	}

	if(key == GLFW_KEY_S && action == GLFW_PRESS) {
		m = 's';
	}

	if(key == GLFW_KEY_D && action == GLFW_PRESS) {
		m = 'd';
	}

	if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
		m = 'q';
	}

	if(key == GLFW_KEY_E && action == GLFW_PRESS) {
		m = 'e';
	}

	if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		shift = true;	
	}

	m_engine.camera_keyboard(m,shift);
}

void otb_window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {	
	m_engine.camera_scroll(yoffset);
}

void otb_window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		INFO("Right button clicked, " + std::to_string(xpos) + " " + std::to_string(ypos));
		m_engine.camera_press((int)xpos, (int)ypos);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		INFO("Right button released, " + std::to_string(xpos) + " " + std::to_string(ypos));
		m_engine.camera_release((int)xpos, (int)ypos);
	}
}

void otb_window::cursor_position_callback(GLFWwindow* window,  double xpos, double ypos) {
	m_engine.camera_move(xpos, ypos);
}

int otb_window::create_window(int w, int h, const std::string title) {
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_SAMPLES, 16);

	/* Create a windowed mode window and its OpenGL context */
	_window = glfwCreateWindow(w, h, title.c_str(), NULL, NULL);
	if (!_window) {
		glfwTerminate();
		return -1;
	}

	// callbacks
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(_window, key_callback);
	glfwSetScrollCallback(_window, scroll_callback);
	glfwSetCursorPosCallback(_window, cursor_position_callback);
	glfwSetMouseButtonCallback(_window, mouse_button_callback);

	// set up environment
	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);

	if (gladLoadGL() == 0) {
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
	if (GLVersion.major < 2) {
		printf("Your system doesn't support OpenGL >= 2!\n");
		return -1;
	}

	m_engine.init();
	init_gui();
	init_scene();

	return 1;
}

void otb_window::init_scene() {
	int h, w;
	glfwGetWindowSize(_window, &w, &h);
	m_engine.test_scene(w,h);
}

void otb_window::show() {
	glfwMakeContextCurrent(_window);
	static int iter = 0;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(_window)) {
		glfwPollEvents();

		// animation
		iter = (iter+1) % 10000;

		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render(iter);

		draw_gui();
		glfwSwapBuffers(_window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void otb_window::save_framebuffer(const std::string output_file) {
	unsigned int *pixels;
	int w = width(), h = height();
	pixels = new unsigned int[w * h * 4];
	for (int i = 0; i < (w * h * 4); i++) {
		pixels[i] = 0;
	}

	glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// filp pixels
	for (int j = 0; j < h / 2; ++j) for (int i = 0; i < w; ++i) {
		std::swap(pixels[w * j + i], pixels[w * (h-1-j) + i]);
	}

	save_image(output_file, pixels, w, h, 4);
	delete[] pixels;
}

int otb_window::width() {
	int display_w, display_h;
	glfwGetFramebufferSize(_window, &display_w, &display_h);
	return display_w;
}

int otb_window::height() {
	int display_w, display_h;
	glfwGetFramebufferSize(_window, &display_w, &display_h);
	return display_h;
}

void otb_window::init_gui() {
	// imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	const char* glsl_version = "#version 460";
	if(ImGui_ImplOpenGL3_Init(glsl_version)) {
		INFO("ImGui init success");
	} else {
		WARN("ImGui init failed");
	}
}

void otb_window::reload_all_shaders() {
	m_engine.reload_shaders();
}

void otb_window::draw_gui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//// ------------------------ Window ------------------------ //
	ImGui::Begin("PC control");
	static bool draw_vis = true;
	ImGui::Checkbox("visualize", &draw_vis);
	m_engine.draw_visualize(draw_vis);
	
	static bool draw_render = true;
	ImGui::SameLine();
	ImGui::Checkbox("render", &draw_render);
	m_engine.draw_render(draw_render);

	// ImGui::SliderFloat("fov", &asset_manager::instance).cur_camera->_fov, 30.0f, 120.0f);
	ImGui::SliderFloat("fov", &m_engine.get_render_ppc()->_fov, 5.0f, 120.0f);

	if(ImGui::Button("reload shader")) {
		reload_all_shaders();
	}
	if(ImGui::Button("save")) {
		save_framebuffer("test.png");
	}
	
	if (ImGui::Button("dbg")) {
		dbg();
	}
	ImGui::End();

	 //ImGui::ShowTestWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void otb_window::render(int iter) {
	// m_engine.render(iter);
	render_mask();
}

void otb_window::render_shadow() {

}

void otb_window::render_mask() { 
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_engine.render(0);
}

struct bin_data {
	float min, max;
};
bool read_matrix_data(std::string fname, std::vector<bin_data> &out_volume) {
	std::fstream input(fname, std::ios::binary | std::ios::in);
	if (input.is_open()) {
		out_volume.clear();
		while(true) {
			bin_data cur_data;
			input.read((char*)&cur_data.min, sizeof(float));
			input.read((char*)&cur_data.max, sizeof(float));

			out_volume.push_back(cur_data);
			if (!input) {
				break;
			}
		}
	} else {
		INFO("Cannot find matrix bin data");
	}

	INFO("Read finished");
}

void otb_window::dbg() {
	// draw sihouette 
	// vec3 light_pos = vec3(1000.0f);
	
	// int cur_mesh = m_engine.get_rendering_meshes()[0]->get_id();
	// m_engine.draw_shadow_volume(cur_mesh, light_pos);
	

	// visualize voxels
	// let's use point clouds first
	auto cur_mesh = m_engine.get_rendering_meshes().front();
	cur_mesh->clear_vertices();

	// read matrix data
	std::vector<bin_data> out_volume;
	read_matrix_data("test.bin", out_volume);

	// generate point clouds
	int w = 256, h = 256, d = 20;
	int counter = 0, total = w * h * d;
	for(int x = 0; x < w; ++x) for(int y = 0; y < h; ++y) for (int z = 0; z < d; ++z) {
		// pos: [0,1] x [0,1] x [0,1]
		vec3 pos = vec3((float)x/w, (float)y/h, (float)z/d);
		int ind = x * h + y;
		vec3 color(0.5f);
		if (pos.z > out_volume[ind].min && pos.z < out_volume[ind].max) {
			color =	vec3(1.0f,0.0f,0.0f);
		}
		cur_mesh->add_vertex(pos, vec3(0.0f), color);
		
		INFO("Progress: " + std::to_string((float)counter++/total));
	}
	
	m_engine.look_at(cur_mesh->get_id());
	
	m_engine.set_draw_type(draw_type::points);
}