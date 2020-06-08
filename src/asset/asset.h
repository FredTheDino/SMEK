#pragma once

#include <vector>
#include <unordered_map>
#include "SDL2/SDL.h"

#include "../math/smek_vec.h"
#include "../renderer/renderer.h"

struct AssetID {
    AssetID(const char *);
    AssetID(u64 id): id(id) {}
    AssetID(): id(NONE()) {}

    static AssetID NONE() { return 0xFFFFFFFF; }

    u64 id;

    bool operator ==(AssetID &other) const {
        return id == other;
    }

    operator u64() const {
        return id;
    }
};

namespace std {
    template<> struct hash<AssetID> {
        std::size_t operator()(const AssetID &id) const noexcept {
            return u64(id);
        }
    };
}

namespace Asset {

///# Asset types
// These are the types of assets in the asset binary.

typedef enum {
    NONE = 0,
    TEXTURE = 1,
    STRING = 2,
    MESH = 3,
    SHADER = 4,
    SOUND = 5,
    SKINNED_MESH = 6,

    NUM_TYPES,
} AssetType;

struct FileHeader {
    // read from file
    u64 num_assets;
    u64 header_offset;
    u64 data_offset;
};

struct AssetHeader {
    // read from file
    AssetType type;
    u64 name_hash;
    u64 data_hash;
    u64 data_size;
    u64 data_offset;
};

///* Image
struct Image {
    // read from file
    u32 width;
    u32 height;
    u32 components;
    u8  *data;

    u64 size() const {
        return width * height * components;
    }
};

///* StringAsset
struct StringAsset {
    // read from file
    u64 size;
    char *data;
};


struct Sound {
    // read from file
    u32 channels;
    u32 sample_rate;
    u32 num_samples;
    f32 *data;
};

struct UsableAsset {
    union {
        GFX::Texture texture;
        GFX::Shader shader;
        GFX::Mesh mesh;
        GFX::SkinnedMesh skinned;
        StringAsset string;
        Sound sound;
    };

    AssetHeader *header;
    bool dirty;
    bool loaded;
};

struct System {
    // read directly from file
    FileHeader  file_header;
    AssetHeader *headers;
    std::unordered_map<AssetID, UsableAsset> assets;

    // not read directly from file
    u64 num_assets;
    const char *asset_path;

    SDL_mutex *asset_lock;
};

///*
// Check if an ID is valid, i.e points to *some* asset.
bool valid_asset(AssetID id);

///*
// Load the specified binary asset file.
// Does not currently support hot reloading.
bool load(const char *path);

///*
// Hot reloads the asset file passed in.
bool reload();

///*
// Fetch the ID corresponding to the asset with the specified name.
// Returns NO_ASSET if name is not found.
AssetID fetch_id(const char *name);

///*
// Returns true if the asset needs to be reloaded.
bool needs_reload(AssetID id);

///*
// Fetch an image-asset.
GFX::Texture *fetch_image(AssetID id);

///*
// Fetch a string asset.
StringAsset *fetch_string_asset(AssetID id);

///*
// Fetch a shader source asset.
GFX::Shader *fetch_shader(AssetID id);

///*
// Fetch a 3D-model.
GFX::Mesh *fetch_mesh(AssetID id);

///*
// Fetch a skinned 3D-model. (AKA, one with a skeleton/rig)
GFX::SkinnedMesh *fetch_skinned_mesh(AssetID id);

Sound *fetch_sound(AssetID id);

}  // namespace Asset
