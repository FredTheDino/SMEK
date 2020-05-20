import os
import shutil
import re
from glob import glob
from subprocess import Popen, PIPE

# TODO(ed): Fix for Windows
def shell(command):
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

VariantDir("bin", "src", duplicate=0)

env = Environment(ENV=os.environ)
env.Replace(CXX="g++")
env.Append(CXXFLAGS="-Wall")
env.Append(CXXFLAGS="-ggdb")
env.Append(CXXFLAGS="-O0")
env.Append(CXXFLAGS=shell(["sdl2-config", "--cflags"]))
env.Append(LINKFLAGS=shell(["sdl2-config", "--libs"]))
env.Append(LINKFLAGS="-rdynamic") # Gives backtrace information

assets = env.Alias("assets", "", "./asset-gen.py")
AlwaysBuild(assets)

source = glob("src/**/*.c*", recursive=True)

if GetOption("tests"):
    env.Append(CPPDEFINES="TESTS")
else:
    source.remove("src/test.cpp")

if GetOption("verbose"):
    env.Append(ENV={"VERBOSE": "1"})
    env.Append(CPPDEFINES="VERBOSE")

# for variant_dir
source = [re.sub("^src/", "bin/", f) for f in source]

smek = env.Program(target="bin/SMEK", source=source)
Depends(smek, assets)
Default(smek)

run_cmd = smek[0].abspath
if shutil.which("c++filt"):
    # Swap stdout and stderr, de-mangle with c++filt, then swap back.
    # https://serverfault.com/questions/63705/how-to-pipe-stderr-without-piping-stdout
    run_cmd += " 3>&1 1>&2 2>&3 | c++filt 3>&2 2>&1 1>&3 | c++filt"
else:
    # c++filt should be installed but just in case
    run_cmd += "&& echo \"c++filt not found, function names may be mangled\""

AlwaysBuild(env.Alias("run", smek, run_cmd))

docs = env.Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

env.Clean(smek, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(docs, "docs/index.html")
env.Clean(assets, glob("bin/assets*.bin"))
