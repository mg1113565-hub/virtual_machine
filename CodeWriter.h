#include <fstream>
#include <string>

class CodeWriter {
public:
  CodeWriter(const std::string &filename);

  void writeArithmetic(std::string command);

  void writePushPop(std::string command, std::string segment, int index);

private:
};