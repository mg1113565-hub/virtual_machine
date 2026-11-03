// Generates the assembly code that realizes the parsed command
#include "CodeWriter.h"
#include "Globals.h"
#include "Parser.h"
#include <iostream>
#include <string>

// TODO: write assembly instructions that affect the bootstrap code at the
// beginning of the function
CodeWriter::CodeWriter(const std::string &filename) {

  std::string outputFile = filename.substr(0, filename.rfind(".vm")) + ".asm";
  fileName = filename.substr(0, filename.rfind(".vm"));

  file.open(outputFile);

  if (!file.is_open()) {
    std::cerr << "Error: could not open output file" << std::endl;
  }
}

// TODO: inform of translation of new vm file(if passed a folder)
void CodeWriter::setFileName() {}
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
    // all ifs above just involve loading the address's at the the respective
    // memory segments to the A register

    // load address of segment[index] to D register
    file << "D=A\n";
    // load temporary register to A register
    file << "@R13\n";
    // load address of segment[index] to data of temporary register
    file << "M=D\n";
    // pop top of the stack to D
    popStacktoD();
    // load temporary register to A register
    file << "@R13\n";
    // A = address of R13
    // M = address of segment[index]
    file << "A=M\n";
    // data at adress segment[index] = data popped from top of the stack
    file << "M=D\n";
  }
}

// TODO: write assembly code that effects label command
void CodeWriter::writeLabel(std::string label) {}

// TODO; write code that affecst goto command
void CodeWriter::writeGoto(std::string label) {}

// TODO: write code that affects IF command
void CodeWriter::writeIf(std::string label) {}

// TODO: write code that affects Function command
void CodeWriter::writeFunction(std::string functionName, int nVars) {}

// TODO: write the code that affects the call command
void CodeWriter::writeCall(std::string functionName, int nVars) {}

// TODO: write the code that affecst the return  command
void CodeWriter::writeReturn() {}

void CodeWriter::close() { file.close(); }

void CodeWriter::incrementSP() {

  // load mem address of SP into A register
  // how that address is used depends on instructions after
  file << "@SP\n";
  // M register implicitly means memory at A or memory[A]
  file << "M=M+1" << '\n';
}

void CodeWriter::decrementSP() {
  // load mem address of SP into A register
  file << "@SP\n";
  // decrement memory at A
  file << "M=M-1" << '\n';
}

void CodeWriter::pushDtoStack() {
  // load stack ptr to A register
  file << "@SP\n";
  // load value at SP to A register
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
  // load SP to A register
  // stack pointer, holds the address of the top of stack in its data
  // segment(since its a ptr)
  file << "@SP\n";
  // load memory address of top of stack to A
  file << "A=M\n";
}

void CodeWriter::writeCompareLogic(std::string jumpCommand) {

  // get value in top of stack to D
  popStacktoD();
  // move down SP
  decrementSP();
  // load address of top of stack to A register
  loadSPtoA();
  // D now holds difference between the data helds in the two upper most stack
  // values
  file << "D=M-D\n";
  // print out label for wherever we are before jump
  file << "@LABEL" << labelCount << '\n';
  // D jump
  file << "D" << jumpCommand << '\n';
  // reload current top of stack to A
  loadSPtoA();
  // set M register to 0?
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
  // load memory segment pointer
  file << "@" << segment << '\n';
  // set D register equal to memory address of the beginning of that memory
  // segment
  file << "D=M" << '\n';
  // load index into A register, or basically just like the offset
  file << "@" << index << '\n';
  // add offset plus base memory to get the effective address of the segment we
  // want, e.g. segment[index]
  file << "A=D+A" << '\n';
}