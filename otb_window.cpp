#include <memory>
#include <imgui/imgui.h>

#include <stdio.h>
#include "otb_window.h"
#include "render_engine.h"
#include "glm/ext/matrix_float4x4.hpp"

#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>

namespace pd=purdue;

otb_window::otb_window() {
	m_begin_save_frame = false;
	m_frame_folder = "tmp";
	pd::safe_create_folder(m_frame_folder);
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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

image ori_img, dep_img; 
void otb_window::init_scene() {
	int h, w;
	glfwGetWindowSize(_window, &w, &h);
	// m_engine.test_scene(w,h);

	const std::string target_file = "Meshes/bunny.obj"; 

	// read target model
	int target_model = m_engine.load_mesh(target_file);
	auto target_mesh = m_engine.get_mesh(target_model);
	m_engine.recompute_normal(target_model);
	target_mesh->normalize_position_orientation();
	m_engine.set_mesh_color(target_mesh, vec3(1.0f));

	// camera 
	m_engine.init_camera(w, h, 60.0f);
	m_engine.look_at(target_model, vec3(0.0f, 0.0f, 3.0f));


    INFO("Reading Orginal & Depth image");
	const std::string ori_path = "test_ori.jpeg"; 
	const std::string dep_path = "test_depth.png"; 
	if (!pd::file_exists(ori_path) || !pd::file_exists(dep_path)) {
		WARN("Original or Depth image not found!");
		return;
	}

	/* Read Original & Depth Image */
	if (!ori_img.load(ori_path) || !dep_img.load(dep_path)) {
		WARN("original or depth image load failed");
		return;
	}
	unsigned int ori_texid = ori_img.ogl_texid(), dep_texid = dep_img.ogl_texid();
	printf("Ori: %d, Dep: %d \n", ori_texid, dep_texid);
}

int iter = 0;
void otb_window::show() {
	glfwMakeContextCurrent(_window);

	auto deg_ani=[](int iter, float deg, float speed=0.01){
		return deg * std::sin(iter * speed); 
	};

	float total_deg = 80.0f;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(_window)) {
		glfwPollEvents();

		// animation
		iter = (iter+1) % 10000;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float delta_deg = deg_ani(iter, total_deg) - deg_ani(iter-1,total_deg);
		m_engine.mesh_add_transform(m_engine.get_rendering_meshes()[0], glm::rotate(pd::deg2rad(delta_deg),vec3(0.0f,1.0f,0.0f)));
		render(iter);

		if (m_begin_save_frame) {
			char buffer[50];
			std::sprintf(buffer, "%05d.png", iter);
			std::string out_fname = m_frame_folder + "/" + buffer; 
			save_framebuffer(out_fname);
		}

		draw_gui();
		glfwSwapBuffers(_window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void otb_window::save_framebuffer(const std::string output_file) {
	image cur_img(width(), height(), 4);

	int w = width(), h = height();
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, cur_img.data());

	// filp pixels
	for (int j = 0; j < h / 2; ++j) for (int i = 0; i < w; ++i) {
		std::swap(cur_img.at(i, j), cur_img.at(i, h-1-j));
	}

	cur_img.save(output_file);
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
	const char* glsl_version = "#version 330";
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
	// ImGui::SliderFloat("fov", &asset_manager::instance).cur_camera->_fov, 30.0f, 120.0f);
	ImGui::SliderFloat("fov", &m_engine.get_render_ppc()->_fov, 5.0f, 120.0f);
	GLuint ori_texid = ori_img.ogl_texid(), dep_texid = dep_img.ogl_texid();
	ImGui::Image((ImTextureID)ori_texid, ImVec2((float)128, (float)128), ImVec2(0, 0), ImVec2(1, 1));
	ImGui::SameLine();
	ImGui::Image((ImTextureID)dep_texid, ImVec2((float)128, (float)128), ImVec2(0, 0), ImVec2(1, 1));

	if(ImGui::Button("reload shader")) {
		reload_all_shaders();
	}
	ImGui::SameLine();
	if (ImGui::Button("dbg")) {
		dbg();
	}
	ImGui::End();

	 //ImGui::ShowTestWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void otb_window::render(int iter) {
	m_engine.render(iter);
}

void otb_window::dbg() {
	/* Recompute point clouds  */
	auto ori_dep_mesh=[](image &ori, image &dep, std::shared_ptr<mesh> ptr) {
		ptr->clear_vertices();
		int h = ori.height(), w = ori.width();

		/* Assume camera is at the center
		*     fov: 60
		*/
		ppc tmp_camera(w,h,1.0f);
		tmp_camera.PositionAndOrient(vec3(0.0f), vec3(0.0f,0.0f,-1.0f), vec3(0.0f,1.0f,0.0f));

		float counter = 0.0f;
		for(int i = 0; i < w; ++i) {
			for(int j = 0; j < h; ++j) {
				float depth = dep.get_rgb(i,h-j)[0];
				vec3 c = ori.get_rgb(i, h-j);
				vec3 p = tmp_camera.unproject(i, j, (depth) * 1000.0f);
				ptr->add_vertex(p, vec3(0.0f), c);
				counter++;
				// std::printf("Depth: %f, position: %f,%f,%f, color: %f, %f, %f\n", depth, p.x, p.y, p.z, c.x,c.y,c.x);
				INFO("Finished " + std::to_string(counter/(w* h) * 100.0f));
			}
		}
	};

	auto cur_mesh = m_engine.get_rendering_meshes()[0];
	m_engine.set_draw_type(draw_type::points);

	int total = 2401;
	std::string render_root = "render_out/";
	pd::safe_create_folder(render_root);	
	for (int i = 0; i <= total; ++i) {
		std::string ori_fname = "out/" + pd::get_prefix(i, "%05d.png");
		std::string dep_fname = "out/" + pd::get_prefix(i, "%05d_disp.png");
		if (!ori_img.load(ori_fname) || !dep_img.load(dep_fname)) {
			WARN("Iter: " + std::to_string(i) + " failed!");
			continue;
		}

		ori_dep_mesh(ori_img, dep_img, cur_mesh);

		/* Calibrate Mesh and PPC */
		cur_mesh->normalize_position_orientation();
		m_engine.look_at(cur_mesh->get_id(), vec3(0.0f,0.0f,1.0f));

        std::string out_folder = render_root + pd::get_prefix(i,"%05d");
		pd::safe_create_folder(out_folder);

	    render_animation(out_folder);
		INFO("Rendering: " + std::to_string((float)i/total * 100.0f) + "%");
	}
}

void otb_window::render_animation(std::string out_folder) {
	if (!pd::file_exists(out_folder)) {
		WARN("There is no folder: " + out_folder);
		return;
	}

	auto deg_ani=[](float deg, int iter, int period){
		float fract = (float)iter/period;
		return deg * std::sin(pd::deg2rad(fract * 360.0f)); 
	};

	auto cur_mesh = m_engine.get_rendering_meshes()[0];
	int ani_num = 90 * 4;
	float total_deg = 90.0f;
	for(int iter = 0; iter < ani_num; ++iter) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render(iter);
		glFlush();

		std::string fname = out_folder + "/" + pd::get_prefix(iter, "%05d.png");
		save_framebuffer(fname);

		/* Animation */
		float delta_deg = deg_ani(total_deg, iter, ani_num) - deg_ani(total_deg, iter-1, ani_num);
		INFO("Delta: " + std::to_string(delta_deg));
		m_engine.mesh_add_transform(cur_mesh, glm::rotate(pd::deg2rad(delta_deg),vec3(0.0f,1.0f,0.0f)));
		INFO("Animation: " + std::to_string((float)iter/ani_num * 100.0f) + "%");
	}
}