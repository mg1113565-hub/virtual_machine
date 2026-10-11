#include <fstream>
#include <string>

class CodeWriter {
public:
  CodeWriter(const std::string &filename);

  void writeArithmetic(std::string command);

  void writePushPop(std::string command, std::string segment, int index);

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
};