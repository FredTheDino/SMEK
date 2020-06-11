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
import sys
import struct
import wave
from glob import glob
from PIL import Image
from collections import defaultdict
from bs4 import BeautifulSoup

FILE_HEADER_FMT = "QQQ"
ASSET_HEADER_FMT = "IQQQQ"

HEADER_OFFSET = struct.calcsize(FILE_HEADER_FMT)
HEADER_SIZE = struct.calcsize(ASSET_HEADER_FMT)

TYPE_NONE = 0
TYPE_TEXTURE = 1
TYPE_STRING = 2
TYPE_MODEL = 3
TYPE_SHADER = 4
TYPE_SOUND = 5
TYPE_SKINNED_MESH = 6


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
    return {
        "type": TYPE_NONE,
        "name_hash": 0,
        "data_hash": 0,
        "data_size": 0,
        "data_offset": 0,
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

    return header, struct.pack(fmt, w, h, c, 0, *data)


def string_asset(path, verbose):
    """Load an ASCII text file.

    Data format:
    - I  Number of characters
    - P  Data pointer
    - s> Data
    """

    data = "".join(open(path, "r").readlines()).rstrip()
    fmt = "QP{}s".format(len(data) + 1)

    header = default_header()
    header["type"] = TYPE_STRING
    header["data_size"] = struct.calcsize(fmt)

    return header, struct.pack(fmt, len(data)+1, 0, str.encode(data, "ascii"))


def shader_asset(path, verbose):
    """Load a shader.

    Format is the same as for strings but with another type.
    """
    header, data = string_asset(path, verbose)
    header["type"] = TYPE_SHADER
    return header, data


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

    return header, struct.pack(fmt, points_per_face, num_faces, 0, *data)


def collada_asset(path, verbose):

    # Some helper functions
    def to_int_array(text):
        return [int(x) for x in text.split()]

    def to_float_array(text):
        return [float(x) for x in text.split()]

    def css(id):
        found = soup.select(id)
        if found:
            return found[0]
        return None

    def find_rec(id):
        if input_tag := css(id + " input"):
            return find_rec(input_tag["source"])
        return css(id + " float_array")

    with open(path, "r") as f:
        soup = BeautifulSoup(f, features="lxml")

    # This assumes there's only one skinned mesh in the file
    skin = soup("skin")[0]

    #
    # Parses the <vertex_weights> tag
    #
    vertex_weights = skin.find("vertex_weights")
    data = {}
    for input_tag in vertex_weights.find_all("input"):
        if input_tag["semantic"] == "WEIGHT":
            data[input_tag["semantic"]] = to_float_array(find_rec(input_tag["source"]).text)
        else:
            # Why, WHY!? Is "Name_array" capitalized?
            data[input_tag["semantic"]] = css(input_tag["source"] + " Name_array").text.split()

    def list_eater(l, sizes):
        gen_wj_pair = lambda: (l.pop(0), data["WEIGHT"][l.pop(0)])[::-1]
        return [[gen_wj_pair() for _ in range(size)] for size in sizes]

    def fill_out_to_three(l):
        # Contains side effects
        for i, weights in enumerate(l):
            weights = (sorted(weights) + [(0,0)] * 3)[:3]
            total = sum(map(lambda x: x[0], weights))
            if total:
                weights = [(w / total, j) for w, j in weights]
            l[i] = weights

    weights = to_int_array(skin.find("v").text)
    count =  to_int_array(skin.find("vcount").text)
    joint_weight_list = list_eater(weights, count)
    print(list_eater)
    assert len(weights) == 0
    fill_out_to_three(joint_weight_list) # Note: Side effects

    #
    # Parses the geometry for a skin, the <triangles> tag
    #
    triangles = css(skin["source"] + " triangles")
    for input_tag in triangles.find_all("input"):
        source = find_rec(input_tag["source"])
        stride = int(source.parent.find("accessor")["stride"])
        raw_floats = to_float_array(source.text)
        data[input_tag["semantic"]] = list(zip(*[raw_floats[i::stride] for i in range(stride)]))

    #
    # Parse out the indicies
    #
    ia = to_int_array(triangles.find("p").text)
    stride = 3
    vb = []
    for v, n, t in zip(ia[0::3], ia[1::3], ia[2::3]):
        vert = []
        vert.extend(data["VERTEX"][v])
        vert.extend(data["TEXCOORD"][t])
        vert.extend(data["NORMAL"][n])
        vert.extend(map(lambda x: x[0], joint_weight_list[v]))
        vert.extend(map(lambda x: x[1], joint_weight_list[v]))
        vb.extend(vert)

    fmt = "IP{}f".format(len(vb))
    header = default_header()
    header["type"] = TYPE_SKINNED_MESH
    header["data_size"] = struct.calcsize(fmt)

    return header, struct.pack(fmt, len(vb), 0, *vb)


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
        return header, struct.pack(fmt, channels, sample_rate, len(data), 0, *data)


EXTENSIONS = {
    "png": sprite_asset,
    "jpg": sprite_asset,
    "txt": string_asset,
    "glsl": shader_asset,
    "obj": model_asset,
    "dae": collada_asset,
    "wav": wav_asset,
}


def pack(asset_files, out_file, verbose=False):
    print("=== PACKING INTO {} ===".format(out_file))
    seen_name_hashes = set()

    num_assets = 0
    cur_asset_offset = 0

    headers = []
    data = []
    names = []

    for asset in asset_files:
        if asset.count(".") != 1:
            continue
        ext = asset.split(".")[-1]
        name = re.sub(r"[^A-Z0-9]", "_", ""
                      .join(asset
                            .split("/")[-1]
                            .split(".")[:-1])
                      .upper())
        print(asset + " -> ", end="")
        if ext in EXTENSIONS:
            name_hash = hash_string(name)
            if verbose:
                print(f"{name} ({name_hash})")
            else:
                print(name)
            if name_hash in seen_name_hashes:
                print(f"\nName hash collision! ({name})")
                sys.exit(1)
            seen_name_hashes.add(name_hash)
            asset_header, asset_data = EXTENSIONS[ext](asset, verbose)
            if asset_header and asset_data:
                num_assets += 1
                asset_header["data_offset"] = cur_asset_offset
                asset_header["name_hash"] = name_hash
                asset_header["data_hash"] = hash_bytes(asset_data)
                cur_asset_offset += asset_header["data_size"]
                headers.append(asset_header)
                data.append(asset_data)
                names.append(name)
        else:
            print("Extension {} not supported".format(ext))

    data_offset = HEADER_OFFSET + HEADER_SIZE * num_assets

    if verbose:
        print("=== PACKING THE FOLLOWING ASSETS ===")
        print("\n".join(names))
    with open(out_file, "wb") as f:
        f.write(struct.pack(FILE_HEADER_FMT, num_assets, HEADER_OFFSET, data_offset))
        for h in sorted(headers, key=lambda x: x["name_hash"]):
            f.write(struct.pack(ASSET_HEADER_FMT, *h.values()))
            if verbose:
                print("Writing header {} as {}".format(h, [hex(val) for val in [*h.values()]]))
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
