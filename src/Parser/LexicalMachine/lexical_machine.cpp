#include "lexical_machine.h"

#include <stack>
namespace frontend::parser::lexicalmachine {

bool LexicalMachine::Parse(const std::string& filename) {
  bool result = dfa_machine_.SetInputFile(filename);
  if (result == false) {
    [[unlikely]] fprintf(stderr, "���ļ�ʧ�ܣ�����\n");
    return false;
  }
  std::stack<ParsingData> parsing_data;
  GetNextWord();
  // ������ʽ�ķ�������
  ParsingData root_parsing_data;
  root_parsing_data.parsing_table_entry_id = GetRootParsingEntryId();
  // �����ڵ�ѹ��ջ
  parsing_data.push(std::move(root_parsing_data));

  while (!parsing_data.empty()) {
    const DfaReturnData& data = GetWaitingShiftWordData();
    switch (data.saved_data_.node_type) {
      case ProductionNodeType::kTerminalNode:
        break;
      case ProductionNodeType::kOperatorNode:
        break;
      case ProductionNodeType::kNonTerminalNode:
        break;
      default:
        assert(false);
        break;
    }
  }
}

void LexicalMachine::TerminalWordWaitingShift(
    std::queue<ParsingData>* parsing_data, ParsingData* parsing_data_now) {
  const ActionAndAttachedData& action_and_attached_data = *GetActionAndTarget(
      parsing_data_now->parsing_table_entry_id,
      ProductionNodeId(
          GetWaitingShiftWordData().saved_data_.production_node_id));
  switch (action_and_attached_data.action_type_) {
    [[unlikely]] case ActionType::kAccept : break;
    [[unlikely]] case ActionType::kError : break;
    case ActionType::kReduct:
      TerminalWordReduct(action_and_attached_data, parsing_data,
                         parsing_data_now);
      break;
    case ActionType::kShift:
      // ����Ҫ����Ľڵ��ID
      parsing_data_now->shift_node_id = ProductionNodeId(
          GetWaitingShiftWordData().saved_data_.production_node_id);
      TerminalWordShift(action_and_attached_data, parsing_data,
                        parsing_data_now);
      break;
    case ActionType::kShiftReduct:
      // ������ĵ��ʱ��������������������ȼ����벻Ϊ0
      assert(GetWaitingShiftWordData().saved_data_.priority != 0);

      break;
    default:
      assert(false);
      break;
  }
}

inline void LexicalMachine::TerminalWordShift(
    const ActionAndAttachedData& action_and_target,
    std::queue<ParsingData>* parsing_data, ParsingData* parsing_data_now) {
  // ����ǰ��״̬ѹ��ջ����Ҫ��������������Ľڵ��
  parsing_data->push(*parsing_data_now);
  // ����״̬Ϊ����ýڵ�󵽴����Ŀ
  parsing_data_now->parsing_table_entry_id =
      action_and_target.GetShiftAttachedData().next_entry_id_;
  GetNextWord();
}

void LexicalMachine::TerminalWordReduct(
    const ActionAndAttachedData& action_and_target,
    std::queue<ParsingData>* parsing_data, ParsingData* parsing_data_now) {}

}  // namespace frontend::parser::lexicalmachine