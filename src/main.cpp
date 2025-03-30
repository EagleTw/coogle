
#include <clang-c/Index.h>
#include <iostream>
#include <string>

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

  return 0;
}
