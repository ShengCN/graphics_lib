cmake_minimum_required(VERSION 3.8)

project(libgraphics_lib)
enable_language(CUDA)
find_package(CUDA  REQUIRED)

set (CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(OpenMP)

file(GLOB render_src
    "${PROJECT_SOURCE_DIR}/Render/*.cpp"
    "${PROJECT_SOURCE_DIR}/Utilities/*.cpp"
    "${PROJECT_SOURCE_DIR}/Dep/tinyobjloader/*.cc"
    "${PROJECT_SOURCE_DIR}/Dep/tinyxml2/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/Dep/glad/glad.c"
)

find_package(Boost COMPONENTS system filesystem REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/glm)
# add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/glfw)


add_library(graphics_lib STATIC ${render_src})

set_property(TARGET graphics_lib PROPERTY CUDA_ARCHITECTURES 35 50 72)
set_property(TARGET graphics_lib
             PROPERTY CUDA_SEPARABLE_COMPILATION ON)

target_include_directories(graphics_lib PUBLIC 
${PROJECT_SOURCE_DIR}/Dep/stb
${PROJECT_SOURCE_DIR}/Dep
${PROJECT_SOURCE_DIR}
${CUDA_INCLUDE_DIRS}
${Boost_INCLUDE_DIR})

add_definitions(-DGLM_FORCE_RADIANS)

target_link_libraries(graphics_lib 
${Boost_LIBRARIES}
${OPENGL_gl_LIBRARY}
OpenMP::OpenMP_CXX
)

         
# glfw)
# install(TARGETS graphics_lib DESTINATION lib)
# install(FILES *.h DESTINATION include)