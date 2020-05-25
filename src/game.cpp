#include "util/log.h"

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
GFX::Camera camera;

void init_game(GameState *gamestate) {
    _global_gs = gamestate;
    Asset::load("assets.bin");

    GFX::init(GAMESTATE(), 600, 600);

    GAMESTATE()->running = true;

    Input::bind(Ac::AButton, 0, SDLK_a, 1.0);
    Input::bind(Ac::AButton, 1, SDLK_s, 0.1);
    Input::bind(Ac::BButton, 0, SDLK_b, 1.0);
    Input::bind(Ac::BButton, 1, SDLK_n, 0.1);
    Input::bind(Ac::Rebind, 1, SDLK_r);
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);

    mesh = GFX::Mesh::init(Asset::fetch_model("MONKEY"));
    texture = GFX::Texture::upload(Asset::fetch_image("RGBA"), GFX::Texture::Sampling::NEAREST);
    camera = GFX::Camera::init();
}

GameState update_game(GameState *game, GSUM mode) { // Game entry point
    glClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Input::pressed(Ac::Rebind)) {
        Input::rebind(Ac::AButton);
    }

    real time = SDL_GetTicks() / 1000.0;

    GFX::MasterShader shader = GFX::master_shader();
    shader.upload_t(time);

    Mat proj_matrix = Mat::perspective(PI / 3, 0.01, 3.0);
    shader.upload_proj(proj_matrix);

    Vec3 from = {};
    Vec3 rotation = {};

    ImGui::Begin("Hello, world!");
    ImGui::DragFloat3("pos.", (float *) &from, 0.01);
    ImGui::DragFloat3("rot.", (float *) &rotation, 0.01);
    ImGui::End();

    Vec3 target = Vec3(Math::cos(time) * 0.2, Math::sin(time) * 0.0, -0.5);
    camera.look_at(target);
    camera.move_relative(-from);
    camera.turn(rotation);
    camera.upload(shader);


    shader.use();

    texture.bind(0);
    shader.upload_tex(0);

    Mat model_matrix = Mat::translate(Math::cos(time) * 0.2, Math::sin(time) * 0.2, -0.5) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();

#if 0
    model_matrix = Mat::translate(-Math::cos(time) * 0.2, -Math::sin(time) * 0.2, -0.5) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();
#endif


    ImGui::ShowDemoWindow();

    model_matrix = Mat::translate(0, 0, -0.6) * Mat::scale(0.1);
    shader.upload_model(model_matrix);
    mesh.draw();


    return *game;
}

