#pragma once

#include <Dep/glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <render_engine.h>

class otb_window {
	// public variables
public:
	GLFWwindow* _window;

public:
	otb_window();
	~otb_window();

	// callback functions
public:
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	// static void mouse_callback()
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	static void error_callback(int error, const char* description) {
		std::cerr << "Error: %s\n" << description << std::endl;
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void get_mouse_pos(GLFWwindow* window, double *xpos, double *ypos);

	// public functions
public:
	int create_window();
	
	void show();		// one thread one window
	void save_framebuffer(const std::string output_file);
	int width();
	int height();


private:
	/* GUI */
	void init_gui();
	void draw_gui();
	void guizmo();

	/* Others */
	void reload_all_shaders();
	void render(int iter);
	void dbg();

private:
	mesh_id m_cur_mesh=1;
	static std::shared_ptr<render_engine> m_engine;
	float m_distance = 2.0f;
	static float m_dpi_scale;
};

