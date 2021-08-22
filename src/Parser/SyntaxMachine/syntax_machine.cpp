#include "syntax_machine.h"

namespace frontend::parser::syntaxmachine {

bool SyntaxMachine::Parse(const std::string& filename) {
  bool result = dfa_machine_.SetInputFile(filename);
  if (result == false) [[unlikely]] {
    fprintf(stderr, "���ļ�\"%s\"ʧ�ܣ�����\n", filename.c_str());
    return false;
  }
  GetNextWord();
  // ��ʼ����ǰ��������
  GetParsingDataNow().parsing_table_entry_id = GetRootParsingEntryId();

#ifdef USE_AMBIGUOUS_GRAMMAR
  GetOperatorPriorityNow() = OperatorPriority(0);
#endif  // USE_AMBIGUOUS_GRAMMAR

  while (!GetParsingStack().empty()) {
    const WordInfo& dfa_return_data = GetWaitingProcessWordInfo();
    switch (dfa_return_data.word_attached_data_.node_type) {
      case ProductionNodeType::kTerminalNode:
      case ProductionNodeType::kOperatorNode:
      case ProductionNodeType::kEndNode:
        TerminalWordWaitingProcess();
        // TODO ����û������������ջ�Ĺ���
        break;
      // ���ս�ڵ���Reduct�����Ĵ���������������룬�������������
      case ProductionNodeType::kNonTerminalNode:
      default:
        assert(false);
        break;
    }
  }
  return true;
}

void SyntaxMachine::TerminalWordWaitingProcess() {
  ProductionNodeId production_node_to_shift_id(
      GetWaitingProcessWordInfo().word_attached_data_.production_node_id);
  const ActionAndAttachedData& action_and_attached_data = *GetActionAndTarget(
      GetParsingDataNow().parsing_table_entry_id, production_node_to_shift_id);
  switch (action_and_attached_data.action_type_) {
    // TODO ��ӽ���ʱ�ĺ�������
    [[unlikely]] case ActionType::kAccept : break;
    // TODO ��Ӵ�������
    [[unlikely]] case ActionType::kError : break;
    case ActionType::kReduct:
      Reduct(action_and_attached_data);
      break;
    case ActionType::kShift:
      ShiftTerminalWord(action_and_attached_data);
      break;

#ifdef USE_AMBIGUOUS_GRAMMAR
    case ActionType::kShiftReduct: {
      // ������ڴ����뵥��Ϊ�����ʱ��Ч
      // ��������ȼ����벻Ϊ0
      auto& terminal_node_info =
          GetWaitingProcessWordInfo().word_attached_data_;
      OperatorPriority& priority_now = GetOperatorPriorityNow();
      assert(terminal_node_info.node_type ==
                 ProductionNodeType::kOperatorNode &&
             terminal_node_info.operator_priority != 0);
      if (priority_now > terminal_node_info.operator_priority) {
        // ��ǰ���ȼ����ڴ����������������ȼ���ִ�й�Լ����
        Reduct(action_and_attached_data);
      } else if (priority_now == terminal_node_info.operator_priority) {
        // ��ǰ���ȼ����ڴ����������������ȼ�����Ҫ�ж������
        if (terminal_node_info.associate_type ==
            AssociatityType::kLeftToRight) {
          // �����Ϊ�����ҽ�ϣ�ִ�й�Լ����
          Reduct(action_and_attached_data);
        } else {
          // �����Ϊ���ҵ����ϣ�ִ���������
          ShiftTerminalWord(action_and_attached_data);
        }
      } else {
        // ��ǰ���ȼ����ڴ����������������ȼ���ִ���������
        ShiftTerminalWord(action_and_attached_data);
      }
    } break;
#endif  // USE_AMBIGUOUS_GRAMMAR

    default:
      assert(false);
      break;
  }
}

inline void SyntaxMachine::ShiftTerminalWord(
    const ActionAndAttachedData& action_and_target) {
  ParsingData& parsing_data_now = GetParsingDataNow();
  const WordInfo& word_info = GetWaitingProcessWordInfo();
  // ������ǰ���ʵ�����
  // ������ڵ��ID
  parsing_data_now.shift_node_id =
      ProductionNodeId(word_info.word_attached_data_.production_node_id);
  // ������ڵ������
  TerminalWordData terminal_word_data;
  // ���ô�����ڵ���ַ�������������
  terminal_word_data.word = word_info.symbol_;
  terminal_word_data.line = word_info.line_;
  // ��Ӵ�����ڵ���Ϣ����ǰ��������Ϣ
  parsing_data_now.word_data_to_user.SetWordDataToUser(
      std::move(terminal_word_data));
  // ����ǰ��״̬ѹ��ջ
  GetParsingStack().emplace(std::move(parsing_data_now));
  // ����״̬Ϊ����ýڵ�󵽴����Ŀ
  parsing_data_now.parsing_table_entry_id =
      action_and_target.GetShiftAttachedData().next_entry_id_;

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ����������������������ȼ�Ϊ�µ����ȼ�
  if (word_info.word_attached_data_.node_type ==
      ProductionNodeType::kOperatorNode) {
    parsing_data_now.operator_priority =
        OperatorPriority(word_info.word_attached_data_.operator_priority);
  } else {
    // ����ʹ��ԭ�������ȼ�
    parsing_data_now.operator_priority =
        GetParsingStack().top().operator_priority;
  }
#endif  // USE_AMBIGUOUS_GRAMMAR
  // ��ȡ��һ������
  GetNextWord();
}

void SyntaxMachine::Reduct(const ActionAndAttachedData& action_and_target) {
  const ReductAttachedData& reduct_attached_data =
      action_and_target.GetReductAttachedData();
  ProcessFunctionInterface& process_function_object =
      GetProcessFunctionClass(reduct_attached_data.process_function_class_id_);
  const auto& production_body = reduct_attached_data.production_body_;
  // ���ݸ��û����庯��������
  std::vector<WordDataToUser> word_data_to_user;
  // Ԥ����ռ䣬��ֹ�������
  word_data_to_user.resize(production_body.size());
  auto& parsing_stack = GetParsingStack();
  // ���ɹ�Լ���ݣ�����������˳��Ϊ����ʽ��д˳��
  // �Ӻ���ǰ�������
  auto production_node_id_iter = production_body.rbegin();
  for (auto user_data_iter = word_data_to_user.rbegin();
       user_data_iter != word_data_to_user.rend();
       ++user_data_iter, ++production_node_id_iter) {
    if (parsing_stack.top().shift_node_id == *production_node_id_iter) {
      // �ýڵ����������Լ
      *user_data_iter = std::move(GetParsingStack().top().word_data_to_user);
      GetParsingStack().pop();
    } else {
      // �ýڵ�չ�Լ�����ÿչ�Լ�ڵ���
      // �չ�Լ�ڵ�洢NonTerminalWordData
      // ����NonTerminalWordData::user_data_Ϊ��
      user_data_iter->SetWordDataToUser(NonTerminalWordData());
    }
  }
  // �ָ����������ݵ��÷��ս����ʽ����ǰ
  GetParsingDataNow() = std::move(GetParsingStack().top());
  GetParsingStack().pop();
  ShiftNonTerminalWord(
      process_function_object.Reduct(std::move(word_data_to_user)),
      action_and_target.GetReductAttachedData().reducted_nonterminal_node_id_);
}

void SyntaxMachine::ShiftNonTerminalWord(
    NonTerminalWordData&& non_terminal_word_data,
    ProductionNodeId reducted_nonterminal_node_id) {
  ParsingData& parsing_data_now = GetParsingDataNow();
  // ��ȡ������ս�ڵ��ת�Ƶ����﷨��������Ŀ
  ParsingTableEntryId next_entry_id = GetNextEntryId(
      parsing_data_now.parsing_table_entry_id, reducted_nonterminal_node_id);
  parsing_data_now.shift_node_id = reducted_nonterminal_node_id;
  parsing_data_now.word_data_to_user.SetWordDataToUser(
      std::move(non_terminal_word_data));
  GetParsingStack().emplace(std::move(parsing_data_now));
  parsing_data_now.parsing_table_entry_id = next_entry_id;

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ���赱ǰ���ȼ�Ϊ����ýڵ�ʱ�����ȼ�
  parsing_data_now.operator_priority =
      GetParsingStack().top().operator_priority;
#endif  // USE_AMBIGUOUS_GRAMMAR
}

}  // namespace frontend::parser::syntaxmachine