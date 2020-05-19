#!/usr/bin/env python3
"""Asset-file generator.

This module takes all supported asset files in the
res/-folder and packages them into a binary file
intended to be read as C-style structs.

The assets are parsed by functions which all
return a (header, data)-tuple. A header template
is returned by default_header() and additional
parameters cannot be added. The data consists of
already packed bytes according to any arbitrary
C-struct format. All data is packaged using the
`struct`-module from the Python standard library.

The following specification uses the same letters
for formats as https://docs.python.org/3/library/struct.html.

Format of binary file:
  - File header
  - Asset headers
  - Asset data

Format of file header:
- Q Number of assets
- Q Adress of first header-byte
- Q Adress of first data-byte

Format of asset header:
- I Type (texture, font, sprite etc)
- Q Name hash
- Q Data hash
- Q Data size                     [bytes]
- Q Data offset (first asset = 0) [bytes]

Format of asset data is arbitrary. In addition to
the formats specified by `struct`-documentation,
the data format specifies "variable amount" as >.
For example, images specifies the following
format:

    - I  Pixel width
    - I  Pixel height
    - I  Color channels
    - P  Data pointer
    - B> Data

In this case, the length of the data can be
calculated with width * height * channels. A
variable amount of data should never be sent
without a means of calculating the length before
reading it, which means you should not depend on,
for example, a terminating 0x00.
"""
import re
import struct
import pyhash
from glob import glob
from PIL import Image

VERBOSE = False  # set to True to debug asset headers

FILE_HEADER_FMT = "QQQ"
ASSET_HEADER_FMT = "IQQQQ"

HEADER_OFFSET = struct.calcsize(FILE_HEADER_FMT)
HEADER_SIZE = struct.calcsize(ASSET_HEADER_FMT)

TYPE_NONE = 0
TYPE_TEXTURE = 1
TYPE_STRING = 2
TYPE_MODEL = 3


def ll(x):
    return x % (2**64)

def asset_hash(string):
    h = 5351
    for c in string:
        h = ll(ll(h * ord(c)) + ord(c))
    return h;


def default_header():
    """Return a default header.

    Any additional parameters will invalidate the
    binary file."""
    return {
        "type": TYPE_NONE,
        "name_hash": 0,
        "data_hash": 0,
        "data_size": 0,
        "data_offset": 0,
    }

def sprite_asset(path):
    """Load an image.

    Data format:
    - I  Pixel width
    - I  Pixel height
    - I  Color channels
    - P  Data pointer
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
    for pixel in im.getdata():
        data += [*pixel]

    fmt = "IIIP{}B".format(len(data))

    header = default_header()
    header["type"] = TYPE_TEXTURE
    header["data_size"] = struct.calcsize(fmt)

    return header, struct.pack(fmt, w, h, c, 0, *data)


def string_asset(path):
    """Load an ASCII text file.

    Data format:
    - I  Number of characters
    - P  Data pointer
    - s> Data """

    data = "".join(open(path, "r").readlines()).rstrip()
    fmt = "QP{}s".format(len(data) + 1)

    header = default_header()
    header["type"] = TYPE_STRING
    header["data_size"] = struct.calcsize(fmt)

    return header, struct.pack(fmt, len(data)+1, 0, str.encode(data, "ascii"))

def model_asset(path):
    """Load a .obj model-file.

    Data format:
    - I  Points per face
    - I  Number of faces
    - P  Data pointer
    - f> Data

    Not included but present in file:
    - s   material library
    - s   material name
    - s   object name
    - s   smoothing group
    """

    vertices         = []
    texture_vertices = []
    normal_vertices  = []
    faces            = []

    for line in open(path, "r"):
        if line.startswith("#"):
            continue
        line = line.split("#")[0].strip()
        if line.startswith("v "):
            vertices.append([float(f) for f in line[2:].split(" ")])
        elif line.startswith("vt "):
            texture_vertices.append([float(f) for f in line[3:].split(" ")])
        elif line.startswith("vn "):
            normal_vertices.append([float(f) for f in line[3:].split(" ")])
        elif line.startswith("f "):
            faces.append([[int(i) for i in v.split("/")] for v in line[2:].split(" ")])
        else:
            print("Unable to parse line '{}' in file {}".format(line, path))
            continue

    points_per_face = len(faces[0])
    num_faces = len(faces)

    data = []
    for face in faces:
        for p in range(points_per_face):
            data += vertices[face[p][0]-1]
            data += texture_vertices[face[p][1]-1]
            data += normal_vertices[face[p][2]-1]

    fmt = "IIP{}f".format(len(data))

    header = default_header()
    header["type"] = TYPE_MODEL
    header["data_size"] = struct.calcsize(fmt)

    return header, struct.pack(fmt, points_per_face, num_faces, 0, *data)

if __name__ == "__main__":
    extensions = {
        "png": sprite_asset,
        "jpg": sprite_asset,
        "txt": string_asset,
        "glsl": string_asset,
        "obj": model_asset,
        #"wav": sound_asset,
    }

    hasher = pyhash.metro_64()

    num_assets = 0
    cur_asset_offset = 0

    headers = []
    data = []
    names = []

    asset_files = glob("res/*.*")
    print("=== FINDING ASSETS ===")
    for asset in asset_files:
        ext = asset.split(".")[-1]
        name = re.sub(r"[^A-Z0-9]", "_", "".join(asset.split("/")[-1].split(".")[:-1]).upper())
        print(asset + " -> ", end="")
        if ext in extensions:
            print(name)
            asset_header, asset_data = extensions[ext](asset)
            if asset_header and asset_data:
                num_assets += 1
                asset_header["data_offset"] = cur_asset_offset
                asset_header["name_hash"] = asset_hash(name)
                asset_header["data_hash"] = hasher(asset_data)
                cur_asset_offset += asset_header["data_size"]
                headers.append(asset_header)
                data.append(asset_data)
                names.append(name)
        else:
            print("Extension {} not supported".format(ext))

    data_offset = HEADER_OFFSET + HEADER_SIZE * num_assets

    print("\n=== WRITING DATA ===")
    print("\n".join(names))
    with open("bin/assets.bin", "wb") as f:
        if VERBOSE: print("== File header ==")
        if VERBOSE: print("Writing file header: {}, {}, {}".format(hex(num_assets), hex(HEADER_OFFSET), hex(data_offset)))
        f.write(struct.pack(FILE_HEADER_FMT, num_assets, HEADER_OFFSET, data_offset))
        if VERBOSE: print("== Headers ==")
        for h in headers:
            f.write(struct.pack(ASSET_HEADER_FMT, *h.values()))
            if VERBOSE: print("Writing header {} as {}".format(h, [hex(val) for val in [*h.values()]]))
        if VERBOSE: print("== Data ==")
        for d in data:
            f.write(d)
