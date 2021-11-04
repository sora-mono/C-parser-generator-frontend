#include <format>

#include "Parser/DfaMachine/dfa_machine.h"
#include "action_scope_system.h"
namespace c_parser_frontend::action_scope_system {
// ���һ�����ݣ�����ת����ջ���Զ�ִ��
// �����ȴ����սڵ����øú�����������������
// ����ֵ����������Ͷ��崦

inline DefineVarietyResult
ActionScopeSystem::VarietyData::AddVarietyOrFunctionData(
    const std::shared_ptr<const OperatorNodeInterface>& operator_node,
    ActionScopeLevelType action_scope_level_) {
  assert(operator_node->GetGeneralOperatorType() ==
             operator_node::GeneralOperationType::kVariety ||
         operator_node->GetGeneralOperatorType() ==
             operator_node::GeneralOperationType::kInitValue);
  auto& variety_data = GetVarietyData();
  std::monostate* empty_status_pointer =
      std::get_if<std::monostate>(&variety_data);
  if (empty_status_pointer != nullptr) [[likely]] {
    // δ�洢�κζ�������variety_data_�д洢����ָ��
    variety_data.emplace<SinglePointerType>(operator_node, action_scope_level_);
    return DefineVarietyResult::kNew;
  } else {
    auto* single_object = std::get_if<SinglePointerType>(&variety_data);
    if (single_object != nullptr) [[likely]] {
      // ����Ƿ�����ض���
      if (single_object->second == action_scope_level_) [[unlikely]] {
        // ��������ȼ��Ѿ�����ͬ������
        return DefineVarietyResult::kReDefine;
      }
      // ԭ���洢����shared_ptr������һ��ָ��ת��Ϊָ��ջ
      // ��ԭ���Ĵ洢��ʽ��Ϊջ�洢
      auto stack_pointer = std::make_unique<PointerStackType>();
      // ���ԭ�е�ָ��
      stack_pointer->emplace(std::move(*single_object));
      // ���������ָ��
      stack_pointer->emplace(operator_node, action_scope_level_);
      // ��������variety_data������
      variety_data = std::move(stack_pointer);
      return DefineVarietyResult::kShiftToStack;
    } else {
      auto& stack_pointer =
          *std::get_if<std::unique_ptr<PointerStackType>>(&variety_data);
      // ����Ƿ�����ض���
      if (stack_pointer->top().second == action_scope_level_) [[unlikely]] {
        // ��������ȼ��Ѿ�����ͬ������
        return DefineVarietyResult::kReDefine;
      }
      // �Ѿ�����ָ��ջ����ָ��ջ����Ӹ���ָ��
      stack_pointer->emplace(operator_node, action_scope_level_);
      return DefineVarietyResult::kAddToStack;
    }
  }
}
bool ActionScopeSystem::VarietyData::PopTopData() {
  auto& variety_data = GetVarietyData();
  auto* single_pointer = std::get_if<SinglePointerType>(&variety_data);
  if (single_pointer != nullptr) [[likely]] {
    // ����true�������������Ѿ�ɾ����Ӧ�Ƴ��ýڵ�
    return true;
  } else {
    auto& stack_pointer =
        *std::get_if<std::unique_ptr<PointerStackType>>(&variety_data);
    stack_pointer->pop();
    if (stack_pointer->size() == 1) {
      // ջ��ֻʣһ��ָ�룬�˻���ֻ�洢��ָ��Ľṹ
      // ���¸�ֵvariety_data_����ջ�Զ�����
      // �˴�����ʹ���ƶ����壬��ֹ��ֵǰջ�Ѿ�����
      variety_data = SinglePointerType(stack_pointer->top());
    }
    return false;
  }
}

std::pair<ActionScopeSystem::VarietyData::SinglePointerType, bool>
ActionScopeSystem::VarietyData::GetTopData() {
  auto& variety_data = GetVarietyData();
  if (std::get_if<std::monostate>(&variety_data) != nullptr) [[unlikely]] {
    // �ýڵ�δ�洢�κα���ָ��
    return std::make_pair(SinglePointerType(), false);
  }
  auto* single_pointer = std::get_if<SinglePointerType>(&variety_data);
  if (single_pointer != nullptr) [[likely]] {
    return std::make_pair(*single_pointer, true);
  } else {
    return std::make_pair(
        (*std::get_if<std::unique_ptr<PointerStackType>>(&variety_data))->top(),
        true);
  }
}

ActionScopeSystem::~ActionScopeSystem() {
  if (!flow_control_stack_.empty()) {
    // ��Ҫ��ȷ����ջ�еĺ�������ָ�룬��ֹ�ͷ��޹�ϽȨ��ָ��
    while (flow_control_stack_.size() > 1) {
      flow_control_stack_.pop();
    }
    // ��ǰ���̿������ջ��ʣ���ڹ����ĺ�������ָ��
    // release��ָ���Է�ֹ���ͷ�
    flow_control_stack_.top().first.release();
  }
}

std::pair<std::shared_ptr<const operator_node::OperatorNodeInterface>, bool>
ActionScopeSystem::GetVarietyOrFunction(const std::string& target_name) {
  auto iter = GetVarietyOrFunctionNameToOperatorNodePointer().find(target_name);
  if (iter != GetVarietyOrFunctionNameToOperatorNodePointer().end())
      [[likely]] {
    auto [pointer_data, has_pointer] = iter->second.GetTopData();
    return std::make_pair(pointer_data.first, has_pointer);
  } else {
    return std::make_pair(std::shared_ptr<VarietyOperatorNode>(), false);
  }
}

bool ActionScopeSystem::PushFunctionFlowControlNode(
    c_parser_frontend::flow_control::FunctionDefine* function_data) {
  assert(function_data);
  if (GetActionScopeLevel() != 0) [[unlikely]] {
    // ����ֻ�ܶ�����0����ȫ�֣�������
    return false;
  }
  // ����ȫ�ֳ�ʼ���������ڸ�����ָ�븳ֵʱʹ��
  auto [ignore_iter, define_variety_result] = DefineVarietyOrFunction(
      function_data->GetFunctionTypeReference().GetFunctionName(),
      std::make_shared<operator_node::BasicTypeInitializeOperatorNode>(
          function_data->GetFunctionTypePointer()));
  // �ڴ�֮ǰӦ���Ѿ��жϹ��Ƿ�����ض���/�������⣨����ӵ�FlowControlSystemʱ��
  assert(define_variety_result != DefineVarietyResult::kReDefine);
  // ����������ָ��ѹ��ջ����ָ���ϽȨ����ActionScopeSystem
  // ����FlowControlSystem�����Ե���ʱҪ����release��ֹ���ݱ�unique_ptr�ͷ�
  bool push_result = PushFlowControlSentence(
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>(
          function_data));
  // ���ѹ�벻�ɹ��ᵼ��unique_ptr�ں�����������ʱ���쳣
  assert(push_result);
  return true;
}

bool ActionScopeSystem::SetFunctionToConstruct(
    c_parser_frontend::flow_control::FunctionDefine* function_data) {
  assert(function_data != nullptr);
  // ���ȫ�ֵĺ������ͱ��������ں���ָ�븳ֵ
  // ����������flow_control�ڵ�ѹ��������ջ��
  bool result = PushFunctionFlowControlNode(function_data);
  if (!result) [[unlikely]] {
    return false;
  }
  // ������������ӵ���������
  for (auto& argument_node :
       function_data->GetFunctionTypeReference().GetArguments()) {
    const std::string* argument_name =
        argument_node.variety_operator_node->GetVarietyNamePointer();
    // ���������������ӵ���������
    if (argument_name != nullptr) [[likely]] {
      DefineVarietyOrFunction(*argument_name, std::shared_ptr<const VarietyOperatorNode>(
                                        argument_node.variety_operator_node));
    }
  }
  return true;
}

bool ActionScopeSystem::PushFlowControlSentence(
    std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
        flow_control_sentence) {
  if (flow_control_sentence->GetFlowType() !=
      flow_control::FlowType::kFunctionDefine) [[likely]] {
    if (flow_control_stack_.empty()) {
      return false;
    }
  } else if (!flow_control_stack_.empty()) [[unlikely]] {
    return false;
  }
  // ������������ȼ������������̿�������������ȼ�
  AddActionScopeLevel();
  flow_control_stack_.emplace(
      std::make_pair(std::move(flow_control_sentence), GetActionScopeLevel()));
  return true;
}

bool ActionScopeSystem::RemoveEmptyNode(
    const std::string& empty_node_to_remove_name) {
  auto iter = variety_or_function_name_to_operator_node_pointer_.find(
      empty_node_to_remove_name);
  // Ŀ��ڵ�գ�ɾ���ڵ�
  if (iter != variety_or_function_name_to_operator_node_pointer_.end() &&
      iter->second.Empty()) [[likely]] {
    variety_or_function_name_to_operator_node_pointer_.erase(iter);
    return true;
  }
  return false;
}

bool ActionScopeSystem::AddSwitchSimpleCase(
    const std::shared_ptr<
        c_parser_frontend::flow_control::BasicTypeInitializeOperatorNode>&
        case_value) {
  if (GetTopFlowControlSentence().GetFlowType() !=
      c_parser_frontend::flow_control::FlowType::kSwitchSentence) [[unlikely]] {
    return false;
  }
  return static_cast<c_parser_frontend::flow_control::SwitchSentence&>(
             GetTopFlowControlSentence())
      .AddSimpleCase(case_value);
}

bool ActionScopeSystem::AddSwitchDefaultCase() {
  if (GetTopFlowControlSentence().GetFlowType() !=
      c_parser_frontend::flow_control::FlowType::kSwitchSentence) [[unlikely]] {
    return false;
  }
  return static_cast<c_parser_frontend::flow_control::SwitchSentence&>(
             GetTopFlowControlSentence())
      .AddDefaultCase();
}
bool ActionScopeSystem::AddSentences(
    std::list<std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&&
        sentence_container) {
  if (flow_control_stack_.empty()) [[unlikely]] {
    return false;
  }
  return flow_control_stack_.top().first->AddMainSentences(
      std::move(sentence_container));
}

}  // namespace c_parser_frontend::action_scope_system