#include <clang-c/Index.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

constexpr std::size_t BUFFER_SIZE = 512;
constexpr int EXPECTED_ARG_COUNT = 3;

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
    std::cerr << "Error closing pipe for clang include path detection.\n";
  }
  return includePaths;
}

int main(int argc, char *argv[]) {
  if (argc != EXPECTED_ARG_COUNT) { // Use constant for argument count
    // clang-format off
    std::fprintf(stderr, "✖ Error: Incorrect number of arguments.\n\n");
    std::fprintf(stderr, "Usage:\n");
    std::fprintf(stderr, "  %s <filename> <function_signature_prefix>\n\n", argv[0]);
    std::fprintf(stderr, "Example:\n");
    std::fprintf(stderr, "  %s example.c int(int, char *)\n\n", argv[0]);
    // clang-format on
    return 1;
  }

  const std::string filename = argv[1];
  const std::string targetSignature = argv[2];

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
    std::cerr << "Error parsing translation unit for file: " << filename
              << std::endl;
    clang_disposeIndex(index);
    return 1;
  }

  CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(rootCursor, visitor, nullptr);

  clang_disposeTranslationUnit(tu); /p/ Clean up translation unit
  clang_disposeIndex(index);        // Clean up Clang index

  return 0;
}
