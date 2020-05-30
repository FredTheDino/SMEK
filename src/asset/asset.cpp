#include <cstdio>

#include "../util/log.h"

#include "asset.h"
#include "../game.h"

AssetID::AssetID(const char *str) {
    *this = Asset::fetch_id(str);
}

namespace Asset {

bool valid_asset(AssetID id) {
    return id.id < GAMESTATE()->asset_system.num_assets;
}

static u64 hash(const char *string) {
    u64 hash = 5351;
    while (*string) {
        char c = (*string++);
        hash = hash * c + c;
    }
    return hash;
}

AssetID fetch_id(const char *name) {
    u64 name_hash = hash(name);
    for (u32 asset = 0; asset < GAMESTATE()->asset_system.num_assets; asset++) {
        if (GAMESTATE()->asset_system.headers[asset].name_hash == name_hash) {
            return asset;
        }
    }
    WARN("Unable to find asset with name {} (hash is {})", name, name_hash);
    return AssetID::NONE();
}


AssetData *_raw_fetch(AssetType type, AssetID id) {
    ASSERT(id < GAMESTATE()->asset_system.num_assets, "Invalid asset id '{}'", id);
    ASSERT(GAMESTATE()->asset_system.headers[id].type == type, "Type mismatch, type={}, id={}", type, id);
    return &GAMESTATE()->asset_system.data[id];
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

template <typename T>
void read(FILE *file, void *ptr, size_t num=1) {
    size_t gobbled = fread(ptr, sizeof(T), num, file);
    CHECK(gobbled == num, "Faild to read a part of an asset.");
}

void reload(const char *path) {

}

void load(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return;

    read<FileHeader>(file, &GAMESTATE()->asset_system.file_header, 1);
    u64 num_assets = GAMESTATE()->asset_system.file_header.num_assets;
    GAMESTATE()->asset_system.num_assets = num_assets;

    GAMESTATE()->asset_system.headers = new AssetHeader[num_assets];
    read<AssetHeader>(file, GAMESTATE()->asset_system.headers, num_assets);

    GAMESTATE()->asset_system.data = new AssetData[num_assets];
    for (u64 asset = 0; asset < num_assets; asset++) {
        AssetHeader header = GAMESTATE()->asset_system.headers[asset];
        AssetData *data_ptr = &GAMESTATE()->asset_system.data[asset];
        fseek(file, GAMESTATE()->asset_system.file_header.data_offset + header.data_offset, SEEK_SET);
        switch (header.type) {
        case AssetType::TEXTURE: {
            read<Image>(file, data_ptr);
            u64 size = data_ptr->image.size();
            data_ptr->image.data = new u8[size];
            read<u8>(file, data_ptr->image.data, size);
        } break;
        case AssetType::STRING: {
            read<StringAsset>(file, data_ptr);
            u32 size = data_ptr->string.size;
            data_ptr->string.data = new char[size];
            read<char>(file, data_ptr->string.data, size);
        } break;
        case AssetType::MODEL: {
            read<Model>(file, data_ptr);
            u32 num_faces = data_ptr->model.num_faces;
            u32 size = num_faces * 3;
            data_ptr->model.data = new Vertex[size];
            read<Vertex>(file, data_ptr->model.data, size);
        } break;
        case AssetType::SHADER: {
            read<ShaderSource>(file, data_ptr);
            u32 size = data_ptr->shader.size;
            data_ptr->shader.data = new char[size];
            read<char>(file, data_ptr->shader.data, size);
        } break;
        case AssetType::SOUND: {
            read<Sound>(file, data_ptr);
            u32 size = data_ptr->sound.num_samples;
            data_ptr->sound.data = new f32[size];
            read<f32>(file, data_ptr->sound.data, size);
        } break;
        default:
            ERROR("Unknown asset type {} for id {} in asset file {}", header.type, asset, path);
            break;
        }
    }
}

}  // namespace Asset

#include "../test.h"
#include <cstring>

TEST_CASE("asset text", {
    Asset::load("assets-tests.bin");
    AssetID id = Asset::fetch_id("ALPHABET");

    return std::strcmp(Asset::fetch_string_asset(id)->data,
                       "abcdefghijklmnopqrstuvwxyz")
           == 0;
});

TEST_CASE("asset 1x1x3 png white", {
    Asset::load("assets-tests.bin");
    AssetID id = Asset::fetch_id("ONE_BY_ONE_RGB_PNG_WHITE");
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
    AssetID id = Asset::fetch_id("TWO_BY_ONE");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 2
        && image->height     == 1
        && image->components == 4
        ;
});

TEST_CASE("asset 1x2x4 png", {
    Asset::load("assets-tests.bin");
    AssetID id = Asset::fetch_id("ONE_BY_TWO");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 2
        && image->components == 4
        ;
});

TEST_CASE("asset 1x1x4 png white", {
    Asset::load("assets-tests.bin");
    AssetID id = Asset::fetch_id("ONE_BY_ONE_RGBA_PNG_WHITE");
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
    AssetID id = Asset::fetch_id("ONE_BY_ONE_RGBA_PNG_RED");
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
    AssetID id = Asset::fetch_id("ONE_BY_ONE_RGBA_PNG_GREEN");
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
    AssetID id = Asset::fetch_id("ONE_BY_ONE_RGBA_PNG_BLUE");
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
    AssetID id = Asset::fetch_id("ONE_BY_ONE_RGBA_PNG_TRANS");
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
    AssetID id = Asset::fetch_id("ONE_BY_ONE_JPG_WHITE");
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
