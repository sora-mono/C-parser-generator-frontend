#include "flow_control.h"

#include "action_scope_system.h"
namespace c_parser_frontend::flow_control {
// ���һ��������ִ�е���䣨������˳����ӣ�
// �ɹ���ӷ���true������������Ȩ
// �����������򷵻�false�����Ȩ

// ����������Ƿ������Ϊ�����ڳ��ֵ����

bool SimpleSentence::SetSentenceOperateNode(
    const std::shared_ptr<OperatorNodeInterface>& sentence_operate_node) {
  if (CheckOperatorNodeValid(*sentence_operate_node)) [[likely]] {
    sentence_operate_node_ = sentence_operate_node;
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

// ����if��������ж�Ӧ����ӵ��ĸ�����
// �����FlowType::kIfSentence����ӵ�true_branch
// �����FlowType::kIfElseSentence����ӵ�false_branch

bool IfSentence::AddMainSentence(std::unique_ptr<FlowInterface>&& sentence) {
  if (GetFlowType() == FlowType::kIfSentence) [[likely]] {
    return AddTrueBranchSentence(std::move(sentence));
  } else {
    assert(GetFlowType() == FlowType::kIfElseSentence);
    return AddFalseBranchSentence(std::move(sentence));
  }
}

bool IfSentence::AddMainSentences(
    std::list<std::unique_ptr<FlowInterface>>&& sentences) {
  if (GetFlowType() == FlowType::kIfSentence) [[likely]] {
    return AddTrueBranchSentences(std::move(sentences));
  } else {
    assert(GetFlowType() == FlowType::kIfElseSentence);
    return AddFalseBranchSentences(std::move(sentences));
  }
}

void IfSentence::ConvertToIfElse() {
  assert(GetFlowType() == FlowType::kIfSentence);
  // �޸��������
  SetFlowType(FlowType::kIfElseSentence);
  // ����else�ڵ�����
  else_body_ = std::make_shared<std::list<std::unique_ptr<FlowInterface>>>();
}

inline bool IfSentence::AddFalseBranchSentence(
    std::unique_ptr<FlowInterface>&& else_body_sentence) {
  if (CheckElseBodySentenceValid(*else_body_sentence)) [[likely]] {
    else_body_->emplace_back(std::move(else_body_sentence));
    return true;
  } else {
    return false;
  }
}
bool IfSentence::AddFalseBranchSentences(
    std::list<std::unique_ptr<FlowInterface>>&& else_body_sentences) {
  for (const auto& sentence : else_body_sentences) {
    if (!CheckElseBodySentenceValid(*sentence)) [[unlikely]] {
      return false;
    }
  }
  else_body_->merge(std::move(else_body_sentences));
  return true;
}

inline bool ForSentence::AddForInitSentence(
    std::unique_ptr<FlowInterface>&& init_body_sentence) {
  if (CheckForBodySentenceValid(*init_body_sentence)) [[likely]] {
    AddSentence(std::move(init_body_sentence));
    return true;
  } else {
    return false;
  }
}
bool ForSentence::AddForInitSentences(
    std::list<std::unique_ptr<FlowInterface>>&& init_body_sentences) {
  for (const auto& sentence : init_body_sentences) {
    if (!CheckForBodySentenceValid(*sentence)) [[unlikely]] {
      return false;
    }
  }
  init_block_.merge(std::move(init_body_sentences));
  return true;
}
inline bool ForSentence::AddForRenewSentence(
    std::unique_ptr<FlowInterface>&& after_body_sentence) {
  if (CheckForBodySentenceValid(*after_body_sentence)) [[likely]] {
    AddSentence(std::move(after_body_sentence));
    return true;
  } else {
    return false;
  }
}

bool ForSentence::AddForRenewSentences(
    std::list<std::unique_ptr<FlowInterface>>&& after_body_sentences) {
  for (const auto& sentence : after_body_sentences) {
    if (!CheckForBodySentenceValid(*sentence)) [[unlikely]] {
      return false;
    }
  }
  after_body_sentences_.merge(std::move(after_body_sentences));
  return true;
}

// Ĭ�϶�ѭ�������ڵ�������ͼ��ĺ���
// ���ظ����ڵ��Ƿ������Ϊ������֧����������

// �������������䣬��������飬���������鲻ͨ�������

bool ConditionBlockInterface::SetCondition(
    const std::shared_ptr<const OperatorNodeInterface>& condition,
    std::list<std::unique_ptr<FlowInterface>>&& sentences_to_get_condition) {
  if (DefaultConditionCheck(*condition)) [[likely]] {
    condition_ = condition;
    sentence_to_get_condition_ = std::move(sentences_to_get_condition);
    return true;
  } else {
    return false;
  }
}

bool ConditionBlockInterface::AddSentence(
    std::unique_ptr<FlowInterface>&& sentence) {
  if (DefaultMainBlockSentenceCheck(*sentence)) {
    main_block_.emplace_back(std::move(sentence));
    return true;
  } else {
    return false;
  }
}

bool ConditionBlockInterface::AddSentences(
    std::list<std::unique_ptr<FlowInterface>>&& sentences) {
  for (const auto& sentence : sentences) {
    if (!DefaultMainBlockSentenceCheck(*sentence)) [[unlikely]] {
      return false;
    }
  }
  main_block_.merge(std::move(sentences));
  return true;
}

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

bool SwitchSentence::CheckSwitchCaseAbleToAdd(
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

// �����ͨ��case�������Ƿ���ӳɹ���������ʧ�����޸Ĳ���

bool SwitchSentence::AddSimpleCase(
    const std::shared_ptr<const BasicTypeInitializeOperatorNode>& case_value) {
  if (CheckSwitchCaseAbleToAdd(*this, *case_value)) [[likely]] {
    // �������
    auto [label_for_jmp, label_in_body] =
        ConvertCaseValueToLabel(*this, *case_value);
    auto [iter, inserted] = simple_cases_.emplace(
        case_value->GetValue(),
        std::make_pair(case_value, std::move(label_for_jmp)));
    if (inserted) [[likely]] {
      // ��ǩδ����������һ�ݱ�ǩ��ӵ�switch����
      AddSentence(std::move(label_in_body));
    }
    return inserted;
  } else {
    return false;
  }
}
bool SwitchSentence::AddDefaultCase() {
  if (default_case_ != nullptr) [[unlikely]] {
    // �Ѿ�������Ĭ�ϱ�ǩ
    return false;
  }
  auto [label_for_jmp, label_in_body] = GetDefaultLabel(*this);
  AddSentence(std::move(label_in_body));
  default_case_ = std::move(label_for_jmp);
  return true;
}
std::pair<std::unique_ptr<Label>, std::unique_ptr<Label>>
SwitchSentence::ConvertCaseValueToLabel(
    const SwitchSentence& switch_sentence,
    const BasicTypeInitializeOperatorNode& case_value) {
  // switch���ID���������ɶ�һ�޶��ı�ǩ
  auto switch_node_id = switch_sentence.GetFlowId();
  // ���ɱ�ǩ��
  std::string label_name =
      std::format("switch_{:}_{:}", switch_node_id.GetThisNodeValue(),
                  case_value.GetValue());
  // switch��ת�����ʹ�õı�ǩ
  auto label_for_jmp = std::make_unique<Label>(label_name);
  // switch������ʹ�õı�ǩ
  auto label_in_body = std::make_unique<Label>(std::move(label_name));
  return std::make_pair(std::move(label_for_jmp), std::move(label_in_body));
}
std::pair<std::unique_ptr<Label>, std::unique_ptr<Label>>
SwitchSentence::GetDefaultLabel(const SwitchSentence& switch_sentence) {
  // switch���ID���������ɶ�һ�޶��ı�ǩ
  auto switch_node_id = switch_sentence.GetFlowId();
  // ���ɱ�ǩ��
  std::string label_name =
      std::format("switch_{:}_default", switch_node_id.GetThisNodeValue());
  // switch��ת�����ʹ�õı�ǩ
  auto label_for_jmp = std::make_unique<Label>(label_name);
  // switch������ʹ�õı�ǩ
  auto label_in_body = std::make_unique<Label>(std::move(label_name));
  return std::make_pair(std::move(label_for_jmp), std::move(label_in_body));
}
bool Return::SetReturnTarget(
    const std::shared_ptr<const c_parser_frontend::type_system::FunctionType>
        function_to_return_from,
    const std::shared_ptr<const OperatorNodeInterface>& return_target) {
  assert(function_to_return_from != nullptr);
  if (return_target == nullptr) {
    auto& function_return_type =
        function_to_return_from->GetReturnTypeReference();
    if (function_return_type.GetType() == StructOrBasicType::kBasic &&
        static_cast<const c_parser_frontend::type_system::BasicType&>(
            function_return_type)
                .GetBuiltInType() ==
            c_parser_frontend::type_system::BuiltInType::kVoid) [[likely]] {
      // �����޷���ֵ
      return_target_ = return_target;
      function_to_return_from_ = function_to_return_from;
      return true;
    } else {
      return false;
    }
  }
  // ������ʱ�������ڵ���CheckAssignable����
  c_parser_frontend::operator_node::VarietyOperatorNode
      temp_node_to_be_assigned(
          nullptr, ConstTag::kNonConst,
          c_parser_frontend::operator_node::LeftRightValueTag::kLeftValue);
  bool result = temp_node_to_be_assigned.SetVarietyType(
      function_to_return_from->GetReturnTypePointer());
  assert(result);
  switch (c_parser_frontend::operator_node::AssignOperatorNode::CheckAssignable(
      temp_node_to_be_assigned, *return_target, true)) {
    case AssignableCheckResult::kZeroConvertToPointer:
    case AssignableCheckResult::kUpperConvert:
    case AssignableCheckResult::kConvertToVoidPointer:
    case AssignableCheckResult::kNonConvert:
    case AssignableCheckResult::kSignedToUnsigned:
    case AssignableCheckResult::kUnsignedToSigned:
      return_target_ = return_target;
      function_to_return_from_ = function_to_return_from;
      return true;
      break;
    default:
      return false;
      break;
  }
}
}  // namespace c_parser_frontend::flow_control

// ʹ��ǰ�����������Ƶ�������ʹ������ָ��
namespace c_parser_frontend::type_system {
FunctionType::~FunctionType() {}

bool FunctionType::CheckSentenceInFunctionValid(
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

const std::list<std::unique_ptr<FlowInterface>>& FunctionType::GetSentences()
    const {
  return *sentences_in_function_;
}

bool FunctionType::AddSentence(
    std::unique_ptr<FlowInterface>&& sentence_to_add) {
  if (CheckSentenceInFunctionValid(*sentence_to_add)) [[likely]] {
    sentences_in_function_->emplace_back(std::move(sentence_to_add));
    return true;
  } else {
    // ������ӣ����ؿ���Ȩ
    return false;
  }
}

bool FunctionType::AddSentences(
    std::list<std::unique_ptr<FlowInterface>>&& sentence_container) {
  // ����Ƿ����������нڵ㶼�������
  for (const auto& sentence : sentence_container) {
    if (CheckSentenceInFunctionValid(*sentence)) [[unlikely]] {
      return false;
    }
  }
  // ͨ����飬���������������ȫ���ϲ�����������
  sentences_in_function_->merge(std::move(sentence_container));
  return true;
}
}  // namespace c_parser_frontend::type_system

// ʹ��ǰ�����������Ƶ�������ʹ������ָ��
namespace c_parser_frontend::operator_node {
FunctionCallOperatorNode::FunctionCallArgumentsContainer::
    FunctionCallArgumentsContainer() {}

FunctionCallOperatorNode::FunctionCallArgumentsContainer::
    ~FunctionCallArgumentsContainer() {}

const std::list<
    std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
TemaryOperatorNode::GetFlowControlNodeToGetConditionReference() const {
  return *condition_flow_control_node_container_;
}

const std::list<
    std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
TemaryOperatorNode::GetFlowControlNodeToGetTrueBranchReference() const {
  return *true_branch_flow_control_node_container_;
}

const std::list<
    std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
TemaryOperatorNode::GetFlowControlNodeToGetFalseBranchReference() const {
  return *false_branch_flow_control_node_container_;
}

bool FunctionCallOperatorNode::SetArguments(
    FunctionCallArgumentsContainer&& container) {
  FunctionCallArgumentsContainer::ContainerType& raw_container =
      container.GetFunctionCallArguments();
  // ��׼�ĺ������ò��������������������в����������ת����λ����ͬ�Ĳ���
  const auto& function_argument_standard =
      GetFunctionTypePointer()->GetArguments();
  if (raw_container.size() != function_argument_standard.size()) [[unlikely]] {
    // ���ø����Ĳ�����Ŀ�뺯��������Ŀ�����
    return false;
  }
  // ����Ƿ�ÿ������������ת��Ϊ���յ���ʹ�õĲ�������
  auto iter_standard = function_argument_standard.begin();
  auto iter_container_to_set = raw_container.begin();
  for (size_t i = 0; i < raw_container.size(); i++) {
    AssignableCheckResult check_result = AssignOperatorNode::CheckAssignable(
        *iter_standard->variety_operator_node, *iter_container_to_set->first,
        true);
    switch (check_result) {
      case AssignableCheckResult::kNonConvert:
      case AssignableCheckResult::kUpperConvert:
      case AssignableCheckResult::kConvertToVoidPointer:
      case AssignableCheckResult::kZeroConvertToPointer:
      case AssignableCheckResult::kUnsignedToSigned:
      case AssignableCheckResult::kSignedToUnsigned:
        // �������
        break;
      default:
        // ���������
        return false;
        break;
    }
  }
  // ���в���ͨ�����
  function_arguments_offerred_ = std::move(container);
  return true;
}

}  // namespace c_parser_frontend::operator_node