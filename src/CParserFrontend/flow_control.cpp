#include "flow_control.h"

namespace c_parser_frontend::flow_control {
// ���һ��������ִ�е���䣨������˳����ӣ�
// �ɹ���ӷ���true������������Ȩ
// �����������򷵻�false�����Ȩ

inline std::pair<std::unique_ptr<FlowInterface>, bool>
FunctionDefine::AddSentence(std::unique_ptr<FlowInterface>&& sentence_to_add) {
  if (CheckSentenceInFunctionValid(*sentence_to_add)) [[likely]] {
    sentences_in_function_.emplace_back(std::move(sentence_to_add));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    // ������ӣ����ؿ���Ȩ
    return std::make_pair(std::move(sentence_to_add), false);
  }
}

inline bool FunctionDefine::CheckSentenceInFunctionValid(
    const FlowInterface& flow_interface) {
  switch (flow_interface.GetFlowType()) {
    case FlowType::kFunctionDefine:
      // C���Բ�����Ƕ�׶��庯��
      return false;
      break;
    default:
      return true;
      break;
  }
}
inline bool SimpleSentence::SetSentenceOperateNode(
    std::shared_ptr<const OperatorNodeInterface>&& sentence_operate_node) {
  if (CheckOperatorNodeValid(*sentence_operate_node)) [[likely]] {
    sentence_operate_node_ = std::move(sentence_operate_node);
    return true;
  } else {
    return false;
  }
}
inline bool SimpleSentence::CheckOperatorNodeValid(
    const OperatorNodeInterface& operator_node) {
  switch (operator_node.GetGeneralOperatorType()) {
    case GeneralOperationType::kAllocate:
    case GeneralOperationType::kAssign:
    case GeneralOperationType::kDeReference:
    case GeneralOperationType::kFunctionCall:
    case GeneralOperationType::kLogicalOperation:
    case GeneralOperationType::kMathematicalOperation:
    case GeneralOperationType::kMemberAccess:
    case GeneralOperationType::kObtainAddress:
      // ����Ϊ���Գ�Ϊ�������Ľڵ�
      return true;
      break;
    default:
      return false;
      break;
  }
}
inline bool IfSentenceInterface::SetIfCondition(
    std::shared_ptr<const OperatorNodeInterface>&& if_condition) {
  if (CheckIfConditionValid(*if_condition)) [[likely]] {
    SetCondition(std::move(if_condition));
    return true;
  } else {
    return false;
  }
}

// ���if�����ִ�е�����
// �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
// ��ӳɹ��򲻷������̽ڵ����Ȩ

inline std::pair<std::unique_ptr<FlowInterface>, bool>
IfSentenceInterface::AddIfBodySentence(
    std::unique_ptr<FlowInterface>&& if_body_sentence) {
  if (CheckIfBodySentenceValid(*if_body_sentence)) [[likely]] {
    AddMainBlockSentence(std::move(if_body_sentence));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    return std::make_pair(std::move(if_body_sentence), false);
  }
}
// ����������Ƿ������Ϊif�ڲ������

// ���else�����ִ�е�����
// �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
// ��ӳɹ��򲻷������̽ڵ����Ȩ

inline std::pair<std::unique_ptr<FlowInterface>, bool>
IfElseSentence::AddElseBodySentence(
    std::unique_ptr<FlowInterface>&& else_body_sentence) {
  if (CheckElseBodySentenceValid(*else_body_sentence)) [[likely]] {
    else_body_.emplace_back(std::move(else_body_sentence));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    return std::make_pair(std::move(else_body_sentence), false);
  }
}
inline bool WhileSentence::SetWhileCondition(
    std::shared_ptr<const OperatorNodeInterface>&& while_condition) {
  if (CheckWhileConditionValid(*while_condition)) [[unlikely]] {
    SetCondition(std::move(while_condition));
    return true;
  } else {
    return false;
  }
}

// ���while�����ִ�е�����
// �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
// ��ӳɹ��򲻷������̽ڵ����Ȩ

inline std::pair<std::unique_ptr<FlowInterface>, bool>
WhileSentence::AddWhileBodySentence(
    std::unique_ptr<FlowInterface>&& while_body_sentence) {
  if (CheckWhileBodySentenceValid(*while_body_sentence)) [[likely]] {
    AddMainBlockSentence(std::move(while_body_sentence));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    return std::make_pair(std::move(while_body_sentence), false);
  }
}

// ���for�����ִ�е�����
// �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
// ��ӳɹ��򲻷������̽ڵ����Ȩ

// �����Ƿ��������if�ж�������������������򷵻�false�Ҳ�����

inline bool ForSentence::SetForCondition(
    std::shared_ptr<const OperatorNodeInterface>&& for_condition) {
  if (CheckForConditionValid(*for_condition)) [[likely]] {
    SetCondition(std::move(for_condition));
    return true;
  } else {
    return false;
  }
}

inline std::pair<std::unique_ptr<FlowInterface>, bool>
ForSentence::AddForBodySentence(
    std::unique_ptr<FlowInterface>&& for_body_sentence) {
  if (CheckForBodySentenceValid(*for_body_sentence)) [[likely]] {
    AddMainBlockSentence(std::move(for_body_sentence));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    return std::make_pair(std::move(for_body_sentence), false);
  }
}
inline std::pair<std::unique_ptr<FlowInterface>, bool>
ForSentence::AddForInitSentence(
    std::unique_ptr<FlowInterface>&& init_body_sentence) {
  if (CheckForBodySentenceValid(*init_body_sentence)) [[likely]] {
    AddMainBlockSentence(std::move(init_body_sentence));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    return std::make_pair(std::move(init_body_sentence), false);
  }
}
inline std::pair<std::unique_ptr<FlowInterface>, bool>
ForSentence::AddForAfterBodySentence(
    std::unique_ptr<FlowInterface>&& after_body_sentence) {
  if (CheckForBodySentenceValid(*after_body_sentence)) [[likely]] {
    AddMainBlockSentence(std::move(after_body_sentence));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    return std::make_pair(std::move(after_body_sentence), false);
  }
}

// �����ͨ��case�������Ƿ���ӳɹ���������ʧ���򷵻�case_label�Ŀ���Ȩ

// Ĭ�϶�ѭ�������ڵ�������ͼ��ĺ���
// ���ظ����ڵ��Ƿ������Ϊ������֧����������

inline bool ConditionBlockInterface::DefaultConditionCheck(
    const OperatorNodeInterface& condition_node) {
  switch (condition_node.GetResultTypePointer()->GetType()) {
    case StructOrBasicType::kBasic:
    case StructOrBasicType::kPointer:
      return true;
    default:
      return false;
      break;
  }
}

// Ĭ�ϼ�����������ĺ���
// ���ظ������̿��ƽڵ��Ƿ������Ϊ������֧������������

inline bool ConditionBlockInterface::DefaultMainBlockSentenceCheck(
    const FlowInterface& flow_interface) {
  switch (flow_interface.GetFlowType()) {
    case FlowType::kJmp:
    case FlowType::kLabel:
    case FlowType::kReturn:
    case FlowType::kWhileSentence:
    case FlowType::kDoWhileSentence:
    case FlowType::kForSentence:
    case FlowType::kIfSentence:
    case FlowType::kSimpleSentence:
    case FlowType::kSwitchSentence:
      return true;
      break;
    default:
      // ֻ�к������岻������if����г���
      return false;
      break;
  }
}
bool CheckSwitchCaseAbleToAdd(
    const SwitchSentence& switch_node,
    const BasicTypeInitializeOperatorNode& case_value) {
  if (case_value.GetInitializeType() != InitializeType::kBasic) [[unlikely]] {
    // case����������Ϊ������
    return false;
  }
  // �жϸ���case�����Ƿ��Ѿ�����
  auto iter = switch_node.GetSimpleCases().find(case_value.GetValue());
  return iter == switch_node.GetSimpleCases().end();
}

// �����ͨ��case�������Ƿ���ӳɹ���������ʧ���򷵻�case_label�Ŀ���Ȩ

inline std::pair<std::unique_ptr<Label>, bool> SwitchSentence::AddSimpleCase(
    std::shared_ptr<BasicTypeInitializeOperatorNode>&& case_value,
    std::unique_ptr<Label>&& case_label) {
  if (CheckSwitchCaseAbleToAdd(*this, *case_value)) [[likely]] {
    // �������
    simple_cases_.emplace(case_value->GetValue(),
                          std::make_pair(case_value, std::move(case_label)));
    return std::make_pair(std::unique_ptr<Label>(), true);
  } else {
    return std::make_pair(std::move(case_label), false);
  }
}

// ���switch������岿��
// �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
// ��ӳɹ��򲻷������̽ڵ����Ȩ

inline std::pair<std::unique_ptr<FlowInterface>, bool>
SwitchSentence::AddSwitchBodySentence(
    std::unique_ptr<FlowInterface>&& switch_body_sentence) {
  if (CheckSwitchBodySentenceValid(*switch_body_sentence)) [[likely]] {
    AddMainBlockSentence(std::move(switch_body_sentence));
    return std::make_pair(std::unique_ptr<FlowInterface>(), true);
  } else {
    return std::make_pair(std::move(switch_body_sentence), false);
  }
}

inline bool SwitchSentence::SetSwitchCondition(
    std::shared_ptr<const OperatorNodeInterface>&& switch_condition) {
  if (CheckSwitchConditionValid(*switch_condition)) [[likely]] {
    SetCondition(std::move(switch_condition));
    return true;
  } else {
    return false;
  }
}
}  // namespace c_parser_frontend::flow_control
