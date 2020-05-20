#include "../test.h"
#include "renderer.h"
#include "../util/util.h"
#include "../asset/asset.h"

#include "opengl.h"

TEST_CASE("rendering_init", {
    Asset::load("bin/assets-test.bin");
    AssetID id = Asset::fetch_id("SIMPLE_SHADER");
    Asset::Shader *shader = Asset::fetch_shader(id);
    const u32 window_dim = 100;
    GFX::init(game, shader->data, window_dim, window_dim);
    defer { GFX::deinit(game); };
    GL::Finish();

    const u32 slize_dim = 10;
    const u32 buffer_size = slize_dim * slize_dim + slize_dim;
    const u32 start = window_dim / 2 - slize_dim / 2;
    u8 data[buffer_size];
    GL::ReadPixels(start, start, slize_dim, slize_dim, GL::cRED, GL::cUNSIGNED_BYTE, data);

    bool success = true;
    for (u32 i = 0; i < buffer_size / 2; i++)
        success &= data[i] == 0;

    return success;
});

TEST_CASE("rendering_triangle", {
    Asset::load("bin/assets-test.bin");
    AssetID id = Asset::fetch_id("SIMPLE_SHADER");
    Asset::Shader *shader_asset = Asset::fetch_shader(id);
    const u32 window_dim = 100;
    GFX::init(game, shader_asset->data, window_dim, window_dim);
    defer { GFX::deinit(game); };

    std::vector<Vec3> points(6);
    points[0] = Vec3(-0.,  1., 0.);
    points[1] = Vec3( 1., -1., 0.);
    points[2] = Vec3( 1.,  1., 0.);

    points[3] = Vec3(-0.,  1., 0.);
    points[4] = Vec3( 1., -1., 0.);
    points[5] = Vec3( 0., -1., 0.);

    GFX::Mesh cube = GFX::Mesh::init(points);
    GFX::Shader shader = GFX::default_shader();

    shader.use();
    cube.draw();

    SDL_GL_SwapWindow(game->window);
    GL::Finish();

    const u32 slize_dim = 10;
    const u32 buffer_size = slize_dim;
    const u32 start = window_dim / 2 - slize_dim / 2;
    u8 data[buffer_size];
    GL::ReadPixels(start, start, slize_dim, 1, GL::cRED, GL::cUNSIGNED_BYTE, data);

    bool success = true;
    for (u32 x = 0; x < slize_dim; x++) {
        bool passed = success;
        if (x < slize_dim / 2) success &= data[x] == 0;
        else success &= data[x] != 0;
        if (passed != success) {
            LOG("line: %d", x);
        }
    }

    return success;
});
