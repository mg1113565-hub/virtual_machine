// Reads and parses a VM command

#include "Parser.h"
#include "Globals.h"
#include <algorithm>
#include <sstream>

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

  /*
  //Remove whitespace removal for now since my curr idea for command argument
  parsing involves the whitespace currCommand.erase(
      std::remove_if(currCommand.begin(), currCommand.end(),
                     [](unsigned char c) { return std::isspace(c); }),
      currCommand.end());
  */
}

ProjectEnums::Command Parser::commandType() {

  // Not sure what to do if curr command is if-goto????

  if (currCommand.contains("push")) {
    return ProjectEnums::Command::C_PUSH;
  } else if (currCommand.contains("pop")) {
    return ProjectEnums::Command::C_POP;
  } else if (currCommand.contains("label")) {
    return ProjectEnums::Command::C_LABEL;
  } else if (currCommand.contains("goto")) {
    return ProjectEnums::Command::C_GOTO;
  } else if (currCommand.contains("function")) {
    return ProjectEnums::Command::C_FUNCTION;
  } else if (currCommand.contains("call")) {
    return ProjectEnums::Command::C_CALL;
  } else if (currCommand.contains("return")) {
    return ProjectEnums::Command::C_RETURN;
  } else {
    return ProjectEnums::Command::C_ARITHMETIC;
  }
}

// Should not be called if the current command is C_RETURN
std::string Parser::arg1() {
  std::stringstream ss(currCommand);
  std::string arg;
  ss >> arg;

  if (commandType() == ProjectEnums::Command::C_ARITHMETIC) {
    return arg;
  } else {
    ss >> arg;
    return arg;
  }
}
// Should be called only if curr command is C_PUSH, C_POP, C_FUNCTION, or C_CALL
int Parser::arg2() {
  std::stringstream ss(currCommand);
  std::string arg;

  while (ss >> arg) {
  }

  return std::stoi(arg);
}
