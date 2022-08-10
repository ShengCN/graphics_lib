#pragma once
#include <common.h>
#include <memory>
#include "scene.h"
#include "ppc.h"
#include "shader.h"

class renderer {
private:
    std::unordered_map<std::string, std::shared_ptr<shader>> m_shaders;

public:
    renderer();
    void render(std::shared_ptr<scene> cur_scene, std::shared_ptr<ppc> cur_ppc);

private:
    template<typename T>
    std::shared_ptr<shader> init_shaders(const std::string vs, const std::string fs);
};
