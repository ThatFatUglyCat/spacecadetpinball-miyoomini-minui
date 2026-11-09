// SPDX-License-Identifier: MIT
#pragma once
#include "SDL.h"
#include "imgui.h"

inline bool ImGui_ImplSDL2_InitForSDLRenderer(SDL_Window*, SDL_Renderer*) { return true; }
inline void ImGui_ImplSDL2_Shutdown() {}
inline void ImGui_ImplSDL2_NewFrame() {}
inline bool ImGui_ImplSDL2_ProcessEvent(const SDL_Event*) { return false; }
