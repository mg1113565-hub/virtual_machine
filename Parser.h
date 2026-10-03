#include "Globals.h"
#include <fstream>
#include <string>

class Parser {
public:
  Parser(const std::string &filename);

  bool hasMoreLines();

  void advance();

  ProjectEnums::Command commandType();

  std::string arg1();

  int arg2();

private:
  std::string currCommand;
  std::ifstream file;
};