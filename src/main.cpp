#include "util/log.h"

#include "main.h"
#include "test.h"
#include "renderer/opengl.h"
#include "renderer/renderer.h"

GameState _global_gs;

#ifdef TESTS

int main() { // Test entry point
    _global_tests.run();
    return 0;
}

#else

#include "math/smek_mat4.h"
#include "math/smek_math.h"

int main() { // Game entry point
    GFX::init();

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

    GFX::Mesh rect = GFX::Mesh::init(points);
    GFX::Shader shader = GFX::default_shader();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    running = false;
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
        // view_matrix = Mat::scale(1);
        GL::UniformMatrix4fv(view_loc, 1, false, view_matrix.data());

        auto model_loc = GL::GetUniformLocation(shader.program_id, "model");
        Mat model_matrix = Mat::translate(Math::cos(time) * 0.2, Math::sin(time) * 0.2, -0.5) * Mat::scale(0.001);
        GL::UniformMatrix4fv(model_loc, 1, false, model_matrix.data());

        shader.use();
        rect.draw();


        SDL_GL_SwapWindow(_global_gs.window);
    }

    GFX::deinit();
    return 0;
}

#endif
