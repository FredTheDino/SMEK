#!/usr/bin/env python3
import os
import re
import struct
import pyhash
from glob import glob
from PIL import Image
"""
Format:
-------------------
  File header
-------------------
  Asset headers
-------------------
  Asset data
-------------------

###################

File header:
- Q Number of assets
- Q Header offset
- Q Data offset

Header format:
- I Type (texture, font, sprite etc)
- Q Name hash
- Q Data hash
- I Data size
- Q Data offset (first asset = 0)

"""

VERBOSE = os.environ.get("VERBOSE", None)

HEADER_OFFSET = struct.calcsize("QQQ")
HEADER_SIZE = struct.calcsize("IQQIQ")

TYPE_NONE = 0
TYPE_SPRITE = 1


def default_header():
    return {
        "type": TYPE_NONE,
        "name_hash": 0,
        "data_hash": 0,
        "data_size": 0,
        "data_offset": 0,
    }

def sprite_asset(name, path):
    """

    Data format:
    - I  Pixel width
    - I  Pixel height
    - I  Color channels
    - B> Data """

    im = Image.open(path)
    w, h = im.size

    mode = im.mode
    if mode == "RGB":
        c = 3
    elif mode == "RGBA":
        c = 4
    else:
        print(f"Image mode {mode} not supported")
        return None, None

    data = []
    for pixel in list(im.getdata()):
        data += [*pixel]

    header = default_header()
    header["type"] = TYPE_SPRITE
    header["data_size"] = len(data)

    fmt = "III{}B".format(len(data))
    return header, struct.pack(fmt, w, h, c, *data)


if __name__ == "__main__":
    extensions = {
        "png": sprite_asset,
        "jpg": sprite_asset,
        #"wav": sound_asset,
    }

    hasher = pyhash.metro_64()

    num_assets = 0
    cur_asset_offset = 0

    headers = []
    data = []

    asset_files = glob("res/*.*")
    print("=== FINDING ASSETS ===")
    for asset in asset_files:
        ext = asset.split(".")[-1]
        name = re.sub(r"[^A-Z0-9]", "_", "".join(asset.split("/")[-1].split(".")[:-1]).upper())
        print(asset + " -> ", end="")
        if ext in extensions:
            print(name)
            asset_header, asset_data = extensions[ext](name, asset)
            if asset_header and asset_data:
                num_assets += 1
                asset_header["data_offset"] = cur_asset_offset
                asset_header["name_hash"] = hasher(name)
                asset_header["data_hash"] = hasher(asset_data)
                cur_asset_offset += asset_header["data_size"]
                headers.append(asset_header)
                data.append(asset_data)
        else:
            print("Extension {} not supported".format(ext))

    data_offset = HEADER_OFFSET + HEADER_SIZE * num_assets

    if VERBOSE: print("\n=== WRITING DATA ===")
    with open("assets.bin", "wb") as f:
        if VERBOSE: print("== File header ==")
        if VERBOSE: print("writing file header: {}, {}, {}".format(hex(num_assets), hex(HEADER_OFFSET), hex(data_offset)))
        f.write(struct.pack("QQQ", num_assets, HEADER_OFFSET, data_offset))
        if VERBOSE: print("== Headers ==")
        for h in headers:
            f.write(struct.pack("IQQIQ", *h.values()))
            if VERBOSE: print("writing header {} as {}".format(h, [hex(val) for val in [*h.values()]]))
        if VERBOSE: print("== Data ==")
        for d in data:
            f.write(d)
