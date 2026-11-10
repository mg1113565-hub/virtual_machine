// Generates the assembly code that realizes the parsed command
#include "CodeWriter.h"
#include "Globals.h"
#include "Parser.h"
#include <iostream>
#include <string>

// VMTranslator now figures out the output path (Foo.vm -> Foo.asm, or
// FolderName/ -> FolderName/FolderName.asm) and passes it in
CodeWriter::CodeWriter(const std::string &outputPath, bool writeBootstrap) {

  file.open(outputPath);

  if (!file.is_open()) {
    std::cerr << "Error: could not open output file" << std::endl;
    return;
  }

  if (writeBootstrap) {
    file << "// bootstrap\n";
    // SP = 256, the stack always starts at RAM[256]
    file << "@256\n";
    file << "D=A\n";
    file << "@SP\n";
    file << "M=D\n";
    // call Sys.init the same way any other function is called, this pushes
    // the 5 saved values so Sys.init starts with SP = 261, just jumping to
    // (Sys.init) would NOT be the same
    writeCall("Sys.init", 0);
  }
}

// called by VMTranslator each time it starts a new .vm file, name should be
// just the file name like "Main", static variables become Main.0, Main.1,
// etc. (the old version included the folder, giving "Folder/Main.0", and '/'
// isn't allowed in Hack symbols)
void CodeWriter::setFileName(const std::string &name) { fileName = name; }

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
  } else if (command == "eq") {
    writeCompareLogic("JEQ");
  } else if (command == "gt") {
    writeCompareLogic("JGT");
  } else if (command == "lt") {
    writeCompareLogic("JLT");
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
      // static variables are named filename.index, e.g. @Main.3
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

// VM labels only exist inside the function they are written in, two different
// functions can both have "label LOOP". To keep them from clashing in the
// single .asm file, prefix them with the function name: Main.fib$LOOP
std::string CodeWriter::scopedLabel(const std::string &label) {
  // BasicLoop / FibonacciSeries have no "function" command, so there is no
  // current function yet, just use the label as is
  if (currentFunction.empty()) {
    return label;
  }
  return currentFunction + "$" + label;
}

void CodeWriter::writeLabel(std::string label) {
  file << "// label " << label << '\n';
  // (NAME) declares a label in Hack assembly, it takes up no instruction, it
  // just marks the address of the next instruction
  file << "(" << scopedLabel(label) << ")\n";
}

void CodeWriter::writeGoto(std::string label) {
  file << "// goto " << label << '\n';
  // load the label's address into A
  file << "@" << scopedLabel(label) << '\n';
  // 0;JMP always jumps to the address in A
  file << "0;JMP\n";
}

void CodeWriter::writeIf(std::string label) {
  file << "// if-goto " << label << '\n';
  // pop the condition off the stack into D
  popStacktoD();
  // load the label's address into A
  file << "@" << scopedLabel(label) << '\n';
  // jump if D != 0, the VM treats any non zero value as true (not only -1)
  file << "D;JNE\n";
}

void CodeWriter::writeFunction(std::string functionName, int nVars) {
  file << "// function " << functionName << ' ' << nVars << '\n';
  // remember which function we're in so labels/return addresses get scoped to
  // it
  currentFunction = functionName;
  // entry point of the function, call jumps here
  file << "(" << functionName << ")\n";
  // the function's local variables live right where the stack is when it
  // starts (LCL == SP after a call), so pushing nVars zeros creates and zeroes
  // them all
  for (int i = 0; i < nVars; i++) {
    file << "@0\n";
    file << "D=A\n";
    pushDtoStack();
  }
}

void CodeWriter::pushRegister(const std::string &reg) {
  // @LCL etc. A = address of the pointer, M = the pointer's value
  file << "@" << reg << '\n';
  // we want to save the pointer's value, not what it points to
  file << "D=M\n";
  pushDtoStack();
}

void CodeWriter::writeCall(std::string functionName, int nArgs) {
  file << "// call " << functionName << ' ' << nArgs << '\n';

  // unique label for where to come back to once the function returns, the
  // counter makes each call site different even if the same function is
  // called many times
  std::string caller = currentFunction.empty() ? "Bootstrap" : currentFunction;
  std::string returnLabel = caller + "$ret." + std::to_string(returnCount++);

  // push returnAddress, @label puts the label's address into A, D=A copies
  // that address (not memory at it) into D
  file << "@" << returnLabel << '\n';
  file << "D=A\n";
  pushDtoStack();

  // save the caller's segment pointers so return can restore them
  pushRegister("LCL");
  pushRegister("ARG");
  pushRegister("THIS");
  pushRegister("THAT");

  // ARG = SP - 5 - nArgs
  // the arguments were pushed by the caller right before the call, and we
  // just pushed 5 more values on top of them, so going back 5 + nArgs from SP
  // lands on argument 0
  file << "@SP\n";
  file << "D=M\n";
  file << "@5\n";
  file << "D=D-A\n";
  file << "@" << nArgs << '\n';
  file << "D=D-A\n";
  file << "@ARG\n";
  file << "M=D\n";

  // LCL = SP, the called function's locals start at the current top of stack
  file << "@SP\n";
  file << "D=M\n";
  file << "@LCL\n";
  file << "M=D\n";

  // goto functionName
  file << "@" << functionName << '\n';
  file << "0;JMP\n";

  // (returnAddress), the callee's return jumps back here
  file << "(" << returnLabel << ")\n";
}

void CodeWriter::restoreFromFrame(const std::string &reg) {
  // R14 = endFrame address, AM=M-1 decrements R14 AND points A at the new
  // address in one instruction, so each call to this walks one step down the
  // saved frame: THAT, THIS, ARG, LCL
  file << "@R14\n";
  file << "AM=M-1\n";
  // D = saved value at that slot
  file << "D=M\n";
  // put it back into the register
  file << "@" << reg << '\n';
  file << "M=D\n";
}

void CodeWriter::writeReturn() {
  file << "// return\n";

  // the stack around a function looks like this:
  //   ARG -> arg 0 ... arg n-1
  //          return address    (endFrame - 5)
  //          saved LCL         (endFrame - 4)
  //          saved ARG         (endFrame - 3)
  //          saved THIS        (endFrame - 2)
  //          saved THAT        (endFrame - 1)
  //   LCL -> local 0 ...       (endFrame)
  //          ... working stack, return value on top

  // endFrame = LCL, stored in R14 (R13 is used by pop, so avoid it)
  file << "@LCL\n";
  file << "D=M\n";
  file << "@R14\n";
  file << "M=D\n";

  // retAddr = *(endFrame - 5), stored in R15
  // this MUST be saved before the next step, if the function had 0 arguments
  // then ARG[0] is the same slot as the return address and gets overwritten
  file << "@5\n";
  file << "A=D-A\n";
  file << "D=M\n";
  file << "@R15\n";
  file << "M=D\n";

  // *ARG = pop(), the return value goes where argument 0 was, which will be
  // the top of the caller's stack after we return
  popStacktoD();
  file << "@ARG\n";
  file << "A=M\n";
  file << "M=D\n";

  // SP = ARG + 1, drop everything above the return value
  file << "@ARG\n";
  file << "D=M+1\n";
  file << "@SP\n";
  file << "M=D\n";

  // restore the caller's pointers, order matters since R14 walks downward
  restoreFromFrame("THAT"); // *(endFrame - 1)
  restoreFromFrame("THIS"); // *(endFrame - 2)
  restoreFromFrame("ARG");  // *(endFrame - 3)
  restoreFromFrame("LCL");  // *(endFrame - 4)

  // goto retAddr, A=M loads the saved address from R15 into A
  file << "@R15\n";
  file << "A=M\n";
  file << "0;JMP\n";
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
  // D;JEQ / D;JGT / D;JLT, the ';' is required between the comp and jump
  // parts of a Hack instruction
  file << "D;" << jumpCommand << '\n';
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