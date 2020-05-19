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
    u32 channels;
    u8  *data;

    u64 size() const {
        return width * height * channels;
    }
};

struct StringAsset {
    // read from file
    u64 size;
    char *data;
};

struct Model {
    // read from file
    u32 points_per_face;
    u32 num_faces;
    f32 *data;
};

union AssetData {
    Image image;
    StringAsset string;
    Model model;
};

// not read directly from file
struct Asset {
    AssetHeader header;
    AssetData data;

    u64 name_hash;
    u64 data_hash;
};

///*
void load(const char *path);

///*
AssetID fetch_id(const char *name);

///*
StringAsset *fetch_string_asset(AssetID id);

}  // namespace Asset
