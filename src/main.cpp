
#include <clang-c/Index.h>
#include <iostream>
#include <regex>
#include <string>

// Windows is not supported for now
std::vector<std::string> detectSystemIncludePaths() {
  std::vector<std::string> includePaths;
  FILE *pipe = popen("clang -E -x c /dev/null -v 2>&1", "r");
  if (!pipe) {
    std::cerr << "Failed to run clang for include path detection.\n";
    return includePaths;
  }

  char buffer[512];
  bool insideSearch = false;

  while (fgets(buffer, sizeof(buffer), pipe)) {
    std::string line = buffer;

    if (line.find("#include <...> search starts here:") != std::string::npos) {
      insideSearch = true;
      continue;
    }

    if (insideSearch && line.find("End of search list.") != std::string::npos) {
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

  pclose(pipe);
  return includePaths;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
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
    clangArgs.push_back(s.c_str());
  }

  CXTranslationUnit tu = clang_parseTranslationUnit(
      index, filename.c_str(), clangArgs.data(), clangArgs.size(), nullptr, 0,
      CXTranslationUnit_None);

  return 0;
}
