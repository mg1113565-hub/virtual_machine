// Reads and parses a VM command

#include "Parser.h"
#include "Globals.h"
#include <algorithm>

Parser::Parser(const std::string &filename) : file(filename) {}

bool Parser::hasMoreLines() {
  std::streampos previousLinePos = file.tellg();
  std::string line;
  bool result;

  if (std::getline(file, line)) {
    result = true;
  } else {
    file.clear();
    result = false;
  }

  file.seekg(previousLinePos);
  return result;
}

// Must be called after hasMoreLines
//  *Still needs to handle whitespace
void Parser::advance() {
  std::getline(file, currCommand);
  auto it = currCommand.find("//");
  if (it != std::string::npos) {
    currCommand.erase(it);
  }

  currCommand.erase(
      std::remove_if(currCommand.begin(), currCommand.end(),
                     [](unsigned char c) { return std::isspace(c); }),
      currCommand.end());
}

ProjectEnums::Command Parser::commandType() {

  if (currCommand.contains("push")) {
    return ProjectEnums::Command::C_PUSH;
  }
}

std::string Parser::arg1() {}

int arg2();
