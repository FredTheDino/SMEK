#include "../math/smek_math.h"

using AssetID = u64;

const AssetID NO_ASSET = 0xFFFFFFFF;

namespace Asset {

typedef enum {
    NONE = 0,
    TEXTURE = 1,

    NUM_TYPES,
} AssetType;

// read directly from file
struct FileHeader {
    u64 num_assets;
    u64 header_offset;
    u64 data_offset;
};

// read directly from file
struct AssetHeader {
    AssetType type;
    u64 name_hash;
    u64 data_hash;
    u64 data_size;
    u64 data_offset;
};

// read directly from file
struct Image {
    u32 width;
    u32 height;
    u32 channels;
    u8  *data;

    u64 size() const {
        return width * height * channels;
    }
};

// read directly from file
struct AssetData {
    union {
        Image image;
    };
};

// not read directly from file
struct Asset {
    AssetHeader header;
    AssetData data;

    u64 name_hash;
    u64 data_hash;
};

///*
u64 asset_hash(const char *str);  //TODO(gu)

///*
// Return the asset id for the specified name-hash.
// NO_ASSET if no asset is found.
AssetID fetch_id(const char *str);

///*
// Fetch the specified image from the asset file as a pointer. 
Image *fetch_image(AssetID id);

///*
void load(const char *path);

}  // namespace Asset
