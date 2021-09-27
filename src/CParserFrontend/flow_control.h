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
  kIfElseSentence,   // if-else���
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
using c_parser_frontend::operator_node::InitializeOperatorNodeInterface;
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
  enum class IdWrapper { kFlowNodeId };

 public:
  // ���̽ڵ�ID
  using FlowId = frontend::common::ExplicitIdWrapper<size_t, IdWrapper,
                                                     IdWrapper::kFlowNodeId>;

  FlowInterface(FlowType flow_type) : flow_type_(flow_type){};
  virtual ~FlowInterface();

  // ����������������
  virtual bool AddMainSentence(std::unique_ptr<FlowInterface>&& sentence) = 0;
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) = 0;

  FlowType GetFlowType() const { return flow_type_; }
  FlowId GetFlowId() const { return flow_id_; }

 protected:
  void SetFlowType(FlowType flow_type) { flow_type_ = flow_type; }

 private:
  // ��ȡһ����δ���������FlowId
  static FlowId GetNewFlowId() {
    static thread_local FlowId flow_id(0);
    return flow_id++;
  }

  // ���̿��ƽڵ���
  const FlowId flow_id_ = GetNewFlowId();
  // ���̿��ƽڵ�����
  FlowType flow_type_;
};

// ��ǩ
class Label : public FlowInterface {
 public:
  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    // ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    // ��ֹ����
    return false;
  }

  template <class LabelName>
  Label(LabelName&& label_name)
      : FlowInterface(FlowType::kLabel),
        label_name_(std::forward<LabelName>(label_name)) {}
  Label(const Label&) = delete;
  Label(Label&&) = delete;

  Label& operator=(const Label&) = default;
  Label& operator=(Label&&) = default;

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

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    // ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    // ��ֹ����
    return false;
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

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    // ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    // ��ֹ����
    return false;
  }

  void SetReturnTarget(
      const std::shared_ptr<const OperatorNodeInterface>& return_target) {
    return_target_ = return_target;
  }

  std::shared_ptr<const OperatorNodeInterface> GetReturnValuePointer() const {
    return return_target_;
  }

 private:
  // Ҫ���ص�ֵ��nullptr�����޷���ֵ
  std::shared_ptr<const OperatorNodeInterface> return_target_;
};

// �������ƿ���ࣨif,for,while�ȣ�
// Ӧ���ֽӿ�һ���ԣ������಻Ӧ�޸Ľӿ������Ҳ������κη�ʽ��д���ƹ��ӿ�
class ConditionBlockInterface : public FlowInterface {
 public:
  ConditionBlockInterface(FlowType flow_type) : FlowInterface(flow_type) {}

  bool SetCondition(
      const std::shared_ptr<const OperatorNodeInterface>& condition,
      std::list<std::unique_ptr<FlowInterface>>&& sentences_to_get_condition);
  const std::list<std::unique_ptr<FlowInterface>>& GetSentenceToGetCondition()
      const {
    return sentence_to_get_condition_;
  }
  std::shared_ptr<const OperatorNodeInterface> GetConditionPointer() const {
    return condition_;
  }
  const OperatorNodeInterface& GetConditionReference() const {
    return *condition_;
  }
  // �������������䣬������鲻ͨ������ӣ����޸Ĳ���
  // �����Ƿ���ӳɹ�
  bool AddSentence(std::unique_ptr<FlowInterface>&& sentence);
  // ����������Ӷ����䣬������������鲻ͨ������ӣ����޸Ĳ���
  // �����Ƿ���ӳɹ�
  bool AddSentences(std::list<std::unique_ptr<FlowInterface>>&& sentences);
  const auto& GetSentences() const { return main_block_; }

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
  // ��ȡ���ƿ�����
  std::list<std::unique_ptr<FlowInterface>> sentence_to_get_condition_;
  // ���ƿ����������
  std::list<std::unique_ptr<FlowInterface>> main_block_;
};

class FunctionDefine : public FlowInterface {
 public:
  FunctionDefine(const std::shared_ptr<FunctionType>& function_type)
      : FlowInterface(FlowType::kFunctionDefine),
        function_type_(function_type) {}

  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return GetFunctionTypeReference().AddSentences(std::move(sentences));
  }
  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return GetFunctionTypeReference().AddSentence(std::move(sentence));
  }

  void SetFunctionObjectToCall(
      const std::shared_ptr<FunctionType>& function_type) {
    function_type_ = function_type;
  }
  std::shared_ptr<FunctionType> GetFunctionTypePointer() const {
    return function_type_;
  }
  FunctionType& GetFunctionTypeReference() const { return *function_type_; }

 private:
  // ����������
  std::shared_ptr<FunctionType> function_type_;
};

class SimpleSentence : public FlowInterface {
 public:
  SimpleSentence() : FlowInterface(FlowType::kSimpleSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    // ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    // ��ֹ����
    return false;
  }

  bool SetSentenceOperateNode(
      const std::shared_ptr<OperatorNodeInterface>& sentence_operate_node);
  std::shared_ptr<OperatorNodeInterface> GetSentenceOperateNodePointer() const {
    return sentence_operate_node_;
  }
  OperatorNodeInterface& GetSentenceOperateNodeReference() const {
    return *sentence_operate_node_;
  }

  // �������ڵ��Ƿ������Ϊ�������Ľڵ�
  static bool CheckOperatorNodeValid(
      const OperatorNodeInterface& operator_node);

 private:
  // ִ�в����Ľڵ�
  std::shared_ptr<OperatorNodeInterface> sentence_operate_node_;
};

// ����if���
class IfSentence : public ConditionBlockInterface {
 public:
  // �մ���ʱֻ��true��֧
  IfSentence() : ConditionBlockInterface(FlowType::kIfSentence) {}

  // ����if��������ж�Ӧ����ӵ��ĸ�����
  // �����FlowType::kIfSentence����ӵ�true_branch
  // �����FlowType::kIfElseSentence����ӵ�false_branch
  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override;
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override;

  // ���ýڵ�ת��Ϊif-else���
  // ������else��Ĵ洢�ṹ
  void ConvertToIfElse();
  // ���if�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����Ҳ��޸Ĳ���
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddTrueBranchSentence(
      std::unique_ptr<FlowInterface>&& if_body_sentence) {
    // ��AddMainBlockSentence�м��
    return AddSentence(std::move(if_body_sentence));
  }
  // ��Ӷ���if�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����Ҳ��޸Ĳ���
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddTrueBranchSentences(
      std::list<std::unique_ptr<FlowInterface>>&& if_body_sentences) {
    return AddSentences(std::move(if_body_sentences));
  }
  // ���else�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddFalseBranchSentence(
      std::unique_ptr<FlowInterface>&& else_body_sentence);
  // ��Ӷ���else�����ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����ҷ������̽ڵ�Ŀ���Ȩ
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddFalseBranchSentences(
      std::list<std::unique_ptr<FlowInterface>>&& else_body_sentences);
  const auto& GetTrueBranchSentences() const { return GetSentences(); }
  const auto& GetFalseBranchSentences() const { return else_body_; }

  static bool CheckElseBodySentenceValid(
      const FlowInterface& else_body_sentence) {
    // if�����else����Ҫ����ͬ
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        else_body_sentence);
  }

 private:
  // else�����ִ�е�����
  std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>> else_body_ =
      nullptr;
};

// ѭ��������
class LoopSentenceInterface : public ConditionBlockInterface {
 protected:
  LoopSentenceInterface(FlowType flow_type)
      : ConditionBlockInterface(flow_type) {
    assert(flow_type == FlowType::kDoWhileSentence ||
           flow_type == FlowType::kWhileSentence ||
           flow_type == FlowType::kForSentence);
  }
};

class WhileSentenceInterface : public LoopSentenceInterface {
 protected:
  WhileSentenceInterface(FlowType while_type)
      : LoopSentenceInterface(while_type) {
    assert(while_type == FlowType::kWhileSentence ||
           while_type == FlowType::kDoWhileSentence);
  }
};

class WhileSentence : public WhileSentenceInterface {
 public:
  WhileSentence() : WhileSentenceInterface(FlowType::kWhileSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }
};

class DoWhileSentence : public WhileSentenceInterface {
 public:
  DoWhileSentence() : WhileSentenceInterface(FlowType::kDoWhileSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }
};

class ForSentence : public LoopSentenceInterface {
 public:
  ForSentence() : LoopSentenceInterface(FlowType::kForSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }

  // ���for���ִ��ǰ������
  // �����Ƿ������ӣ��������������򲻻�����Ҳ��޸Ĳ���
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddForInitSentence(std::unique_ptr<FlowInterface>&& init_body_sentence);
  // ��Ӷ���for���ִ��ǰ������
  // �����Ƿ������ӣ��������������򲻻�����Ҳ��޸Ĳ���
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddForInitSentences(
      std::list<std::unique_ptr<FlowInterface>>&& init_body_sentences);
  // ���for�������ִ��һ�κ�ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����Ҳ��޸Ĳ���
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddForRenewSentence(
      std::unique_ptr<FlowInterface>&& after_body_sentence);
  // ��Ӷ���for�������ִ��һ�κ�ִ�е�����
  // �����Ƿ������ӣ��������������򲻻�����Ҳ��޸Ĳ���
  // ��ӳɹ��򲻷������̽ڵ����Ȩ
  bool AddForRenewSentences(
      std::list<std::unique_ptr<FlowInterface>>&& after_body_sentences);
  const auto& GetForBody() const { return GetSentences(); }
  const auto& GetSentencesToGetForCondition() const {
    return GetSentenceToGetCondition();
  }
  const auto& GetForInitSentences() const { return init_block_; }
  const auto& GetForAfterBodySentences() const { return after_body_sentences_; }

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
  std::list<std::unique_ptr<FlowInterface>> after_body_sentences_;
};

class SwitchSentence : public ConditionBlockInterface {
 public:
  SwitchSentence() : ConditionBlockInterface(FlowType::kSwitchSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }

  // �����ͨ��case�������Ƿ���ӳɹ�
  // ���ʧ�ܲ��޸Ĳ�������ӳɹ����ȡcase_label����Ȩ
  bool AddSimpleCase(
      const std::shared_ptr<const BasicTypeInitializeOperatorNode>& case_value);
  bool AddDefaultCase();
  const auto& GetSimpleCases() const { return simple_cases_; }
  Label& GetDefaultCase() const { return *default_case_; }

  static bool CheckSwitchBodySentenceValid(
      const FlowInterface& switch_body_sentence) {
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        switch_body_sentence);
  }
  // ��������case�Ƿ������ӵ������Ľڵ���
  static bool CheckSwitchCaseAbleToAdd(
      const SwitchSentence& switch_node,
      const BasicTypeInitializeOperatorNode& case_value);
  // ��case��ֵת��Ϊ��ǩ
  // ����ֵǰ�벿��Ϊ��ת�����ʹ�õı�ǩ����벿��Ϊswitch������ʹ�õı�ǩ
  static std::pair<std::unique_ptr<Label>, std::unique_ptr<Label>>
  ConvertCaseValueToLabel(const SwitchSentence& switch_sentence,
                          const BasicTypeInitializeOperatorNode& case_value);
  // ����switch��䴴��Ĭ�ϱ�ǩ
  // ����ֵǰ�벿��Ϊ��ת�����ʹ�õı�ǩ����벿��Ϊswitch������ʹ�õı�ǩ
  static std::pair<std::unique_ptr<Label>, std::unique_ptr<Label>>
  GetDefaultLabel(const SwitchSentence& switch_sentence);

 private:
  // switch�ķ�֧(case)��ʹ��case��ֵ��Ϊ��ֵ
  std::unordered_map<
      std::string,
      std::pair<std::shared_ptr<const BasicTypeInitializeOperatorNode>,
                std::unique_ptr<Label>>>
      simple_cases_;
  // default��֧
  std::unique_ptr<Label> default_case_ = nullptr;
};

class FlowControlSystem {
 public:
  void SetFunctionToConstruct(
      const std::shared_ptr<FunctionType>& active_function) {
    active_function_ = active_function;
  }
  std::shared_ptr<FunctionType> GetActiveFunctionPointer() const {
    return active_function_;
  }
  FunctionType& GetActiveFunctionReference() const { return *active_function_; }
  // ��ɺ������������
  void FinishFunctionConstruct() {
    functions_.emplace_back(std::move(active_function_));
    active_function_ = nullptr;
  }

 private:
  // ��ǰ��ĺ��������ڱ������ĺ�����
  std::shared_ptr<FunctionType> active_function_ = nullptr;
  // ���й�����ɵĺ���
  std::list<std::shared_ptr<FunctionType>> functions_;
};
}  // namespace c_parser_frontend::flow_control

#endif