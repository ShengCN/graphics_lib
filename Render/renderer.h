#pragma once
#include <common.h>
#include <memory>
#include "shader.h"

class renderer {
public:
    renderer();

private:
    std::unordered_map<std::string, std::shared_ptr<shader>> m_shaders;
};
