all: build/coogle

compile_commands.json: Makefile
	bear -- make build/coogle

build/coogle: src/main.cpp | build
	clang++ -o build/coogle src/main.cpp -I/opt/homebrew/opt/llvm/include \
	    -L/opt/homebrew/opt/llvm/lib -lclang

build:
	mkdir -p build

clean:
	rm -rf build compile_commands.json .cache

