#include "parser.h"

#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string_view>

std::string_view trim(std::string_view sv) {
  const size_t start = sv.find_first_not_of(" \t\n\r");
  if (start == std::string_view::npos)
    return {};
  const size_t end = sv.find_last_not_of(" \t\n\r");
  return sv.substr(start, end - start + 1);
}

bool parseFunctionSignature(std::string_view input, Signature &output) {
  size_t parenOpen = input.find('(');
  size_t parenClose = input.find(')', parenOpen);

  if (parenOpen == std::string_view::npos ||
      parenClose == std::string_view::npos || parenClose <= parenOpen) {
    std::fprintf(stderr, "Invalid function signature: '%.*s'\n",
                 (int)input.size(), input.data());
    return false;
  }

  output.retType = input.substr(0, parenOpen);
  std::cout << "retType: " << output.retType << std::endl;

  size_t start = parenOpen + 1;
  while (start < input.size()) {
    size_t end = input.find_first_of(",)", start);
    std::string_view token = input.substr(start, end - start);
    std::string_view cleaned = trim(token);
    if (!cleaned.empty())
      output.argType.push_back(std::string(cleaned));

    if (end == std::string_view::npos)
      break;

    start = end + 1;
  }

  return true;
}
