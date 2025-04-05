#include <clang-c/Index.h>

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

// TODO: line number of funciton decl
// TODO: Rename function using LLVM style name

constexpr std::size_t BUFFER_SIZE = 512;
constexpr int EXPECTED_ARG_COUNT = 3;

std::string_view trim(std::string_view sv) {
  const size_t start = sv.find_first_not_of(" \t\n\r");
  if (start == std::string_view::npos)
    return {};

  const size_t end = sv.find_last_not_of(" \t\n\r");
  return sv.substr(start, end - start + 1);
}

struct Signature {
  std::string retType;
  std::vector<std::string> argType;
};

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

    std::cout << "argType: " << cleaned << std::endl;
  }

  return true;
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor parent,
                           CXClientData client_data) {
  CXCursorKind kind = clang_getCursorKind(cursor);
  if (kind == CXCursor_FunctionDecl || kind == CXCursor_CXXMethod) {
    CXString funcName = clang_getCursorSpelling(cursor);
    std::printf("f: %s\n", clang_getCString(funcName));
    clang_disposeString(funcName);

    int numArgs = clang_Cursor_getNumArguments(cursor);
    for (int i = 0; i < numArgs; ++i) {
      CXCursor argCursor = clang_Cursor_getArgument(cursor, i);
      CXString argName = clang_getCursorSpelling(argCursor);
      CXType argType = clang_getCursorType(argCursor);
      CXString typeSpelling = clang_getTypeSpelling(argType);

      std::printf("  Args %d: %s (%s)\n", i, clang_getCString(argName),
                  clang_getCString(typeSpelling));

      clang_disposeString(argName);
      clang_disposeString(typeSpelling);
    }

    CXType retType = clang_getCursorResultType(cursor);
    CXString retSpelling = clang_getTypeSpelling(retType);
    std::printf("  return: %s\n", clang_getCString(retSpelling));
    clang_disposeString(retSpelling);
    std::printf("\n");
  }

  return CXChildVisit_Recurse;
}

// Windows is not supported for now
std::vector<std::string> detectSystemIncludePaths() {
  std::vector<std::string> includePaths;
  FILE *pipe = popen("clang -E -x c /dev/null -v 2>&1", "r");
  if (!pipe) {
    std::cerr << "Failed to run clang for include path detection.\n";
    return includePaths;
  }

  char buffer[BUFFER_SIZE];
  bool insideSearch = false;

  constexpr char INCLUDE_START[] = "#include <...> search starts here:";
  constexpr char INCLUDE_END[] = "End of search list.";

  while (fgets(buffer, sizeof(buffer), pipe)) {
    std::string line = buffer;

    if (line.find(INCLUDE_START) != std::string::npos) {
      insideSearch = true;
      continue;
    }

    if (insideSearch && line.find(INCLUDE_END) != std::string::npos) {
      break;
    }

    if (insideSearch) { // Extract path
      std::string path =
          std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");
      if (!path.empty()) {
        includePaths.push_back("-I" + path);
      }
    }
  }

  if (pclose(pipe) != 0) { // Check for errors when closing the pipe
    std::fprintf(stderr,
                 "Error closing pipe for clang include path detection.\n");
  }
  pclose(pipe);
  return includePaths;
}

int main(int argc, char *argv[]) {
  if (argc != EXPECTED_ARG_COUNT) { // Use constant for argument count
    // clang-format off
    std::fprintf(stderr, "✖ Error: Incorrect number of arguments.\n\n");
    std::fprintf(stderr, "Usage:\n");
    std::fprintf(stderr, "  %s <filename> \"<function_signature_prefix>\"\n\n", argv[0]);
    std::fprintf(stderr, "Example:\n");
    std::fprintf(stderr, "  %s example.c \"int(int, char *)\"\n\n", argv[0]);
    // clang-format on
    return 1;
  }

  const std::string filename = argv[1];
  Signature sig;
  if (!parseFunctionSignature(argv[2], sig)) {
    return 1;
  }

  CXIndex index = clang_createIndex(0, 0);
  if (!index) {
    std::fprintf(stderr, "Error creating Clang index\n");
    return 1;
  }

  std::vector<std::string> argsVec = detectSystemIncludePaths();
  std::vector<const char *> clangArgs;
  for (const auto &s : argsVec) {
    clangArgs.push_back(s.c_str());
  }

  CXTranslationUnit tu = clang_parseTranslationUnit(
      index, filename.c_str(), clangArgs.data(), clangArgs.size(), nullptr, 0,
      CXTranslationUnit_None);
  if (!tu) { // Check for null translation unit
    // clang-fomat off
    std::fprintf(stderr, "Error parsing translation unit for file: %s\n",
                 filename.c_str());
    // clang-fomat on
    clang_disposeIndex(index);
    return 1;
  }

  CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(rootCursor, visitor, nullptr);

  clang_disposeTranslationUnit(tu); // Clean up translation unit
  clang_disposeIndex(index);        // Clean up Clang index

  return 0;
}
