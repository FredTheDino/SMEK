import os
import shutil
import re
import platform
from glob import glob
from subprocess import Popen, PIPE
from collections import defaultdict
from itertools import chain


def shell(command):
    """Runs a command as if it were in the shell."""
    proc = Popen(command, stdout=PIPE)
    return proc.communicate()[0].decode()

def is_windows():
    return "MINGW64" in platform.uname().system or "MSYS_NT" in platform.uname().system

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


if GetOption("windows"):
    # linux -> windows
    if is_windows():
        print("It look's like you're currently using Windows.\nTherefore, --windows isn't needed.")
        Exit(1)
    print("Targeting foreign Windows")
    native = False
    env = Environment(ENV=os.environ, tools=["mingw"])
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
    env = Environment(ENV=os.environ)
    env.Replace(CXX="g++")
    env.MergeFlags(shell(["sdl2-config", "--cflags"]))
    env.MergeFlags(shell(["sdl2-config", "--libs"]))

    if is_windows():
        # native windows
        print("Targeting native Windows")
        env.Append(CPPDEFINES="WINDOWS")
        smek_game_lib = "./libSMEK.dll"
        env.Replace(SHLIBSUFFIX="dll")
    else:
        # native linux
        print("Targeting native Linux")
        env.Append(LINKFLAGS="-rdynamic")  # Gives backtrace information
        smek_game_lib = "./libSMEK.so"
        env.Replace(SHLIBSUFFIX="so")

env.MergeFlags("-Wall -Wno-unused -Wno-format-security -Wno-invalid-offsetof -Wno-class-memaccess -std=c++20")
env.MergeFlags("-Iinc -Llib")
env.Append(LIBS="dl")
env.Append(CPPDEFINES="IMGUI_IMPL_OPENGL_LOADER_GLAD")
env.Append(CPPDEFINES={"SMEK_GAME_LIB": smek_game_lib})
env.Replace(SHLIBPREFIX="lib")

# Generate a compilation database, has to be placed above all source files
# Requires scons 4.
if GetOption("gen_compilation_db"):
    try:
        env.Tool("compilation_db")
        compd = env.CompilationDatabase("compile_commands.json")
    except:
        print("\nLooks like you don't have Scons 4, required for compilation DB\n")
        raise
    print("Generating compilation DB")


if GetOption("verbose"):
    env.Append(CPPDEFINES="VERBOSE")
    env.Append(ASSETS_VERBOSE="--verbose")

if GetOption("no_color"):
    env.Append(CPPDEFINES="COLOR_DISABLE")

smek_dir = "bin/"

debug_flags = ["-ggdb", "-O0", "-DDEBUG"]
release_flags = ["-O2", "-DRELEASE"]
if GetOption("release"):
    smek_dir += "release"
    env.Append(CXXFLAGS=release_flags)
else:
    smek_dir += "debug"
    env.Append(CXXFLAGS=debug_flags)

if GetOption("windows"):
    smek_dir += "-windows"

smek_dir += "/"

if GetOption("no_imgui"):
    env.Append(CPPDEFINES="IMGUI_DISABLE")

VariantDir(smek_dir, "src", duplicate=0)

tests_dir = smek_dir + "tests/"
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
            asset_files["{}-{}".format("assets", "-".join(f.split("/")[1:-1]))].append(f)
    assets = [env.Assets(build_dir + "assets.bin", global_asset_files)]
    for out_file, files in asset_files.items():
        assets.append(env.Assets(build_dir + out_file + ".bin", files))
    return assets

tests_assets = env.Assets(tests_dir + "assets-tests.bin", glob("res/tests/*.*"))
assets = all_asset_targets(smek_dir)
env.Alias("assets", assets)
if native and not is_windows():
    AddPostAction(assets, "(pidof SMEK >/dev/null && kill -USR1 $$(pidof SMEK)) || true")

#TODO(gu) don't execute on clean
Execute("./tools/typesystem-gen.py")  # creates `src/entity/entity_types.{cpp,h}` so has to be run before the glob

source = glob("src/**/*.c*", recursive=True)

imgui = env.Object(smek_dir + "imgui.cpp")
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

if native and not is_windows():
    AddPostAction(libsmek, "(pidof SMEK >/dev/null && kill -USR1 $$(pidof SMEK)) || true")
smek_target = env.Alias("smek", smek)
Depends(smek_target, assets)
Depends(smek_target, libsmek)
Depends(smek_target, smek)

if GetOption("gen_compilation_db"):
    Depends(smek_target, compd)

Default(smek_target)

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
    tests_source = [tests_dir + "test.cpp"]
else:
    tests_source = [re.sub("^src/", tests_dir, f) for f in source]
tests = tests_env.Program(target=tests_dir + "tests", source=tests_source)

zip_name = "smek"
if GetOption("windows") or is_windows():
    files = "bin/debug-windows/libSMEK.dll bin/debug-windows/SMEK.exe bin/debug-windows/assets.bin lib/dll/*"
    zip_name += "-windows"
else:
    files = "bin/debug/libSMEK.so bin/debug/SMEK bin/debug/assets.bin"
    zip_name += "-linux"
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


env.Clean(smek_target, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(tests, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(docs, "docs/index.html")

