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

int main() { // Game entry point
    GFX::init();

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

        GL::ClearColor(0, 1, 0, 1);
        GL::Clear(GL::COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(_global_gs.window);
    }

    GFX::deinit();
    return 0;
}

#endif
