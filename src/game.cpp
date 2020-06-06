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

    *GFX::main_camera() = GFX::Camera::init();
    GFX::main_camera()->position = Vec3(0.0, 0.2, 0.0);

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

    Player player;
    GAMESTATE()->entity_system.add(player);
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);
    Asset::reload();


    game->audio_struct->lock();
    Audio::SoundSource test_source = game->audio_struct->sources[0];
    test_source.asset_id = AssetID("NOISE_STEREO_8K");
    Asset::fetch_sound(test_source.asset_id); // Should always be done on main thread.
    test_source.active = true;
    test_source.repeat = false;
    test_source.sample = 0;
    game->audio_struct->sources[0] = test_source;
    game->audio_struct->unlock();
}

void update() {
    if (Input::released(Ac::MouseToggle)) {
        GAMESTATE()->input.mouse_capture ^= 1;
    }

    EventSystem::handle_events();

    // Debug camera movement, overwritten by player currently.
    {
        Vec3 move = {Input::value(Ac::MoveX), 0, Input::value(Ac::MoveZ)};
        Vec2 turn = Input::mouse_move();
        move = move * delta();
        turn = turn * delta();
        GFX::main_camera()->turn(turn.y, turn.x);
        GFX::main_camera()->move_relative(move);
        GFX::main_camera()->move(Vec3(0, Input::value(Ac::MoveY) * delta(), 0));
    }
    GAMESTATE()->entity_system.update();
}

void draw() {
    glClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GFX::MasterShader shader = GFX::master_shader();
    shader.upload_t(time());

    const i32 grid_size = 10;
    const f32 width = 0.005;
    const Vec4 color = GFX::color(1);
    for (f32 x = 0; x <= grid_size; x += 0.5) {
        GFX::push_point(Vec3(x, 0, x), GFX::color(2), width * 10);
        GFX::push_line(Vec3(x, 0, grid_size), Vec3(x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(-x, 0, grid_size), Vec3(-x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(grid_size, 0, x), Vec3(-grid_size, 0, x), color, width);
        GFX::push_line(Vec3(grid_size, 0, -x), Vec3(-grid_size, 0, -x), color, width);
    }

    shader.use();
    GFX::main_camera()->upload(shader);

    Asset::fetch_image("RGBA")->bind(0);
    shader.upload_tex(0);

    GAMESTATE()->entity_system.draw();

#if 0
    GFX::Mesh mesh = *Asset::fetch_mesh("MONKEY");
    Mat model_matrix = Mat::translate(Math::cos(time()) * 0.2, Math::sin(time()) * 0.2, -0.5) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();

    model_matrix = Mat::translate(-Math::cos(time()) * 0.2, -Math::sin(time()) * 0.2, -0.5) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();

    model_matrix = Mat::translate(0, 0, -0.6) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();
#endif

    if (ImGui::Button("Reset camera"))
        *GFX::main_camera() = GFX::Camera::init();
    // ImGui::DragFloat3("pos.", (float *) &from, 0.01);
    // ImGui::DragFloat3("rot.", (float *) &rotation, 0.01);

    GFX::debug_shader().use();
    GFX::main_camera()->upload(GFX::debug_shader());
    GFX::draw_primitivs();
}

GameState update_game(GameState *game, GSUM mode) { // Game entry point
    _global_gs = game;
    if (should_update(mode)) { update(); }
    if (should_draw(mode)) { draw(); }
    return *game;
}
