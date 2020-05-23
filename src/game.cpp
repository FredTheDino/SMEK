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

GFX::Mesh mesh;
GFX::Shader shader;
GFX::Texture texture;

void init_game(GameState *gamestate) {
    _global_gs = gamestate;
    Asset::load("assets.bin");

    GFX::init(GAMESTATE(), Asset::fetch_shader("MASTER_SHADER")->data, 600, 600);

    GAMESTATE()->running = true;
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);

    mesh = GFX::Mesh::init(Asset::fetch_model("MONKEY"));
    shader = GFX::default_shader();
    texture = GFX::Texture::upload(Asset::fetch_image("RGBA"), GFX::Texture::Sampling::NEAREST);
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

    auto view_loc = GL::GetUniformLocation(shader.program_id, "view");
    Mat view_matrix = Mat::perspective(PI / 4, 0.01, 1.0);
    view_matrix = view_matrix * Mat::look_at(Vec3(0, 0, 0),
                Vec3(Math::cos(time) * 0.2, Math::sin(time) * 0.0, 0.5), Vec3(0, 1, 0));
    GL::UniformMatrix4fv(view_loc, 1, false, view_matrix.data());


    shader.use();

    auto tex_loc = GL::GetUniformLocation(shader.program_id, "tex");
    texture.bind(0);
    GL::Uniform1i(tex_loc, 0);

    auto model_loc = GL::GetUniformLocation(shader.program_id, "model");
    Mat model_matrix = Mat::translate(Math::cos(time) * 0.2, Math::sin(time) * 0.2, -0.5) * Mat::scale(0.001);
    GL::UniformMatrix4fv(model_loc, 1, false, model_matrix.data());
    mesh.draw();

#if 1
    model_matrix = Mat::translate(-Math::cos(time) * 0.2, -Math::sin(time) * 0.2, -0.5) * Mat::scale(0.001);
    GL::UniformMatrix4fv(model_loc, 1, false, model_matrix.data());
    mesh.draw();
#endif

    model_loc = GL::GetUniformLocation(shader.program_id, "model");
    model_matrix = Mat::translate(0, 0, -0.5) * Mat::scale(0.0005);
    GL::UniformMatrix4fv(model_loc, 1, false, model_matrix.data());
    mesh.draw();


    SDL_GL_SwapWindow(GAMESTATE()->window);

    // GFX::deinit(GAMESTATE());
    return *game;
}
