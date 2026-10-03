#include <fstream>
#include <string>

class Parser {
public:
  Parser(const std::string &filename);

  bool hasMoreLines();

  void advance();

  const std::string commandType();

  std::string arg1();

  int arg2();

private:
  std::ifstream file;
};