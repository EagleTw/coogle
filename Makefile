CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -I/opt/homebrew/opt/llvm/include
LDFLAGS = -L/opt/homebrew/opt/llvm/lib

SRC_DIR  := src
BUILD    := build
BIN      := $(BUILD)/coogle

SRCS     := $(wildcard $(SRC_DIR)/*.cpp)
OBJS     := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD)/%.o,$(SRCS))

LLVM_FLAGS := $(shell llvm-config --cflags --libs --system-libs) -lclang

all: $(BIN)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC_DIR)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LLVM_FLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf build $(BIN) compile_commands.json .cache

.PHONY: compile_commands.json
compile_commands.json:
	rm -f $(BIN)
	bear -- make all
