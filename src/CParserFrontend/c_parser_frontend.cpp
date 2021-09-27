#include "c_parser_frontend.h"

#include <format>
// ��ȡ������Ϣ
// ��������ڸ������Ƶĺ����򷵻ؿ�ָ��
namespace c_parser_frontend {

thread_local CParserFrontend parser_frontend;

// ��ȡ��ǰ����
size_t GetLine() { return frontend::parser::line_; }
// ��ȡ��ǰ����
size_t GetColumn() { return frontend::parser::column_; }

void CParserFrontend::PushFlowControlSentence(
    std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
        flow_control_sentence) {
  bool result = action_scope_system_.PushFlowControlSentence(
      std::move(flow_control_sentence));
  if (!result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} ���̿����������ں��������ڲ�ʹ��",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
}
}  // namespace c_parser_frontend

namespace c_parser_frontend::action_scope_system {
void ActionScopeSystem::PopVarietyOverLevel(ActionScopeLevelType level) {
  auto& variety_stack = GetVarietyStack();
  // ��Ϊ�����ڱ������������ж�ջ�Ƿ�Ϊ��
  // ��������
  while (variety_stack.top()->second.GetTopData().first.second > level) {
    bool should_erase_this_node = variety_stack.top()->second.PopTopData();
    if (should_erase_this_node) [[likely]] {
      // �ñ������������κνڵ��
      // ɾ�������ݽڵ�
      GetVarietyNameToOperatorNodePointer().erase(variety_stack.top());
    }
    variety_stack.pop();
  }
  // �������̿������
  if (flow_control_stack_.top().second > level) {
    auto flow_control_sentence = std::move(flow_control_stack_.top().first);
    flow_control_stack_.pop();
    if (!flow_control_stack_.empty()) [[likely]] {
      // ���ջ��������ӵ�ջ�������̿��������
      AddSentence(std::move(flow_control_sentence));
    } else {
      // ���ջ����˵����ǰ�����ĺ��������
      // ����������ɣ�����������ɵĺ������Ͷ�����ӵ�ȫ�ֳ�Ա��������
      assert(flow_control_sentence->GetFlowType() ==
             c_parser_frontend::flow_control::FlowType::kFunctionDefine);
      auto function_type =
          static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
              *flow_control_sentence)
              .GetFunctionTypePointer();
      bool result = CreateFunctionTypeVarietyAndPush(function_type);
      assert(result);
      // ֪ͨ��������ɺ�������
      c_parser_frontend::parser_frontend.FinishFunctionConstruct(function_type);
    }
  } 
}
}  // namespace c_parser_frontend::action_scope_system