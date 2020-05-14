env = Environment()
env.Replace(CXX="g++")

env.Program(target="SMEK", source=["src/main.cpp", "src/test.cpp"])
