// Drives the processes for the translation
#include "CodeWriter.h"
#include "Parser.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cerr << "Error: No filename supplied.\n";
    return 1;
  }

  std::string filename = argv[1];

  // Constructs a Parser to handle the input file

  Parser parser(filename);

  // Constructs a CodeWriter the handle the output file

  // Iterates through the input file, parsing each line and generating assembly
  // code from it, using the serives of Parser and CodeWriter
}