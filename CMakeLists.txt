cmake_minimum_required(VERSION 3.8)

project(libgraphics_lib)

file(GLOB render_src
    "${PROJECT_SOURCE_DIR}/Render/*.cpp"
    "${PROJECT_SOURCE_DIR}/Utilities/*.cpp"
    "${PROJECT_SOURCE_DIR}/Dep/tinyobjloader/*.cpp"
    "${PROJECT_SOURCE_DIR}/Dep/glad/src/glad.c") 

find_package(Boost REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/Dep/glm)
add_library(graphics_lib SHARED asset_manager.cpp ${render_src})

target_include_directories(graphics_lib PUBLIC 
${PROJECT_SOURCE_DIR}/Dep/glad/include 
${PROJECT_SOURCE_DIR}/Dep/stb
${PROJECT_SOURCE_DIR}/Dep
${PROJECT_SOURCE_DIR}
${Boost_INCLUDE_DIR})

target_link_libraries(graphics_lib LINK_PUBLIC ${Boost_LIBRARIES})

install(TARGETS graphics_lib DESTINATION lib)
install(FILES *.h DESTINATION include)