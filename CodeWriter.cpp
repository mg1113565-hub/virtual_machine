// Generates the assembly code that realizes the parsed command
#include "CodeWriter.h"
#include "Globals.h"
#include "Parser.h"
#include <iostream>
#include <string>
CodeWriter::CodeWriter(const std::string &filename) {

  std::string outputFile = filename.substr(0, filename.rfind(".vm")) + ".asm";

  file.open(outputFile);

  if (!file.is_open()) {
    std::cerr << "Error: could not open output file" << std::endl;
  }
}
void CodeWriter::writeArithmetic(std::string command) {

  file << "//" << command;

  if (command == "add") {
    popStacktoD();
    decrementSP();
    loadSPtoA();
    file << "M=D+M\n";
    incrementSP();
  } else if (command == "sub") {
    popStacktoD();
    decrementSP();
    loadSPtoA();
    file << "M=M-D\n";
    incrementSP();
  } else if (command == "neg") {
    decrementSP();
    loadSPtoA();
    file << "M=-M\n";
    incrementSP();
  } else if (command == "and") {
    popStacktoD();
    popStacktoD();
    decrementSP();
    loadSPtoA();
    file << "M=D&M\n";
    incrementSP();
  } else if (command == "or") {
    popStacktoD();
    popStacktoD();
    decrementSP();
    loadSPtoA();
    file << "M=D|M\n";
    incrementSP();
  } else if (command == "not") {
    decrementSP();
    loadSPtoA();
    file << "M=!M\n";
    incrementSP();
  }
}

// pass command type into this function
void CodeWriter::writePushPop(ProjectEnums::Command commandType,
                              std::string segment, int index) {
  if (commandType == ProjectEnums::Command::C_PUSH) {
    file << "// push " << segment << ' ' << index << '\n';
    if (segment == "constant") {
      file << "@" << index << '\n';
      file << "D=A\n";
    } else if (segment == "local") {
      loadSegment("LCL", index);
      file << "D=M\n";
    } else if (segment == "argument") {
      loadSegment("ARG", index);
      file << "D=M\n";
    } else if (segment == "this") {
      loadSegment("THIS", index);
      file << "D=M"
    } else if (segment == "that") {
      loadSegment("THAT", index);
      file << "D=M\n";
    } else if (segment == "pointer") {
      file << "@R" << std::to_string(index + 3) << '\n';
      file << "D=M\n";
    } else if (segment == "temp") {
      file << "@R" << std::to_string(index + 3) << '\n';
      file << "D=M\n";
    } else if (segment == "static") {
      // TODO: name of static part is filename.index
      file << "@" <<
    }
  }

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

  void CodeWriter::loadSPtoA() {
    file << "@SP\n";
    file << "A=M\n";
  }

  void CodeWriter::loadSegment(std::string segment, int index) {
    file << "@" << segment << '\n';
    file << "D=M" << '\n';
    file << "@" << index << '\n';
    file << "A=D+A" << '\n';
  }