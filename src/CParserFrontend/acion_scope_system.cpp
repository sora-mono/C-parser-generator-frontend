#include <format>

#include "action_scope_system.h"
namespace c_parser_frontend::action_scope_system {
// ���һ�����ݣ�����ת����ջ���Զ�ִ��
// �����ȴ����սڵ����øú�����������������
// ����ֵ����������Ͷ��崦

inline DefineVarietyResult ActionScopeSystem::VarietyData::AddVarietyOrInitData(
    const std::shared_ptr<const OperatorNodeInterface>& operator_node,
    ActionScopeLevel action_scope_level) {
  assert(operator_node->GetGeneralOperatorType() ==
             operator_node::GeneralOperationType::kVariety ||
         operator_node->GetGeneralOperatorType() ==
             operator_node::GeneralOperationType::kInitValue);
  auto& variety_data = GetVarietyData();
  std::monostate* empty_status_pointer =
      std::get_if<std::monostate>(&variety_data);
  if (empty_status_pointer != nullptr) [[likely]] {
    // δ�洢�κζ�������variety_data_�д洢����ָ��
    variety_data.emplace<SinglePointerType>(operator_node, action_scope_level);
    return DefineVarietyResult::kNew;
  } else {
    auto* single_object = std::get_if<SinglePointerType>(&variety_data);
    if (single_object != nullptr) [[likely]] {
      // ����Ƿ�����ض���
      if (single_object->second == action_scope_level) [[unlikely]] {
        // ��������ȼ��Ѿ�����ͬ������
        return DefineVarietyResult::kReDefine;
      }
      // ԭ���洢����shared_ptr������һ��ָ��ת��Ϊָ��ջ
      // ��ԭ���Ĵ洢��ʽ��Ϊջ�洢
      auto stack_pointer = std::make_unique<PointerStackType>();
      // ���ԭ�е�ָ��
      stack_pointer->emplace(std::move(*single_object));
      // ���������ָ��
      stack_pointer->emplace(operator_node, action_scope_level);
      // ��������variety_data������
      variety_data = std::move(stack_pointer);
      return DefineVarietyResult::kShiftToStack;
    } else {
      auto& stack_pointer =
          *std::get_if<std::unique_ptr<PointerStackType>>(&variety_data);
      // ����Ƿ�����ض���
      if (stack_pointer->top().second == action_scope_level) [[unlikely]] {
        // ��������ȼ��Ѿ�����ͬ������
        return DefineVarietyResult::kReDefine;
      }
      // �Ѿ�����ָ��ջ����ָ��ջ����Ӹ���ָ��
      stack_pointer->emplace(operator_node, action_scope_level);
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

ActionScopeSystem::VarietyData::SinglePointerType
ActionScopeSystem::VarietyData::GetTopData() const {
  auto& variety_data = GetVarietyData();
  assert(std::get_if<std::monostate>(&variety_data) == nullptr);
  auto* single_pointer = std::get_if<SinglePointerType>(&variety_data);
  if (single_pointer != nullptr) [[likely]] {
    return *single_pointer;
  } else {
    return (*std::get_if<std::unique_ptr<PointerStackType>>(&variety_data))
        ->top();
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

inline std::pair<ActionScopeSystem::ActionScopeContainerType::const_iterator,
                 DefineVarietyResult>
ActionScopeSystem::DefineVariety(
    const std::shared_ptr<const VarietyOperatorNode>& variety_node_pointer) {
  return DefineVarietyOrInitValue(variety_node_pointer->GetVarietyName(),
                                  variety_node_pointer);
}

std::pair<ActionScopeSystem::ActionScopeContainerType::const_iterator,
          DefineVarietyResult>
ActionScopeSystem::DefineVarietyOrInitValue(
    const std::string& name,
    const std::shared_ptr<const OperatorNodeInterface>& operator_node_pointer) {
  assert(
      operator_node_pointer->GetGeneralOperatorType() ==
          c_parser_frontend::operator_node::GeneralOperationType::kInitValue ||
      operator_node_pointer->GetGeneralOperatorType() ==
              c_parser_frontend::operator_node::GeneralOperationType::
                  kVariety &&
          name ==
              static_cast<const VarietyOperatorNode&>(*operator_node_pointer)
                  .GetVarietyName());
  auto [iter, inserted] =
      GetVarietyOrFunctionNameToOperatorNodePointer().emplace(name,
                                                              VarietyData());
  DefineVarietyResult add_variety_result = iter->second.AddVarietyOrInitData(
      operator_node_pointer, GetActionScopeLevel());
  switch (add_variety_result) {
    case DefineVarietyResult::kNew:
    case DefineVarietyResult::kAddToStack:
    case DefineVarietyResult::kShiftToStack:
      // ȫ�ֱ������ᱻ������������ջ
      if (GetActionScopeLevel() != 0) [[likely]] {
        // ��Ӹýڵ����Ϣ���Ա�������ʧЧʱ��ȷ����
        // ���Ա����û��ṩ��Ҫ���������У��򻯲���
        // ͬʱ��������������������ڵ�ӳ���Ҳ����ÿ��ָ�붼�洢��Ӧ��level
        GetVarietyStack().emplace(iter);
      }
      break;
    case DefineVarietyResult::kReDefine:
      break;
    default:
      assert(false);
      break;
  }
  return std::make_pair(std::move(iter), add_variety_result);
}

std::pair<std::shared_ptr<const operator_node::OperatorNodeInterface>, bool>
ActionScopeSystem::GetVarietyOrFunction(const std::string& target_name) const {
  auto iter = GetVarietyOrFunctionNameToOperatorNodePointer().find(target_name);
  if (iter != GetVarietyOrFunctionNameToOperatorNodePointer().end())
      [[likely]] {
    auto pointer_data = iter->second.GetTopData();
    return std::make_pair(pointer_data.first, true);
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
  auto [ignore_iter, define_variety_result] = DefineVarietyOrInitValue(
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
    const std::string& argument_name =
        argument_node.variety_operator_node->GetVarietyName();
    // ���������������ӵ���������
    if (!argument_name.empty()) [[likely]] {
      DefineVariety(argument_node.variety_operator_node);
    }
  }
  return true;
}

bool ActionScopeSystem::PushFlowControlSentence(
    std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
        flow_control_sentence) {
  // ���̿���ջ�����ҽ���һ���������̿��ƽڵ㣬λ�ڵײ���������ȼ�Ϊ1
  if (flow_control_sentence->GetFlowType() !=
      flow_control::FlowType::kFunctionDefine) [[likely]] {
    if (flow_control_stack_.empty()) {
      return false;
    }
  } else if (!flow_control_stack_.empty()) [[unlikely]] {
    return false;
  }
  // ������������ȼ������������̿�������������ȼ�
  // �����ڵ���������ȼ�ʱ����һ���������̿������
  AddActionScopeLevel();
  flow_control_stack_.emplace(
      std::make_pair(std::move(flow_control_sentence), GetActionScopeLevel()));
  return true;
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