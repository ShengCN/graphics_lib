#pragma once
#include <common.h>
#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui_internal.h>

#define IMAPP_IMPL

#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>

#include "ImGuizmo/ImCurveEdit.h"
#include "ImGuizmo/ImGuizmo.h"
#include "ImGuizmo/ImSequencer.h"
#include "ImGuizmo/ImZoomSlider.h"

bool imgui_guizmo(glm::mat4 view, glm::mat4 perspective, glm::mat4 &toworld);
