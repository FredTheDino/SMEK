import os
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

VariantDir("bin", "src", duplicate=0)

env = Environment(ENV=os.environ)
env.Replace(CXX="g++")
env.Append(CXXFLAGS="-Wall")
env.Append(CXXFLAGS="-ggdb")
env.Append(CXXFLAGS=shell(["sdl2-config", "--cflags"]))
env.Append(LINKFLAGS=shell(["sdl2-config", "--libs"]))

source = glob("src/**/*.c*", recursive=True)

if GetOption("tests"):
    env.Append(CPPDEFINES="TESTS")
else:
    source.remove("src/test.cpp")

source = [re.sub("^src/", "bin/", f) for f in source]

smek = env.Program(target="bin/SMEK", source=source, variant_dir="bin")
Default(smek)
AlwaysBuild(env.Alias("run", smek, smek[0].abspath))

docs = env.Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

env.Clean(smek, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(docs, "docs/index.html")
