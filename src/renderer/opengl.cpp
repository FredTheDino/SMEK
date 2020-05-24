#include "SDL2/SDL.h"
#include "opengl.h"

namespace GL {

bool load_procs() {
    return gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
}

} // namespace GL
