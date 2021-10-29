#ifndef PARSER_SYNTAXMACHINE_SYNTAXMACHINE_H_
#define PARSER_SYNTAXMACHINE_SYNTAXMACHINE_H_
#include <queue>
#include <stack>

#include "Common/object_manager.h"
#include "Generator/SyntaxGenerator/process_function_interface.h"
#include "Generator/SyntaxGenerator/syntax_analysis_table.h"
#include "Generator/export_types.h"
#include "Parser/DfaMachine/dfa_machine.h"

namespace frontend::parser::syntax_machine {

class SyntaxMachine {
  using DfaMachine = frontend::parser::dfamachine::DfaMachine;

 public:
  // DFA���淵�صĵ�����Ϣ
  using WordInfo = DfaMachine::WordInfo;

  // �﷨������ID
  using SyntaxAnalysisTableEntryId =
      frontend::generator::syntax_generator::SyntaxAnalysisTableEntryId;
  // �﷨����������
  using SyntaxAnalysisTableType =
      frontend::generator::syntax_generator::SyntaxAnalysisTableType;
  // ����ʽ��ID��ID������������ʽ�������������ú����ʽ�����û�м�ֵ��
  using ProductionNodeId =
      frontend::generator::syntax_generator::ProductionNodeId;
  // �ڵ�����
  using ProductionNodeType =
      frontend::generator::syntax_generator::ProductionNodeType;
  // ����������
  using OperatorAssociatityType =
      frontend::generator::syntax_generator::OperatorAssociatityType;
  // �û�����ķ����ú��������ݶ���Ĺ�����
  using ProcessFunctionClassManagerType =
      frontend::generator::syntax_generator::ProcessFunctionClassManagerType;
  // �������еĶ����ID
  using ProcessFunctionClassId =
      frontend::generator::syntax_generator::ProcessFunctionClassId;
  // �������еĶ���Ļ���
  using ProcessFunctionInterface =
      frontend::generator::syntax_generator::ProcessFunctionInterface;
  //// �﷨��������Ŀ
  // using SyntaxAnalysisTableEntry =
  //    frontend::generator::syntax_generator::SyntaxAnalysisTableEntry;
  // ������Ŀ��
  using ActionAndAttachedDataInterface = frontend::generator::syntax_generator::
      SyntaxAnalysisTableEntry::ActionAndAttachedDataInterface;
  // ���嶯��
  using ActionType = frontend::generator::syntax_generator::ActionType;
  // ����ʱʹ�õ�����
  using ShiftAttachedData = frontend::generator::syntax_generator::
      SyntaxAnalysisTableEntry::ShiftAttachedData;
  // ��Լʱʹ�õ�����
  using ReductAttachedData = frontend::generator::syntax_generator::
      SyntaxAnalysisTableEntry::ReductAttachedData;
  // ���ݸ��û��ĵ������ʵ�����
  using WordDataToUser = frontend::generator::syntax_generator::
      ProcessFunctionInterface::WordDataToUser;
  // �ս�ڵ�����
  using TerminalWordData = ProcessFunctionInterface::TerminalWordData;
  // ���ս�ڵ�����
  using NonTerminalWordData = ProcessFunctionInterface::NonTerminalWordData;
  // ��������ȼ������ݵ�ǰ������ȼ������ȷ����0����Ϊ����������ȼ�
  using OperatorPriority =
      frontend::generator::syntax_generator::OperatorPriority;

  // �洢��������ʹ�õ�����
  struct ParsingData {
    // ��ǰ�﷨��������ĿID
    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id;
    // ��syntax_analysis_table_entry_id��Ŀ�Ļ���������Ĳ���ʽ�ڵ��ID
    // �ṩ����Ϊ��֧�ֿչ�Լ����
    ProductionNodeId shift_node_id;
    // ������ս�ڵ����ݻ���ս�ڵ��Լ���û����ص�����
    WordDataToUser word_data_to_user;
    // ����������ȼ�Ϊ0
    OperatorPriority operator_priority = OperatorPriority(0);
  };

  SyntaxMachine() { LoadConfig(); }
  SyntaxMachine(const SyntaxMachine&) = delete;
  SyntaxMachine& operator=(SyntaxMachine&&) = delete;

  // ��������
  void LoadConfig();
  // ����DFA���ص�����
  void SetDfaReturnData(WordInfo&& dfa_return_data) {
    dfa_return_data_ = std::move(dfa_return_data);
  }
  // ��ȡDFA���صĴ�����Ľڵ������
  WordInfo& GetWaitingProcessWordInfo() { return dfa_return_data_; }
  // ��ȡ��һ�����ʵ����ݲ�����dfa_return_data_��
  void GetNextWord() { SetDfaReturnData(dfa_machine_.GetNextWord()); }
  // ���ø�������ID
  void SetRootParsingEntryId(SyntaxAnalysisTableEntryId root_parsing_entry_id) {
    root_parsing_entry_id_ = root_parsing_entry_id;
  }
  // ��ȡ��������ID
  SyntaxAnalysisTableEntryId GetRootParsingEntryId() const {
    return root_parsing_entry_id_;
  }
  // ��ȡ�û�����Ĵ�������Ķ���
  ProcessFunctionInterface& GetProcessFunctionClass(
      ProcessFunctionClassId class_id) {
    return manager_process_function_class_[class_id];
  }
  // ��ȡ�ڸ��ս�ڵ������µĶ���������/��Լ���͸�������
  // �����ڸ�ת�������򷵻ؿ�ָ��
  const ActionAndAttachedDataInterface* GetActionAndTarget(
      SyntaxAnalysisTableEntryId src_entry_id, ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return syntax_analysis_table_[src_entry_id].AtTerminalNode(node_id);
  }
  // ��ȡ����÷��ս�ڵ�󵽴�Ĳ���ʽ��Ŀ
  SyntaxAnalysisTableEntryId GetNextEntryId(
      SyntaxAnalysisTableEntryId src_entry_id, ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return syntax_analysis_table_[src_entry_id].AtNonTerminalNode(node_id);
  }
  // ��ȡ��ǰ��Ծ�Ľ������ݣ���������ջ������
  ParsingData& GetParsingDataNow() { return parsing_stack_.top(); }
  template <class ParsingDataType>
  void PushParsingData(ParsingDataType&& parsing_data) {
    parsing_stack_.push(std::forward<ParsingDataType>(parsing_data));
  }
  // ������������ջ��������
  void PopTopParsingData() { parsing_stack_.pop(); }
  bool IsParsingStackEmpty() const { return parsing_stack_.empty(); }
  // ���������ļ�
  bool Parse(const std::string& filename);

 private:
  // �������л������
  friend class boost::serialization::access;

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar >> root_parsing_entry_id_;
    // ת��const���������л������ȡ����
    ar >> const_cast<SyntaxAnalysisTableType&>(syntax_analysis_table_);
    ar >> manager_process_function_class_;
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  // ��������뵥�����ս�ڵ�����
  // �Զ���������͹鲢���鲢��ִ��һ��������ս�ڵ��ִ��GetNextWord()
  void TerminalWordWaitingProcess();
  // TerminalWordWaitingShift�ӹ��̣���������������������Զ�GetNextWord()
  void ShiftTerminalWord(
      const ActionAndAttachedDataInterface& action_and_target);
  // TerminalWordWaitingShifg�ӹ��̣������Լ�����
  // ������Զ�ִ��һ��������ս�ڵ�Ĳ�����GetNextWord()
  void Reduct(const ActionAndAttachedDataInterface& action_and_target);
  // ������ս�ڵ�
  // non_terminal_word_data�ǹ�Լ���û����ص�����
  // reducted_nonterminal_node_id�ǹ�Լ��õ��ķ��ս����ʽID
  void ShiftNonTerminalWord(NonTerminalWordData&& non_terminal_word_data,
                            ProductionNodeId reducted_nonterminal_node_id);
  void SetLastOperateIsReduct() { last_operate_is_reduct_ = true; }
  void SetLastOperateIsNotReduct() { last_operate_is_reduct_ = false; }
  bool LastOperateIsReduct() const { return last_operate_is_reduct_; }

  // DFA������
  frontend::parser::dfamachine::DfaMachine dfa_machine_;
  // ����������ĿID
  SyntaxAnalysisTableEntryId root_parsing_entry_id_;
  // �û�����ķ����ú��������ݶ���Ĺ�����
  ProcessFunctionClassManagerType manager_process_function_class_;
  // �﷨������ֻ�м�������ʱ�����޸�
  const SyntaxAnalysisTableType syntax_analysis_table_;

  // DFA���ص�����
  WordInfo dfa_return_data_;
  // ����������ջ��ջ��Ϊ��ǰ��������
  std::stack<ParsingData> parsing_stack_;
  // ����ϴβ����Ƿ�Ϊ��Լ����
  // ����֧����������ȼ�ʱͬһ�����������ϸ��Ϊ��൥Ŀ�������˫Ŀ���������
  bool last_operate_is_reduct_ = true;
};

}  // namespace frontend::parser::syntax_machine
#endif  // !PARSER_SYNTAXMACHINE_SYNTAXMACHINE_H_