from glob import glob

AddOption("--tests",
          dest="tests",
          action="store_true",
          help="Compile with tests")

env = Environment()
env.Replace(CXX="clang++")
env.Append(CXXFLAGS="-Wall")

source = glob("src/**/*.c*", recursive=True)

if GetOption("tests"):
    env.Append(CPPDEFINES="TESTS")
else:
    source.remove("src/test.cpp")

smek = env.Program(target="SMEK", source=source)
Default(smek)
AlwaysBuild(Alias("run", smek, smek[0].abspath))

docs = Alias("docs", "", "docs/doc-builder.py")
AlwaysBuild(docs)

env.Clean(smek, glob("src/**/*.o", recursive=True))  # always remove *.o
env.Clean(docs, "docs/index.html")
