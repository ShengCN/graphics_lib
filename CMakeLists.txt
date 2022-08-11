cmake_minimum_required(VERSION 3.8)

project(libgraphics_lib)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

# ImGui
#add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD=1)
add_definitions(-DGLM_FORCE_RADIANS)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
#add_compile_definitions(GLM_FORCE_RADIANS=1)

add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/fmt)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/rapidjson)
add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/yaml-cpp)

file(GLOB render_src
    "${PROJECT_SOURCE_DIR}/Render/*.cpp"
    "${PROJECT_SOURCE_DIR}/UI/*.cpp"
    "${PROJECT_SOURCE_DIR}/Utilities/*.cpp"
    "${PROJECT_SOURCE_DIR}/Dep/tinyobjloader/*.cc"
    "${PROJECT_SOURCE_DIR}/Dep/tinyxml2/*.cpp"
    "${PROJECT_SOURCE_DIR}/Dep/ImGuizmo/*.cpp"
    "${PROJECT_SOURCE_DIR}/asset_manager.cpp"
    "${PROJECT_SOURCE_DIR}/render_engine.cpp"
    "${PROJECT_SOURCE_DIR}/otb_window.cpp"
    "${PROJECT_SOURCE_DIR}/Dep/glad/glad.c"
    "${PROJECT_SOURCE_DIR}/imgui/*.cpp"
    "${PROJECT_SOURCE_DIR}/imgui/examples/imgui_impl_glfw.cpp"
    "${PROJECT_SOURCE_DIR}/imgui/examples/imgui_impl_opengl3.cpp")

add_library(graphics_lib STATIC ${render_src})

find_package(Boost COMPONENTS system filesystem REQUIRED)

target_include_directories(graphics_lib PUBLIC 
${PROJECT_SOURCE_DIR}/Dep/
${PROJECT_SOURCE_DIR}/Dep/stb
${PROJECT_SOURCE_DIR}/Dep/glad
${PROJECT_SOURCE_DIR}/Dep/fmt/include
${PROJECT_SOURCE_DIR}/Dep/glfw/include
${PROJECT_SOURCE_DIR}/Dep/rapidjson/include
${PROJECT_SOURCE_DIR}/Dep/yaml-cpp/include
${PROJECT_SOURCE_DIR}/Dep/ImGuizmo
${PROJECT_SOURCE_DIR}/Dep/glm
${PROJECT_SOURCE_DIR}/imgui
${PROJECT_SOURCE_DIR}/UI
${PROJECT_SOURCE_DIR}
${Boost_INCLUDE_DIR})


set_target_properties(graphics_lib
PROPERTIES 
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(graphics_lib PUBLIC 
${Boost_LIBRARIES}
${OPENGL_gl_LIBRARY}
glfw
OpenMP::OpenMP_CXX
yaml-cpp
)
