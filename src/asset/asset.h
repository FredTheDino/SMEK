#pragma once

#include <vector>
#include <unordered_map>
#include "SDL.h"

#include "../math/smek_vec.h"
#include "../renderer/renderer.h"

namespace std {
template <>
struct hash<AssetID> {
    std::size_t operator()(const AssetID &id) const noexcept {
        return u64(id);
    }
};
}

namespace Asset {

///# Asset System
// The asset system is in charge of managing the assets, an asset
// packer (tools/asset-gen.py) packs assets into a binary format
// which is then read by the engine. Assets are loaded when they
// are needed.

//// Asset Types and serialization
// Structs and functions related to reading assets from file. Doesn't
// really concern the rest of the engine or game code much.

///* AssetType
// A listing of all available asset types. Note that these must match
// the constants in the python script.
// TODO(ed): Make this an enum class, and add a format function.
enum AssetType {
    NONE = 0,
    TEXTURE = 1,
    STRING = 2,
    MESH = 3,
    SHADER = 4,
    SOUND = 5,
    SKINNED = 6,
    SKELETON = 7,
    ANIMATION = 8,
    LEVEL = 9,

    NUM_TYPES,
};

///* FileHeader
struct FileHeader {
    // read from file
    u64 num_assets;
    u64 header_offset;
    u64 name_offset;
    u64 data_offset;
};

///* AssetType
struct AssetHeader {
    // read from file
    AssetType type;
    u64 name_hash;
    u64 data_hash;
    u64 name_size;
    u64 name_offset;
    u64 data_size;
    u64 data_offset;

    char *name;
};

///* StringAsset
struct StringAsset {
    // read from file
    u64 size;
    char *data;
};

///* Sound
struct Sound {
    // read from file
    u32 channels;
    u32 sample_rate;
    u32 num_samples;
    f32 *data;
};

///* Level
struct Level {
    // TODO(ed): Change this representation
    // to not store the string. Parsing is slow.
    u64 size;
    char *data;
};

///* UsableAsset
struct UsableAsset {
    union {
        GFX::Texture texture;
        GFX::Shader shader;
        GFX::Mesh mesh;
        GFX::Skin skin;
        GFX::Skeleton skeleton;
        GFX::Animation animation;
        StringAsset string;
        Sound sound;
        Level level;
    };

    AssetHeader *header;
    bool dirty;
    bool loaded;
};

struct System {
    // read directly from file
    FileHeader file_header;
    AssetHeader *headers;
    std::unordered_map<AssetID, UsableAsset> assets;

    // not read directly from file
    u64 num_assets;
    const char *asset_path;

    SDL_mutex *asset_lock;
};

///*
// Check if an ID is valid, i.e points to *some* asset.
bool is_valid(AssetID id);

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

/// Asset Requests
// Functions for interacting with the asset system.

using namespace GFX;

///* fetch anything
// Queries the asset system, loads it to ram if needed.
// These functions are cheap to call, most of the time.
//
// The first time a fetch is called for an asset is when
// it is loaded.
Animation *fetch_animation(AssetID id);
Mesh *fetch_mesh(AssetID id);
Shader *fetch_shader(AssetID id);
Skeleton *fetch_skeleton(AssetID id);
Skin *fetch_skin(AssetID id);
Texture *fetch_texture(AssetID id);
Sound *fetch_sound(AssetID id);
StringAsset *fetch_string_asset(AssetID id);
Level *fetch_level(AssetID id);

} // namespace Asset
