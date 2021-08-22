#ifndef CPARSERFRONTEND_FLOW_CONTROL_H_
#define CPARSERFRONTEND_FLOW_CONTROL_H_

#include <list>
#include <memory>
#include <unordered_map>

#include "operator_node.h"
#include "type_system.h"

namespace c_parser_frontend::flow_control {
enum class FlowType {
  kFunctionDefine,   // ��������
  kSimpleSentence,   // ���������̿������
  kLabel,            // ��ӱ�ǩ���
  kJmp,              // ��ת���
  kReturn,           // �������
  kIfSentence,       // if���
  kWhileSentence,    // while���
  kDoWhileSentence,  // do-while���
  kForSentence,      // for���
  kSwitchSentence    // switch���
};

// ���ò����ڵ�͸����������͵Ķ���
using c_parser_frontend::operator_node::AllocateOperatorNode;
using c_parser_frontend::operator_node::AssignOperatorNode;
using c_parser_frontend::operator_node::BasicTypeInitializeOperatorNode;
using c_parser_frontend::operator_node::DereferenceOperatorNode;
using c_parser_frontend::operator_node::FunctionCallOperatorNode;
using c_parser_frontend::operator_node::GeneralOperationType;
using c_parser_frontend::operator_node::InitializeOperatorNode;
using c_parser_frontend::operator_node::InitializeType;
using c_parser_frontend::operator_node::LogicalOperation;
using c_parser_frontend::operator_node::LogicalOperationOperatorNode;
using c_parser_frontend::operator_node::MathematicalOperatorNode;
using c_parser_frontend::operator_node::MemberAccessOperatorNode;
using c_parser_frontend::operator_node::OperatorNodeInterface;
using c_parser_frontend::operator_node::VarietyOperatorNode;
using c_parser_frontend::type_system::AssignableCheckResult;
using c_parser_frontend::type_system::BuiltInType;
using c_parser_frontend::type_system::CommonlyUsedTypeGenerator;
using c_parser_frontend::type_system::ConstTag;
using c_parser_frontend::type_system::FunctionType;
using c_parser_frontend::type_system::StructOrBasicType;
using c_parser_frontend::type_system::TypeInterface;

class FlowInterface {
 public:
  FlowInterface(FlowType flow_type) : flow_type_(flow_type){};
  virtual ~FlowInterface();

  void SetFlowType(FlowType flow_type) { flow_type_ = flow_type; }
  FlowType GetFlowType() const { return flow_type_; }

 private:
  FlowType flow_type_;
};

// ��ǩ
class Label : public FlowInterface {
 public:
  template <class LabelName>
  Label(LabelName&& label_name)
      : FlowInterface(FlowType::kLabel),
        label_name_(std::forward<LabelName>(label_name)) {}
  Label(const Label&) = default;
  Label& operator=(const Label&) = default;

  template <class LabelName>
  void SetLabelName(LabelName&& label_name) {
    label_name_ = std::forward<LabelName>(label_name);
  }
  const std::string& GetLabelName() const { return label_name_; }

 private:
  std::string label_name_;
};

// ��תָ��
class Jmp : public FlowInterface {
 public:
  Jmp() : FlowInterface(FlowType::kJmp) {}

  void SetTagetLabel(const Label& target_label) {
    target_label_ = std::make_unique<Label>(target_label);
  }
  const Label& GetTargetLabel() const { return *target_label_; }

 private:
  // ��תָ��Ҫ�����ı�ǩ
  std::unique_ptr<Label> target_label_;
};

//�Ӻ����з���
class Return : public FlowInterface {
 public:
  Return() : FlowInterface(FlowType::kReturn) {}

  void SetReturnTarget(
      std::shared_ptr<const VarietyOperatorNode>&& return_target) {
    return_target_ = std::move(return_target);
  }

  std::shared_ptr<const VarietyOperatorNode> GetReturnTargetPointer() const {
    return return_target_;
  }
  const VarietyOperatorNode& GetReturnTargetReference() const {
    return *return_target_;
  }

 private:
  // Ҫ���صĽڵ㣬���ڵ���Ϊvoid�����޷���ֵ
  std::shared_ptr<const VarietyOperatorNode> return_target_;
};

// �������ƿ���ࣨif,for,while�ȣ�
class ConditionBlockInterface : public FlowInterface {
 public:
  ConditionBlockInterface(FlowType flow_type) : FlowInterface(flow_type) {}

  void SetCondition(std::shared_ptr<const OperatorNodeInterface>&& condition) {
    condition_ = std::move(condition);
  }
  std::shared_ptr<const OperatorNodeInterface> GetConditionPointer() const {
    return condition_;
  }
  const OperatorNodeInterface& GetConditionReference() const {
    return *condition_;
  }
  void AddMainBlockSentence(std::unique_ptr<FlowInterface>&& sentence) {
    main_block_.emplace_back(std::move(sentence));
  }
  const auto& GetMainBlock() const { return main_block_; }

  // Ĭ�϶�ѭ�������ڵ�������ͼ��ĺ���
  // ���ظ����ڵ��Ƿ������Ϊ������֧����������
  static bool DefaultConditionCheck(
      const OperatorNodeInterface& condition_node);
  // Ĭ�ϼ�����������ĺ���
  // ���ظ������̿��ƽڵ��Ƿ������Ϊ������֧������������
  static bool DefaultMainBlockSentenceCheck(
      const FlowInterface& flow_interface);

 private:
  // ���ƿ������
  std::shared_ptr<const OperatorNodeInterface> condition_;
  // ���ƿ����������
  std::list<std::unique_ptr<FlowInterface>> main_block_;
};

class FunctionDefine : public FlowInterface {
 public:
  FunctionDefine(std::shared_ptr<const FunctionType>&& function_type)
      : FlowInterface(FlowType::kFunctionDefine),
        function_type_(std::move(function_type)) {}

  // ���һ��������ִ�е���䣨������˳����ӣ�
  // �ɹ���ӷ���true������������Ȩ
  // �����������򷵻�false�����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddSentence(
      std::unique_ptr<FlowInterface>&& sentence_to_add);
  const auto& GetSentences() const { return sentences_in_function_; }
  void SetFunctionType(std::shared_ptr<const FunctionType>&& function_type) {
    function_type_ = std::move(function_type);
  }
  std::shared_ptr<const FunctionType> GetFunctionTypePointer() const {
    return function_type_;
  }
  const FunctionType& GetFunctionTypeReference() const {
    return *function_type_;
  }
  // ����������Ƿ������Ϊ�����ڳ��ֵ����
  static bool CheckSentenceInFunctionValid(const FlowInterface& flow_interface);

 private:
  // ����������
  std::shared_ptr<const FunctionType> function_type_;
  // ������ִ�е����
  std::list<std::unique_ptr<FlowInterface>> sentences_in_function_;
};

class SimpleSentence : public FlowInterface {
 public:
  SimpleSentence() : FlowInterface(FlowType::kSimpleSentence) {}

  bool SetSentenceOperateNode(
      std::shared_ptr<const OperatorNodeInterface>&& sentence_operate_node);
  std::shared_ptr<const OperatorNodeInterface> GetSentenceOperateNodePointer()
      const {
    return sentence_operate_node_;
  }
  const OperatorNodeInterface& GetSentenceOperateNodeReference() const {
    return *sentence_operate_node_;
  }

  // �������ڵ��Ƿ������Ϊ�������Ľڵ�
  static bool CheckOperatorNodeValid(
      const OperatorNodeInterface& operator_node);

 private:
  // ִ�в����Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> sentence_operate_node_;
};

// ����if���Ļ�����
class IfSentenceInterface : public ConditionBlockInterface {
 public:
  IfSentenceInterface() : ConditionBlockInterface(FlowType::kIfSentence) {}

  // ��ȡif����else���֣������������Ӧ���ؿ�ָ��
  virtual const std::list<std::unique_ptr<FlowInterface>>* GetElseBody()
      const = 0;

  // �����Ƿ��������if�ж�������������������򷵻�false�Ҳ�����
  bool SetIfCondition(
      std::shared_ptr<const OperatorNodeInterface>&& if_condition);
  std::shared_ptr<const OperatorNodeInterface> GetIfConditionPointer() const {
    return GetConditionPointer();
  }
  const OperatorNodeInterface& GetIfConditionReference() const {
    return GetConditionReference();
  }
  // ���if�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddIfBodySentence(
      std::unique_ptr<FlowInterface>&& if_body_sentence);
  const auto& GetIfBody() const { return GetMainBlock(); }

  // �������ڵ��Ƿ������Ϊif������
  static bool CheckIfConditionValid(
      const OperatorNodeInterface& condition_node) {
    return DefaultConditionCheck(condition_node);
  }
  // ����������Ƿ������Ϊif�ڲ������
  static bool CheckIfBodySentenceValid(const FlowInterface& if_body_sentence) {
    return DefaultMainBlockSentenceCheck(if_body_sentence);
  }
};

// ֻ��ifû��else��if���
class IfSentence : public IfSentenceInterface {
 public:
  IfSentence() = default;

  virtual const std::list<std::unique_ptr<FlowInterface>>* GetElseBody()
      const override {
    return nullptr;
  }
};

// ��ifҲ��else�����
class IfElseSentence : public IfSentenceInterface {
 public:
  IfElseSentence() = default;

  virtual const std::list<std::unique_ptr<FlowInterface>>* GetElseBody()
      const override {
    return &else_body_;
  }

  // ���else�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddElseBodySentence(
      std::unique_ptr<FlowInterface>&& else_body_sentence);

  static bool CheckElseBodySentenceValid(
      const FlowInterface& else_body_sentence) {
    // if�����else����Ҫ����ͬ
    return IfSentenceInterface::CheckIfBodySentenceValid(else_body_sentence);
  }

 private:
  // else�����ִ�е�����
  std::list<std::unique_ptr<FlowInterface>> else_body_;
};

class WhileSentence : public ConditionBlockInterface {
 public:
  WhileSentence() : ConditionBlockInterface(FlowType::kWhileSentence) {}
  WhileSentence(FlowType flow_type) : ConditionBlockInterface(flow_type) {
    // �ṩ��do-whileѭ���̳��ù��캯��
    assert(flow_type == FlowType::kDoWhileSentence);
  }

  // �����Ƿ��������while�ж�������������������򷵻�false�Ҳ�����
  bool SetWhileCondition(
      std::shared_ptr<const OperatorNodeInterface>&& while_condition);
  std::shared_ptr<const OperatorNodeInterface> GetWhileConditionPointer()
      const {
    GetConditionPointer();
  }
  const OperatorNodeInterface& GetWhileConditionReference() const {
    return GetConditionReference();
  }
  const auto& GetWhileBody() const { return GetMainBlock(); }
  // ���while�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddWhileBodySentence(
      std::unique_ptr<FlowInterface>&& while_body_sentence);

  static bool CheckWhileConditionValid(
      const OperatorNodeInterface& while_condition) {
    return ConditionBlockInterface::DefaultConditionCheck(while_condition);
  }

  static bool CheckWhileBodySentenceValid(
      const FlowInterface& while_body_sentence) {
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        while_body_sentence);
  }
};

class DoWhileSentence : public WhileSentence {
 public:
  // do-while��whileѭ��AST��ͬ�����ڷ���ʱ��ͬ
  DoWhileSentence() : WhileSentence(FlowType::kDoWhileSentence) {}
};
class ForSentence : public ConditionBlockInterface {
 public:
  ForSentence() : ConditionBlockInterface(FlowType::kForSentence) {}

  // �����Ƿ��������if�ж�������������������򷵻�false�Ҳ�����
  bool SetForCondition(
      std::shared_ptr<const OperatorNodeInterface>&& for_condition);
  // ���for�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddForBodySentence(
      std::unique_ptr<FlowInterface>&& for_body_sentence);
  // ���for���ִ��ǰ������
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddForInitSentence(
      std::unique_ptr<FlowInterface>&& init_body_sentence);
  // ���for�������ִ��һ�κ�ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddForAfterBodySentence(
      std::unique_ptr<FlowInterface>&& after_body_sentence);
  const auto& GetForBody() const { return GetMainBlock(); }
  const auto& GetForInitSentences() const { return init_block_; }
  const auto& GetForAfterBodySentences() const { return after_body_; }

  static bool CheckForConditionValid(
      const OperatorNodeInterface& for_condition) {
    return ConditionBlockInterface::DefaultConditionCheck(for_condition);
  }
  static bool CheckForBodySentenceValid(
      const FlowInterface& for_body_sentence) {
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        for_body_sentence);
  }

 private:
  // ��ʼ�����
  std::list<std::unique_ptr<FlowInterface>> init_block_;
  // һ��ѭ��������ִ�е����
  std::list<std::unique_ptr<FlowInterface>> after_body_;
};

class SwitchSentence : public ConditionBlockInterface {
 public:
  SwitchSentence() : ConditionBlockInterface(FlowType::kSwitchSentence) {}

  // �����ͨ��case�������Ƿ���ӳɹ���������ʧ���򷵻�case_label�Ŀ���Ȩ
  std::pair<std::unique_ptr<Label>, bool> AddSimpleCase(
      std::shared_ptr<BasicTypeInitializeOperatorNode>&& case_value,
      std::unique_ptr<Label>&& case_label);
  // ���switch������岿��
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  std::pair<std::unique_ptr<FlowInterface>, bool> AddSwitchBodySentence(
      std::unique_ptr<FlowInterface>&& switch_body_sentence);
  // ����switch����֧����
  // �����Ƿ�������ã���������������򲻻��޸�
  bool SetSwitchCondition(
      std::shared_ptr<const OperatorNodeInterface>&& switch_condition);
  void SetDefaultCase(std::unique_ptr<Label>&& default_case) {
    default_case_ = std::move(default_case);
  }
  const auto& GetSimpleCases() const { return simple_cases_; }
  Label& GetDefaultCase() const { return *default_case_; }

  static bool CheckSwitchConditionValid(
      const OperatorNodeInterface& switch_condition) {
    return ConditionBlockInterface::DefaultConditionCheck(switch_condition);
  }
  static bool CheckSwitchBodySentenceValid(
      const FlowInterface& switch_body_sentence) {
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        switch_body_sentence);
  }
  // ��������case�Ƿ������ӵ������Ľڵ���
  static bool CheckSwitchCaseAbleToAdd(
      const SwitchSentence& switch_node,
      const BasicTypeInitializeOperatorNode& case_value);

 private:
  // switch�ķ�֧(case)��ʹ��case��ֵ��Ϊ��ֵ
  std::unordered_map<std::string,
                     std::pair<std::shared_ptr<BasicTypeInitializeOperatorNode>,
                               std::unique_ptr<Label>>>
      simple_cases_;
  // default��֧
  std::unique_ptr<Label> default_case_;
};

}  // namespace c_parser_frontend::flow_control

#endif