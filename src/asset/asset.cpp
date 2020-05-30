#include <cstdio>

#include "../util/log.h"

#include "asset.h"
#include "../game.h"

static u64 hash(const char *string) {
    u64 hash = 5351;
    while (*string) {
        char c = (*string++);
        hash = hash * c + c;
    }
    return hash;
}

AssetID::AssetID(const char *str) {
    id = hash(str);
}

namespace Asset {

bool valid_asset(AssetID id) {
    return GAMESTATE()->asset_system.assets.count(id);
}

template <typename T>
static void read(FILE *file, T *ptr, size_t num=1) {
    size_t gobbled = fread(ptr, sizeof(T), num, file);
    CHECK(gobbled == num, "Faild to read a part of an asset.");
}

static void load_asset(UsableAsset *asset) {
    FILE *file = fopen(GAMESTATE()->asset_system.asset_path, "rb");
    defer { fclose(file); };
    fseek(file, GAMESTATE()->asset_system.file_header.data_offset + asset->header->data_offset, SEEK_SET);
    switch (asset->header->type) {
    case AssetType::TEXTURE: {
        read(file, &asset->data.image);
        u64 size = asset->data.image.size();
        asset->data.image.data = new u8[size];
        read<u8>(file, asset->data.image.data, size);
    } break;
    case AssetType::STRING: {
        read(file, &asset->data.string);
        u32 size = asset->data.string.size;
        asset->data.string.data = new char[size];
        read<char>(file, asset->data.string.data, size);
    } break;
    case AssetType::MODEL: {
        read(file, &asset->data.model);
        u32 num_faces = asset->data.model.num_faces;
        u32 size = num_faces * 3;
        asset->data.model.data = new Vertex[size];
        read(file, asset->data.model.data, size);
    } break;
    case AssetType::SHADER: {
        read(file, &asset->data.shader);
        u32 size = asset->data.shader.size;
        asset->data.shader.data = new char[size];
        read<char>(file, asset->data.shader.data, size);
    } break;
    case AssetType::SOUND: {
        read(file, &asset->data.sound);
        u32 size = asset->data.sound.num_samples;
        asset->data.sound.data = new f32[size];
        read<f32>(file, asset->data.sound.data, size);
    } break;
    default:
        ERROR("Unknown asset type {} in asset file {}",
              asset->header->type, GAMESTATE()->asset_system.asset_path);
        break;
    }
}

AssetData *_raw_fetch(AssetType type, AssetID id) {
    ASSERT(valid_asset(id), "Invalid asset id '{}'", id);
    UsableAsset *asset = &GAMESTATE()->asset_system.assets[id];
    ASSERT(asset->header->type == type, "Type mismatch, type={}, id={}", type, id);

    if (!asset->loaded) {
        load_asset(asset);
    }

    return &asset->data;
}

Image *fetch_image(AssetID id) {
    return &_raw_fetch(AssetType::TEXTURE, id)->image;
}

StringAsset *fetch_string_asset(AssetID id) {
    return &_raw_fetch(AssetType::STRING, id)->string;
}

ShaderSource *fetch_shader(AssetID id) {
    return &_raw_fetch(AssetType::SHADER, id)->shader;
}

Model *fetch_model(AssetID id) {
    return &_raw_fetch(AssetType::MODEL, id)->model;
}

Sound *fetch_sound(AssetID id) {
    return &_raw_fetch(AssetType::SOUND, id)->sound;
}

void reload() {

}

bool load(const char *path) {
    System *system = &GAMESTATE()->asset_system;
    system->asset_path = path;
    FILE *file = fopen(path, "rb");
    if (!file) return false;

    read(file, &system->file_header, 1);
    u64 num_assets = system->file_header.num_assets;
    system->num_assets = num_assets;

    system->headers = new AssetHeader[num_assets];
    read(file, system->headers, num_assets);
    fclose(file);

    for (u64 slot = 0; slot < num_assets; slot++) {
        AssetHeader header = system->headers[slot];
        AssetID id(header.name_hash);
        ASSERT(system->assets.count(id) == 0, "Collision in asset system! Possibly reloading datafile.");
        UsableAsset data = {};
        data.header = system->headers + slot;
        data.dirty = true;
        data.loaded = false;
        system->assets[id] = data;
    }

    return true;
}

}  // namespace Asset

#include "../test.h"
#include <cstring>

TEST_CASE("asset text", {
    Asset::load("assets-tests.bin");
    AssetID id("ALPHABET");

    return std::strcmp(Asset::fetch_string_asset(id)->data,
                       "abcdefghijklmnopqrstuvwxyz")
           == 0;
});

TEST_CASE("asset 1x1x3 png white", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_ONE_RGB_PNG_WHITE");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 1
        && image->components == 3
        && image->data[0]    == 255
        && image->data[1]    == 255
        && image->data[2]    == 255
        ;
});

TEST_CASE("asset 2x1x4 png", {
    Asset::load("assets-tests.bin");
    AssetID id("TWO_BY_ONE");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 2
        && image->height     == 1
        && image->components == 4
        ;
});

TEST_CASE("asset 1x2x4 png", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_TWO");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 2
        && image->components == 4
        ;
});

TEST_CASE("asset 1x1x4 png white", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_ONE_RGBA_PNG_WHITE");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 1
        && image->components == 4
        && image->data[0]    == 255
        && image->data[1]    == 255
        && image->data[2]    == 255
        && image->data[3]    == 255
        ;
});

TEST_CASE("asset 1x1x4 png red", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_ONE_RGBA_PNG_RED");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 1
        && image->components == 4
        && image->data[0]    == 255
        && image->data[1]    == 0
        && image->data[2]    == 0
        && image->data[3]    == 255
        ;
});

TEST_CASE("asset 1x1x4 png green", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_ONE_RGBA_PNG_GREEN");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 1
        && image->components == 4
        && image->data[0]    == 0
        && image->data[1]    == 255
        && image->data[2]    == 0
        && image->data[3]    == 255
        ;
});

TEST_CASE("asset 1x1x4 png blue", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_ONE_RGBA_PNG_BLUE");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 1
        && image->components == 4
        && image->data[0]    == 0
        && image->data[1]    == 0
        && image->data[2]    == 255
        && image->data[3]    == 255
        ;
});

TEST_CASE("asset 1x1x4 png transparent", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_ONE_RGBA_PNG_TRANS");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 1
        && image->components == 4
        && image->data[3]    == 0
        ;
});

TEST_CASE("asset 1x1x3 jpg white", {
    Asset::load("assets-tests.bin");
    AssetID id("ONE_BY_ONE_JPG_WHITE");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 1
        && image->components == 3
        && image->data[0]    == 255
        && image->data[1]    == 255
        && image->data[2]    == 255
        ;
});
