#include <stdio.h>
#include <stdlib.h>

#include "asset.h"

namespace Asset {

struct System {
    // read directly from file
    FileHeader  file_header;
    AssetHeader *headers;
    AssetData   *data;

    // not read directly from file
    u64 num_assets;
} system = {};

AssetData *raw_fetch(AssetID id, AssetType type) {
    if (system.file_header.num_assets < id) {
        // invalid asset id
        return nullptr;
    }
    if (type == AssetType::NONE || system.headers[id].type == type) {
        return &system.data[id];  //TODO(gu) ??
    } else {
        // invalid type
        return nullptr;
    }
}

Image *fetch_image(AssetID id) {
    return &raw_fetch(id, AssetType::TEXTURE)->image;
}

AssetID fetch_id(const char *str) {
    AssetHeader *headers = system.headers;
    // u64 name_hash = hash(str);
    u64 name_hash = 0;
    for (u64 i = 0; i < system.file_header.num_assets; i++) {
        if (headers[i].name_hash == name_hash) return i;
    }
    return NO_ASSET;
}

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
#ifdef VERBOSE
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif
    FILE *file = fopen(path, "rb");
    if (!file) return;

    LOG("%ld %#lx\n", ftell(file), ftell(file));
    read<FileHeader>(file, &system.file_header, 1);
    LOG("read<FileHeader>: %ld %#lx\n", ftell(file), ftell(file));
    u64 num_assets = system.file_header.num_assets;
    system.num_assets = num_assets;

    system.headers = (AssetHeader *) malloc(sizeof(AssetHeader) * num_assets);
    read<AssetHeader>(file, system.headers, num_assets);
    LOG("read<AssetHeader>: %ld %#lx\n", ftell(file), ftell(file));

#ifdef VERBOSE
    for (u64 asset = 0; asset < num_assets; asset++) {
        printf("%u %20lu %20lu %lu %#lx\n",
                system.headers[asset].type,
                system.headers[asset].name_hash,
                system.headers[asset].data_hash,
                system.headers[asset].data_size,
                system.headers[asset].data_offset);
    }
#endif

    system.data = (AssetData *) malloc(sizeof(AssetData) * num_assets);
    for (u64 asset = 0; asset < num_assets; asset++) {
        AssetHeader header = system.headers[asset];
        AssetData *data_ptr = &system.data[asset];
        fseek(file, system.file_header.data_offset + header.data_offset, SEEK_SET);
        LOG("fseek: %ld %#lx\n", ftell(file), ftell(file));
        switch (header.type) {
        case AssetType::TEXTURE: {
            read<Image>(file, data_ptr);
            LOG("read<Image>%ld %#lx\n", ftell(file), ftell(file));
            u64 size = data_ptr->image.size();
            data_ptr->image.data = (u8 *) malloc(sizeof(u8[size]));
            read<u8>(file, data_ptr->image.data, size);
            LOG("read<u8>: %ld %#lx\n", ftell(file), ftell(file));

            free(data_ptr->image.data);
        } break;
        case AssetType::STRING: {
            read<StringAsset>(file, data_ptr);
            LOG("read<StringAsset>%ld %#lx\n", ftell(file), ftell(file));
            u32 size = data_ptr->string.size;
            data_ptr->string.data = (char *) malloc(sizeof(char[size]));
            read<char>(file, data_ptr->string.data, size);
            LOG("read<char>: %ld %#lx\n", ftell(file), ftell(file));

            free(data_ptr->string.data);
        } break;
        default:
            break;
        }
    }
}

}  // namespace Asset
