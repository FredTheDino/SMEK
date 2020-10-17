#include "util/util.h"

#include "game.h"
#include "test.h"
#include "renderer/opengl.h"
#include "renderer/renderer.h"
#include "asset/asset.h"
#include "physics/physics.h"

#include "math/smek_mat4.h"
#include "math/smek_math.h"

#include "imgui/imgui.h"

GameState *_global_gs;
#ifndef TESTS
GameState *GAMESTATE() { return _global_gs; }
#endif

f32 delta() { return GAMESTATE()->delta; }
f32 time() { return GAMESTATE()->time; }
u32 frame() { return GAMESTATE()->frame; }

void do_imgui_stuff();

void init_game(GameState *gamestate, int width, int height) {
    _global_gs = gamestate;
    GAMESTATE()->main_thread = SDL_GetThreadID(NULL);

    GAMESTATE()->entity_system.m_client_id = SDL_CreateMutex();
    GAMESTATE()->m_event_queue = SDL_CreateMutex();

    Asset::load("assets.bin");

#if IMGUI_ENABLE
    GAMESTATE()->imgui.screen_resolution = { width, height };
#endif
    GFX::init(GAMESTATE(), width, height);

    GFX::lighting()->sun_direction = Vec3(0.0, 1.0, 0.0);
    *GFX::debug_camera() = GFX::Camera::init();
    GFX::debug_camera()->position = Vec3(0.0, 0.2, 0.0);

    *GFX::gameplay_camera() = GFX::Camera::init();
    GFX::gameplay_camera()->position = Vec3(0.0, 0.2, 0.0);

    GAMESTATE()->running = true;

    Input::bind(Ac::MoveX, 0, SDLK_a, -1.0);
    Input::bind(Ac::MoveX, 1, SDLK_d, 1.0);
    Input::bind(Ac::MoveY, 0, SDLK_q, 1.0);
    Input::bind(Ac::MoveY, 1, SDLK_e, -1.0);
    Input::bind(Ac::MoveZ, 0, SDLK_w, -1.0);
    Input::bind(Ac::MoveZ, 1, SDLK_s, 1.0);
    Input::bind(Ac::Jump, 0, SDLK_SPACE, 1.0);
    Input::bind(Ac::Shoot, 0, SDLK_f, 1.0);
    Input::bind(Ac::MouseToggle, 0, SDLK_m);
    Input::bind(Ac::Rebind, 1, SDLK_r);
    Input::bind(Ac::ESelect, 1, SDLK_y);

    Light l = Light();
    GAMESTATE()->lights[0] = GAMESTATE()->entity_system.add(l);
    GAMESTATE()->lights[1] = GAMESTATE()->entity_system.add(l);

    Player player = {};
    GAMESTATE()->entity_system.add(player);

    Physics::AABody b;
    b.position = { 0, 0, 0 };
    b.half_size = { 5, 1, 5 };
    b.mass = 0.0;
    GAMESTATE()->physics_engine.add_box(b);
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);
    Asset::reload();
#ifdef IMGUI_ENABLE
    ImGui::SetCurrentContext((ImGuiContext *)game->imgui.context);
#endif
    GFX::remake_render_target();
}

void update() {
    if (Input::released(Ac::MouseToggle)) {
        GAMESTATE()->input.mouse_capture ^= 1;
    }

    handle_events();

    // Debug camera movement, overwritten by player currently.
    if (GFX::current_camera() == GFX::debug_camera()) {
        Vec3 move = { Input::value(Ac::MoveX), 0, Input::value(Ac::MoveZ) };
        Vec2 turn = Input::mouse_move();
        move = move * delta();
        turn = turn * delta();
        GFX::debug_camera()->turn(turn.y, turn.x);
        GFX::debug_camera()->move_relative(move);
        GFX::debug_camera()->move(
            Vec3(0, Input::value(Ac::MoveY) * delta(), 0));
    }
    GAMESTATE()->entity_system.update();
    GAMESTATE()->physics_engine.update(delta());
}

void draw() {
    if (GAMESTATE()->resized_window) {
        GAMESTATE()->resized_window = false;
        GFX::set_screen_resolution();
    }
    GFX::RenderTexture target = GFX::render_target();
    target.use();
    glClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // NOTE(ed): Note that this function has sideeffects,
    // it can set the camera and do things like that.
    do_imgui_stuff();

    GFX::MasterShader shader = GFX::master_shader();
    shader.use();
    shader.upload_t(time());

    GFX::current_camera()->upload(shader);
    shader.upload_bones(0, nullptr);

    shader.upload_sun_dir(GFX::lighting()->sun_direction);
    shader.upload_sun_color(GFX::lighting()->sun_color);
    shader.upload_ambient_color(GFX::lighting()->ambient_color);

    Vec3 position = Vec3(3, 0.5, 3);
    if (GAMESTATE()->entity_system.is_valid(GAMESTATE()->lights[0])) {
        Light *l = GAMESTATE()->entity_system.fetch<Light>(GAMESTATE()->lights[0]);
        if (GAMESTATE()->entity_system.have_ownership(l->entity_id)) {
            l->position = position + Vec3(0.5, 1.0 + sin(time()), 0.0);
            // l->color = Vec3(sin(time()) * 0.5 + 0.5, cos(time()) * 0.5 + 0.5, 0.2);
        }
    }

    if (GAMESTATE()->entity_system.is_valid(GAMESTATE()->lights[1])) {
        Light *l = GAMESTATE()->entity_system.fetch<Light>(GAMESTATE()->lights[1]);
        if (GAMESTATE()->entity_system.have_ownership(l->entity_id)) {
            l->position = position + Vec3(1.0 + cos(time()), 1.0, sin(time()));
            // l->color = Vec3(0.5, 0.5, 0.9);
        }
    }

    shader.upload_lights(GFX::lighting()->light_positions,
                         GFX::lighting()->light_colors);

    Asset::fetch_texture("RGBA")->bind(0);
    shader.upload_tex(0);

    GAMESTATE()->entity_system.draw();

    GFX::debug_shader().use();
    GFX::current_camera()->upload(GFX::debug_shader());
    GFX::draw_primitivs();

    GFX::resolve_to_screen(target);
}

#ifdef IMGUI_ENABLE
void do_imgui_stuff() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Reload")) {
                if (SDL_LockMutex(GAMESTATE()->m_reload_lib) == 0) {
                    *GAMESTATE()->reload_lib = true;
                    SDL_UnlockMutex(GAMESTATE()->m_reload_lib);
                } else {
                    ERR("Unable to lock mutex: {}", SDL_GetError());
                }
            }
            if (ImGui::MenuItem("Exit")) {
                SDL_Event quit_event;
                quit_event.type = SDL_QUIT;
                SDL_PushEvent(&quit_event);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Networking", "",
                            &GAMESTATE()->imgui.networking_enabled);
            ImGui::MenuItem("Show Rendered Target", "",
                            &GAMESTATE()->imgui.render_target_enabled);
            ImGui::MenuItem("Show Depth Map", "",
                            &GAMESTATE()->imgui.depth_map_enabled);
            ImGui::MenuItem("Show Entities", "",
                            &GAMESTATE()->imgui.entities_enabled);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::SliderFloat("Time",
                       &GAMESTATE()->imgui.t,
                       GAMESTATE()->imgui.min_t,
                       GAMESTATE()->imgui.max_t, "%.2f");

    {
        static int dim[2] = { 100, 100 };
        ImGui::SliderInt2("Screen size", dim, 100, 1600);
        if (ImGui::Button("Resize window")) {
            GFX::set_screen_resolution(dim[0], dim[1]);
        }
    }

    ImGui::Checkbox("Debug Draw Physics", &GAMESTATE()->imgui.debug_draw_physics);
    if (GAMESTATE()->imgui.debug_draw_physics) {
        GAMESTATE()->physics_engine.draw();
    }

    ImGui::Checkbox("Debug Camera", &GAMESTATE()->imgui.use_debug_camera);
    GFX::set_camera_mode(GAMESTATE()->imgui.use_debug_camera);

    ImGui::Checkbox("Show Grid", &GAMESTATE()->imgui.show_grid);
    if (GAMESTATE()->imgui.show_grid) {
        const i32 grid_size = 10;
        const f32 width = 0.005;
        const Color4 color = GFX::color(7) * 0.4;
        for (f32 x = 0; x <= grid_size; x += 0.5) {
            GFX::push_line(Vec3(x, 0, grid_size), Vec3(x, 0, -grid_size), color, width);
            GFX::push_line(Vec3(-x, 0, grid_size), Vec3(-x, 0, -grid_size), color, width);
            GFX::push_line(Vec3(grid_size, 0, x), Vec3(-grid_size, 0, x), color, width);
            GFX::push_line(Vec3(grid_size, 0, -x), Vec3(-grid_size, 0, -x), color, width);
        }
    }

    // TODO(ed): The lighting should have better default values.
    GFX::Lighting *lighting = GFX::lighting();
    ImGui::InputFloat3("Sun Color", lighting->sun_color._, 3);
    ImGui::InputFloat3("Sun Direction", lighting->sun_direction._, 3);
    lighting->sun_direction = normalized(lighting->sun_direction);
    ImGui::InputFloat3("Ambient Color", lighting->ambient_color._, 3);

    // Draws what is drawn to the internal buffers.
    auto draw_render_target = [](i32 tex, i32 width, i32 height) {
        const Vec2 window_size = ImGui::GetWindowSize();
        const i32 height_with_spacing = ImGui::GetFrameHeightWithSpacing();
        const i32 frame_height = ImGui::GetFrameHeightWithSpacing();
        if (ImGui::Button("Default width/height")) {
            ImGui::SetWindowSize(Vec2(width, height + 36 + frame_height));
        }
        ImGui::SameLine();
        if (ImGui::Button("Set width to height")) {
            ImGui::SetWindowSize(Vec2(window_size.y - (36 + height_with_spacing),
                                      window_size.y));
        }
        void *tex_as_ptr = (void *)(u64)tex;
        ImGui::Image(tex_as_ptr,
                     window_size - Vec2(0, 36 + height_with_spacing),
                     ImVec2(0, 1),
                     ImVec2(1, 0));
    };

    GFX::RenderTexture target = GFX::render_target();
    if (GAMESTATE()->imgui.render_target_enabled) {
        ImGui::Begin("Game View");
        draw_render_target(target.color, target.width, target.height);
        ImGui::End();
    }

    if (GAMESTATE()->imgui.depth_map_enabled) {
        ImGui::Begin("Depth");
        draw_render_target(target.depth_output, target.width, target.height);
        ImGui::End();
    }

    GAMESTATE()->network.imgui_draw();
}
#else
void do_imgui_stuff() {}
#endif

GameState update_game(GameState *game, GSUM mode) { // Game entry point
    _global_gs = game;
    if (mode.update)
        update();
    if (mode.draw)
        draw();
    if (mode.send) {
        GAMESTATE()->network.send_state_to_server();
        GAMESTATE()->network.send_state_to_clients();
    }
    SDL_LockMutex(game->m_event_queue);
    return *game;
}

void shutdown_game(GameState *game) {
    _global_gs = game;
    GAMESTATE()->network.disconnect_from_server();
    GAMESTATE()->network.stop_server();
}
