#include "otb_window.h"
#include <memory>

#include "GLFW/glfw3.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include "imgui_guizmo.h"
#include "Render/shader.h"
#include "Utilities/Utils.h"
#include "Utilities/model_loader.h"

using namespace purdue;

std::string default_config = "Configs/default.json";
std::shared_ptr<render_engine> otb_window::m_engine =
        std::make_shared<render_engine>(default_config);
float otb_window::m_dpi_scale = 1.0;

otb_window::otb_window() {}

otb_window::~otb_window() {}

void otb_window::key_callback(GLFWwindow *window, int key, int scancode,
                              int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    char m = '0';
    bool shift = false;

    std::unordered_map<int, char> key_maps = {
            {GLFW_KEY_W, 'w'},
            {GLFW_KEY_A, 'a'},
            {GLFW_KEY_S, 's'},
            {GLFW_KEY_D, 'd'},
            {GLFW_KEY_Q, 'q'},
            {GLFW_KEY_E, 'e'}};

    m = key_maps[key];

    if (key == GLFW_KEY_LEFT_SHIFT) {
        shift = true;
    }

    INFO("Key {}", m);
    m_engine->camera_keyboard(m, shift);
}

void otb_window::scroll_callback(GLFWwindow *window, double xoffset,
                                 double yoffset) {
    m_engine->camera_scroll(yoffset);
}

void otb_window::get_mouse_pos(GLFWwindow *window, double *xpos, double *ypos) {
    glfwGetCursorPos(window, xpos, ypos);

    *xpos = *xpos / m_dpi_scale;
    *ypos = *ypos / m_dpi_scale;
}

void otb_window::mouse_button_callback(GLFWwindow *window, int button,
                                       int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        double xpos, ypos;
        get_mouse_pos(window, &xpos, &ypos);
        INFO("Right button clicked, " + std::to_string(xpos) + " " +
             std::to_string(ypos));
        m_engine->camera_press((int) xpos, (int) ypos);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        double xpos, ypos;
        get_mouse_pos(window, &xpos, &ypos);

        INFO("Right button released, " + std::to_string(xpos) + " " +
             std::to_string(ypos));
        m_engine->camera_release((int) xpos, (int) ypos);
    }
}

void otb_window::cursor_position_callback(GLFWwindow *window, double xpos,
                                          double ypos) {
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
    _window = glfwCreateWindow(m_engine->width(), m_engine->height(),
                               m_engine->title().c_str(), NULL, NULL);
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
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
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
        iter = (iter + 1) % 10000;

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
    for (int j = 0; j < h / 2; ++j)
        for (int i = 0; i < w; ++i) {
            std::swap(pixels[w * j + i], pixels[w * (h - 1 - j) + i]);
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
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    const char *glsl_version = "#version 460";
    if (ImGui_ImplOpenGL3_Init(glsl_version)) {
        INFO("ImGui init success");
    } else {
        WARN("ImGui init failed");
    }
}

void otb_window::reload_all_shaders() {
    // m_engine.reload_shaders();
}

void otb_window::guizmo() {
    auto camera_view = m_engine->get_camera_view();
    auto camera_pers = m_engine->get_camera_perspective();
    auto toworld = m_engine->get_obj_toworld(m_cur_mesh);

    if(imgui_guizmo(camera_view, camera_pers, toworld)) {
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

    if (ImGui::Button("reload shader")) {
        reload_all_shaders();
    }
    ImGui::SameLine();
    if (ImGui::Button("save")) {
        save_framebuffer("test.png");
    }

    ImGui::SameLine();
    if (ImGui::Button("dbg")) {
        dbg();
    }

    guizmo();

    ImGui::End();

    // ImGui::ShowTestWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void otb_window::render(int iter) { m_engine->render(iter); }

void otb_window::dbg() {
    INFO("Begin DBG");

    INFO("End DBG");
}
