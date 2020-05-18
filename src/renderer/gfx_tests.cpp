#include "../test.h"
#include "renderer.h"
#include "../util/util.h"

TEST_CASE("rendering_init", {
    // TODO(ed): Defer macro.
    GFX::init(game, 100, 100);
    defer { GFX::deinit(game); };

    return false;
});
