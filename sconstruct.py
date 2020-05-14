from glob import glob

env = Environment()
env.Replace(CXX="g++")

source = glob("src/**/*.c*", recursive=True)

if ARGUMENTS.get("test", 0):
    env.Append(CPPDEFINES="TESTS")
else:
    source.remove("src/test.cpp")

smek = env.Program(target="SMEK", source=source)
Default(smek)

env.Clean(smek, glob("src/**/*.o", recursive=True))  # always remove *.o
