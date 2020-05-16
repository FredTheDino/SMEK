from glob import glob
import re

AddOption("--tests",
          dest="tests",
          action="store_true",
          help="Compile with tests")

VariantDir("bin", "src", duplicate=0)

env = Environment()
env.Replace(CXX="g++")
env.Append(CXXFLAGS="-Wall")

source = glob("src/**/*.c*", recursive=True)

if GetOption("tests"):
    env.Append(CPPDEFINES="TESTS")
else:
    source.remove("src/test.cpp")

source = [re.sub("^src/", "bin/", f) for f in source]

smek = env.Program(target="bin/SMEK", source=source, variant_dir="bin")
Default(smek)
AlwaysBuild(Alias("run", smek, smek[0].abspath))

docs = Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

env.Clean(smek, glob("bin/**/*.o", recursive=True))  # always remove *.o
env.Clean(docs, "docs/index.html")
