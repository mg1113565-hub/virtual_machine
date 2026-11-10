#pragma once

#include "Globals.h"
#include <fstream>
#include <string>

class CodeWriter {
public:
  // outputPath: the .asm file to write to
  // writeBootstrap: true when translating more than one .vm file, the
  // bootstrap code sets SP=256 and calls Sys.init
  CodeWriter(const std::string &outputPath, bool writeBootstrap);

  // tells the CodeWriter a new .vm file is being translated, the name is just
  // the file name with no folder and no extension (e.g. "Main"), it is used to
  // name static variables (Main.0, Main.1, ...)
  void setFileName(const std::string &name);

  void writeArithmetic(std::string command);

  void writePushPop(ProjectEnums::Command command, std::string segment,
                    int index);

  // label X  ->  (functionName$X)
  void writeLabel(std::string label);

  // goto X  ->  unconditional jump to functionName$X
  void writeGoto(std::string label);

  // if-goto X  ->  pop the stack, jump to functionName$X if value != 0
  void writeIf(std::string label);

  // function f nVars  ->  entry label for f, then push nVars zeros (locals)
  void writeFunction(std::string functionName, int nVars);

  // call f nArgs  ->  save caller's frame, reposition ARG/LCL, jump to f
  void writeCall(std::string functionName, int nArgs);

  // return  ->  copy return value to caller, restore caller's frame, jump back
  void writeReturn();

  void close();

  // helper functions

  void incrementSP();

  void decrementSP();

  void pushDtoStack();

  void popStacktoD();

  void loadSPtoA();

  void loadSegment(std::string segment, int index);

  void writeCompareLogic(std::string jumpCommand);

  // turns a VM label into the unique assembly label functionName$label
  std::string scopedLabel(const std::string &label);

  // pushes the value stored in a pointer register (LCL, ARG, THIS, THAT)
  void pushRegister(const std::string &reg);

  // used by return: R14 holds a frame address, this moves R14 down by one and
  // copies the value stored there into reg
  void restoreFromFrame(const std::string &reg);

private:
  std::ofstream file;
  std::string fileName;
  // name of the function we are currently inside, labels are scoped to it
  std::string currentFunction;
  int labelCount = 0;
  // counts calls, so every return address label is unique
  int returnCount = 0;
};
