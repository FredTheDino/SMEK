#pragma once

#include <vector>

#include "../math/smek_vec.h"

struct AssetID {
    AssetID(const char *);
    AssetID(u64 id): id(id) {}
    AssetID(): id(NONE()) {}

    static AssetID NONE() { return 0xFFFFFFFF; }

    u64 id;

    operator u64() const {
        return id;
    }
};

namespace Asset {

///# Asset types
// These are the types of assets in the asset binary.

typedef enum {
    NONE = 0,
    TEXTURE = 1,
    STRING = 2,
    MODEL = 3,
    SHADER = 4,
    SOUND = 5,

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

///* ShaderSource
struct ShaderSource {
    // read from file
    u64 size;
    char *data;
};

///* Model
struct Vertex {
    Vec3 position;
    Vec2 texture;
    Vec3 normal;
};
//
struct ModelFace {
    Vertex vertices[3];
};
//
struct Model {
    // read from file
    u32 points_per_face;
    u32 num_faces;
    Vertex *data;
};

struct Sound {
    // read from file
    u32 channels;
    u32 sample_rate;
    u32 num_samples;
    f32 *data;
};

union AssetData {
    Image image;
    StringAsset string;
    ShaderSource shader;
    Model model;
    Sound sound;
};

struct System {
    // read directly from file
    FileHeader  file_header;
    AssetHeader *headers;
    AssetData   *data;

    // not read directly from file
    u64 num_assets;
};

///*
// Check if an ID is valid, i.e points to *some* asset.
bool valid_asset(AssetID id);

///*
// Load the specified binary asset file.
// Does not currently support hot reloading.
void load(const char *path);

///*
// Hot reloads the asset file passed in.
void reload(const char *path);

///*
// Fetch the ID corresponding to the asset with the specified name.
// Returns NO_ASSET if name is not found.
AssetID fetch_id(const char *name);

///*
// Fetch an image-asset.
Image *fetch_image(AssetID id);

///*
// Fetch a string asset.
StringAsset *fetch_string_asset(AssetID id);

///*
// Fetch a shader source asset.
ShaderSource *fetch_shader(AssetID id);

///*
// Fetch a 3D-model.
Model *fetch_model(AssetID id);

Sound *fetch_sound(AssetID id);

}  // namespace Asset
