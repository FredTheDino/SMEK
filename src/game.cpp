#include "util/log.h"

#include "game.h"
#include "test.h"
#include "renderer/opengl.h"
#include "renderer/renderer.h"
#include "asset/asset.h"

#include "math/smek_mat4.h"
#include "math/smek_math.h"

GameState *_global_gs;
#ifndef TESTS
GameState *GAMESTATE() { return _global_gs; }
#endif

GFX::Mesh rect;
GFX::Shader shader;
GFX::Texture texture;

void init_game(GameState *gamestate) {
    _global_gs = gamestate;
    Asset::load("assets.bin");

    GFX::init(GAMESTATE(), Asset::fetch_shader(Asset::fetch_id("MASTER_SHADER"))->data, 600, 600);

    GAMESTATE()->running = true;
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);

    std::vector<Vec3> XX = {
        Vec3(-1.0, -1.0, 1.0),
        Vec3(-1.0, 1.0, 1.0),
        Vec3(-1.0, -1.0, -1.0),
        Vec3(-1.0, 1.0, -1.0),
        Vec3(1.0, -1.0, 1.0),
        Vec3(1.0, 1.0, 1.0),
        Vec3(1.0, -1.0, -1.0),
        Vec3(1.0, 1.0, -1.0)
    };

    std::vector<Vec3> points = {
#if 1
        XX[1], XX[2], XX[0],
        XX[3], XX[6], XX[2],
        XX[7], XX[4], XX[6],
        XX[5], XX[0], XX[4],
        XX[6], XX[0], XX[2],
        XX[3], XX[5], XX[7],
        XX[1], XX[3], XX[2],
        XX[3], XX[7], XX[6],
        XX[7], XX[5], XX[4],
        XX[5], XX[1], XX[0],
        XX[6], XX[4], XX[0],
        XX[3], XX[1], XX[5],
#else
        Vec3(-0.5, -0.5, 0.0),
        Vec3( 0.0,  0.5, 0.0),
        Vec3( 0.5, -0.5, 0.0),
#endif
    };
    rect = GFX::Mesh::init(points);
    shader = GFX::default_shader();

    u8 image[] = {
        255, 0, 0, 255,   0, 0, 0, 255,  0, 0, 255, 255,
        255, 255, 0, 255,   0, 255, 0, 255,  0, 0, 255, 255,
        255, 255, 255, 255,   0, 0, 0, 0,  0, 0, 0, 255,
    };

    texture = GFX::Texture::upload(3, 3, 4, image, GFX::Texture::Sampling::NEAREST);

}

GameState update_game(GameState *game, GSUM mode) { // Game entry point
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                GAMESTATE()->running = false;
        }
    }

    SDL_Delay(10);

    GL::ClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
    GL::Clear(GL::cCOLOR_BUFFER_BIT | GL::cDEPTH_BUFFER_BIT);

    real time = SDL_GetTicks() / 1000.0;

    auto t_loc = GL::GetUniformLocation(shader.program_id, "t");
    GL::Uniform1f(t_loc, time);

    auto proj_loc = GL::GetUniformLocation(shader.program_id, "proj");
    Mat proj_matrix = Mat::perspective(PI / 3, 0.01, 3.0);
    GL::UniformMatrix4fv(proj_loc, 1, true, proj_matrix.data());

    auto view_loc = GL::GetUniformLocation(shader.program_id, "view");

    Vec3 from = Vec3(0.5, 0.5, 0.1);

    Vec3 target = Vec3(Math::cos(time) * 0.2, Math::sin(time) * 0.0, -0.5);
    Vec3 up = Vec3(0, 1, 0);
    Mat view_matrix = Mat::look_at(from, target, up);
    GL::UniformMatrix4fv(view_loc, 1, true, view_matrix.data());


    shader.use();

    auto tex_loc = GL::GetUniformLocation(shader.program_id, "tex");
    texture.bind(0);
    GL::Uniform1i(tex_loc, 0);

    auto model_loc = GL::GetUniformLocation(shader.program_id, "model");
    Mat model_matrix = Mat::translate(Math::cos(time) * 0.2, Math::sin(time) * 0.2, -0.5) * Mat::scale(0.1);
    GL::UniformMatrix4fv(model_loc, 1, true, model_matrix.data());
    rect.draw();

#if 0
    model_matrix = Mat::translate(-Math::cos(time) * 0.2, -Math::sin(time) * 0.2, -0.5) * Mat::scale(0.1);
    GL::UniformMatrix4fv(model_loc, 1, true, model_matrix.data());
    rect.draw();
#endif

    model_loc = GL::GetUniformLocation(shader.program_id, "model");
    model_matrix = Mat::translate(0, 0, -0.6) * Mat::scale(0.1);
    GL::UniformMatrix4fv(model_loc, 1, true, model_matrix.data());
    rect.draw();


    SDL_GL_SwapWindow(GAMESTATE()->window);

    // GFX::deinit(GAMESTATE());
    return *game;
}

