#include <queue>
#include <stack>

#include "Common/object_manager.h"
#include "Generator/SyntaxGenerator/syntax_generator.h"
#include "Parser/DfaMachine/dfa_machine.h"
#ifndef PARSER_SYNTAXMACHINE_SYNTAXMACHINE_H_
#define PARSER_SYNTAXMACHINE_SYNTAXMACHINE_H_

namespace frontend::parser::syntaxmachine {

class SyntaxMachine {
  using SyntaxGenerator =
      frontend::generator::syntaxgenerator::SyntaxGenerator;
  using DfaMachine = frontend::parser::dfamachine::DfaMachine;

 public:
  // DFA���淵�صĵ�����Ϣ
  using WordInfo = DfaMachine::WordInfo;
  // �﷨������ID
  using ParsingTableEntryId = SyntaxGenerator::ParsingTableEntryId;
  // ����ʽ��ID��ID������������ʽ�������������ú����ʽ�����û�м�ֵ��
  using ProductionNodeId = SyntaxGenerator::ProductionNodeId;
  // �ڵ�����
  using ProductionNodeType = SyntaxGenerator::ProductionNodeType;

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ����������
  using AssociatityType = SyntaxGenerator::AssociatityType;
#endif  // USE_AMBIGUOUS_GRAMMAR

  // �û�����ķ����ú��������ݶ���Ĺ�����
  using ProcessFunctionClassManagerType =
      SyntaxGenerator::ProcessFunctionClassManagerType;
  // �������еĶ����ID
  using ProcessFunctionClassId = SyntaxGenerator::ProcessFunctionClassId;
  // �������еĶ���Ļ���
  using ProcessFunctionInterface =
      frontend::generator::syntaxgenerator::ProcessFunctionInterface;
  // �﷨��������Ŀ
  using ParsingTableEntry = SyntaxGenerator::ParsingTableEntry;
  // ������Ŀ��
  using ActionAndAttachedData =
      SyntaxGenerator::ParsingTableEntry::ActionAndAttachedData;
  // ���嶯��
  using ActionType = SyntaxGenerator::ActionType;
  // ����ʱʹ�õ�����
  using ShiftAttachedData =
      SyntaxGenerator::ParsingTableEntry::ShiftAttachedData;
  // �鲢ʱʹ�õ�����
  using ReductAttachedData =
      SyntaxGenerator::ParsingTableEntry::ReductAttachedData;
  // ���ݸ��û��ĵ������ʵ�����
  using WordDataToUser = ProcessFunctionInterface::WordDataToUser;
  // �ս�ڵ�����
  using TerminalWordData = ProcessFunctionInterface::TerminalWordData;
  // ���ս�ڵ�����
  using NonTerminalWordData = ProcessFunctionInterface::NonTerminalWordData;
#ifdef USE_AMBIGUOUS_GRAMMAR
  // ��������ȼ������ݵ�ǰ������ȼ������ȷ����0����Ϊ����������ȼ�
  using OperatorPriority = SyntaxGenerator::OperatorPriority;
#endif  // USE_AMBIGUOUS_GRAMMAR

  // �洢��������ʹ�õ�����
  struct ParsingData {
    // ��ǰ�﷨��������ĿID
    ParsingTableEntryId parsing_table_entry_id;
    // ��parsing_table_entry_id��Ŀ�Ļ���������ĵ��ʵ�ID
    // �ṩ����Ϊ��֧�ֿչ�Լ����
    ProductionNodeId shift_node_id;
    // �ս�ڵ����ݻ���ս�ڵ��Լ���û����ص�����
    WordDataToUser word_data_to_user_;

#ifdef USE_AMBIGUOUS_GRAMMAR
    // ����������ȼ�Ϊ0
    OperatorPriority operator_priority = OperatorPriority(0);
#endif  // USE_AMBIGUOUS_GRAMMAR
  };

  SyntaxMachine();
  SyntaxMachine(const SyntaxMachine&) = delete;
  SyntaxMachine& operator=(SyntaxMachine&&) = delete;

  // ��������
  void LoadConfig(const std::string& filename);
  // ����DFA���ص�����
  void SetDfaReturnData(WordInfo&& dfa_return_data) {
    dfa_return_data_ = std::move(dfa_return_data);
  }
  // ��ȡDFA���صĴ�����Ľڵ������
  WordInfo& GetWaitingProcessWordInfo() { return dfa_return_data_; }
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
  ProcessFunctionInterface& GetProcessFunctionClass(
      ProcessFunctionClassId class_id) {
    return manager_process_function_class_[class_id];
  }
  // ��ȡ�ڸ��ս�ڵ������µĶ���������/��Լ���͸�������
  // �����ڸ�ת�������򷵻ؿ�ָ��
  const ActionAndAttachedData* GetActionAndTarget(
      ParsingTableEntryId src_entry_id, ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return parsing_table_[src_entry_id].AtTerminalNode(node_id);
  }
  // ��ȡ����÷��ս�ڵ�󵽴�Ĳ���ʽ��Ŀ
  ParsingTableEntryId GetNextEntryId(ParsingTableEntryId src_entry_id,
                                     ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return parsing_table_[src_entry_id].AtNonTerminalNode(node_id);
  }
  std::stack<ParsingData>& GetParsingStack() { return parsing_stack_; }
  ParsingData& GetParsingDataNow() { return parsing_data_now_; }
  
#ifdef USE_AMBIGUOUS_GRAMMAR
  OperatorPriority& GetOperatorPriorityNow() { return operator_priority_now_; }
#endif  // USE_AMBIGUOUS_GRAMMAR

  // ��������������ջ���˵����ݵ�parsing_data_now
  void PopParsingStack() {
    GetParsingDataNow() = std::move(GetParsingStack().top());
    GetParsingStack().pop();
  }
  // �Żص�ǰ��������
  void PutbackWordNow() {
    dfa_machine_.PutbackWord(std::move(GetWaitingProcessWordInfo()));
  }
  // ��������뵥�����ս�ڵ�����
  // �Զ���������͹鲢���鲢��ִ��һ��������ս�ڵ��ִ��GetNextWord()
  void TerminalWordWaitingProcess();
  // TerminalWordWaitingShift�ӹ��̣���������������������Զ�GetNextWord()
  void ShiftTerminalWord(const ActionAndAttachedData& action_and_target);
  // TerminalWordWaitingShifg�ӹ��̣������Լ�����
  // ������Զ�ִ��һ��������ս�ڵ�Ĳ�����GetNextWord()
  void Reduct(const ActionAndAttachedData& action_and_target);
  // ������ս�ڵ�
  // non_terminal_word_data�ǹ�Լ���û����ص�����
  // reducted_nonterminal_node_id�ǹ�Լ��õ��ķ��ս����ʽID
  void ShiftNonTerminalWord(NonTerminalWordData&& non_terminal_word_data,
                            ProductionNodeId reducted_nonterminal_node_id);
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
  const SyntaxGenerator::ParsingTableType parsing_table_;

  // DFA���ص�����
  WordInfo dfa_return_data_;
  // ����������ջ
  std::stack<ParsingData> parsing_stack_;
  // ��ǰ����������
  ParsingData parsing_data_now_;

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ��ǰ���ȼ�
  OperatorPriority operator_priority_now_;
#endif  // USE_AMBIGUOUS_GRAMMAR
};

}  // namespace frontend::parser::syntaxmachine
#endif  // !PARSER_SYNTAXMACHINE_SYNTAXMACHINE_H_