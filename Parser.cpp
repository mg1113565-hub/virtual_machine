// Reads and parses a VM command

#include "Parser.h"
#include "Globals.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

Parser::Parser(const std::string &filename) : file(filename) {

  if (!file.is_open()) {
    std::cerr << "Error could not open the file!" << std::endl;
  }
}

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
  parsing involves the whitespace
  currCommand.erase(
      std::remove_if(currCommand.begin(), currCommand.end(),
                     [](unsigned char c) { return std::isspace(c); }),
      currCommand.end());
  */
}

bool Parser::isEmpty() {
  // reading a word out of a stringstream skips all whitespace (spaces, tabs,
  // and the '\r' left over from windows line endings), so if we can't read a
  // single word the line has no command on it (blank line or comment only)
  std::stringstream ss(currCommand);
  std::string word;
  return !(ss >> word);
}

ProjectEnums::Command Parser::commandType() {

  // only look at the FIRST word and compare it exactly, contains() broke on
  // "if-goto" since it contains "goto"
  std::stringstream ss(currCommand);
  std::string op;
  ss >> op;

  if (op == "push") {
    return ProjectEnums::Command::C_PUSH;
  } else if (op == "pop") {
    return ProjectEnums::Command::C_POP;
  } else if (op == "label") {
    return ProjectEnums::Command::C_LABEL;
  } else if (op == "goto") {
    return ProjectEnums::Command::C_GOTO;
  } else if (op == "if-goto") {
    return ProjectEnums::Command::C_IF;
  } else if (op == "function") {
    return ProjectEnums::Command::C_FUNCTION;
  } else if (op == "call") {
    return ProjectEnums::Command::C_CALL;
  } else if (op == "return") {
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
