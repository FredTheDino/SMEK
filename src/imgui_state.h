#pragma once
#include "math/types.h"

struct ImGuiState {
    void *context;
    void *implot_context;

    // Toggleable views
    bool networking_enabled = false;
    bool entities_enabled = true;
    bool depth_map_enabled = false;
    bool render_target_enabled = false;
    bool performance_enabled = false;

    bool show_create_sound_window = false;

    // Debug
    f32 t = 0;
    f32 min_t = 0;
    f32 max_t = 100;
    bool use_debug_camera = true;
    bool debug_draw_physics = false;
    bool show_grid = true;

    // Settings
    struct {
        int width;
        int height;
    } screen_resolution = {};
    int network_port = 8888;
};
