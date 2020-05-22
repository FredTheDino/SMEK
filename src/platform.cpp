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
        return true;

    void *tmp = dlopen(path, RTLD_NOW);
    if (!tmp) {
        return game_lib.handle != nullptr;
    }

    dlerror(); // Clear all errors.
    dlsym(tmp, "init_game");
    if (const char *error = dlerror()) {
        dlclose(tmp);
        WARN("Failed to load symbol. (%s)", error);
        return game_lib.handle != nullptr;
    }
    dlclose(tmp);

    // TODO(ed): Do this move all at once. To avoid potentiall raceconditions.
    if (game_lib.handle) { dlclose(game_lib.handle); }

    void *lib;
    do {
        lib = dlopen(path, RTLD_NOW);
        WARN("Reloading DLL");
    } while (lib == nullptr);

    game_lib.handle = lib;
    game_lib.init = (GameInitFunc) dlsym(lib, "init_game");
    if (!game_lib.init) {
        UNREACHABLE("Failed to load \"init_game\" (%s)", dlerror());
        return false;
    }
    game_lib.update = (GameUpdateFunc) dlsym(lib, "update_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"init_update\" (%s)", dlerror());
        return false;
    }
    game_lib.reload = (GameReloadFunc) dlsym(lib, "reload_game");
    if (!game_lib.update) {
        UNREACHABLE("Failed to load \"init_update\" (%s)", dlerror());
        return false;
    }

    game_lib.last_time = edit_time;
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
    const int RELOAD_TIME = 10;
    while (gs.running) {
        // TODO(ed): Add a delay for when to actually reload it.
        frame++;
        if (frame == RELOAD_TIME) {
            frame = 0;
            if (!load_gamelib()) {
                UNREACHABLE("Faild to reload the library");
            }
            game_lib.reload(&gs);
        }

        gs = game_lib.update(&gs, GSUM::UPDATE_AND_RENDER);
    }
    return 0;
}
