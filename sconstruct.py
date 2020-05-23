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
env.Append(CXXFLAGS="-Wno-unused")
env.Append(CXXFLAGS=shell(["sdl2-config", "--cflags"]))
env.Append(LINKFLAGS=shell(["sdl2-config", "--libs"]))
env.Append(LINKFLAGS="-ldl")
env.Append(LINKFLAGS="-rdynamic")  # Gives backtrace information

if GetOption("verbose"):
    env.Append(CPPDEFINES="VERBOSE")
    env.Append(ASSETS_VERBOSE="--verbose")

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

tests_assets = env.Assets(tests_dir + "assets-tests.bin", glob("res/tests/*.*"))
assets = all_asset_targets(smek_dir)
env.Alias("assets", assets)

source = glob("src/**/*.c*", recursive=True)
source.remove("src/platform.cpp")  # The platform layer

smek_source = [re.sub("^src/", smek_dir, f) for f in source]
smek = env.Program(target=smek_dir + "SMEK", source=[smek_dir + "platform.cpp"])
libsmek = env.SharedLibrary(target=smek_dir + "libSMEK", source=smek_source)
Depends(smek, assets)
Depends(smek, libsmek)
Default(smek)

env.Append(CPPDEFINES="TESTS")
tests_source = [re.sub("^src/", tests_dir, f) for f in source]
tests = env.Program(target=tests_dir + "tests", source=tests_source)
Depends(tests, tests_assets)

def create_run_command(run_dir, program):
    run_cmd = "(set -o pipefail; "  # propagate exit codes from earlier pipes
    run_cmd += "cd " + run_dir + "; " + program[0].abspath
    if shutil.which("c++filt"):
        # Swap stdout and stderr, de-mangle with c++filt, then swap back.
        # https://serverfault.com/questions/63705/how-to-pipe-stderr-without-piping-stdout
        run_cmd += " 3>&1 1>&2 2>&3 | c++filt 3>&2 2>&1 1>&3 | c++filt"
    else:
        # c++filt should be installed but just in case
        run_cmd += "; echo \"c++filt not found, function names may be mangled\""
    run_cmd += ")"
    return run_cmd

AlwaysBuild(env.Alias("run", smek, create_run_command(smek_dir, smek)))
AlwaysBuild(env.Alias("tests", tests, create_run_command(tests_dir, tests)))

docs = env.Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

if shutil.which("ctags"):
    ctags = env.Alias("tags", "", "ctags -R src")
    Depends(smek, ctags)
    AlwaysBuild(ctags)

env.Clean(smek, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(tests, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(libsmek, glob("bin/**/libSMEK*", recursive=True))
env.Clean(docs, "docs/index.html")
env.Clean(assets, glob("bin/**/assets*.bin"))
