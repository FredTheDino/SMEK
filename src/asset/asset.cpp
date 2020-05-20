#include <cstdio>

#include "../util/log.h"

#include "asset.h"
#include "../main.h"

namespace Asset {

bool valid_asset(AssetID id) {
    return id < global_gamestate()->asset_system.num_assets;
}

u64 hash(const char *string) {
    u64 hash = 5351;
    while (*string) {
        char c = (*string++);
        hash = hash * c + c;
    }
    return hash;
}

AssetID fetch_id(const char *name) {
    u64 name_hash = hash(name);
    for (u32 asset = 0; asset < global_gamestate()->asset_system.num_assets; asset++) {
        if (global_gamestate()->asset_system.headers[asset].name_hash == name_hash) {
            return asset;
        }
    }
    WARN("Unable to find asset with name %s (hash is %lu)", name, name_hash);
    return NO_ASSET;
}

AssetData *_raw_fetch(AssetType type, AssetID id) {
    ASSERT(id < global_gamestate()->asset_system.num_assets, "Invalid asset id '%lu'", id);
    ASSERT(global_gamestate()->asset_system.headers[id].type == type, "Type mismatch, type=%ld, id=%ld", type, id);
    return &global_gamestate()->asset_system.data[id];
}

Image *fetch_image(AssetID id) {
    return &_raw_fetch(AssetType::TEXTURE, id)->image;
}

StringAsset *fetch_string_asset(AssetID id) {
    return &_raw_fetch(AssetType::STRING, id)->string;
}

Shader *fetch_shader(AssetID id) {
    return &_raw_fetch(AssetType::SHADER, id)->shader;
}

Model *fetch_model(AssetID id) {
    return &_raw_fetch(AssetType::MODEL, id)->model;
}

Image *fetch_image(const char *name) {
    return &_raw_fetch(AssetType::TEXTURE, fetch_id(name))->image;
}

StringAsset *fetch_string_asset(const char *name) {
    return &_raw_fetch(AssetType::STRING, fetch_id(name))->string;
}

Shader *fetch_shader(const char *name) {
    return &_raw_fetch(AssetType::SHADER, fetch_id(name))->shader;
}

Model *fetch_model(const char *name) {
    return &_raw_fetch(AssetType::MODEL, fetch_id(name))->model;
}

std::vector<Vec3> Model::positions() {
    std::vector<Vec3> positions = {};
    for (u32 i = 0; i < 8 * points_per_face * num_faces; i += 8*3) {
        positions.push_back(Vec3(data[i + 0],  data[i + 1],  data[i + 2]));
        positions.push_back(Vec3(data[i + 8],  data[i + 9],  data[i + 10]));
        positions.push_back(Vec3(data[i + 16], data[i + 17], data[i + 18]));
    }
    return positions;
}

//TODO(gu) re-implement
template <typename T>
size_t read(FILE *file, void *ptr, size_t num=1) {
    if (!num) return 0;
    size_t read = 0;
    while (read < num)  {
        size_t last_read = fread(ptr, sizeof(T), num - read, file);
        if (!last_read) return 0;
        read += last_read;
        ptr = (void *) ((u8 *) ptr + last_read + sizeof(T));
    }
    return read;
}

void load(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return;

    read<FileHeader>(file, &global_gamestate()->asset_system.file_header, 1);
    u64 num_assets = global_gamestate()->asset_system.file_header.num_assets;
    global_gamestate()->asset_system.num_assets = num_assets;

    global_gamestate()->asset_system.headers = new AssetHeader[num_assets];
    read<AssetHeader>(file, global_gamestate()->asset_system.headers, num_assets);

    global_gamestate()->asset_system.data = new AssetData[num_assets];
    for (u64 asset = 0; asset < num_assets; asset++) {
        AssetHeader header = global_gamestate()->asset_system.headers[asset];
        AssetData *data_ptr = &global_gamestate()->asset_system.data[asset];
        fseek(file, global_gamestate()->asset_system.file_header.data_offset + header.data_offset, SEEK_SET);
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
            u32 points_per_face = data_ptr->model.points_per_face;
            u32 num_faces = data_ptr->model.num_faces;
            u32 size = 8 * points_per_face * num_faces;  // 3+2+3 values per point
            data_ptr->model.data = new f32[size];
            read<f32>(file, data_ptr->model.data, size);
#if 0
            for (u32 i = 0; i < size; i += 8*3) {
                ModelFace face = {};
                face.vertices[0].position = Vec3(data[0], data[1], data[2]);
                face.vertices[0].texture = Vec2(data[3], data[4]);
                face.vertices[0].normal = Vec3(data[5], data[6], data[7]);
                face.vertices[1].position = Vec3(data[8], data[9], data[10]);
                face.vertices[1].texture = Vec2(data[11], data[12]);
                face.vertices[1].normal = Vec3(data[13], data[14], data[15]);
                face.vertices[2].position = Vec3(data[16], data[17], data[18]);
                face.vertices[2].texture = Vec2(data[19], data[20]);
                face.vertices[2].normal = Vec3(data[21], data[22], data[23]);
            }
#endif
        } break;
        case AssetType::SHADER: {
            read<Shader>(file, data_ptr);
            u32 size = data_ptr->shader.size;
            data_ptr->shader.data = new char[size];
            read<char>(file, data_ptr->shader.data, size);
        } break;
        default:
            ERROR("Unknown asset type %d for id %lu in asset file %s", header.type, asset, path);
            break;
        }
    }
}

}  // namespace Asset

#include "../test.h"
#include <cstring>

TEST_CASE("asset text", {
    Asset::load("bin/assets-test.bin");
    AssetID id = Asset::fetch_id("ALPHABET");

    return std::strcmp(Asset::fetch_string_asset(id)->data,
                       "abcdefghijklmnopqrstuvwxyz")
           == 0;
});

TEST_CASE("asset 1x1x3 png white", {
    Asset::load("bin/assets-test.bin");
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
    Asset::load("bin/assets-test.bin");
    AssetID id = Asset::fetch_id("TWO_BY_ONE");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 2
        && image->height     == 1
        && image->components == 4
        ;
});

TEST_CASE("asset 1x2x4 png", {
    Asset::load("bin/assets-test.bin");
    AssetID id = Asset::fetch_id("ONE_BY_TWO");
    if (!Asset::valid_asset(id)) return false;
    Asset::Image *image = Asset::fetch_image(id);

    return image->width      == 1
        && image->height     == 2
        && image->components == 4
        ;
});

TEST_CASE("asset 1x1x4 png white", {
    Asset::load("bin/assets-test.bin");
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
    Asset::load("bin/assets-test.bin");
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
    Asset::load("bin/assets-test.bin");
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
    Asset::load("bin/assets-test.bin");
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
    Asset::load("bin/assets-test.bin");
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
    Asset::load("bin/assets-test.bin");
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
