#pragma once

#include <vector>

#include "../math/smek_vec.h"

using AssetID = u64;

const AssetID NO_ASSET = 0xFFFFFFFF;

namespace Asset {

struct Vertex {
    Vec3 position;
    Vec2 texture;
    Vec3 normal;
};

struct ModelFace {
    Vertex vertices[3];
};

typedef enum {
    NONE = 0,
    TEXTURE = 1,
    STRING = 2,
    MODEL = 3,
    SHADER = 4,

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

struct StringAsset {
    // read from file
    u64 size;
    char *data;
};

struct Shader {
    // read from file
    u64 size;
    char *data;
};

struct Model {
    // read from file
    u32 points_per_face;
    u32 num_faces;
    f32 *data;

    std::vector<Vec3> positions();
};

union AssetData {
    Image image;
    StringAsset string;
    Shader shader;
    Model model;
};

// not read directly from file
struct Asset {
    AssetHeader header;
    AssetData data;

    u64 name_hash;
    u64 data_hash;
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
bool valid_asset(AssetID id);

///*
void load(const char *path);

///*
AssetID fetch_id(const char *name);

///*
Image *fetch_image(AssetID id);

///*
StringAsset *fetch_string_asset(AssetID id);

///*
Shader *fetch_shader(AssetID id);

///*
Model *fetch_model(AssetID id);

Image *fetch_image(const char *name);
StringAsset *fetch_string_asset(const char *name);
Shader *fetch_shader(const char *name);
Model *fetch_model(const char *name);

}  // namespace Asset
