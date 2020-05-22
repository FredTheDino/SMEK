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


AddOption("--tests",
          dest="tests",
          action="store_true",
          help="Compile with tests")

AddOption("--verbose",
          dest="verbose",
          action="store_true",
          help="Print verbose output. Not fully respected.")

env = Environment(ENV=os.environ)
env.Replace(CXX="g++")
env.Append(CXXFLAGS="-Wall")
env.Append(CXXFLAGS="-ggdb")
env.Append(CXXFLAGS="-O0")
env.Append(CXXFLAGS=shell(["sdl2-config", "--cflags"]))
env.Append(LINKFLAGS=shell(["sdl2-config", "--libs"]))
env.Append(LINKFLAGS="-rdynamic")  # Gives backtrace information

if GetOption("verbose"):
    env.Append(CPPDEFINES="VERBOSE")
    env.Append(ASSETS_VERBOSE="--verbose")

source = glob("src/**/*.c*", recursive=True)

if GetOption("tests"):
    env.Append(CPPDEFINES="TESTS")
    build_dir = "bin/tests/"
else:
    source.remove("src/test.cpp")
    build_dir = "bin/debug/"

VariantDir(build_dir, "src", duplicate=0)
source = [re.sub("^src/", build_dir, f) for f in source]

asset_gen = Builder(action="./asset-gen.py -o $TARGET -f $SOURCES $ASSETS_VERBOSE")
env.Append(BUILDERS={"Assets": asset_gen})

if GetOption("tests"):
    assets = env.Assets(build_dir + "assets-tests.bin", glob("res/tests/*.*"))
else:
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

env.Alias("assets", assets)

smek = env.Program(target=build_dir + "SMEK", source=source)
Depends(smek, assets)
Default(smek)

run_cmd = "(set -o pipefail; "  # propagate exit codes from earlier pipes
run_cmd += "cd " + build_dir + "; " + smek[0].abspath
if shutil.which("c++filt"):
    # Swap stdout and stderr, de-mangle with c++filt, then swap back.
    # https://serverfault.com/questions/63705/how-to-pipe-stderr-without-piping-stdout
    run_cmd += " 3>&1 1>&2 2>&3 | c++filt 3>&2 2>&1 1>&3 | c++filt"
else:
    # c++filt should be installed but just in case
    run_cmd += "; echo \"c++filt not found, function names may be mangled\""
run_cmd += ")"

AlwaysBuild(env.Alias("run", smek, run_cmd))

docs = env.Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

env.Clean(smek, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(docs, "docs/index.html")
env.Clean(assets, glob("bin/**/assets*.bin"))
