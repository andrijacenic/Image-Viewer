#pragma once
#include <imgui.h>
struct ImageShaderModification {
	ImVec2 positions[4] = { ImVec2(-1.0f, 1.0f), ImVec2(1.0f, 1.0f) , ImVec2(1.0f, -1.0f) , ImVec2(-1.0f, -1.0f) };
	ImVec4 colors[4] = { ImVec4(1,1,1,1), ImVec4(1,1,1,1), ImVec4(1,1,1,1), ImVec4(1,1,1,1) };
	float saturation = 1.0f;
	float hue = 0.0f;
	float brightness = 1.0f;
};