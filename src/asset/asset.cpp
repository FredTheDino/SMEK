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
    FILE *file = fopen(path, "rb");
    if (!file) return;

    //printf("%ld %#lx\n", ftell(file), ftell(file));
    read<FileHeader>(file, &system.file_header, 1);
    //printf("read<FileHeader>: %ld %#lx\n", ftell(file), ftell(file));
    u64 num_assets = system.file_header.num_assets;
    system.num_assets = num_assets;

    system.headers = (AssetHeader *) malloc(sizeof(AssetHeader) * num_assets);
    read<AssetHeader>(file, system.headers, num_assets);
    //printf("read<AssetHeader>: %ld %#lx\n", ftell(file), ftell(file));

    // for (u64 asset = 0; asset < num_assets; asset++) {
    //     printf("%u %lu %lu %lu %lu\n",
    //             system.headers[asset].type,
    //             system.headers[asset].name_hash,
    //             system.headers[asset].data_hash,
    //             system.headers[asset].data_size,
    //             system.headers[asset].data_offset);
    // }

    system.data = (AssetData *) malloc(sizeof(AssetData) * num_assets);
    for (u64 asset = 0; asset < num_assets; asset++) {
        AssetHeader header = system.headers[asset];
        AssetData *data_ptr = &system.data[asset];
        fseek(file, system.file_header.data_offset + header.data_offset, SEEK_SET);
        //printf("fseek: %ld %#lx\n", ftell(file), ftell(file));
        read<AssetData>(file, data_ptr);
        //printf("read<AssetData>%ld %#lx\n", ftell(file), ftell(file));
        switch (header.type) {
        case AssetType::TEXTURE: {
            u64 size = data_ptr->image.size();
            data_ptr->image.data = (u8 *) malloc(sizeof(u8[size]));
            read<u8>(file, data_ptr->image.data, size);
            //printf("read<u8>: %ld %#lx\n", ftell(file), ftell(file));

            printf("found image %03ux%03ux%1u: ",
                    data_ptr->image.width,
                    data_ptr->image.height,
                    data_ptr->image.channels);
            for (u32 p = 0; p < size; p++) {
                printf(" %3u", data_ptr->image.data[p]);
            }
            printf("\n");
            free(data_ptr->image.data);
        } break;
        default:
            break;
        }
    }
}

}  // namespace Asset
