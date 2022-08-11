#include "otb_window.h"
#include <memory>



#include <glm/ext/matrix_float4x4.hpp>
#include "GLFW/glfw3.h"
#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#define IMAPP_IMPL
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>

#include "ImGuizmo/ImGuizmo.h"
#include "ImGuizmo/ImSequencer.h"
#include "ImGuizmo/ImZoomSlider.h"
#include "ImGuizmo/ImCurveEdit.h"

#include "Render/shader.h"
#include "Utilities/Utils.h"
#include "Utilities/model_loader.h"

using namespace purdue;

std::string default_config = "Configs/default.json";
std::shared_ptr<render_engine> otb_window::m_engine = std::make_shared<render_engine>(default_config);
float otb_window::m_dpi_scale = 1.0;

otb_window::otb_window() {
}

otb_window::~otb_window() {

}

void otb_window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) {
		return;
	}

	char m = '0';
	bool shift = false;

	std::unordered_map<int, char> key_maps = {{GLFW_KEY_W, 'w'},
											  {GLFW_KEY_A, 'a'},
											  {GLFW_KEY_S, 's'},
											  {GLFW_KEY_D, 'd'},
											  {GLFW_KEY_Q, 'q'},
											  {GLFW_KEY_E, 'e'}};


	m = key_maps[key];

	if(key == GLFW_KEY_LEFT_SHIFT) {
		shift = true;	
	}

	INFO("Key {}", m);
	m_engine->camera_keyboard(m,shift);
}

void otb_window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {	
	m_engine->camera_scroll(yoffset);
}


void otb_window::get_mouse_pos(GLFWwindow* window, double *xpos, double *ypos) {
	glfwGetCursorPos(window, xpos, ypos);

	*xpos = *xpos / m_dpi_scale;
	*ypos = *ypos / m_dpi_scale;
}

void otb_window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		double xpos, ypos;
		get_mouse_pos(window, &xpos, &ypos);
		INFO("Right button clicked, " + std::to_string(xpos) + " " + std::to_string(ypos));
		m_engine->camera_press((int)xpos, (int)ypos);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		double xpos, ypos;
		get_mouse_pos(window, &xpos, &ypos);

		INFO("Right button released, " + std::to_string(xpos) + " " + std::to_string(ypos));
		m_engine->camera_release((int)xpos, (int)ypos);
	}
}



void otb_window::cursor_position_callback(GLFWwindow* window,  double xpos, double ypos) {
	get_mouse_pos(window, &xpos, &ypos);
	m_engine->camera_move(xpos, ypos);
}

int otb_window::create_window() {
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	/* Create a windowed mode window and its OpenGL context */
	_window = glfwCreateWindow(m_engine->width(), m_engine->height(), m_engine->title().c_str(), NULL, NULL);
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
	glfwSetWindowPos(_window, 100, 500);

	// DPI scale
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	glfwGetMonitorContentScale(monitor, &m_dpi_scale, &m_dpi_scale);

	if (gladLoadGL() == 0) {
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
	if (GLVersion.major < 2) {
		printf("Your system doesn't support OpenGL >= 2!\n");
		return -1;
	}

	m_engine->init_ogl_states();

	init_gui();
	return 1;
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
	// m_engine.reload_shaders();
}

bool useWindow = true;
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static const float identityMatrix[16] =
{ 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f };

float objectMatrix[4][16] = {
  { 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  2.f, 0.f, 0.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  2.f, 0.f, 2.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 2.f, 1.f }
};

int gizmoCount = 1;
float camDistance = 8.0;

void EditTransform(float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition)
{
   static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
   static bool useSnap = false;
   static float snap[3] = { 1.f, 1.f, 1.f };
   static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
   static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
   static bool boundSizing = false;
   static bool boundSizingSnap = false;

   if (editTransformDecomposition)
   {
      if (ImGui::IsKeyPressed(90))
         mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
      if (ImGui::IsKeyPressed(69))
         mCurrentGizmoOperation = ImGuizmo::ROTATE;
      if (ImGui::IsKeyPressed(82)) // r Key
         mCurrentGizmoOperation = ImGuizmo::SCALE;
      if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
         mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
      ImGui::SameLine();
      if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
         mCurrentGizmoOperation = ImGuizmo::ROTATE;
      ImGui::SameLine();
      if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
         mCurrentGizmoOperation = ImGuizmo::SCALE;
      if (ImGui::RadioButton("Universal", mCurrentGizmoOperation == ImGuizmo::UNIVERSAL))
         mCurrentGizmoOperation = ImGuizmo::UNIVERSAL;
      float matrixTranslation[3], matrixRotation[3], matrixScale[3];
      ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
      ImGui::InputFloat3("Tr", matrixTranslation);
      ImGui::InputFloat3("Rt", matrixRotation);
      ImGui::InputFloat3("Sc", matrixScale);
      ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

      if (mCurrentGizmoOperation != ImGuizmo::SCALE)
      {
         if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;
         ImGui::SameLine();
         if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
      }
      if (ImGui::IsKeyPressed(83))
         useSnap = !useSnap;
      ImGui::Checkbox("##UseSnap", &useSnap);
      ImGui::SameLine();

      switch (mCurrentGizmoOperation)
      {
      case ImGuizmo::TRANSLATE:
         ImGui::InputFloat3("Snap", &snap[0]);
         break;
      case ImGuizmo::ROTATE:
         ImGui::InputFloat("Angle Snap", &snap[0]);
         break;
      case ImGuizmo::SCALE:
         ImGui::InputFloat("Scale Snap", &snap[0]);
         break;
      }
      ImGui::Checkbox("Bound Sizing", &boundSizing);
      if (boundSizing)
      {
         ImGui::PushID(3);
         ImGui::Checkbox("##BoundSizing", &boundSizingSnap);
         ImGui::SameLine();
         ImGui::InputFloat3("Snap", boundsSnap);
         ImGui::PopID();
      }
   }

   ImGuiIO& io = ImGui::GetIO();
   float viewManipulateRight = io.DisplaySize.x;
   float viewManipulateTop = 0;
   static ImGuiWindowFlags gizmoWindowFlags = 0;
   if (useWindow)
   {
      ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Appearing);
      ImGui::SetNextWindowPos(ImVec2(400,20), ImGuiCond_Appearing);
      ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
      ImGui::Begin("Gizmo", 0, gizmoWindowFlags);
      ImGuizmo::SetDrawlist();
      float windowWidth = (float)ImGui::GetWindowWidth();
      float windowHeight = (float)ImGui::GetWindowHeight();
      ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
      viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
      viewManipulateTop = ImGui::GetWindowPos().y;
      ImGuiWindow* window = ImGui::GetCurrentWindow();
      gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0;
   }
   else
   {
      ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
   }

   ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
   ImGuizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);
   // ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

   ImGuizmo::ViewManipulate(cameraView, camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);

   if (useWindow)
   {
      ImGui::End();
      ImGui::PopStyleColor(1);
   }
}

void otb_window::guizmo() {
	ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

	ImGuizmo::SetID(0);

	auto camera_view = m_engine->get_camera_view();
	auto camera_pers = m_engine->get_camera_perspective();
	auto toworld = m_engine->get_obj_toworld(m_cur_mesh);

	float *cameraView = (float*)(&camera_view[0]);
	float *cameraProjection = (float*)(&camera_pers[0]);
	float *objectMatrix = (float*)(&toworld[0]);
	// EditTransform(cameraView, cameraProjection, objectMatrix, true);

	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

	if (ImGui::IsKeyPressed(90))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r Key
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Universal", mCurrentGizmoOperation == ImGuizmo::UNIVERSAL))
		mCurrentGizmoOperation = ImGuizmo::UNIVERSAL;

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(objectMatrix, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation);
	ImGui::InputFloat3("Rt", matrixRotation);
	ImGui::InputFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, objectMatrix);


	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(cameraView,
						 cameraProjection,
						 mCurrentGizmoOperation,
						 mCurrentGizmoMode,
						 objectMatrix,
						 NULL,
						 NULL,
						 NULL,
						 NULL);

	if (ImGuizmo::IsUsing()) {
		m_engine->set_obj_toworld(m_cur_mesh, toworld);
	}

}

void otb_window::draw_gui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();


	//// ------------------------ Window ------------------------ //
	ImGui::Begin("PC control");
	ImGui::SetWindowFontScale(2.0); // use 1.0 if not using HDI

	if(ImGui::Button("reload shader")) {
		reload_all_shaders();
	}
	if(ImGui::Button("save")) {
		save_framebuffer("test.png");
	}
	
	if (ImGui::Button("dbg")) {
		dbg();
	}
	guizmo();

	ImGui::End();

	 //ImGui::ShowTestWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void otb_window::render(int iter) {
	 m_engine->render(iter);
}


void otb_window::dbg() {
	INFO("Begin DBG");

	INFO("End DBG");
}
