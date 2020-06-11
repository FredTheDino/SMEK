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

static void load_texture(UsableAsset *asset, FILE *file) {
    if (asset->loaded) {
        asset->texture.destroy();
    }

    struct {
        // read from file
        u32 width;
        u32 height;
        u32 components;
        u8  *data;

        u64 size() const {
            return width * height * components;
        }
    } raw_image;

    read(file, &raw_image);
    u64 size = raw_image.size();
    u8 *data = new u8[size];
    read<u8>(file, data, size);
    asset->texture = GFX::Texture::upload(raw_image.width,
                                          raw_image.height,
                                          raw_image.components,
                                          data,
                                          GFX::Texture::Sampling::LINEAR);
}

static void load_shader(UsableAsset *asset, FILE *file) {
    struct {
        // read from file
        u64 size;
        char *data;
    } source;
    read(file, &source);
    u32 size = source.size;
    char *data = new char[size];
    read<char>(file, data, size);

    GFX::Shader new_shader = GFX::Shader::compile(data);

    if (new_shader.is_valid()) {
        if (asset->loaded) {
            asset->shader.destroy();
        }
        asset->shader = new_shader;
    } else {
        ERROR("Failed to load shader due to errors.");
    }
}

static void load_model(UsableAsset *asset, FILE *file) {
    if (asset->loaded) {
        asset->mesh.destroy();
    }

    struct {
        // read from file
        u32 points_per_face;
        u32 num_faces;
        GFX::Mesh::Vertex *data;
    } model;

    read(file, &model);
    u32 num_faces = model.num_faces;
    u32 size = num_faces * model.points_per_face;
    model.data = new GFX::Mesh::Vertex[size];
    read(file, model.data, size);
    asset->mesh = GFX::Mesh::init(model.data, size);
}

static void load_skinned_mesh(UsableAsset *asset, FILE *file) {
    if (asset->loaded) {
        asset->skinned.destroy();
    }

    struct {
        // read from file
        u32 num_floats;
        GFX::SkinnedMesh::Vertex *data;
    } mesh;

    read(file, &mesh);
    u32 floats_per_vertex = (sizeof(GFX::SkinnedMesh::Vertex) / sizeof(real));
    u32 size = mesh.num_floats / floats_per_vertex;
    mesh.data = new GFX::SkinnedMesh::Vertex[size];
    read(file, mesh.data, size);
    asset->skinned = GFX::SkinnedMesh::init(mesh.data, size);
}

static void load_asset(UsableAsset *asset) {
    ASSERT(GAMESTATE()->main_thread == SDL_GetThreadID(NULL), "Should only be called from main thread");

    FILE *file = fopen(GAMESTATE()->asset_system.asset_path, "rb");
    defer { fclose(file); };
    fseek(file, GAMESTATE()->asset_system.file_header.data_offset + asset->header->data_offset, SEEK_SET);
    switch (asset->header->type) {
    case AssetType::TEXTURE: {
        load_texture(asset, file);
    } break;
    case AssetType::STRING: {
        read(file, &asset->string);
        u32 size = asset->string.size;
        asset->string.data = new char[size];
        read<char>(file, asset->string.data, size);
    } break;
    case AssetType::MESH: {
        load_model(asset, file);
    } break;
    case AssetType::SKINNED_MESH: {
        load_skinned_mesh(asset, file);
    } break;
    case AssetType::SHADER: {
        load_shader(asset, file);
    } break;
    case AssetType::SOUND: {
        // TODO(ed): If you unload, make sure there aren't race conditions with
        // the audio thread. :*
        read(file, &asset->sound);
        u32 size = asset->sound.num_samples;
        asset->sound.data = new f32[size];
        read<f32>(file, asset->sound.data, size);
    } break;
    default:
        ERROR("Unknown asset type {} in asset file {}",
              asset->header->type, GAMESTATE()->asset_system.asset_path);
        break;
    }

    asset->loaded = true;
    asset->dirty = false;
}

static
UsableAsset *_raw_fetch(AssetType type, AssetID id) {
    ASSERT(SDL_LockMutex(GAMESTATE()->asset_system.asset_lock) == 0, "Failed to aquire lock");
    ASSERT(valid_asset(id), "Invalid asset id '{}'", id);
    UsableAsset *asset = &GAMESTATE()->asset_system.assets[id];
    ASSERT(asset->header->type == type, "Type mismatch, type={}, id={}", type, id);

    if ((!asset->loaded) || asset->dirty) {
        load_asset(asset);
    }

    SDL_UnlockMutex(GAMESTATE()->asset_system.asset_lock);
    return asset;
}

GFX::Texture *fetch_image(AssetID id) {
    return &_raw_fetch(AssetType::TEXTURE, id)->texture;
}

StringAsset *fetch_string_asset(AssetID id) {
    return &_raw_fetch(AssetType::STRING, id)->string;
}

GFX::Shader *fetch_shader(AssetID id) {
    return &_raw_fetch(AssetType::SHADER, id)->shader;
}

GFX::Mesh *fetch_mesh(AssetID id) {
    return &_raw_fetch(AssetType::MESH, id)->mesh;
}

GFX::SkinnedMesh *fetch_skinned_mesh(AssetID id) {
    return &_raw_fetch(AssetType::SKINNED_MESH, id)->skinned;
}

Sound *fetch_sound(AssetID id) {
    return &_raw_fetch(AssetType::SOUND, id)->sound;
}

bool needs_reload(AssetID id) {
    ASSERT(valid_asset(id), "Invalid asset id '{}'", id);
    UsableAsset *asset = &GAMESTATE()->asset_system.assets[id];
    return (!asset->loaded) || asset->dirty;
}


bool reload() {
    System *system = &GAMESTATE()->asset_system;
    FILE *file = fopen(system->asset_path, "rb");
    if (!file) return false;

    FileHeader file_header;
    read(file, &file_header, 1);
    u64 num_assets = file_header.num_assets;

    AssetHeader *headers = new AssetHeader[num_assets];
    read(file, headers, num_assets);
    fclose(file);

    ASSERT(SDL_LockMutex(system->asset_lock) == 0, "Failed to lock asset system");
    system->file_header = file_header;
    for (u64 slot = 0; slot < num_assets; slot++) {
        AssetHeader *header = headers + slot;
        AssetID id(header->name_hash);
        if (system->assets.count(id)) {
            // Old asset
            UsableAsset &asset = system->assets[id];
            asset.dirty = asset.header->data_hash != header->data_hash;
            asset.dirty = true;
            asset.header = header;
        } else {
            // New asset
            UsableAsset data = {};
            data.header = header;
            data.dirty = true;
            system->assets[id] = data;
        }
    }
    delete[] system->headers;
    system->headers = headers;

    for (auto it = system->assets.cbegin(), next = it; it != system->assets.cend(); it = next) {
        ++next;
        if (headers > it->second.header && it->second.header >= (headers + num_assets)) {
            system->assets.erase(it);
        }
    }
    SDL_UnlockMutex(system->asset_lock);

    return true;
}

bool load(const char *path) {
    System *system = &GAMESTATE()->asset_system;
    system->asset_path = path;
    system->asset_lock = SDL_CreateMutex();
    ASSERT(SDL_LockMutex(system->asset_lock) == 0, "Failed to lock asset system");
    FILE *file = fopen(system->asset_path, "rb");
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
    SDL_UnlockMutex(system->asset_lock);

    return true;
}

}  // namespace Asset

#include "../test.h"
#include <cstring>

#if 0
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
#endif
