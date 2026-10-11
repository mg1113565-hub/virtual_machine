// Generates the assembly code that realizes the parsed command
#include "CodeWriter.h"
#include "Globals.h"
#include <iostream>
CodeWriter::CodeWriter(const std::string &filename) {

  std::string outputFile = filename.substr(0, filename.rfind(".vm")) + ".asm";

  file.open(outputFile);

  if (!file.is_open()) {
    std::cerr << "Error: could not open output file" << std::endl;
  }
}
void CodeWriter::writeArithmetic(std::string command) {

  // need to see first command
}

void CodeWriter::writePushPop(std::string command, std::string segment,
                              int index) {}

void CodeWriter::close() {}

void CodeWriter::incrementSP() {

  // load mem address of SP into A register
  // how that address is used depends on instructions after
  file << "@SP\n";

  // M register implicitly means memory at A or memory[A]
  file << "M=M+1";
}

void CodeWriter::decrementSP() {
  file << "@SP\n";
  file << "M=M-1";
}

void CodeWriter::pushDtoStack() {
  // load stack ptr to A register
  file << "@SP\n";
  file << "A=M\n";
  // assign D to address of stack ptr
  file << "M=D\n";
  incrementSP();
}

void CodeWriter::popStacktoD() {

  decrementSP();

  // writing to memory using a ptr
  // M is now pointing to previous stack address
  // so address in A(@SP) is now pointing to new address M
  file << "A=M\n";

  // store that value in D
  file << "D=M\n";
}

void CodeWriter::loadSegment(std::string segment, int index) {}