// #pragma once stops this header from being pasted in twice when more than one
// file includes it (VMTranslator.cpp and CodeWriter.cpp both include it)
#pragma once

#include "Globals.h"
#include <fstream>
#include <string>

class Parser {
public:
  Parser(const std::string &filename);

  bool hasMoreLines();

  void advance();

  // true if the current line has no command on it (blank line or only a
  // comment), the main loop uses this to skip those lines
  bool isEmpty();

  ProjectEnums::Command commandType();

  std::string arg1();

  int arg2();

private:
  std::string currCommand;
  std::ifstream file;
};
