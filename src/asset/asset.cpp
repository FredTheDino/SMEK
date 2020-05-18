#include <cstdio>

#include "../util/log.h"

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
    LOG("Reading asset file %s", path);
    FILE *file = fopen(path, "rb");
    if (!file) return;

    LOG("Reading file header");
    read<FileHeader>(file, &system.file_header, 1);
    u64 num_assets = system.file_header.num_assets;
    system.num_assets = num_assets;

    LOG("Reading %d asset headers", num_assets);
    system.headers = new AssetHeader[num_assets];
    read<AssetHeader>(file, system.headers, num_assets);

    for (u64 asset = 0; asset < num_assets; asset++) {
        LOG("Found asset with type %u, name %020lu, data %020lu",
            system.headers[asset].type,
            system.headers[asset].name_hash,
            system.headers[asset].data_hash);
    }

    system.data = new AssetData[num_assets];
    for (u64 asset = 0; asset < num_assets; asset++) {
        AssetHeader header = system.headers[asset];
        AssetData *data_ptr = &system.data[asset];
        fseek(file, system.file_header.data_offset + header.data_offset, SEEK_SET);
        switch (header.type) {
        case AssetType::TEXTURE: {
            LOG("Reading Texture at %#lx", ftell(file));
            read<Image>(file, data_ptr);
            u64 size = data_ptr->image.size();
            data_ptr->image.data = new u8[size];
            LOG("Reading %ld bytes (%dx%dx%d)",
                size,
                data_ptr->image.height,
                data_ptr->image.width,
                data_ptr->image.channels);
            read<u8>(file, data_ptr->image.data, size);

            delete[] data_ptr->image.data;
        } break;
        case AssetType::STRING: {
            LOG("Reading String at %#lx", ftell(file));
            read<StringAsset>(file, data_ptr);
            u32 size = data_ptr->string.size;
            data_ptr->string.data = new char[size];
            LOG("Reading %ld bytes", size);
            read<char>(file, data_ptr->string.data, size);

            delete[] data_ptr->string.data;
        } break;
        case AssetType::MODEL: {
            LOG("Reading Model at %#lx", ftell(file));
            read<Model>(file, data_ptr);
            u32 points_per_face = data_ptr->model.points_per_face;
            u32 num_faces = data_ptr->model.num_faces;
            u32 size = 8 * points_per_face * num_faces;  // 3+2+3 values per point
            data_ptr->model.data = new f32[size];
            LOG("Reading %ld bytes (%d faces)", size, num_faces);
            read<f32>(file, data_ptr->model.data, size);
            f32 *data = data_ptr->model.data;

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

            delete[] data_ptr->model.data;
        } break;
        default:
            ERROR("Unknown asset type %d for id %d in asset file %s", header.type, asset, path);
            break;
        }
    }
}

}  // namespace Asset
