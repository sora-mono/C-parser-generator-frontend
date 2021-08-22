#include "action_scope_system.h"
namespace c_parser_frontend::action_scope_system {
// ���һ�����ݣ�����ת����ջ���Զ�ִ��
// �����ȴ����սڵ����øú�����������������
// ����ֵ����������Ͷ��崦

inline VarietyScopeSystem::VarietyData::AddVarietyResult
VarietyScopeSystem::VarietyData::AddVarietyData(
    std::shared_ptr<VarietyOperatorNode>&& operator_node,
    ActionScopeLevelType action_scope_level) {
  auto& variety_data = GetVarietyData();
  std::monostate* empty_status_pointer =
      std::get_if<std::monostate>(&variety_data);
  if (empty_status_pointer != nullptr) [[likely]] {
    // δ�洢�κζ�������variety_data_�д洢����ָ��
    variety_data.emplace<SinglePointerType>(std::move(operator_node),
                                            action_scope_level);
    return AddVarietyResult::kNew;
  } else {
    auto* single_object = std::get_if<SinglePointerType>(&variety_data);
    if (single_object != nullptr) [[likely]] {
      // ����Ƿ�����ض���
      if (single_object->second == action_scope_level) [[unlikely]] {
        // ��������ȼ��Ѿ�����ͬ������
        return AddVarietyResult::kReDefine;
      }
      // ԭ���洢����shared_ptr������һ��ָ��ת��Ϊָ��ջ
      // ��ԭ���Ĵ洢��ʽ��Ϊջ�洢
      auto stack_pointer = std::make_unique<PointerStackType>();
      // ���ԭ�е�ָ��
      stack_pointer->emplace(std::move(*single_object));
      // ���������ָ��
      stack_pointer->emplace(std::move(operator_node), action_scope_level);
      // ��������variety_data������
      variety_data = std::move(stack_pointer);
      return AddVarietyResult::kShiftToStack;
    } else {
      // ����Ƿ�����ض���
      if (single_object->second == action_scope_level) [[unlikely]] {
        // ��������ȼ��Ѿ�����ͬ������
        return AddVarietyResult::kReDefine;
      }
      auto& stack_pointer =
          *std::get_if<std::unique_ptr<PointerStackType>>(&variety_data);
      // �Ѿ�����ָ��ջ����ָ��ջ����Ӹ���ָ��
      stack_pointer->emplace(std::move(operator_node), action_scope_level);
      return AddVarietyResult::kAddToStack;
    }
  }
}
bool VarietyScopeSystem::VarietyData::PopTopData() {
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

VarietyScopeSystem::VarietyData::SinglePointerType&
VarietyScopeSystem::VarietyData::GetTopData() {
  auto& variety_data = GetVarietyData();
  auto* single_pointer = std::get_if<SinglePointerType>(&variety_data);
  if (single_pointer != nullptr) [[likely]] {
    return *single_pointer;
  } else {
    return (*std::get_if<std::unique_ptr<PointerStackType>>(&variety_data))
        ->top();
  }
}

std::pair<std::shared_ptr<VarietyScopeSystem::VarietyOperatorNode>, bool>
VarietyScopeSystem::GetVariety(const std::string& variety_name) {
  auto iter = GetVarietyNameToOperatorNodePointer().find(variety_name);
  if (iter != GetVarietyNameToOperatorNodePointer().end()) [[likely]] {
    return std::make_pair(iter->second.GetTopData().first, true);
  } else {
    return std::make_pair(std::shared_ptr<VarietyOperatorNode>(), false);
  }
}

void VarietyScopeSystem::PopVarietyOverLevel(ActionScopeLevelType level) {
  auto& variety_stack = GetVarietyStack();
  // ��Ϊ�����ڱ������������ж�ջ�Ƿ�Ϊ��
  while (variety_stack.top()->second.GetTopData().second > level) {
    bool should_erase_this_node = variety_stack.top()->second.PopTopData();
    if (should_erase_this_node) [[likely]] {
      // �ñ������������κνڵ��
      // ɾ�������ݽڵ�
      GetVarietyNameToOperatorNodePointer().erase(variety_stack.top());
    }
    variety_stack.pop();
  }
}

}  // namespace c_parser_frontend::action_scope_system