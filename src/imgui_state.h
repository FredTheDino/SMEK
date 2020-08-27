#pragma once
#include "math/types.h"

struct ImGuiState {
    void *context;

    bool show_create_sound_window = false;
    bool depth_map_enabled = true;
};