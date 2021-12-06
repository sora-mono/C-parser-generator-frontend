#include "c_parser_frontend.h"

#include <format>
/// ��ȡ������Ϣ
/// ��������ڸ������Ƶĺ����򷵻ؿ�ָ��
namespace c_parser_frontend {

thread_local CParserFrontend c_parser_frontend;

/// ��������ʹ�øýӿ�

std::pair<CParserFrontend::TypeSystem::TypeNodeContainerIter,
          CParserFrontend::AddTypeResult>
CParserFrontend::AnnounceFunction(
    const std::shared_ptr<const FunctionType>& function_type) {
  FlowControlSystem::FunctionCheckResult check_result =
      flow_control_system_.AnnounceFunction(function_type);
  switch (check_result) {
    case FlowControlSystem::FunctionCheckResult::kSuccess:
      break;
    case FlowControlSystem::FunctionCheckResult::kOverrideFunction:
      return std::make_pair(TypeSystem::TypeNodeContainerIter(),
                            AddTypeResult::kOverrideFunction);
      break;
    case FlowControlSystem::FunctionCheckResult::kDoubleAnnounce:
      return std::make_pair(TypeSystem::TypeNodeContainerIter(),
                            AddTypeResult::kDoubleAnnounceFunction);
      break;
    case FlowControlSystem::FunctionCheckResult::kFunctionConstructed:
      return std::make_pair(TypeSystem::TypeNodeContainerIter(),
                            AddTypeResult::kTypeAlreadyIn);
      break;
    default:
      assert(false);
      break;
  }
  return type_system_.AnnounceFunctionType(function_type);
}

/// ���õ�ǰ����������
/// ��������������������ȫ�������ͺ������͵Ķ������Զ�����������ȼ�
/// �����Ƿ����óɹ�

bool CParserFrontend::SetFunctionToConstruct(
    const std::shared_ptr<const FunctionType>& function_to_construct) {
  auto result =
      flow_control_system_.SetFunctionToConstruct(function_to_construct);
  if (result == FlowControlSystem::FunctionCheckResult::kSuccess) [[likely]] {
    return action_scope_system_.SetFunctionToConstruct(
        flow_control_system_.GetActiveFunctionPointer());
  } else {
    return false;
  }
}

bool CParserFrontend::PushFlowControlSentence(
    std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
        flow_control_sentence) {
  bool result = action_scope_system_.PushFlowControlSentence(
      std::move(flow_control_sentence));
  return result;
}

}  /// namespace c_parser_frontend

namespace c_parser_frontend::action_scope_system {
void ActionScopeSystem::PopOverLevel(ActionScopeLevelType level) {
  auto& variety_stack = GetVarietyStack();
  /// ��Ϊ�����ڱ������������ж�ջ�Ƿ�Ϊ��
  /// ��������
  while (variety_stack.top()->second.GetTopData().first.second > level) {
    bool should_erase_this_node = variety_stack.top()->second.PopTopData();
    if (should_erase_this_node) [[likely]] {
      /// �ñ������������κνڵ��
      /// ɾ�������ݽڵ�
      GetVarietyOrFunctionNameToOperatorNodePointer().erase(variety_stack.top());
    }
    variety_stack.pop();
  }
  while (!flow_control_stack_.empty() &&
         flow_control_stack_.top().second > level) {
    /// �������̿������
    auto flow_control_sentence = std::move(flow_control_stack_.top().first);
    flow_control_stack_.pop();
    if (!flow_control_stack_.empty()) [[likely]] {
      /// ���ջ��������ӵ�ջ�������̿��������
      AddSentence(std::move(flow_control_sentence));
    } else {
      /// ���ջ����˵����ǰ�����ĺ��������
      /// ����������ɣ�����������ɵĺ������Ͷ�����ӵ�ȫ�ֳ�Ա��������
      assert(flow_control_sentence->GetFlowType() ==
             c_parser_frontend::flow_control::FlowType::kFunctionDefine);
      /// �ͷ�ָ�������ݵ�ָ�룬��ֹ��ָ����flow_control_sentence����ʱ���ͷ�
      /// ��ָ���ϽȨ��FlowControlSystem
      flow_control_sentence.release();
      /// ֪ͨ��������ɺ�������
      c_parser_frontend::c_parser_frontend.FinishFunctionConstruct();
    }
  }
}
}  /// namespace c_parser_frontend::action_scope_system