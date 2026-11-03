// Drives the processes for the translation
#include "CodeWriter.h"
#include "Globals.h"
#include "Parser.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

  // TODO: update to handle folder inputs as well as files
  // TODO: update logic to loop and repeat for each file if we are passed a
  // folder
  if (argc < 2) {
    std::cerr << "Error: No filename supplied.\n";
    return 1;
  }

  std::string filename = argv[1];

  // Constructs a Parser to handle the input file

  Parser parser(filename);

  // Constructs a CodeWriter the handle the output file

  CodeWriter codeWriter(filename);

  // Iterates through the input file, parsing each line and generating assembly
  // code from it, using the serives of Parser and CodeWriter

  // check if has more lines to loop
  while (parser.hasMoreLines()) {

    parser.advance();
    ProjectEnums::Command commandType = parser.commandType();

    if (commandType == ProjectEnums::Command::C_POP ||
        commandType == ProjectEnums::Command::C_PUSH) {
      codeWriter.writePushPop(commandType, parser.arg1(), parser.arg2());
    } else if (commandType == ProjectEnums::Command::C_ARITHMETIC) {
      codeWriter.writeArithmetic(parser.arg1());
    }
  }
}