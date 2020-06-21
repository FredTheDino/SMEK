#!/usr/bin/env python3
"""Asset-file generator.

This module takes all supported asset files in the
res/-folder and packages them into binary files
intended to be read as C-style structs. Assets in
subfolders are packaged into different binary
files. Take for example the following structure.

res/
├── master_shader.glsl
└── test/
   └── alphabet.txt

Two asset files will be created.

- bin/assets.bin,      containing master_shader.glsl,
- bin/assets-test.bin, containing alphabet.txt.

The assets are parsed by functions which all
return a (header, data)-tuple. A header template
is returned by default_header() and additional
parameters cannot be added. The data consists of
already packed bytes according to any arbitrary
C-struct format. All data is packaged using the
`struct`-module from the Python standard library.
If an asset in the game wants to store a pointer
to something, that pointer has to be specified in
the format as well so it's read correctly.

The following specification uses the same letters
for formats as https://docs.python.org/3/library/struct.html.

Format of binary file:
- File header
- Asset headers
- Asset names
- Asset data

Format of file header:
- Q Number of assets
- Q Adress of first header-byte
- Q Adress of first name-byte
- Q Adress of first data-byte

Format of asset header:
- I Type (texture, font, sprite etc)
- Q Name hash
- Q Data hash
- Q Name size                     [bytes]
- Q Name offset (first asset = 0) [bytes]
- Q Data size                     [bytes]
- Q Data offset (first asset = 0) [bytes]
- P Name pointer

The names are a number of character-bytes (ASCII)
with length as specified by the asset header.

Format of asset data is arbitrary. In addition to
the formats specified by `struct`-documentation,
this data format specifies "variable amount" as >.
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
import sys
import struct
import wave
from glob import glob
from PIL import Image
from collections import defaultdict

FILE_HEADER_FMT = "QQQQ"
ASSET_HEADER_FMT = "IQQQQQQP"

HEADER_OFFSET = struct.calcsize(FILE_HEADER_FMT)
HEADER_SIZE = struct.calcsize(ASSET_HEADER_FMT)

TYPE_NONE = 0
TYPE_TEXTURE = 1
TYPE_STRING = 2
TYPE_MODEL = 3
TYPE_SHADER = 4
TYPE_SOUND = 5
TYPE_SKINNED = 6
TYPE_SKELETON = 7
TYPE_ANIMATION = 8


def ll(x):
    """Overflow unsigned long."""
    return x % (2**64)


def hash_string(string):
    h = 5351
    for c in string:
        h = ll(ll(h * ord(c)) + ord(c))
    return h


def hash_bytes(bytes):
    h = 5351
    for b in bytes:
        h = ll(ll(h * b) + b)
    return h


def default_header():
    """Return a default header.

    Any additional parameters will invalidate the
    binary file."""
    # the ordering here matters and has to match the format string
    return {
        "type": TYPE_NONE,
        "name_hash": 0,
        "data_hash": 0,
        "name_size": 0,
        "name_offset": 0,
        "data_size": 0,
        "data_offset": 0,

        "name_pointer": 0,
    }


def sprite_asset(path, verbose):
    """Load an image.

    Data format:
    - I  Pixel width
    - I  Pixel height
    - I  Color channels
    - P  Data pointer
    - B> Data
    """

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

    yield header, struct.pack(fmt, w, h, c, 0, *data), ""


def string_asset(path, verbose):
    """Load an ASCII text file.

    Data format:
    - Q  Number of characters
    - P  Data pointer
    - s> Data
    """

    data = "".join(open(path, "r").readlines()).rstrip()
    fmt = "QP{}s".format(len(data) + 1)

    header = default_header()
    header["type"] = TYPE_STRING
    header["data_size"] = struct.calcsize(fmt)

    yield header, struct.pack(fmt, len(data)+1, 0, str.encode(data, "ascii")), ""


def shader_asset(path, verbose):
    """Load a shader.

    Format is the same as for strings but with another type.
    """
    header, data, _ = next(string_asset(path, verbose))
    header["type"] = TYPE_SHADER
    yield header, data, ""


def model_asset(path, verbose):
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
    vertices = []
    texture_vertices = []
    normal_vertices = []
    faces = []

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
            if verbose: print("  Unable to parse line '{}' in file {}".format(line, path))
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

    yield header, struct.pack(fmt, points_per_face, num_faces, 0, *data), ""


def wav_asset(path, verbose):
    """Load a .wav-file.

    Data format:
    - I  Channels
    - I  Sample rate
    - I  Number of samples
    - P  Data pointer
    - f> Data
    """
    with wave.open(path, "rb") as file:
        sample_rate = file.getframerate()
        sample_width = file.getsampwidth()
        channels = file.getnchannels()

        def read_frames(fmt, size_bytes):
            data = []

            if fmt.isupper():
                sign_bit = 0
                sign_mov = 0.5
            else:
                sign_bit = 1
                sign_mov = 0

            div = 2**(8 * size_bytes - sign_bit) - 1

            frames = file.readframes(file.getnframes())
            if channels == 2:
                data = struct.unpack("{}{}".format(file.getnframes()*2, fmt), frames)
            elif channels == 1:
                data = struct.unpack("{}{}".format(file.getnframes(), fmt), frames)
            else:
                print("Unsupport number of channels ({}) in file '{}'".format(channels, path))
                sys.exit(1)

            return [d/div - sign_mov for d in data]

        types = { 1: "B", 2: "h", 4: "i" }
        if sample_width not in types:
            print("Unsupported bit depth {} for file '{}'", 8*sample_width, path)
            sys.exit(1)

        data = read_frames(types[sample_width], sample_width)
        fmt = "IIIP{}f".format(len(data))
        header = default_header()
        header["type"] = TYPE_SOUND
        header["data_size"] = struct.calcsize(fmt)
        yield header, struct.pack(fmt, channels, sample_rate, len(data), 0, *data), ""


def skinned_asset(path, verbose):
    """Reads a skinned mesh in the .edan format.

    Spits out 2+N assets, 1 mesh, 1 skeleton and N animations.
    """
    flatmap = lambda x, y: list(map(x, y))

    def parse_geo(line):
        data = flatmap(float, line.split())
        return [len(data)] + data, TYPE_SKINNED, f"I{len(data)}f", "SKIN_"

    def parse_arm(line):
        bones = []
        num_bones = line.count("|") + 1
        for bone in line.split("|"):
            splits = bone.split()
            parent, id = int(splits[0]), int(splits[1])
            transform = flatmap(float, splits[3:])
            bones += [parent, id, *transform]
        return [num_bones] + bones, TYPE_SKELETON, "i" + "ii10f" * num_bones, "SKEL_"

    def parse_anim(name, line):
        num_frames = line.count(";") + 1
        num_bones = None

        data = []
        for frame in line.split(";"):
            data.append(int(frame.split("=")[0]))

        for frame in line.split(";"):
            transforms = frame.split("=")[1]
            assert num_bones is None or num_bones == (transforms.count("|") + 1)
            num_bones = transforms.count("|") + 1
            for transform in transforms.split("|"):
                data += flatmap(float, transform.split())
        num_floats = 10 * num_bones * num_frames
        fmt = "II" + f"{num_frames}I"+ f"{num_floats}f"
        data = [num_frames, num_bones] + data
        return data, TYPE_ANIMATION, fmt, "ANIM_" + name.upper() + "_"


    with open(path, "r") as f:
        for line in f:
            splits = line.split(":")
            pre, splits = splits[0], splits[1:]

            if pre == "geo":
                data, t, fmt, post = parse_geo(*splits)
            elif pre == "arm":
                data, t, fmt, post = parse_arm(*splits)
            elif pre == "anim":
                data, t, fmt, post = parse_anim(*splits)

            header = default_header()
            header["type"] = t
            header["data_size"] = struct.calcsize(fmt)
            out = struct.pack(fmt, *(data[0:]))
            print(header["data_size"])
            yield header, out, post




EXTENSIONS = {
    "png": sprite_asset,
    "jpg": sprite_asset,
    "txt": string_asset,
    "glsl": shader_asset,
    "obj": model_asset,
    "edan": skinned_asset,
    "wav": wav_asset,
}


def pack(asset_files, out_file, verbose=False):
    print("=== PACKING INTO {} ===".format(out_file))
    seen_name_hashes = set()

    cur_asset_offset = 0
    cur_name_offset = 0

    headers = []
    data = []
    names = []

    for asset in asset_files:
        if asset.count(".") != 1:
            continue
        ext = asset.split(".")[-1]
        if ext in EXTENSIONS:
            for asset_header, asset_data, asset_prefix in EXTENSIONS[ext](asset, verbose):
                if not (asset_header and asset_data): continue
                name = re.sub(r"[^A-Z0-9]", "_",
                              asset_prefix +
                              "".join((asset)
                                    .split("/")[-1]
                                    .split(".")[:-1])
                              .upper())
                print(asset + " -> ", end="")

                name_hash = hash_string(name)
                if verbose:
                    print(f"{name} ({name_hash})")
                else:
                    print(name)
                if name_hash in seen_name_hashes:
                    print(f"\nName hash collision! ({name})")
                    sys.exit(1)
                seen_name_hashes.add(name_hash)

                asset_header["name_hash"] = name_hash
                asset_header["data_hash"] = hash_bytes(asset_data)
                asset_header["name_size"] = len(name)+1
                asset_header["name_offset"] = cur_name_offset
                asset_header["data_offset"] = cur_asset_offset
                cur_asset_offset += asset_header["data_size"]
                cur_name_offset += asset_header["name_size"]
                headers.append(asset_header)
                names.append(name)
                data.append(asset_data)
        else:
            print("Extension {} not supported".format(ext))

    names = [struct.pack("{}s".format(len(name)+1), str.encode(name, "ascii") + b'\0') for name in names]

    data_offset = name_offset + sum([len(name) for name in names])
    data_offset = HEADER_OFFSET + HEADER_SIZE * len(headers)

    if verbose:
        print("=== PACKING THE FOLLOWING ASSETS ===")
        print("\n".join(names))
    with open(out_file, "wb") as f:
        f.write(struct.pack(FILE_HEADER_FMT, len(headers), HEADER_OFFSET, data_offset))
        for h in sorted(headers, key=lambda x: x["name_hash"]):
            f.write(struct.pack(ASSET_HEADER_FMT, *h.values()))
            if verbose:
                print("Writing header {} as {}".format(h, [hex(val) for val in [*h.values()]]))
        for n in names:
            f.write(n)
        for d in data:
            f.write(d)


if __name__ == "__main__":
    from argparse import ArgumentParser as AP
    parser = AP(description="Processes assets and packs them into a binary format for the SMEK game.")
    parser.add_argument("-f", "--files", nargs="+", help="The data files to parse")
    parser.add_argument("-o", "--out", help="The result file to store in", default="assets")
    parser.add_argument("-v", "--verbose", action="store_true", help="Makes the output verbose and noisy")
    parser.add_argument("-e", "--extensions", action="store_true", help="Prints out the valid extensions, ovrrides all other options")
    args = parser.parse_args()

    if args.extensions:
        print(" ".join(EXTENSIONS.keys()))
        from sys import exit
        exit(0)

    auto_mode = True
    if args.files:
        auto_mode = False
        sources = args.files
    else:
        sources = glob("res/**/*", recursive=True)

    output_file = args.out
    verbose = args.verbose

    if auto_mode:
        asset_files = defaultdict(list)
        global_asset_files = []
        for f in sources:
            if f.count("/") == 1:
                global_asset_files.append(f)
            else:
                asset_files["{}-{}".format(output_file, "-".join(f.split("/")[1:-1]))].append(f)
        pack(global_asset_files, output_file + ".bin", verbose)
        for out, assets in asset_files.items():
            pack(assets, out + ".bin")
    else:
        pack(sources, output_file, verbose)
