#include "Common/object_manager.h"
#include "Generator/LexicalGenerator/lexical_generator.h"
#include "Parser/DfaMachine/dfa_machine.h"

#ifndef PARSER_LEXICALMACHINE_LEXICALMACHINE_H_
#define PARSER_LEXICALMACHINE_LEXICALMACHINE_H_

namespace frontend::parser::lexicalmachine {

class LexicalMachine {
 public:
  // �﷨������ID
  using ParsingTableEntryId = frontend::generator::lexicalgenerator::
      LexicalGenerator::ParsingTableEntryId;
  // ����ʽ��ID��ID������������ʽ�������������ú����ʽ�����û�м�ֵ��
  using ProductionNodeId =
      frontend::generator::lexicalgenerator::LexicalGenerator::ProductionNodeId;
  // DFA���淵�ص�����
  using DfaReturnData = frontend::parser::dfamachine::DfaMachine::ReturnData;
  // �����װ�û��Զ��庯�������ݵ�����ѷ�����������
  using ProcessFunctionClassManagerType = frontend::generator::
      lexicalgenerator::LexicalGenerator::ProcessFunctionClassManagerType;
  LexicalMachine();
  LexicalMachine(const LexicalMachine&) = delete;
  LexicalMachine& operator=(LexicalMachine&&) = delete;
  // ��ȡ��һ�����ʵ�����
  DfaReturnData GetNextNode() { return dfa_machine_.GetNextNode(); }
  //���������ļ�
  bool Parse(const std::string& filename);

 private:
  frontend::parser::dfamachine::DfaMachine dfa_machine_;
  ProcessFunctionClassManagerType manager_process_function_class_;
};

}  // namespace frontend::parser::lexicalmachine
#endif  // !PARSER_LEXICALMACHINE_LEXICALMACHINE_H_