#include "game.h"

#ifdef TESTS
int main() { // Test entry point
    return _global_tests.run();
}

#else

int main() { // Game entrypoint
    GameState gs;
    init_game(&gs);
    update_game(0, &gs);
}

#endif
