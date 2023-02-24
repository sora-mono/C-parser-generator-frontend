#include "syntax_parser.h"

#include <format>
namespace frontend::parser::syntax_parser {
void SyntaxParser::LoadConfig() {
  std::ifstream config_file(frontend::common::kSyntaxConfigFileName,
                            std::ios_base::binary);
  boost::archive::binary_iarchive iarchive(config_file);
  dfa_parser_.LoadConfig();
  iarchive >> *this;
}
bool SyntaxParser::Parse(const std::string& filename) {
  bool result = dfa_parser_.SetInputFile(filename);
  if (result == false) [[unlikely]] {
    fprintf(stderr, "���ļ�\"%s\"ʧ�ܣ�����\n", filename.c_str());
    return false;
  }
  GetNextWord();
  // ��ս�������ջ
  auto old_parsing_stack = std::stack<ParsingData>();
  parsing_stack_.swap(old_parsing_stack);
  // ѹ���ڱ�������Reduct������ÿ�ε���ջ�����ݶ���Ҫ�ж��Ƿ�ջ��
  PushParsingData(ParsingData());
  // ��ʼ����������ջ��ѹ�뵱ǰ��������
  PushParsingData(
      ParsingData{.syntax_analysis_table_entry_id = GetRootParsingEntryId(),
                  .operator_priority = OperatorPriority(0)});
  while (GetParsingStackSize() != 1) {
    // TODO ����û������������ջ�Ĺ���
    TerminalWordWaitingProcess();
  }
  return true;
}

void SyntaxParser::TerminalWordWaitingProcess() {
  assert(GetWaitingProcessWordInfo().word_attached_data_.node_type ==
             ProductionNodeType::kTerminalNode ||
         GetWaitingProcessWordInfo().word_attached_data_.node_type ==
             ProductionNodeType::kOperatorNode ||
         GetWaitingProcessWordInfo().word_attached_data_.node_type ==
             ProductionNodeType::kEndNode);
  ProductionNodeId production_node_to_shift_id =
      GetWaitingProcessWordInfo().word_attached_data_.production_node_id;
  const ActionAndAttachedDataInterface& action_and_attached_data =
      *GetActionAndTarget(GetParsingDataNow().syntax_analysis_table_entry_id,
                          production_node_to_shift_id);
  switch (action_and_attached_data.GetActionType()) {
    // TODO ��ӽ���ʱ�ĺ�������
    [[unlikely]] case ActionType::kAccept:
      exit(0);
      break;
    // TODO ��Ӵ�������
    [[unlikely]] case ActionType::kError:
      std::cerr << std::format("�﷨����") << std::endl;
      exit(-1);
      break;
    case ActionType::kReduct:
      Reduct(action_and_attached_data.GetReductAttachedData());
      break;
    case ActionType::kShift:
      ShiftTerminalWord(action_and_attached_data.GetShiftAttachedData());
      break;
    case ActionType::kShiftReduct: {
      // ��Ҫ����ʵ������ж����뻹�ǹ�Լ
      auto& terminal_node_info =
          GetWaitingProcessWordInfo().word_attached_data_;
      switch (terminal_node_info.node_type) {
        case ProductionNodeType::kTerminalNode:
          // ��������ڵ�ʹ��̰�Ĳ��ԣ���ֹ�й���ǰ׺�Ĳ���ʽֻ����̵�������Ч
          ShiftTerminalWord(action_and_attached_data.GetShiftAttachedData());
          break;
        case ProductionNodeType::kOperatorNode: {
          // ��������ȼ����벻Ϊ0
          OperatorPriority priority_now = GetParsingDataNow().operator_priority;
          auto [operator_associate_type, operator_priority] =
              terminal_node_info.GetAssociatityTypeAndPriority(
                  LastOperateIsReduct());
          if (priority_now > operator_priority) {
            // ��ǰ���ȼ����ڴ����������������ȼ���ִ�й�Լ����
            Reduct(action_and_attached_data.GetReductAttachedData());
          } else if (priority_now == operator_priority) {
            // ��ǰ���ȼ����ڴ����������������ȼ�����Ҫ�ж������
            if (operator_associate_type ==
                OperatorAssociatityType::kLeftToRight) {
              // �����Ϊ�����ҽ�ϣ�ִ�й�Լ����
              Reduct(action_and_attached_data.GetReductAttachedData());
            } else {
              // �����Ϊ���ҵ����ϣ�ִ���������
              ShiftTerminalWord(
                  action_and_attached_data.GetShiftAttachedData());
            }
          } else {
            // ��ǰ���ȼ����ڴ����������������ȼ���ִ���������
            ShiftTerminalWord(action_and_attached_data.GetShiftAttachedData());
          }
        } break;
        case ProductionNodeType::kEndNode:
          // EndNode��ת�Ʊ���Ķ���ֻ�ܹ�Լ��������
        case ProductionNodeType::kNonTerminalNode:
          // NonTerminalNode�ڲ�����ͱ�Reduct��������ShiftNonTerminalNode����
        default:
          assert(false);
          break;
      }
    } break;
    default:
      assert(false);
      break;
  }
}

inline void SyntaxParser::ShiftTerminalWord(
    const ShiftAttachedData& action_and_target) {
  ParsingData& parsing_data_now = GetParsingDataNow();
  WordInfo& word_info = GetWaitingProcessWordInfo();
  // ������ǰ���ʵ�����
  // ������ڵ��ID
  parsing_data_now.shift_node_id =
      word_info.word_attached_data_.production_node_id;
  // ��Ӵ�����ڵ���Ϣ����ǰ��������Ϣ
  parsing_data_now.word_data_to_user = std::move(word_info.symbol_);
  // ����������������������ȼ�Ϊ�µ����ȼ�
  OperatorPriority new_parsing_data_priority;
  if (word_info.word_attached_data_.node_type ==
      ProductionNodeType::kOperatorNode) {
    new_parsing_data_priority =
        word_info.word_attached_data_
            .GetAssociatityTypeAndPriority(LastOperateIsReduct())
            .second;
  } else {
    // ����ʹ��ԭ�������ȼ�
    new_parsing_data_priority = parsing_data_now.operator_priority;
  }
  // ѹ������õ��ʺ�õ��Ľ�������
  PushParsingData(
      ParsingData{.syntax_analysis_table_entry_id =
                      action_and_target.GetNextSyntaxAnalysisTableEntryId(),
                  .operator_priority = new_parsing_data_priority});
  // ��ȡ��һ������
  GetNextWord();
  // ִ���������������Ҫ������һ��Ϊ�ǹ�Լ����
  SetLastOperateIsNotReduct();
}

void SyntaxParser::Reduct(const ReductAttachedData& action_and_target) {
  const ReductAttachedData& reduct_attached_data =
      action_and_target.GetReductAttachedData();
  const ProcessFunctionInterface& process_function_object =
      GetProcessFunctionClass(reduct_attached_data.GetProcessFunctionClassId());
  const auto& production_body = reduct_attached_data.GetProductionBody();
  // ���ݸ��û����庯��������
  std::vector<std::any> word_data_to_user(production_body.size());

  // ����ջ�����ݣ�¶��word_data_to_user�洢��Ч���ݵĲ���
  // ���������κ�ʱ��ջ�����ݵ�word_data_to_user����������Ч����
  ParsingData old_parsing_data = std::move(GetParsingDataNow());
  PopTopParsingData();
  // ���ɹ�Լ���ݣ�����������˳��Ϊ����ʽ��д˳��
  // �Ӻ���ǰ�������
  auto production_node_id_iter = production_body.rbegin();
  for (auto user_data_iter = word_data_to_user.rbegin();
       user_data_iter != word_data_to_user.rend(); ++production_node_id_iter) {
    if (GetParsingDataNow().shift_node_id == *production_node_id_iter)
        [[likely]] {
      // ��ǰ����ʽ���������Լ����ȡ֮ǰ���������ṩ���û���Լʹ�õ�����
      *user_data_iter = std::move(GetParsingDataNow().word_data_to_user);
      // ������������ջ�����޸������ݵĲ��֣�¶��֮ǰ����Ľ�������
      old_parsing_data = std::move(GetParsingDataNow());
      PopTopParsingData();
      // �������ڱ�����������ջ��Զ����
      assert(GetParsingStackSize() != 0);
    }
    ++user_data_iter;
  }
  // �ָ�ջ������
  // ջ�����ݴ�����Ч��Ϣ����ǰ�﷨��������Ŀ����������ȼ�
  PushParsingData(std::move(old_parsing_data));
  ShiftNonTerminalWord(
      process_function_object.Reduct(std::move(word_data_to_user)),
      reduct_attached_data.GetReductedNonTerminalNodeId());
  // ִ����һ�������Ĺ�Լ��������Ҫ������һ��ִ���˹�Լ�����ı��
  SetLastOperateIsReduct();
}

void SyntaxParser::ShiftNonTerminalWord(
    std::any&& non_terminal_word_data,
    ProductionNodeId reducted_nonterminal_node_id) {
  ParsingData& parsing_data_now = GetParsingDataNow();
  // ��ȡ������ս�ڵ��ת�Ƶ����﷨��������Ŀ
  SyntaxAnalysisTableEntryId next_entry_id =
      GetNextEntryId(parsing_data_now.syntax_analysis_table_entry_id,
                     reducted_nonterminal_node_id);
  // ��������ڵ��ID�͸�������
  parsing_data_now.shift_node_id = reducted_nonterminal_node_id;
  parsing_data_now.word_data_to_user = std::move(non_terminal_word_data);
  // ������ս�ڵ㲻�ı���������ȼ�
  PushParsingData(
      ParsingData{.syntax_analysis_table_entry_id = next_entry_id,
                  .operator_priority = parsing_data_now.operator_priority});
}

}  // namespace frontend::parser::syntax_parser