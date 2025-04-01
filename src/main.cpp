#include <clang-c/Index.h>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

constexpr size_t BUFFER_SIZE = 512;
constexpr int EXPECTED_ARG_COUNT = 3;

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
    std::cerr << "✖ Error: Incorrect number of arguments.\n\n";
    std::cerr << "Usage:\n";
    std::cerr << "  coogle <filename.c> <function_signature_prefix>\n\n";
    std::cerr << "Example:\n";
    std::cerr << "  ./coogle example.c int(int, char *)\n\n";
    return 1;
  }

  const std::string filename = argv[1];
  const std::string targetSignature = argv[2];

  CXIndex index = clang_createIndex(0, 0);
  if (!index) {
    std::cerr << "Error creating Clang index." << std::endl;
    return 1;
  }

  std::vector<std::string> argsVec = detectSystemIncludePaths();
  std::vector<const char *> clangArgs;
  for (const auto &s : argsVec) {
    std::cout << s << std::endl;
    clangArgs.push_back(s.c_str());
  }

  CXTranslationUnit tu = clang_parseTranslationUnit(
      index, filename.c_str(), clangArgs.data(), clangArgs.size(), nullptr, 0,
      CXTranslationUnit_None);
  if (!tu) { // Check for null translation unit
    std::cerr << "Error parsing translation unit for file: " << filename << std::endl;
    clang_disposeIndex(index);
    return 1;
  }

  clang_disposeTranslationUnit(tu); // Clean up translation unit
  clang_disposeIndex(index);        // Clean up Clang index

  return 0;
}
