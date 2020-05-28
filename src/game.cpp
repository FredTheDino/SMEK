#include "util/util.h"
#include "util/better_print.h"

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

GFX::Mesh mesh;
GFX::Texture texture;

void init_game(GameState *gamestate, int width, int height) {
    _global_gs = gamestate;
    Asset::load("assets.bin");


    GFX::init(GAMESTATE(), width, height);

    *GFX::main_camera() = GFX::Camera::init();
    GFX::main_camera()->position = Vec3(0.0, 0.2, 0.0);

    GAMESTATE()->running = true;

    Input::bind(Ac::MoveX, 0, SDLK_a, -1.0);
    Input::bind(Ac::MoveX, 1, SDLK_d,  1.0);
    Input::bind(Ac::MoveZ, 0, SDLK_w, -1.0);
    Input::bind(Ac::MoveZ, 1, SDLK_s,  1.0);
    Input::bind(Ac::Jaw, 0, SDLK_q, -1.0);
    Input::bind(Ac::Jaw, 1, SDLK_e,  1.0);
    Input::bind(Ac::Pitch, 0, SDLK_i,  1.0);
    Input::bind(Ac::Pitch, 1, SDLK_k, -1.0);
    Input::bind(Ac::MouseToggle, 0, SDLK_m);
    Input::bind(Ac::Rebind, 1, SDLK_r);
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);

    mesh = GFX::Mesh::init(Asset::fetch_model("MONKEY"));
    texture = GFX::Texture::upload(Asset::fetch_image("RGBA"), GFX::Texture::Sampling::NEAREST);

    game->audio_struct->lock();
    Audio::SoundSource test_source = game->audio_struct->sources[0];
    test_source.asset_id = Asset::fetch_id("NOISE_STEREO_8K");
    test_source.active = true;
    test_source.repeat = false;
    test_source.sample = 0;
    game->audio_struct->sources[0] = test_source;
    game->audio_struct->unlock();

    better_print("FMT", 1, 2, 3);
}

GameState update_game(GameState *game, GSUM mode) { // Game entry point
    glClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Input::released(Ac::MouseToggle)) {
        GAMESTATE()->input.mouse_capture ^= 1;
    }
    real time = SDL_GetTicks() / 1000.0;

    GFX::MasterShader shader = GFX::master_shader();
    shader.upload_t(time);

    Vec3 move = {Input::value(Ac::MoveX), 0, Input::value(Ac::MoveZ)};
    Vec2 turn = Input::mouse_move();
    move = move * 0.01;
    turn = turn * 0.01;
    GFX::main_camera()->turn(turn.y, turn.x);
    GFX::main_camera()->move_relative(move);


    const i32 grid_size = 10;
    const f32 width = 0.03;
    const Vec4 color = GFX::color(1);
    for (f32 x = 0; x <= grid_size; x += 0.5) {
        GFX::push_line(Vec3(x, 0, grid_size), Vec3(x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(-x, 0, grid_size), Vec3(-x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(grid_size, 0, x), Vec3(-grid_size, 0, x), color, width);
        GFX::push_line(Vec3(grid_size, 0, -x), Vec3(-grid_size, 0, -x), color, width);
    }

    shader.use();
    GFX::main_camera()->upload(shader);

    texture.bind(0);
    shader.upload_tex(0);

    Mat model_matrix = Mat::translate(Math::cos(time) * 0.2, Math::sin(time) * 0.2, -0.5) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();

    model_matrix = Mat::translate(-Math::cos(time) * 0.2, -Math::sin(time) * 0.2, -0.5) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();

    model_matrix = Mat::translate(0, 0, -0.6) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();

    ImGui::Begin("Hello, world!");
    if (ImGui::Button("Reset camera"))
        *GFX::main_camera() = GFX::Camera::init();
    // ImGui::DragFloat3("pos.", (float *) &from, 0.01);
    // ImGui::DragFloat3("rot.", (float *) &rotation, 0.01);
    ImGui::End();

    GFX::debug_shader().use();
    GFX::main_camera()->upload(GFX::debug_shader());
    GFX::draw_primitivs();


    return *game;
}

