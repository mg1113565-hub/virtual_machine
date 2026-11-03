#include "Globals.h"
#include <fstream>
#include <string>

class CodeWriter {
public:
  CodeWriter(const std::string &filename);

  void setFileName();

  void writeArithmetic(std::string command);

  void writePushPop(ProjectEnums::Command command, std::string segment,
                    int index);
  // TODO: write assembly code that effects label command
  void writeLabel(std::string label);

  // TODO; write code that affecst goto command
  void writeGoto(std::string label);

  // TODO: write code that affects IF command
  void writeIf(std::string label);

  // TODO: write code that affects Function command
  void writeFunction(std::string functionName, int nVars);

  // TODO: write the code that affects the call command
  void writeCall(std::string functionName, int nVars);

  // TODO: write the code that affecst the return  command
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

private:
  std::ofstream file;
  std::string fileName;
  int labelCount = 0;
};