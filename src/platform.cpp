#include <stdio.h>
#include "game.h"
#include "util/log.h"
#include "util/log.cpp" // I know, just meh.

// This is very UNIX-y
#include <dlfcn.h>
#include <sys/stat.h>

struct GameLibrary {
    GameInitFunc init;
    GameReloadFunc reload;
    GameUpdateFunc update;

    void *handle;

    int last_time;
    int reload_frame_delay;
} game_lib = {};

int get_file_edit_time(const char *path) {
	struct stat attr;
	// Check for success
	if (stat(path, &attr)) {
		return -1;
	}
	return attr.st_ctime;
}

bool load_gamelib() {
    GameLibrary next_library = {};
    //
    // TODO(ed): Check if RTLD_NODELETE lets you reference memory from old loaded DLLs. That would be
    // cool and potentially costly...
    const char *path = "./libSMEK.so";

    int edit_time = get_file_edit_time(path);
    if (edit_time == game_lib.last_time)
        return false;

    if (game_lib.reload_frame_delay) {
        game_lib.reload_frame_delay--;
        return false;
    }

    void *tmp = dlopen(path, RTLD_NOW);
    if (!tmp) {
        return false;
    }

    dlerror(); // Clear all errors.
    dlsym(tmp, "init_game");
    if (const char *error = dlerror()) {
        dlclose(tmp);
        WARN("Failed to load symbol. (%s)", error);
        return false;
    }
    dlclose(tmp); // If it isn't unloaded here, the same library is loaded.

    // TODO(ed): Add locks in when they are needed
    if (game_lib.handle) { dlclose(game_lib.handle); }

    void *lib = dlopen(path, RTLD_NOW);
    ASSERT(lib, "Failed to open library safely");

    game_lib.handle = lib;
    game_lib.init = (GameInitFunc) dlsym(lib, "init_game");
    if (!game_lib.init) {
        UNREACHABLE("Failed to load \"init_game\" (%s)", dlerror());
    }
    game_lib.update = (GameUpdateFunc) dlsym(lib, "update_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"init_update\" (%s)", dlerror());
    }
    game_lib.reload = (GameReloadFunc) dlsym(lib, "reload_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"init_update\" (%s)", dlerror());
    }

    game_lib.last_time = edit_time;
    game_lib.reload_frame_delay = 10;
    return true;
}

int main() { // Game entrypoint
    if (!load_gamelib()) {
        UNREACHABLE("Failed to load the linked library the first time!");
    }
    GameState gs;
    game_lib.init(&gs);
    game_lib.reload(&gs);

    int frame = 0;
    const int RELOAD_TIME = 1; // Set this to a higher number to prevent constant disk-checks.

    while (gs.running) {
        // TODO(ed): Add a delay for when to actually reload it.
        frame++;
        if (frame == RELOAD_TIME) {
            frame = 0;
            if (load_gamelib()) {
                LOG("PLATFORM LAYER RELOAD!");
                game_lib.reload(&gs);
            }
            game_lib.reload(&gs);
        }

        gs = game_lib.update(&gs, GSUM::UPDATE_AND_RENDER);
    }
    return 0;
}
