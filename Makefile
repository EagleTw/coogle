CXX = clang++
CXXFLAGS = -std=c++17 -I/opt/homebrew/opt/llvm/include
LDFLAGS = -L/opt/homebrew/opt/llvm/lib -lclang

all: build/coogle

build/coogle: src/main.cpp | build
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

build:
	mkdir -p build

.PHONY: clean
clean:
	rm -rf build compile_commands.json .cache

.PHONY: compile_commands.json
compile_commands.json:
	rm -f build/coogle
	bear -- make build/coogle
