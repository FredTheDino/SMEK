import os
import shutil
import re
from glob import glob
from subprocess import Popen, PIPE
from collections import defaultdict
from itertools import chain


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

env = Environment(ENV=os.environ)
env.Replace(CXX="g++")
env.Append(CXXFLAGS="-Wall")
env.Append(CXXFLAGS="-std=c++20")
env.Append(CXXFLAGS="-Wno-unused -Wno-return-type-c-linkage")
env.Append(CXXFLAGS=shell(["sdl2-config", "--cflags"]))
env.Append(CXXFLAGS="-Iinc")
env.Append(LINKFLAGS=shell(["sdl2-config", "--libs"]))
env.Append(LINKFLAGS="-ldl")
env.Append(LINKFLAGS="-rdynamic")  # Gives backtrace information
env.Append(CPPDEFINES="IMGUI_IMPL_OPENGL_LOADER_GLAD")

debug_flags = ["-ggdb", "-O0", "-DDEBUG"]
release_flags = ["-O2", "-DRELEASE"]

if GetOption("verbose"):
    env.Append(CPPDEFINES="VERBOSE")
    env.Append(ASSETS_VERBOSE="--verbose")

if GetOption("no_color"):
    env.Append(CPPDEFINES="COLOR_DISABLE")

if GetOption("release"):
    smek_dir = "bin/release/"
    env.Append(CXXFLAGS=release_flags)
else:
    smek_dir = "bin/debug/"
    env.Append(CXXFLAGS=debug_flags)

if GetOption("no_imgui"):
    env.Append(CPPDEFINES="IMGUI_DISABLE")

VariantDir(smek_dir, "src", duplicate=0)

tests_dir = smek_dir + "tests/"
VariantDir(tests_dir, "src", duplicate=0)

asset_gen = Builder(action="./tools/asset-gen.py -o $TARGET -f $SOURCES $ASSETS_VERBOSE")
env.Append(BUILDERS={"Assets": asset_gen})

def all_asset_targets(build_dir):
    asset_files = defaultdict(list)
    global_asset_files = []
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
AddPostAction(assets, "(pidof SMEK >/dev/null && kill -USR1 $$(pidof SMEK)) || true")

source = glob("src/**/*.c*", recursive=True)



imgui = env.Object(smek_dir + "imgui.cpp")
glad = env.Object(smek_dir + "glad.cpp")
if GetOption("jumbo"):
    jumbo_source = source.copy()
    jumbo_source.remove("src/test.cpp")
    jumbo_source.remove("src/platform.cpp")
    jumbo_source.remove("src/game.cpp")
    jumbo_source.remove("src/imgui.cpp")
    jumbo_source.remove("src/glad.cpp")

    jumbo_env = env.Clone()
    jumbo_env.Append(CXXFLAGS=list(chain(("-include", source) for source in jumbo_source)))
    libsmek = jumbo_env.SharedLibrary(smek_dir + "libSMEK", [smek_dir + "game.cpp"])

    plt_env = env.Clone()
    plt_env.Append(CXXFLAGS=["-include", "src/util/tprint.cpp"])
    platform_source = plt_env.Object(smek_dir + "platform.cpp")
    smek = env.Program(smek_dir + "SMEK", [platform_source, glad, imgui])
else:
    smek_source = [re.sub("^src/", smek_dir, f) for f in source]
    smek_source.remove(smek_dir + "test.cpp")
    smek_source.remove(smek_dir + "platform.cpp")
    smek_source.remove(smek_dir + "imgui.cpp")
    smek_source.remove(smek_dir + "glad.cpp")
    libsmek = env.SharedLibrary(smek_dir + "libSMEK", [*smek_source])

    platform_source = [smek_dir + "platform.cpp", smek_dir + "util/tprint.cpp"]
    smek = env.Program(smek_dir + "SMEK", [*platform_source, glad, imgui])

AddPostAction(libsmek, "(pidof SMEK >/dev/null && kill -USR1 $$(pidof SMEK)) || true")
smek_target = env.Alias("smek", smek)
Depends(smek_target, assets)
Depends(smek_target, libsmek)
Depends(smek_target, smek)
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
tests_target = env.Alias("tests", tests, "cd " + tests_dir + "; " + tests[0].abspath + " " + " ".join(tests_runtime_flags))
Depends(tests_target, tests_assets)
Depends(tests_target, tests)

AlwaysBuild(env.Alias("run", smek_target, "cd " + smek_dir + "; " + smek[0].abspath))
AlwaysBuild(env.Alias("debug", smek_target, "cd " + smek_dir + "; " + "gdb " + smek[0].abspath))
AlwaysBuild(tests_target)

docs = env.Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

if GetOption("tags"):
    shell(["ctags", "-R", "src"])

env.Clean(smek_target, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(tests_target, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(docs, "docs/index.html")
