#include <cstdio>
#include <cstdarg>

#include "SDL2/SDL.h"

#include "test.h"
#include "math/types.h"
#include "util/color.h"
#include "renderer/opengl.h"

#define ERROR(...) _smek_error_log(__FILE__, __LINE__, __VA_ARGS__)
void _smek_error_log(const char *file, u32 line, const char *message, ...) {
    std::fprintf(stderr, YELLOW "%s" RESET "|" BLUE "%d" RESET ": ", file, line);
    va_list args;
    va_start(args, message);
    std::vfprintf(stderr, message, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

int main() {
    std::printf("Hello world!\n");
#ifdef TESTS
    _global_tests.run();
    return 0;
#endif
    std::printf("Didn't run tests\n");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        ERROR("Failed to initalize SDL \"%s\"", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    const int width = 640;
    const int height = 480;
    SDL_Window *window = SDL_CreateWindow("SMEK - The new begining",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_OPENGL);

    if (window == NULL) {
        ERROR("Failed to create OpenGL window \"%s\"", SDL_GetError());
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    typedef void(*GL_CLEAR_COLOR_FUNC)(f32, f32, f32, f32);
    typedef void(*GL_CLEAR_FUNC)(u32);
    auto glClearColor = (GL_CLEAR_COLOR_FUNC) SDL_GL_GetProcAddress("glClearColor");
    auto glClear = (GL_CLEAR_FUNC) SDL_GL_GetProcAddress("glClear");

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

        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();
    return 0;
}

