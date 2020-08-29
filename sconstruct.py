import os
import shutil
import re
import platform
from glob import glob
from subprocess import Popen, PIPE
from collections import defaultdict
from itertools import chain

DEBUG_FLAGS = ["-ggdb", "-O0", "-DDEBUG"]
RELEASE_FLAGS = ["-O2", "-DRELEASE"]
WARNINGS = "-Wall -Wno-unused -Wno-format-security -Wno-invalid-offsetof"
CPPSTD = "c++20"

BIN_DIR = "bin/"

reload_action = Action("(pidof SMEK >/dev/null && kill -USR1 $$(pidof SMEK)) || true")

IMGUI_FILES_SRC = [
        "vendor/imgui/imgui.cpp",
        "vendor/imgui/imgui.h",
        "vendor/imgui/imgui_demo.cpp",
        "vendor/imgui/imgui_draw.cpp",
        "vendor/imgui/examples/imgui_impl_opengl3.cpp",
        "vendor/imgui/examples/imgui_impl_opengl3.h",
        "vendor/imgui/examples/imgui_impl_sdl.cpp",
        "vendor/imgui/examples/imgui_impl_sdl.h",
        "vendor/imgui/imgui_internal.h",
        "vendor/imgui/imgui_widgets.cpp",
        "vendor/imgui/imstb_rectpack.h",
        "vendor/imgui/imstb_textedit.h",
        "vendor/imgui/imstb_truetype.h",
        ]

system = platform.uname().system
PLATFORMS = {
        "windows": "MINGW64" in system or "MSYS_NT" in system,
        "linux": "Linux" in system,
        "darwin": "Darwin" in system,
        }


def shell(command):
    """Runs a command as if it were in the shell."""
    proc = Popen(command, stdout=PIPE)
    return proc.communicate()[0].decode()


AddOption("--verbose",
          dest="verbose",
          action="store_true",
          help="Print verbose output. Not fully respected.")

AddOption("--no-color",
          dest="no_color",
          action="store_true",
          help="Remove all color from output.")

AddOption("--ci",
          dest="ci",
          action="store_true",
          help="Print tests-output without \\r.")

AddOption("--report",
          dest="report",
          action="store_true",
          help="Save a tests-report. Only makes sense when running tests.")

AddOption("--tags",
          dest="tags",
          action="store_true",
          help="Runs ctags and generates a tag file.")

AddOption("--release",
          dest="release",
          action="store_true",
          help="Compiles the target in release mode")

AddOption("--no-imgui",
          dest="no_imgui",
          action="store_true",
          help="Compiles out all ImGui code")

AddOption("--jumbo",
          dest="jumbo",
          action="store_true",
          help="Compiles the code using Jumbo, this make it faster for clean builds")

AddOption("--windows",
          dest="windows",
          action="store_true",
          help="Compile a Windows executable")

AddOption("--compilation-db",
          dest="gen_compilation_db",
          action="store_true",
          help="Generates a compilation database for use with linters and tools")


if GetOption("windows") and PLATFORMS["windows"]:
    print("It look's like you're currently using Windows.\nTherefore, --windows isn't needed.")
    Exit(1)

if GetOption("windows"):
    print("Targeting foreign Windows")
    native = False
    env = Environment(tools=["mingw"])
    env.Replace(CXX="x86_64-w64-mingw32-g++")
    env.MergeFlags(shell(["x86_64-w64-mingw32-sdl2-config", "--cflags"]))
    env.MergeFlags(shell(["x86_64-w64-mingw32-sdl2-config", "--libs"]))
    env.Append(LINKFLAGS=["-static-libgcc", "-static-libstdc++"])
    env.Append(LIBS="gcc")
    env.Append(CPPDEFINES="WINDOWS")
    smek_game_lib = "./libSMEK.dll"
    env.Replace(SHLIBSUFFIX="dll")
else:
    # native
    native = True
    env = Environment()
    env.Replace(CXX=ARGUMENTS.get("CXX", "g++"))
    env.MergeFlags(shell(["sdl2-config", "--cflags"]))
    env.MergeFlags(shell(["sdl2-config", "--libs"]))

    if PLATFORMS["windows"]:
        print("Targeting native Windows")
        env.Append(CPPDEFINES="WINDOWS")
        smek_game_lib = "./libSMEK.dll"
        env.Replace(SHLIBSUFFIX="dll")
    else:
        print("Targeting native Linux/macOS")
        env.Append(LINKFLAGS="-rdynamic")  # Gives backtrace information
        smek_game_lib = "./libSMEK.so"
        env.Replace(SHLIBSUFFIX="so")

env.MergeFlags(WARNINGS)
env.MergeFlags(f"-std={CPPSTD}")
env.MergeFlags("-Iinc -Llib")
env.Append(LIBS="dl")
env.Append(CPPDEFINES="IMGUI_IMPL_OPENGL_LOADER_GLAD")
env.Append(CPPDEFINES={"SMEK_GAME_LIB": smek_game_lib})
env.Replace(SHLIBPREFIX="lib")

# Generate a compilation database, has to be placed above all source files
# Requires scons 4.
if GetOption("gen_compilation_db"):
    print("Generating compilation DB")
    EnsureSConsVersion(4, 0)
    env.Tool("compilation_db")
    compd = env.CompilationDatabase("compile_commands.json")

if GetOption("verbose"):
    env.Append(CPPDEFINES="VERBOSE")
    env.Append(ASSETS_VERBOSE="--verbose")

if GetOption("no_color"):
    env.Append(CPPDEFINES="COLOR_DISABLE")

smek_dir = BIN_DIR
if GetOption("jumbo"):
    smek_dir += "jumbo-"
tests_dir = smek_dir + "tests/"
if GetOption("release"):
    smek_dir += "release"
    env.Append(CXXFLAGS=RELEASE_FLAGS)
else:
    smek_dir += "debug"
    env.Append(CXXFLAGS=DEBUG_FLAGS)
if GetOption("windows"):
    smek_dir += "-windows"
smek_dir += "/"

VariantDir(smek_dir, "src", duplicate=0)
VariantDir(tests_dir, "src", duplicate=0)

asset_gen = Builder(action="./tools/asset-gen.py -o $TARGET -f $SOURCES $ASSETS_VERBOSE")
env.Append(BUILDERS={"Assets": asset_gen})

def all_asset_targets(build_dir):
    asset_files = defaultdict(list)
    global_asset_files = ["./tools/asset-gen.py"]
    for f in glob("res/**/*.*", recursive=True):
        if f.count("/") == 1:
            global_asset_files.append(f)
        elif f.startswith("res/tests/"):
            continue
        else:
            asset_files["{}-{}".format("assets", "-".join(os.path.normcase(f).split("/")[1:-1]))].append(f)
    assets = [env.Assets(build_dir + "assets.bin", global_asset_files)]
    for out_file, files in asset_files.items():
        assets.append(env.Assets(build_dir + out_file + ".bin", files))
    return assets

tests_assets = env.Assets(tests_dir + "assets-tests.bin", glob("res/tests/*.*"))
assets = all_asset_targets(smek_dir)
env.Alias("assets", assets)
if native and PLATFORMS["linux"]:
    AddPostAction(assets, reload_action)

#TODO(gu) don't execute on clean
Execute("./tools/typesystem-gen.py")  # creates `src/entity/entity_types.{cpp,h}` so has to be run before the glob

source = glob("src/**/*.c*", recursive=True)

imgui = env.Object(smek_dir + "imgui.cpp")
if GetOption("no_imgui"):
    env.Append(CPPDEFINES="IMGUI_DISABLE")

imgui_files_dest = [f"inc/imgui/{os.path.basename(f)}" for f in IMGUI_FILES_SRC]
for src, dest in zip(IMGUI_FILES_SRC, imgui_files_dest):
    Command(dest, src, Copy(dest, src))
    Depends(imgui, dest)

glad = env.Object(smek_dir + "glad.cpp")
if GetOption("jumbo"):
    jumbo_source = source.copy()
    jumbo_source.remove("src/test.cpp")
    jumbo_source.remove("src/platform.cpp")
    jumbo_source.remove("src/game.cpp")

    jumbo_env = env.Clone()
    jumbo_env.Append(CXXFLAGS=list(chain(("-include", source) for source in jumbo_source)))
    libsmek = jumbo_env.SharedLibrary(smek_dir + smek_game_lib, [smek_dir + "game.cpp"])

    plt_env = env.Clone()
    plt_env.Append(CXXFLAGS=["-include", "src/util/tprint.cpp"])
    platform_source = plt_env.Object(smek_dir + "platform.cpp")
    smek = env.Program(smek_dir + "SMEK", [platform_source, glad, imgui])
else:
    smek_source = [re.sub("^src/", smek_dir, f) for f in source]
    smek_source.remove(smek_dir + "test.cpp")
    smek_source.remove(smek_dir + "platform.cpp")
    libsmek = env.SharedLibrary(smek_dir + smek_game_lib, [*smek_source])

    platform_source = [smek_dir + "platform.cpp", smek_dir + "util/tprint.cpp"]
    smek = env.Program(smek_dir + "SMEK", [*platform_source, glad, imgui])

if native and PLATFORMS["linux"]:
    AddPostAction(libsmek, reload_action)
smek_target = env.Alias("smek", smek)
Depends(smek_target, assets)
Depends(smek_target, libsmek)
Depends(smek_target, smek)
Default(smek_target)

if GetOption("gen_compilation_db"):
    Depends(smek_target, compd)

tests_runtime_flags = []
if GetOption("ci"):
    tests_runtime_flags.append("--ci")

if GetOption("report"):
    tests_runtime_flags.append("--report")

tests_env = env.Clone()
tests_env.Append(CPPDEFINES="TESTS")
tests_env.Append(CPPDEFINES="IMGUI_DISABLE")
if GetOption("jumbo"):
    tests_env.Append(CXXFLAGS=list(chain(("-include", s) for s in source if "imgui" not in s and "test" not in s)))
    tests_objs = [tests_env.Object(tests_dir + "test.cpp")]
    Depends(tests_objs[0], Command("inc/imgui/imgui.h", "vendor/imgui/imgui.h", Copy("inc/imgui/imgui.h", "vendor/imgui/imgui.h")))
else:
    tests_objs = [tests_env.Object(re.sub("^src/", tests_dir, f)) for f in source]
tests = tests_env.Program(target=tests_dir + "tests", source=tests_objs)

zip_name = "smek"
if GetOption("windows") or PLATFORMS["windows"]:
    files = "bin/debug-windows/libSMEK.dll bin/debug-windows/SMEK.exe bin/debug-windows/assets.bin lib/dll/*"
    zip_name += "-windows"
else:
    files = "bin/debug/libSMEK.so bin/debug/SMEK bin/debug/assets.bin"
    if PLATFORMS["linux"]:
        zip_name += "-linux"
    elif PLATFORMS["darwin"]:
        zip_name += "-macos"
    else:
        zip_name += "-unknown"
package_target = env.Alias("package", "", f"rm -f {zip_name}.zip; zip -j {zip_name} {files}")
Depends(package_target, smek_target)
AlwaysBuild(package_target)

if native:
    AlwaysBuild(env.Alias("run", smek_target, "cd " + smek_dir + "; " + smek[0].abspath))
    AlwaysBuild(env.Alias("debug", smek_target, "cd " + smek_dir + "; " + "gdb " + smek[0].abspath))
    tests_target = env.Alias("tests", tests, "cd " + tests_dir + "; " + tests[0].abspath + " " + " ".join(tests_runtime_flags))
    Depends(tests_target, tests_assets)
    Depends(tests_target, tests)
    AlwaysBuild(tests_target)

docs = env.Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

if GetOption("tags"):
    shell(["ctags", "-R", "src"])

env.Clean(smek_target, glob("bin/**/*.*", recursive=True))
env.Clean(tests, glob("bin/**/*.*", recursive=True))
env.Clean(docs, "docs/index.html")
