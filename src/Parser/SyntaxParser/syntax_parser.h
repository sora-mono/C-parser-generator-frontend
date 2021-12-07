/// @file syntax_parser.h
/// @brief �﷨������
#ifndef PARSER_SYNTAXPARSER_SYNTAXPARSER_H_
#define PARSER_SYNTAXPARSER_SYNTAXPARSER_H_
#include <queue>
#include <stack>

#include "Common/object_manager.h"
#include "Generator/SyntaxGenerator/process_function_interface.h"
#include "Generator/SyntaxGenerator/syntax_analysis_table.h"
#include "Generator/export_types.h"
#include "Parser/DfaParser/dfa_parser.h"

namespace frontend::parser::syntax_parser {

/// @class SyntaxParser syntax_parser.h
/// @brief �﷨������
class SyntaxParser {
  using DfaParser = frontend::parser::dfa_parser::DfaParser;

 public:
  /// @brief DFA���淵�صĵ�����Ϣ
  using WordInfo = DfaParser::WordInfo;
  /// @brief �﷨��������ĿID
  using SyntaxAnalysisTableEntryId =
      frontend::generator::syntax_generator::SyntaxAnalysisTableEntryId;
  /// @brief �﷨������
  using SyntaxAnalysisTableType =
      frontend::generator::syntax_generator::SyntaxAnalysisTableType;
  /// @brief ����ʽID
  using ProductionNodeId =
      frontend::generator::syntax_generator::ProductionNodeId;
  /// @brief ����ʽ�ڵ�����
  using ProductionNodeType =
      frontend::generator::syntax_generator::ProductionNodeType;
  /// @brief ����������
  using OperatorAssociatityType =
      frontend::generator::syntax_generator::OperatorAssociatityType;
  /// @brief ��װ��Լ���������ʵ�������������
  using ProcessFunctionClassManagerType =
      frontend::generator::syntax_generator::ProcessFunctionClassManagerType;
  /// @brief ��װ��Լ���������ʵ��������ID
  using ProcessFunctionClassId =
      frontend::generator::syntax_generator::ProcessFunctionClassId;
  /// @brief ��װ��Լ��������Ļ���
  using ProcessFunctionInterface =
      frontend::generator::syntax_generator::ProcessFunctionInterface;
  /// @brief �����͸������ݵĻ���
  using ActionAndAttachedDataInterface = frontend::generator::syntax_generator::
      SyntaxAnalysisTableEntry::ActionAndAttachedDataInterface;
  /// @brief �����ǰ������ʱ�Ķ���
  using ActionType = frontend::generator::syntax_generator::ActionType;
  /// @brief ���붯��������
  using ShiftAttachedData = frontend::generator::syntax_generator::
      SyntaxAnalysisTableEntry::ShiftAttachedData;
  /// @brief ��Լ����������
  using ReductAttachedData = frontend::generator::syntax_generator::
      SyntaxAnalysisTableEntry::ReductAttachedData;
  /// @brief ���ݸ��û��ĵ������ʵ�����
  using WordDataToUser = ProcessFunctionInterface::WordDataToUser;
  /// @brief �ս�ڵ�����
  using TerminalWordData = ProcessFunctionInterface::TerminalWordData;
  /// @brief ���ս�ڵ�����
  using NonTerminalWordData = ProcessFunctionInterface::NonTerminalWordData;
  /// @brief
  /// ��������ȼ��������������������ȼ���������ȼ���0����Ϊ����������ȼ�
  using OperatorPriority =
      frontend::generator::syntax_generator::OperatorPriority;

  /// @class ParsingData syntax_parser.h
  /// @brief ����ʱʹ�õ�����
  struct ParsingData {
    /// @brief ��ǰ�﷨��������ĿID
    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id;
    /// @brief ��syntax_analysis_table_entry_id��Ŀ�Ļ���������Ĳ���ʽ�ڵ��ID
    /// @note �ṩ����Ϊ��֧�ֿչ�Լ����
    ProductionNodeId shift_node_id = ProductionNodeId::InvalidId();
    /// @brief ������ս�ڵ����ݻ���ս�ڵ��Լ���û����ص�����
    WordDataToUser word_data_to_user;
    /// @brief ����������ȼ�Ϊ0
    OperatorPriority operator_priority = OperatorPriority(0);
  };

  SyntaxParser() { LoadConfig(); }
  SyntaxParser(const SyntaxParser&) = delete;
  SyntaxParser& operator=(SyntaxParser&&) = delete;

  /// @brief ��������
  void LoadConfig();
  /// @brief ����DFA���صĴ����뵥������
  /// @param[in] dfa_return_data ��DFA���صĴ����뵥������
  void SetDfaReturnData(WordInfo&& dfa_return_data) {
    dfa_return_data_ = std::move(dfa_return_data);
  }
  /// @brief ��ȡDFA���صĴ����뵥�ʵ�����
  WordInfo& GetWaitingProcessWordInfo() { return dfa_return_data_; }
  /// @brief ��ȡ��һ�����ʵ����ݲ�����dfa_return_data_��
  void GetNextWord() { SetDfaReturnData(dfa_parser_.GetNextWord()); }
  /// @brief ��ȡ���﷨��������ĿD
  /// @return ���ظ��﷨��������ĿID
  SyntaxAnalysisTableEntryId GetRootParsingEntryId() const {
    return root_parsing_entry_id_;
  }
  /// @brief ��ȡ��װ��Լ���������ʵ��������
  /// @return ���ذ�װ��Լ���������ʵ���������const����
  const ProcessFunctionInterface& GetProcessFunctionClass(
      ProcessFunctionClassId class_id) const {
    return manager_process_function_class_[class_id];
  }
  /// @brief ��ȡ�ڸ�����ǰ������ʽ�ڵ������µĶ����͸�������
  /// @param[in] src_entry_id ����ʼ�﷨��������ĿID
  /// @param[in] node_id ����ǰ������ʽ�ڵ�ID
  /// @return ����ָ�����͸������ݻ����constָ��
  /// @retval nullptr �����ڸ�ת�������򷵻ؿ�ָ��
  const ActionAndAttachedDataInterface* GetActionAndTarget(
      SyntaxAnalysisTableEntryId src_entry_id, ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return syntax_analysis_table_[src_entry_id].AtTerminalNode(node_id);
  }
  /// @brief ��ȡ������ս����ʽ�ڵ�󵽴�Ĳ���ʽ��Ŀ
  /// @param[in] src_entry_id ����ʼ�﷨��������ĿID
  /// @param[in] node_id ������ķ��ս����ʽ�ڵ�
  /// @return ����������ս����ʽ��ת�Ƶ����﷨��������ĿID
  /// @return SyntaxAnalysisTableEntryId::InvalidId()
  /// �޷���������ķ��ս����ʽ�ڵ�ID
  SyntaxAnalysisTableEntryId GetNextEntryId(
      SyntaxAnalysisTableEntryId src_entry_id, ProductionNodeId node_id) const {
    assert(src_entry_id.IsValid());
    return syntax_analysis_table_[src_entry_id].AtNonTerminalNode(node_id);
  }
  /// @brief ��ȡ��ǰ��Ծ�Ľ������ݣ���������ջ������
  /// @return ���ؽ�������ջ������������
  ParsingData& GetParsingDataNow() { return parsing_stack_.top(); }
  /// @brief ������ѹ���������ջ
  /// @param[in] parsing_data ����������
  /// @note ��֧��ParsingData���͵�parsing_data��Ϊ����
  template <class ParsingDataType>
  void PushParsingData(ParsingDataType&& parsing_data) {
    parsing_stack_.push(std::forward<ParsingDataType>(parsing_data));
  }
  /// @brief ������������ջ��������
  void PopTopParsingData() { parsing_stack_.pop(); }
  /// @brief ��ѯ��������ջ������Ŀ
  /// @return ���ؽ�������ջ��������Ŀ
  size_t GetParsingStackSize() const { return parsing_stack_.size(); }
  /// @brief ���������ļ�������AST
  /// @param[in] filename �������ļ���
  /// @return �����Ƿ�ɹ�
  /// @retval true �����ɹ�
  /// @retval false �޷����ļ�/����ʧ��
  bool Parse(const std::string& filename);

 private:
  /// @brief �������л������
  friend class boost::serialization::access;

  /// @brief boost-serialization�����﷨�������õĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    // ת��const���������л������ȡ����
    ar >> const_cast<SyntaxAnalysisTableEntryId&>(root_parsing_entry_id_);
    ar >> const_cast<SyntaxAnalysisTableType&>(syntax_analysis_table_);
    ar >> const_cast<ProcessFunctionClassManagerType&>(
              manager_process_function_class_);
  }
  /// �����л���Ϊ��������أ�Parser���������ã�������
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  /// @brief ��������뵥�����ս�ڵ�����
  /// @details
  /// 1.�Զ�ѡ������͹鲢
  /// 2.�����ִ��GetNextWord()
  /// 3.�鲢�󽫵õ��ķ��ս�ڵ�����
  void TerminalWordWaitingProcess();
  /// @brief ������ǰ�����Ŵ���������
  /// @param[in] action_and_target �����붯���͸�������
  /// @note
  /// 1.������Զ�ִ��GetNextWord()
  /// 2.�ú���ΪTerminalWordWaitingShift�������ӹ���
  void ShiftTerminalWord(const ShiftAttachedData& action_and_target);
  /// @brief �������ʽ����Լ�����
  /// @param[in] action_and_target ����Լ�����͸�������
  /// @note
  /// 1.��Լ���Զ�����õ��ķ��ս�ڵ㲢GetNextWord()
  /// 2.TerminalWordWaitingShift�������ӹ���
  void Reduct(const ReductAttachedData& action_and_target);
  /// @brief ������ս�ڵ�
  /// @param[in] non_terminal_word_data ����Լ���û����ص�����
  /// @param[in] reducted_nonterminal_node_id ����Լ��õ��ķ��ս����ʽID
  void ShiftNonTerminalWord(NonTerminalWordData&& non_terminal_word_data,
                            ProductionNodeId reducted_nonterminal_node_id);
  /// @brief ������һ���ǹ�Լ����
  /// @note
  /// Ӱ��˫��������������ѡ����һ��Ϊ��Լ������ʹ����൥Ŀ����
  /// ����ʹ��˫Ŀ����
  void SetLastOperateIsReduct() { last_operate_is_reduct_ = true; }
  /// @brief ������һ�β������ǹ�Լ����
  /// @note
  /// Ӱ��˫��������������ѡ����һ��Ϊ��Լ������ʹ����൥Ŀ����
  /// ����ʹ��˫Ŀ����
  void SetLastOperateIsNotReduct() { last_operate_is_reduct_ = false; }
  /// @brief �ж���һ�β����Ƿ�Ϊ��Լ����
  /// @return ������һ�β����Ƿ�Ϊ��Լ����
  /// @retval true ��һ�β����ǹ�Լ����
  /// @retval false ��һ�β������ǹ�Լ����
  bool LastOperateIsReduct() const { return last_operate_is_reduct_; }

  /// @brief DFA������
  DfaParser dfa_parser_;
  /// @brief ����������ĿID��ֻ�м�������ʱ�����޸�
  const SyntaxAnalysisTableEntryId root_parsing_entry_id_;
  /// @brief ��װ��Լ���������ʵ��������Ĺ�������ֻ�м�������ʱ�����޸�
  const ProcessFunctionClassManagerType manager_process_function_class_;
  /// @brief �﷨������ֻ�м�������ʱ�����޸�
  const SyntaxAnalysisTableType syntax_analysis_table_;

  /// @brief DFA���ص�����
  WordInfo dfa_return_data_;
  /// @brief ����������ջ��ջ��Ϊ��ǰ��������
  std::stack<ParsingData> parsing_stack_;
  /// @brief ����ϴβ����Ƿ�Ϊ��Լ����
  /// @note
  /// ����֧����������ȼ�ʱͬһ�����������ϸ��Ϊ��൥Ŀ�������˫Ŀ���������
  bool last_operate_is_reduct_ = true;
};

}  // namespace frontend::parser::syntax_parser
#endif  /// !PARSER_SYNTAXMACHINE_SYNTAXMACHINE_H_