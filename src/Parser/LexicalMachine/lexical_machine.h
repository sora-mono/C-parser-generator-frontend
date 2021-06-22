#include <queue>

#include "Common/object_manager.h"
#include "Generator/LexicalGenerator/lexical_generator.h"
#include "Parser/DfaMachine/dfa_machine.h"
#ifndef PARSER_LEXICALMACHINE_LEXICALMACHINE_H_
#define PARSER_LEXICALMACHINE_LEXICALMACHINE_H_

namespace frontend::parser::lexicalmachine {

class LexicalMachine {
  using LexicalGenerator =
      frontend::generator::lexicalgenerator::LexicalGenerator;
  using DfaMachine = frontend::parser::dfamachine::DfaMachine;

 public:
  // DFA���淵�ص�����
  using DfaReturnData = DfaMachine::ReturnData;
  // �﷨������ID
  using ParsingTableEntryId = LexicalGenerator::ParsingTableEntryId;
  // ����ʽ��ID��ID������������ʽ�������������ú����ʽ�����û�м�ֵ��
  using ProductionNodeId = LexicalGenerator::ProductionNodeId;
  // �ڵ�����
  using ProductionNodeType = LexicalGenerator::ProductionNodeType;
  // �û�����ķ����ú��������ݶ���Ĺ�����
  using ProcessFunctionClassManagerType =
      LexicalGenerator::ProcessFunctionClassManagerType;
  // �������еĶ����ID
  using ProcessFunctionClassId = LexicalGenerator::ProcessFunctionClassId;
  // �������еĶ���Ļ���
  using ProcessFunctionInterface = frontend::generator::lexicalgenerator::ProcessFunctionInterface;
  // �﷨��������Ŀ
  using ParsingTableEntry = LexicalGenerator::ParsingTableEntry;
  // ������Ŀ��
  using ActionAndAttachedData =
      LexicalGenerator::ParsingTableEntry::ActionAndAttachedData;
  // ���嶯��
  using ActionType = LexicalGenerator::ActionType;
  // ����ʱʹ�õ�����
  using ShiftAttachedData = LexicalGenerator::ParsingTableEntry::ShiftAttachedData;
  // �鲢ʱʹ�õ�����
  using ReductAttachedData = LexicalGenerator::ParsingTableEntry::ReductAttachedData;

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ��������ȼ������ݵ�ǰ������ȼ������ȷ����0����Ϊ����������ȼ�
  using OperatorPriority = LexicalGenerator::OperatorPriority;
#endif  // USE_AMBIGUOUS_GRAMMAR

  // �洢��������ʹ�õ�����
  struct ParsingData {
    // ��ǰ�﷨��������ĿID
    ParsingTableEntryId parsing_table_entry_id;
    // ��parsing_table_entry_id��Ŀ�Ļ���������ĵ��ʵ�ID
    ProductionNodeId shift_node_id;

#ifdef USE_AMBIGUOUS_GRAMMAR
    // ����������ȼ�Ϊ0
    OperatorPriority operator_priority = OperatorPriority(0);
#endif  // USE_AMBIGUOUS_GRAMMAR
  };

  LexicalMachine();
  LexicalMachine(const LexicalMachine&) = delete;
  LexicalMachine& operator=(LexicalMachine&&) = delete;

  // ��������
  void LoadConfig(const std::string& filename);
  // ����DFA���ص�����
  void SetDfaReturnData(DfaReturnData&& dfa_return_data) {
    dfa_return_data_ = std::move(dfa_return_data);
  }
  // ��ȡDFA���صĴ�����Ľڵ������
  DfaReturnData& GetWaitingShiftWordData() { return dfa_return_data_; }
  // ��ȡ��һ�����ʵ����ݲ�����dfa_return_data_��
  void GetNextWord() { SetDfaReturnData(dfa_machine_.GetNextWord()); }
  // ���ø�������ID
  void SetRootParsingEntryId(ParsingTableEntryId root_parsing_entry_id) {
    root_parsing_entry_id_ = root_parsing_entry_id;
  }
  // ��ȡ��������ID
  ParsingTableEntryId GetRootParsingEntryId() const {
    return root_parsing_entry_id_;
  }
  // ��ȡ�û�����Ĵ�������Ķ���
  ProcessFunctionInterface& GetProcessFunctionClassId(
    ProcessFunctionClassId class_id) {
    return manager_process_function_class_[class_id];
  }
  // ��ȡ�ڸ��ս�ڵ������µĶ���������/��Լ���͸�������
  // �����ڸ�ת�������򷵻ؿ�ָ��
  const ActionAndAttachedData* GetActionAndTarget(ParsingTableEntryId src_entry_id,
                                            ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return parsing_table_[src_entry_id].AtTerminalNode(node_id);
  }
  // ��ȡ����÷��ս�ڵ�󵽴�Ĳ���ʽ��Ŀ
  ParsingTableEntryId GetNextEntryId(ParsingTableEntryId src_entry_id,
                                     ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return parsing_table_[src_entry_id].AtNonTerminalNode(node_id);
  }
  // ��������뵥�����ս�ڵ�����
  // �Զ���������͹鲢���鲢���Զ�ִ��һ��������ս�ڵ��GetNextWord()
  void TerminalWordWaitingShift(std::queue<ParsingData>* parsing_data,
                                ParsingData* parsing_data_now);
  // TerminalWordWaitingShift�ӹ��̣���������������������Զ�GetNextWord()
  // ��Ҫ����������parsing_data_now������Ľڵ��
  void TerminalWordShift(const ActionAndAttachedData& action_and_target,
                         std::queue<ParsingData>* parsing_data, ParsingData* parsing_data_now);
  // TerminalWordWaitingShifg�ӹ��̣������Լ�����
  // ������Զ�ִ��һ��������ս�ڵ�Ĳ�����GetNextWord()
  void TerminalWordReduct(const ActionAndAttachedData& action_and_target,
                          std::queue<ParsingData>* parsing_data, ParsingData* parsing_data_now);
  // ��������뵥��������������
  void OperatorWordWaitingShift();
  // ���������ļ�
  bool Parse(const std::string& filename);
 private:
  // DFA������
  frontend::parser::dfamachine::DfaMachine dfa_machine_;
  // ����������ĿID
  ParsingTableEntryId root_parsing_entry_id_;
  // �û�����ķ����ú��������ݶ���Ĺ�����
  ProcessFunctionClassManagerType manager_process_function_class_;
  // �﷨������ֻ�м�������ʱ�����޸�
  const LexicalGenerator::ParsingTableType parsing_table_;

  // DFA���ص�����
  DfaReturnData dfa_return_data_;
};

}  // namespace frontend::parser::lexicalmachine
#endif  // !PARSER_LEXICALMACHINE_LEXICALMACHINE_H_