// Generates the assembly code that realizes the parsed command
#include "CodeWriter.h"
#include "Globals.h"
#include "Parser.h"
#include <iostream>
#include <string>
CodeWriter::CodeWriter(const std::string &filename) {

  std::string outputFile = filename.substr(0, filename.rfind(".vm")) + ".asm";
  fileName = filename.substr(0, filename.rfind(".vm"));

  file.open(outputFile);

  if (!file.is_open()) {
    std::cerr << "Error: could not open output file" << std::endl;
  }
}
void CodeWriter::writeArithmetic(std::string command) {

  file << "// " << command << '\n';

  if (command == "add") {
    // get element at the top of stack to D
    popStacktoD();
    // move down stack
    decrementSP();
    // get element now at the top of stack to A(second elem)
    loadSPtoA();
    // add value of first element in D, to value of element at address A which
    // is in M, so D+M, so now the top of the stack has that result
    file << "M=D+M\n";
    // move SP back to top empty slot
    incrementSP();
  } else if (command == "sub") {
    // get elem at the top of the stack to D
    popStacktoD();
    // move down the stack
    decrementSP();
    // go to address now at the top of the stack(A), value there is in M
    loadSPtoA();
    // do subtraction
    file << "M=M-D\n";
    // move SP back to top empty slot
    incrementSP();
  } else if (command == "neg") {
    // decrement SP to element at
    decrementSP();
    // load address of top elem to A, and M is data at A
    loadSPtoA();
    file << "M=-M\n";
    // increment back to top of stack
    incrementSP();
  } else if (command == "and") {
    // get elem at top of stack
    popStacktoD();
    // move down stack
    decrementSP();
    // get next elem at top of stack address at A, data in M
    loadSPtoA();
    // do & computation of both elems
    file << "M=D&M\n";
    // move SP back to top empty slot
    incrementSP();
  } else if (command == "or") {
    // get elem at top of stack, automatically handles moving down one elem to
    // get actual first value
    popStacktoD();
    // decrement SP to next element at the top of the stack
    decrementSP();
    // load next element on top of stack's address to A, data is in M register
    loadSPtoA();
    // do | computation
    file << "M=D|M\n";
    // increment SP back to top open slot
    incrementSP();
  } else if (command == "not") {
    // decrement SP to point to elem at the top of stack
    decrementSP();
    // load address of top of stack to A, data is in M register
    loadSPtoA();
    // compute negation of that value
    file << "M=!M\n";
    // increment SP back to top open slot
    incrementSP();
  }
}

// pass command type into this function
void CodeWriter::writePushPop(ProjectEnums::Command commandType,
                              std::string segment, int index) {
  if (commandType == ProjectEnums::Command::C_PUSH) {
    // push command requires segment of memory and index in that memory segment
    // you would like to push
    file << "// push " << segment << ' ' << index << '\n';
    if (segment == "constant") {
      // if a constant value then we just need to load the value directly
      file << "@" << index << '\n';
      // load value to D register, when you use @ val will be loaded into A
      // register
      file << "D=A\n";
    } else if (segment == "local") {
      // load address of LCL[index] to A register
      loadSegment("LCL", index);
      // load data in LCL[index] to D register
      file << "D=M\n";
    } else if (segment == "argument") {
      // load address of ARG[index] to A register
      loadSegment("ARG", index);
      // load data in ARG[index] to D register
      file << "D=M\n";
    } else if (segment == "this") {
      // load address of this[index] to A register
      loadSegment("THIS", index);
      // load data of this[index] to D register
      file << "D=M\n";
    } else if (segment == "that") {
      // load address of that[index] to A register
      loadSegment("THAT", index);
      // load data of that[index] to D register
      file << "D=M\n";
    } else if (segment == "pointer") {
      // load address of RAM[index + 3] to A register
      // the 3 is an offset since ptr segment begins at R3
      // push pointer 0, is accessing R3, push pointer 1 is accessing R4
      file << "@R" << std::to_string(index + 3) << '\n';
      // load data of the ptr memory to D register
      file << "D=M\n";
    } else if (segment == "temp") {
      // load address of RAM[index + 5] to A register
      // 5 offset since temp memory begins at R5
      file << "@R" << std::to_string(index + 5) << '\n';
      // load data of that temp memory to D register
      file << "D=M\n";
    } else if (segment == "static") {
      // static memory persists within the file so it must be accessed using the
      // filename.index ex) push static 4 = @SimpleAdd.4
      file << "@" << fileName << "." << index << '\n';
      file << "D=M\n";
    }
    // finally push collected value onto stack
    pushDtoStack();
  } else if (commandType == ProjectEnums::Command::C_POP) {
    file << "// pop " << segment << ' ' << index << '\n';
    if (segment == "constant") {
      file << "@" << index << '\n';
    } else if (segment == "local") {
      loadSegment("LCL", index);
    } else if (segment == "argument") {
      loadSegment("ARG", index);
    } else if (segment == "this") {
      loadSegment("THIS", index);
    } else if (segment == "that") {
      loadSegment("THAT", index);
    } else if (segment == "pointer") {
      file << "@R" << std::to_string(index + 3) << '\n';
    } else if (segment == "temp") {
      file << "@R" << std::to_string(index + 5) << '\n';
    } else if (segment == "static") {
      // TODO: name of static part is filename.index
      file << "@" << fileName << "." << index << '\n';
    }
    // all ifs above just involve loading the Address's at the the respective
    // memory segments to the A register

    // load address of A to D register
    file << "D=A\n";
    // load temporary register to A register
    // LEFT OFFF HERERERERERERERERER
    file << "@R13\n";
    file << "M=D\n";
    popStacktoD();
    file << "@R13\n";
    file << "A=M\n";
    file << "M=D\n";
  }
}

void CodeWriter::close() { file.close(); }

void CodeWriter::incrementSP() {

  // load mem address of SP into A register
  // how that address is used depends on instructions after
  file << "@SP\n";

  // M register implicitly means memory at A or memory[A]
  file << "M=M+1" << '\n';
}

void CodeWriter::decrementSP() {
  file << "@SP\n";
  file << "M=M-1" << '\n';
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

void CodeWriter::writeCompareLogic(std::string jumpCommand) {

  popStacktoD();
  decrementSP();
  loadSPtoA();
  file << "D=M-D\n";
  file << "@LABEL" << labelCount << '\n';
  file << "D" << jumpCommand << '\n';
  loadSPtoA();
  file << "M=0\n";
  file << "@ENDLABEL" << labelCount << '\n';
  file << "0;JMP\n";
  file << "(LABEL" << labelCount << ")\n";
  loadSPtoA();
  file << "M=-1\n";
  file << "(ENDLABEL" << labelCount << ")\n";
  incrementSP();
  labelCount++;
}

void CodeWriter::loadSegment(std::string segment, int index) {
  file << "@" << segment << '\n';
  file << "D=M" << '\n';
  file << "@" << index << '\n';
  file << "A=D+A" << '\n';
}