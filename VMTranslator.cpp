// Drives the processes for the translation
#include "CodeWriter.h"
#include "Globals.h"
#include "Parser.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// shorter name for the filesystem library, it gives us tools for paths,
// checking if something is a folder, and listing a folder's files
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cerr << "Error: No filename supplied.\n";
    return 1;
  }

  fs::path input = argv[1];

  // if the user typed "FibonacciElement/" with a trailing slash, the last part
  // of the path is empty, parent_path() turns it into "FibonacciElement"
  if (input.filename().empty()) {
    input = input.parent_path();
  }

  // collect every .vm file we need to translate, and decide where the output
  // goes
  std::vector<fs::path> vmFiles;
  fs::path outputPath;

  if (fs::is_directory(input)) {
    // folder: translate every .vm file inside it
    for (const auto &entry : fs::directory_iterator(input)) {
      if (entry.path().extension() == ".vm") {
        vmFiles.push_back(entry.path());
      }
    }
    // directory_iterator gives files in no particular order, sort so the
    // output is the same every run
    std::sort(vmFiles.begin(), vmFiles.end());
    // FibonacciElement/ -> FibonacciElement/FibonacciElement.asm
    outputPath = input / (input.filename().string() + ".asm");
  } else {
    // single file: Foo.vm -> Foo.asm
    vmFiles.push_back(input);
    outputPath = input;
    outputPath.replace_extension(".asm");
  }

  if (vmFiles.empty()) {
    std::cerr << "Error: no .vm files found.\n";
    return 1;
  }

  // one CodeWriter for the whole program, all files go into the same .asm
  // per the project 8 spec, bootstrap code is only written when translating
  // more than one file
  bool writeBootstrap = vmFiles.size() > 1;
  CodeWriter codeWriter(outputPath.string(), writeBootstrap);

  // translate each file in turn, a new Parser per file
  for (const fs::path &vmFile : vmFiles) {

    // tell the CodeWriter which file we're in, stem() is the name without
    // folder or extension, "FibonacciElement/Main.vm" -> "Main"
    codeWriter.setFileName(vmFile.stem().string());

    Parser parser(vmFile.string());

    // Iterates through the input file, parsing each line and generating
    // assembly code from it, using the services of Parser and CodeWriter
    while (parser.hasMoreLines()) {

      parser.advance();

      // skip blank lines and comment only lines
      if (parser.isEmpty()) {
        continue;
      }

      ProjectEnums::Command commandType = parser.commandType();

      // hand each command type to the matching CodeWriter function
      switch (commandType) {
      case ProjectEnums::Command::C_ARITHMETIC:
        codeWriter.writeArithmetic(parser.arg1());
        break;
      case ProjectEnums::Command::C_PUSH:
      case ProjectEnums::Command::C_POP:
        codeWriter.writePushPop(commandType, parser.arg1(), parser.arg2());
        break;
      case ProjectEnums::Command::C_LABEL:
        codeWriter.writeLabel(parser.arg1());
        break;
      case ProjectEnums::Command::C_GOTO:
        codeWriter.writeGoto(parser.arg1());
        break;
      case ProjectEnums::Command::C_IF:
        codeWriter.writeIf(parser.arg1());
        break;
      case ProjectEnums::Command::C_FUNCTION:
        codeWriter.writeFunction(parser.arg1(), parser.arg2());
        break;
      case ProjectEnums::Command::C_CALL:
        codeWriter.writeCall(parser.arg1(), parser.arg2());
        break;
      case ProjectEnums::Command::C_RETURN:
        // return has no arguments, so don't call arg1/arg2
        codeWriter.writeReturn();
        break;
      }
    }
  }

  codeWriter.close();
  return 0;
}
