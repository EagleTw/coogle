#pragma once
#include <string>
#include <vector>
#include <string_view>

struct Signature {
  std::string retType;
  std::vector<std::string> argType;
};

bool parseFunctionSignature(std::string_view input, Signature &output);

