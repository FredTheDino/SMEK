#include "util/util.h"

#include "game.h"
#include "test.h"
#include "renderer/opengl.h"
#include "renderer/renderer.h"
#include "asset/asset.h"

#include "math/smek_mat4.h"
#include "math/smek_math.h"

#include "imgui/imgui.h"

GameState *_global_gs;
#ifndef TESTS
GameState *GAMESTATE() { return _global_gs; }
#endif

bool should_update(GSUM gsmu) {
    return gsmu == GSUM::UPDATE || gsmu == GSUM::UPDATE_AND_RENDER;
}

bool should_draw(GSUM gsmu) {
    return gsmu == GSUM::RENDER || gsmu == GSUM::UPDATE_AND_RENDER;
}

f32 delta() { return GAMESTATE()->delta; }
f32 time() { return GAMESTATE()->time; }
u32 frame() { return GAMESTATE()->frame; }

void init_game(GameState *gamestate, int width, int height) {
    _global_gs = gamestate;
    GAMESTATE()->main_thread = SDL_GetThreadID(NULL);

    Asset::load("assets.bin");

    GFX::init(GAMESTATE(), width, height);

    GFX::lighting()->sun_direction = Vec3(0.0, 1.0, 0.0);
    *GFX::debug_camera() = GFX::Camera::init();
    GFX::debug_camera()->position = Vec3(0.0, 0.2, 0.0);

    *GFX::gameplay_camera() = GFX::Camera::init();
    GFX::gameplay_camera()->position = Vec3(0.0, 0.2, 0.0);

    GAMESTATE()->running = true;

    Input::bind(Ac::MoveX, 0, SDLK_a, -1.0);
    Input::bind(Ac::MoveX, 1, SDLK_d,  1.0);
    Input::bind(Ac::MoveY, 0, SDLK_q,  1.0);
    Input::bind(Ac::MoveY, 1, SDLK_e, -1.0);
    Input::bind(Ac::MoveZ, 0, SDLK_w, -1.0);
    Input::bind(Ac::MoveZ, 1, SDLK_s,  1.0);
    Input::bind(Ac::Jump, 0, SDLK_SPACE, 1.0);
    Input::bind(Ac::Shoot, 0, SDLK_f, 1.0);
    Input::bind(Ac::MouseToggle, 0, SDLK_m);
    Input::bind(Ac::Rebind, 1, SDLK_r);

    EventSystem::Event e = {
        .type = EventSystem::EventType::CREATE_PLAYER,
    };
    GAMESTATE()->event_queue.push(e);
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);
    Asset::reload();
#ifndef IMGUI_DISABLE
    ImGui::SetCurrentContext((ImGuiContext *) game->imgui_context);
#endif
}

void update() {
    if (Input::released(Ac::MouseToggle)) {
        GAMESTATE()->input.mouse_capture ^= 1;
    }

    EventSystem::handle_events();

    // Debug camera movement, overwritten by player currently.
    if (GFX::current_camera() == GFX::debug_camera()) {
        Vec3 move = {Input::value(Ac::MoveX), 0, Input::value(Ac::MoveZ)};
        Vec2 turn = Input::mouse_move();
        move = move * delta();
        turn = turn * delta();
        GFX::debug_camera()->turn(turn.y, turn.x);
        GFX::debug_camera()->move_relative(move);
        GFX::debug_camera()->move(
            Vec3(0, Input::value(Ac::MoveY) * delta(), 0));
    }
    GAMESTATE()->entity_system.update();

}


void draw() {

    static f32 t = 0;
#ifndef IMGUI_DISABLE
    ImGui::SliderFloat("Time", &t, 0.0, 16.0, "%.2f");

    static bool use_debug_camera = GFX::current_camera() == GFX::debug_camera();
    ImGui::Checkbox("Debug camera", &use_debug_camera);
    GFX::set_camera_mode(use_debug_camera);

    GFX::Lighting *lighting = GFX::lighting();
    ImGui::InputFloat3("Sun Color", lighting->sun_color._, 3);
    ImGui::InputFloat3("Sun Direction", lighting->sun_direction._, 3);
    lighting->sun_direction = normalized(lighting->sun_direction);
    ImGui::InputFloat3("Ambient Color", lighting->ambient_color._, 3);
#endif

    glClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GFX::MasterShader shader = GFX::master_shader();
    shader.use();
    shader.upload_t(time());

    GFX::current_camera()->upload(shader);
    shader.upload_bones(0, nullptr);

    shader.upload_sun_dir(GFX::lighting()->sun_direction);
    shader.upload_sun_color(GFX::lighting()->sun_color);
    shader.upload_ambient_color(GFX::lighting()->ambient_color);

    shader.upload_lights(2, // GFX::lighting()->num_lights,
                         GFX::lighting()->light_positions,
                         GFX::lighting()->light_colors);
    // Pops all lights
    GFX::lighting()->num_lights = 0;

    const i32 grid_size = 10;
    const f32 width = 0.005;
    const Vec4 color = GFX::color(7) * 0.4;
    for (f32 x = 0; x <= grid_size; x += 0.5) {
        GFX::push_point(Vec3(x, 0, x), GFX::color(2), width * 10);
        GFX::push_line(Vec3(x, 0, grid_size), Vec3(x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(-x, 0, grid_size), Vec3(-x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(grid_size, 0, x), Vec3(-grid_size, 0, x), color, width);
        GFX::push_line(Vec3(grid_size, 0, -x), Vec3(-grid_size, 0, -x), color, width);
    }

    Asset::fetch_image("RGBA")->bind(0);
    shader.upload_tex(0);

    GAMESTATE()->entity_system.draw();

#if 0
    GFX::Skin *mesh = Asset::fetch_skin("SKIN_SWINGING_CUBE");
    Mat model_matrix = Mat::translate(Math::cos(time()) * 0.2, Math::sin(time()) * 0.2, -0.5) * Mat::scale(1.0);
    shader.upload_model(model_matrix);
    shader.upload_t(time());
    mesh->draw();

    Vec3 p(Math::cos(time()) * 0.2, Math::sin(time()) * 0.2, -0.5);
    GFX::push_point(Vec3(p.x, p.y, p.z));
#endif

    AssetID skin, skel, anim;
    skin = "SKIN_UNTITLED";
    // TODO(ed): Not drawing...
    // Asset::fetch_skin(skin)->draw();
    skel = "SKEL_UNTITLED";
    // anim = "ANIM_SKINNEDMESHACTION_RIGGED_SIMPLE_CHARACTER";
    anim = "ANIM_ARMATUREACTION_001_UNTITLED";
    GFX::AnimatedMesh::init(skin, skel, anim).draw_at(t * 60);

    //Asset::fetch_skeleton(skel)->draw();

    GFX::debug_shader().use();
    GFX::current_camera()->upload(GFX::debug_shader());
    GFX::draw_primitivs();
}

GameState update_game(GameState *game, GSUM mode) { // Game entry point
    _global_gs = game;
    if (should_update(mode)) { update(); }
    if (should_draw(mode)) { draw(); }
    return *game;
}
