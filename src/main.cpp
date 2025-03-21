
#include <clang-c/Index.h>
#include <iostream>
#include <string>

void printAST(CXCursor cursor, unsigned int level) {
  CXString cursorSpelling = clang_getCursorSpelling(cursor);
  CXString cursorKindSpelling =
      clang_getCursorKindSpelling(clang_getCursorKind(cursor));

  std::cout << std::string(level * 2, ' ') << "|-- "
            << clang_getCString(cursorKindSpelling) << " ("
            << clang_getCString(cursorSpelling) << ")" << std::endl;

  clang_disposeString(cursorSpelling);
  clang_disposeString(cursorKindSpelling);

  clang_visitChildren(
      cursor,
      [](CXCursor c, CXCursor parent, CXClientData client_data) {
        unsigned int nextLevel = *((unsigned int *)client_data) + 1;
        printAST(c, nextLevel);
        return CXChildVisit_Recurse;
      },
      &level);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <source.cpp>" << std::endl;
    return 1;
  }

  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, argv[1], nullptr, 0, nullptr, 0, CXTranslationUnit_None);

  if (!unit) {
    std::cerr << "Failed to parse translation unit!" << std::endl;
    return 1;
  }

  CXCursor rootCursor = clang_getTranslationUnitCursor(unit);
  printAST(rootCursor, 0);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
  return 0;
}
