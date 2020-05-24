import os
import shutil
import re
from glob import glob
from subprocess import Popen, PIPE
from collections import defaultdict


def shell(command):
    """Runs a command as if it were in the shell."""
    proc = Popen(command, stdout=PIPE)
    return proc.communicate()[0].decode()

AddOption("--verbose",
          dest="verbose",
          action="store_true",
          help="Print verbose output. Not fully respected.")

AddOption("--no-color",
          dest="color",
          action="store_false",
          default=True,
          help="Remove all color from output.")

AddOption("--ci",
          dest="ci",
          action="store_true",
          help="Print tests-output without \\r.")

AddOption("--report",
          dest="report",
          action="store_true",
          help="Save a tests-report. Only makes sense when running tests.")

env = Environment(ENV=os.environ)
env.Replace(CXX="g++")
env.Append(CXXFLAGS="-Wall")
env.Append(CXXFLAGS="-ggdb")
env.Append(CXXFLAGS="-O0")
env.Append(CXXFLAGS="-Wno-unused")
env.Append(CXXFLAGS=shell(["sdl2-config", "--cflags"]))
env.Append(CXXFLAGS="-Iinc")
env.Append(LINKFLAGS=shell(["sdl2-config", "--libs"]))
env.Append(LINKFLAGS="-ldl")
env.Append(LINKFLAGS="-rdynamic")  # Gives backtrace information

if GetOption("verbose"):
    env.Append(CPPDEFINES="VERBOSE")
    env.Append(ASSETS_VERBOSE="--verbose")

if not GetOption("color"):
    env.Append(CPPDEFINES="NO_COLOR")

if GetOption("ci"):
    env.Append(CPPDEFINES="CI")

if GetOption("report"):
    env.Append(CPPDEFINES="REPORT")

smek_dir = "bin/debug/"
VariantDir(smek_dir, "src", duplicate=0)

tests_dir = "bin/tests/"
VariantDir(tests_dir, "src", duplicate=0)


asset_gen = Builder(action="./asset-gen.py -o $TARGET -f $SOURCES $ASSETS_VERBOSE")
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

source = glob("src/**/*.c*", recursive=True)

smek_source = [re.sub("^src/", smek_dir, f) for f in source]
smek_source.remove(smek_dir + "test.cpp")
smek_source.remove(smek_dir + "platform.cpp")  # The platform layer
smek = env.Program(target=smek_dir + "SMEK", source=[smek_dir + "platform.cpp"])
libsmek = env.SharedLibrary(target=smek_dir + "libSMEK", source=smek_source)
Depends(smek, assets)
Depends(smek, libsmek)
Default(smek)

tests_env = env.Clone()
tests_env.Append(CPPDEFINES="TESTS")
tests_source = [re.sub("^src/", tests_dir, f) for f in source]
tests = tests_env.Program(target=tests_dir + "tests", source=tests_source)
Depends(tests, tests_assets)

AlwaysBuild(env.Alias("run", smek, "cd " + smek_dir + "; " + smek[0].abspath))
AlwaysBuild(env.Alias("tests", tests, "cd " + tests_dir + "; " + tests[0].abspath))

docs = env.Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

env.Clean(smek, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(tests, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(smek, glob("bin/**/report.txt", recursive=True))
env.Clean(tests, glob("bin/**/report.txt", recursive=True))
env.Clean(libsmek, glob("bin/**/libSMEK*", recursive=True))
env.Clean(docs, "docs/index.html")
env.Clean(assets, glob("bin/**/assets*.bin"))
