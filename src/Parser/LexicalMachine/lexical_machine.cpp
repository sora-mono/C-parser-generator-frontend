#include "lexical_machine.h"

namespace frontend::parser::lexicalmachine {

bool LexicalMachine::Parse(const std::string& filename) {
  bool result = dfa_machine_.SetInputFile(filename);
  if (result == false) {
    [[unlikely]] fprintf(stderr, "���ļ�ʧ�ܣ�����\n");
    return false;
  }
  DfaReturnData next_node_data = GetNextNode();
}

}  // namespace frontend::parser::lexicalmachine