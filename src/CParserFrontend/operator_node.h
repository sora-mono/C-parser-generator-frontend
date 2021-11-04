#ifndef CPARSERFRONTEND_OPERATOR_NODE_H_
#define CPARSERFRONTEND_OPERATOR_NODE_H_
#include "Common/id_wrapper.h"
#include "type_system.h"

namespace c_parser_frontend::flow_control {
// ǰ������FlowInterface����Ŀ������ڵ㣨TemaryOperatorNode��
// ��SetBranchConditionʹ��
class FlowInterface;
}  // namespace c_parser_frontend::flow_control

// �����ڵ�
namespace c_parser_frontend::operator_node {
// ����ڵ����
enum class GeneralOperationType {
  kAllocate,               // ����ռ�
  kTypeConvert,            // ����ת��
  kVariety,                // ��������
  kInitValue,              // ��ʼ����ֵ
  kTemaryOperator,         // ?:����Ŀ�������
  kAssign,                 // =����ֵ��
  kMathematicalOperation,  // ��ѧ����
  kLogicalOperation,       // �߼�����
  kDeReference,            // ������
  kObtainAddress,          // ȡ��ַ
  kMemberAccess,           // ��Ա����
  kFunctionCall            // ��������
};
// ��ѧ�����
enum class MathematicalOperation {
  kOr,                    // |����λ��
  kXor,                   // ^����λ���
  kAnd,                   // &����λ�룩
  kLeftShift,             // <<�����ƣ�
  kRightShift,            // >>�����ƣ�
  kPlus,                  // +���ӣ�
  kMinus,                 // -������
  kMultiple,              // *���ˣ�
  kDivide,                // /������
  kMod,                   // %����ģ��
  kLogicalNegative,       // ~����λȡ����
  kMathematicalNegative,  // -��ȡ����
  kNot,                   // !���߼��ǣ�
};
// ��ѧ�븳ֵ�����
enum class MathematicalAndAssignOperation {
  kOrAssign,          // |=
  kXorAssign,         // ^=
  kAndAssign,         // &=
  kLeftShiftAssign,   // <<=
  kRightShiftAssign,  // >>=
  kPlusAssign,        // +=
  kMinusAssign,       // -=
  kMultipleAssign,    // *=
  kDivideAssign,      // /=
  kModAssign,         // %=
};
// �߼������
enum class LogicalOperation {
  kAndAnd,        // &&
  kOrOr,          // ||
  kGreater,       // >
  kGreaterEqual,  // >=
  kLess,          // <
  kLessEqual,     // <=
  kEqual,         // ==
  kNotEqual       // !=
};
// ��������
enum class ProcessType {
  kCommon,   // ��ͨ�������
  kIf,       // if��֧���
  kSwitch,   // Switch��֧���
  kDoWhile,  // Do-Whileѭ�����
  kWhile,    // Whileѭ�����
  kFor       // Forѭ�����
};
// ��ʼ��ֵ����
enum class InitializeType {
  kBasic,          // �������ͣ���������ȡ����ֵ�ķ�Χ
  kString,         // �ַ������̶�Ϊconst char*����
  kFunction,       // ���ڸ�����ָ�븳ֵ��ȫ�ֳ�ʼ����������
  kInitializeList  // ��ʼ���б�
};
// ����ֵ����
enum class LeftRightValueTag {
  kLeftValue,  // ��ֵ
  kRightValue  // ��ֵ
};

// ���ñ������ͻ���
using c_parser_frontend::type_system::TypeInterface;
// ���ñ�������
using c_parser_frontend::type_system::StructOrBasicType;
// ���ñ���ϵͳ
using c_parser_frontend::type_system::TypeSystem;
// ���ó�������������
using c_parser_frontend::type_system::CommonlyUsedTypeGenerator;
// �����ж��Ƿ���Ը�ֵ�����ö��
using c_parser_frontend::type_system::AssignableCheckResult;
// ����const���
using c_parser_frontend::type_system::ConstTag;
// ���÷��ű��
using c_parser_frontend::type_system::SignTag;
// ����Ԥ��������
using c_parser_frontend::type_system::BuiltInType;

// ����ѧ�븳ֵ�����ת��Ϊ��ѧ�����
MathematicalOperation MathematicalAndAssignOperationToMathematicalOperation(
    MathematicalAndAssignOperation mathematical_and_assign_operation);

class OperatorNodeInterface {
  enum class IdWrapper { kOperatorNodeId };

 public:
  // �����ڵ�ID
  using OperatorId =
      frontend::common::ExplicitIdWrapper<size_t, IdWrapper,
                                          IdWrapper::kOperatorNodeId>;

  OperatorNodeInterface(GeneralOperationType general_operator_type)
      : general_operator_type_(general_operator_type) {}
  // ���ƹ��캯�������ƽڵ��Ŷ��������µı��
  OperatorNodeInterface(const OperatorNodeInterface& operator_node)
      : general_operator_type_(operator_node.general_operator_type_) {}
  virtual ~OperatorNodeInterface() {}

  OperatorNodeInterface& operator=(const OperatorNodeInterface& old_interface) {
    general_operator_type_ = old_interface.general_operator_type_;
    return *this;
  }

  // ��ȡ�ýڵ�����ConstTag
  virtual ConstTag GetResultConstTag() const = 0;
  // ��ȡ���ս��������
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer() const = 0;
  // ��ȡ���ս���Ľڵ㣬����nullptr��������
  // ��������ؿ�ָ����һ����VarietyOperatorNode��InitializeOperatorNodeInterface
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const = 0;
  // ������������shared_ptr�����ÿ����õ��Ķ�������Ϊnew_type
  // ������ܿ����򷵻�nullptr
  // ��������ת��
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const = 0;

  OperatorId GetOperatorId() const { return operator_id_; }
  void SetGeneralOperatorType(GeneralOperationType operator_type) {
    general_operator_type_ = operator_type;
  }
  GeneralOperationType GetGeneralOperatorType() const {
    return general_operator_type_;
  }

 private:
  // ��ȡһ����δ���������OperatorId
  static OperatorId GetNewOperatorId() {
    static thread_local OperatorId operator_id(0);
    return operator_id++;
  }

  // �����ڵ�ID��ͬʱ��Ϊ��Ž����LLVM�ļĴ������
  const OperatorId operator_id_ = GetOperatorId();
  // ��ģ�������������
  GeneralOperationType general_operator_type_;
};

class AllocateOperatorNode : public OperatorNodeInterface {
 public:
  AllocateOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kAllocate) {}
  AllocateOperatorNode(const AllocateOperatorNode&) = delete;

  AllocateOperatorNode& operator=(const AllocateOperatorNode&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    assert(false);
    // ��ֹ����
    return ConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<const TypeInterface>();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<const OperatorNodeInterface>();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // ����Ҫ�����ڴ�Ľڵ�
  // �����Ƿ���Է��䣬���ܷ����򲻻�����
  // ͬʱ���ô�����ռ��С
  bool SetTargetVariety(
      const std::shared_ptr<const OperatorNodeInterface>& target_variety);
  std::shared_ptr<const OperatorNodeInterface> GetTargetVarietyPointer() const {
    return target_variety_;
  }
  const OperatorNodeInterface& GetTargetVarietyReference() const {
    return *target_variety_;
  }

  static bool CheckAllocatable(const OperatorNodeInterface& node_to_allocate) {
    assert(node_to_allocate.GetGeneralOperatorType() ==
           GeneralOperationType::kVariety);
    return true;
  }

 private:
  // ����ռ��洢����ָ��
  std::shared_ptr<const OperatorNodeInterface> target_variety_;
};

// ����ת��
// Դ�ڵ���Ҫ֧��GetResultOperatorNode()
class TypeConvert : public OperatorNodeInterface {
 public:
  TypeConvert(const std::shared_ptr<const OperatorNodeInterface>& source_node,
              const std::shared_ptr<const TypeInterface>& new_type)
      : OperatorNodeInterface(GeneralOperationType::kTypeConvert),
        source_node_(source_node),
        destination_node_(source_node->SelfCopy(new_type)) {}
  TypeConvert(const TypeConvert&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    return GetDestinationNodeReference().GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetDestinationNodeReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetDestinationNodePointer();
  }
  // ���ܿ�������ת���ڵ㣬Ӧ�ÿ���destination_node_
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  std::shared_ptr<const OperatorNodeInterface> GetSourceNodePointer() const {
    return source_node_;
  }
  const OperatorNodeInterface& GetSourceNodeReference() const {
    return *source_node_;
  }
  std::shared_ptr<OperatorNodeInterface> GetDestinationNodePointer() const {
    return destination_node_;
  }
  OperatorNodeInterface& GetDestinationNodeReference() const {
    return *destination_node_;
  }
  void SetSourceNode(
      const std::shared_ptr<const OperatorNodeInterface>& source_node) {
    source_node_ = source_node;
  }
  // ����Ŀ��ڵ�
  // �����µ����ͺ��µĽڵ��const���
  // �����Ƿ����ת�����������ת���򲻻�����Ŀ��ڵ�
  bool GenerateDestinationNode(
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag);

 private:
  // ��ת���Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> source_node_;
  // ת���õ���Ŀ�Ľڵ�
  std::shared_ptr<OperatorNodeInterface> destination_node_;
};

// �洢����
class VarietyOperatorNode : public OperatorNodeInterface {
 public:
  VarietyOperatorNode(const std::string* variety_name, ConstTag const_tag,
                      LeftRightValueTag left_right_value_tag)
      : OperatorNodeInterface(GeneralOperationType::kVariety),
        variety_name_(variety_name),
        variety_const_tag_(const_tag),
        variety_left_right_value_tag_(left_right_value_tag) {}
  VarietyOperatorNode(const VarietyOperatorNode& variety_node) = default;

  VarietyOperatorNode& operator=(const VarietyOperatorNode& variety_node) =
      default;

  virtual ConstTag GetResultConstTag() const override { return GetConstTag(); }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return variety_type_;
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return nullptr;
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    auto new_node = std::make_shared<VarietyOperatorNode>(*this);
    new_node->SetVarietyType(new_type);
    return new_node;
  }

  void SetVarietyName(const std::string* variety_name) {
    variety_name_ = variety_name;
  }
  const std::string* GetVarietyNamePointer() const { return variety_name_; }
  void SetConstTag(ConstTag const_tag) { variety_const_tag_ = const_tag; }
  ConstTag GetConstTag() const { return variety_const_tag_; }
  void SetLeftRightValueTag(LeftRightValueTag left_right_value_tag) {
    variety_left_right_value_tag_ = left_right_value_tag;
  }
  LeftRightValueTag GetLeftRightValueTag() const {
    return variety_left_right_value_tag_;
  }
  bool SetVarietyType(const std::shared_ptr<const TypeInterface>& variety_type);
  // �����SetVarietyType������ʹ��FunctionType
  // ���ڳ�ʼ���б�ֵʱ�����������VarietyType�ͽ�����
  bool SetVarietyTypeNoCheckFunctionType(
      const std::shared_ptr<const TypeInterface>& variety_type);
  std::shared_ptr<const TypeInterface> GetVarietyTypePointer() const {
    return variety_type_;
  }
  const TypeInterface& GetVarietyTypeReference() const {
    return *variety_type_;
  }

  // �����������Ƿ������Ϊ����������
  static bool CheckVarietyTypeValid(const TypeInterface& variety_type);

 private:
  // ��������������ʾʱ��
  const std::string* variety_name_;
  // ��������
  std::shared_ptr<const TypeInterface> variety_type_;
  // ���������const���
  ConstTag variety_const_tag_;
  // ����������ֵ���
  LeftRightValueTag variety_left_right_value_tag_;
};

// �洢��ʼ�����ݣ������ڳ�����
class InitializeOperatorNodeInterface : public OperatorNodeInterface {
 public:
  InitializeOperatorNodeInterface(InitializeType initialize_type)
      : OperatorNodeInterface(GeneralOperationType::kInitValue),
        initialize_type_(initialize_type) {}
  InitializeOperatorNodeInterface(
      const InitializeOperatorNodeInterface& initialize_node) = default;

  InitializeOperatorNodeInterface& operator=(
      const InitializeOperatorNodeInterface& initialize_node) = default;

  virtual ConstTag GetResultConstTag() const override final {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override final {
    return initialize_value_type_;
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override final {
    return nullptr;
  }

  void SetInitlizeType(InitializeType initialize_type) {
    initialize_type_ = initialize_type;
  }
  InitializeType GetInitializeType() const { return initialize_type_; }
  // ���øýڵ������
  // �����Ƿ�Ϊ��Ч�ĳ�ʼ���������ͣ������Ч�����
  bool SetInitValueType(
      const std::shared_ptr<const TypeInterface>& init_value_type);
  const std::shared_ptr<const TypeInterface>& GetInitValueType() const {
    return initialize_value_type_;
  }
  // �����Ƿ�Ϊ��Ч�ĳ�ʼ����������
  static bool CheckInitValueTypeValid(const TypeInterface& init_value_type);

 private:
  // ��ʼ����������
  std::shared_ptr<const TypeInterface> initialize_value_type_;
  // ��ʼ���ڵ�����
  InitializeType initialize_type_;
};

// ������ʼ�����ͣ���ֵ/�ַ���/������
// ���溯��ʱvalue����
class BasicTypeInitializeOperatorNode : public InitializeOperatorNodeInterface {
 public:
  template <class Value>
  BasicTypeInitializeOperatorNode(InitializeType initialize_type, Value&& value)
      : InitializeOperatorNodeInterface(initialize_type),
        value_(std::forward<Value>(value)) {}
  BasicTypeInitializeOperatorNode(
      const std::shared_ptr<const type_system::FunctionType>& function_type)
      : InitializeOperatorNodeInterface(InitializeType::kFunction) {
    bool result = SetInitDataType(function_type);
    assert(result);
  }
  BasicTypeInitializeOperatorNode(const BasicTypeInitializeOperatorNode&) =
      default;

  BasicTypeInitializeOperatorNode& operator=(
      const BasicTypeInitializeOperatorNode&) = default;

  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    auto new_node = std::make_shared<BasicTypeInitializeOperatorNode>(*this);
    new_node->SetInitDataType(new_type);
    return new_node;
  }

  template <class Value>
  void SetValue(Value&& value) {
    value_ = std::forward<Value>(value);
  }
  const std::string& GetValue() const { return value_; }
  // ���ó�ʼ�����������ͣ���������/const char*/�������ͣ�
  bool SetInitDataType(const std::shared_ptr<const TypeInterface>& data_type);
  // �ö����Ƿ����ʹ�ø���������
  // ��Ч���ͷ���true����Ч���ͷ���false
  static bool CheckBasicTypeInitializeValid(const TypeInterface& variety_type);

 private:
  // �ⲿ����ʱ��Ӧʹ�øú����������ͣ�ʹ��SetInitDataType����
  // ����private��ֹ����
  bool SetInitValueType(
      const std::shared_ptr<const TypeInterface>& init_value_type) {
    return InitializeOperatorNodeInterface::SetInitValueType(init_value_type);
  }

  // �ַ�����ʽ�洢�Է�������ʧ
  std::string value_;
};

// ��ʼ���б�
class ListInitializeOperatorNode : public InitializeOperatorNodeInterface {
 public:
  ListInitializeOperatorNode()
      : InitializeOperatorNodeInterface(InitializeType::kInitializeList) {}
  ListInitializeOperatorNode(const ListInitializeOperatorNode&) = default;

  ListInitializeOperatorNode& operator=(const ListInitializeOperatorNode&) =
      default;

  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // ���ʼ���б�����ӳ�ʼ��ֵ
  // ����ֵ�Ƿ�Ϊ��Ч�ĳ�ʼ���б�������ͣ����������Ч��ִ�в������
  // ��ʼ��ֵ��˳��Ϊ��д˳��
  bool SetListValues(
      std::list<std::shared_ptr<InitializeOperatorNodeInterface>>&&
          list_values);
  const std::list<std::shared_ptr<const InitializeOperatorNodeInterface>>&
  GetListValues() const {
    return list_values_;
  }
  // �������ĳ�ʼ���б������Ƿ�Ϸ�
  static bool CheckInitListTypeValid(const TypeInterface& list_type) {
    return list_type.GetType() == StructOrBasicType::kInitializeList;
  }
  // �������ĳ�ʼ��ֵ�Ƿ�Ϸ�
  static bool CheckInitListValueTypeValid(
      const TypeInterface& list_value_type) {
    return InitializeOperatorNodeInterface::CheckInitValueTypeValid(
        list_value_type);
  }

 private:
  // ��ʼ���б��е�ֵ
  std::list<std::shared_ptr<const InitializeOperatorNodeInterface>>
      list_values_;
};

// ��Ŀ�����
class TemaryOperatorNode : public OperatorNodeInterface {
 public:
  TemaryOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kTemaryOperator) {}

  virtual ConstTag GetResultConstTag() const override {
    return GetResultReference().GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    // ��ֵ���ȼ��ڱ���ֵ�ı���
    return GetResultReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetResultReference().GetResultOperatorNode();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // ���÷�֧����
  // ������������޷���Ϊ��֧�����������ҷ���false
  // �����÷�֧���������÷�֧����
  // ���÷�֧�����������Ҫ�Ķ�����Ҫ��������һ����ٷ�֧
  // ��ʱ�Կ���ʹ��Get������ȡ֮ǰ�洢�ķ�֧��Ϣ
  bool SetBranchCondition(
      const std::shared_ptr<const OperatorNodeInterface>& branch_condition,
      const std::shared_ptr<const std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          flow_control_node_container);
  std::shared_ptr<const OperatorNodeInterface> GetBranchConditionPointer()
      const {
    return branch_condition_;
  }
  const OperatorNodeInterface& GetBranchConditionReference() const {
    return *branch_condition_;
  }
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
  GetFlowControlNodeToGetConditionPointer() const {
    return condition_flow_control_node_container_;
  }
  const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
  GetFlowControlNodeToGetConditionReference() const;
  // ��������Ϊ��ʱ�ķ�֧
  // �����÷�֧���������÷�֧����
  // ������������޷���Ϊ��֧�������ҷ���false
  // ���÷�֧�����������Ҫ�Ķ�����Ҫ��������һ����ٷ�֧
  // ��ʱ�Կ���ʹ��Get������ȡ֮ǰ�洢�ķ�֧��Ϣ
  bool SetTrueBranch(
      const std::shared_ptr<const OperatorNodeInterface>& true_branch,
      const std::shared_ptr<const std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          flow_control_node_container);
  std::shared_ptr<const OperatorNodeInterface> GetTrueBranchPointer() const {
    return true_branch_;
  }
  const OperatorNodeInterface& GetTrueBranchReference() const {
    return *true_branch_;
  }
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
  GetFlowControlNodeToGetTrueBranchPointer() const {
    return true_branch_flow_control_node_container_;
  }
  const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
  GetFlowControlNodeToGetTrueBranchReference() const;
  // ��������Ϊ��ʱ�ķ�֧
  // �����÷�֧���������÷�֧����
  // ������������޷���Ϊ��֧�������ҷ���false
  // ���÷�֧�����������Ҫ�Ķ�����Ҫ��������һ����ٷ�֧
  // ��ʱ�Կ���ʹ��Get������ȡ֮ǰ�洢�ķ�֧��Ϣ
  bool SetFalseBranch(
      const std::shared_ptr<const OperatorNodeInterface>& false_branch,
      const std::shared_ptr<const std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          flow_control_node_container);
  std::shared_ptr<const OperatorNodeInterface> GetFalseBranchPointer() const {
    return false_branch_;
  }
  const OperatorNodeInterface& GetFalseBranchReference() const {
    return *false_branch_;
  }
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
  GetFlowControlNodeToGetFalseBranchPointer() const {
    return false_branch_flow_control_node_container_;
  }
  const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
  GetFlowControlNodeToGetFalseBranchReference() const;
  std::shared_ptr<const OperatorNodeInterface> GetResultPointer() const {
    return result_;
  }
  const OperatorNodeInterface& GetResultReference() const { return *result_; }

  // ����֧�����Ƿ���Ч���Ƿ���ֵ��
  static bool CheckBranchConditionValid(
      const OperatorNodeInterface& branch_condition);
  // ���������֧�Ƿ���Ч���Ƿ���ֵ��
  static bool CheckBranchValid(const OperatorNodeInterface& branch);

 private:
  // �������ؽ���Ľڵ�
  // ���÷�֧������������֧�����
  // ���������֧���ܻ���ת���򷵻�false
  // �����֧����Ϊ�����ڳ��������κβ�������true
  // ��֧�ַǱ����ڳ���������ʹ�ó�ʼ���б�
  bool ConstructResultNode();

  // ��֧����
  std::shared_ptr<const OperatorNodeInterface> branch_condition_;
  // ��ȡ��֧�����Ĳ���
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
      condition_flow_control_node_container_;
  // ����Ϊ��ʱ��ֵ
  std::shared_ptr<const OperatorNodeInterface> true_branch_ = nullptr;
  // ��ȡ���֧�Ĳ���
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
      true_branch_flow_control_node_container_;
  // ����Ϊ��ʱ��ֵ
  std::shared_ptr<const OperatorNodeInterface> false_branch_ = nullptr;
  // ��ȡ�ٷ�֧�Ĳ���
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
      false_branch_flow_control_node_container_;
  // �洢���ؽ���Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> result_;
};

// ��ֵ�ڵ�
class AssignOperatorNode : public OperatorNodeInterface {
 public:
  AssignOperatorNode() : OperatorNodeInterface(GeneralOperationType::kAssign) {}
  AssignOperatorNode(const AssignOperatorNode&) = delete;

  AssignOperatorNode& operator=(const AssignOperatorNode&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    return GetNodeToBeAssignedReference().GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    // ��ֵ���ȼ��ڱ���ֵ�ı���
    return GetNodeToBeAssignedReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetNodeToBeAssignedPointer();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // �����ñ���ֵ�Ľڵ㣬������������ֵ�Ľڵ�
  void SetNodeToBeAssigned(
      const std::shared_ptr<const OperatorNodeInterface>& node_to_be_assigned) {
    node_to_be_assigned_ = node_to_be_assigned;
  }
  std::shared_ptr<const OperatorNodeInterface> GetNodeToBeAssignedPointer()
      const {
    return node_to_be_assigned_;
  }
  const OperatorNodeInterface& GetNodeToBeAssignedReference() const {
    return *node_to_be_assigned_;
  }
  // ������ǿ��Ը�ֵ������򲻻�����
  // ����ָ��Ҫ���õ�������ֵ�Ľڵ��ָ����Ƿ�Ϊ����ʱ��ֵ
  // ����ʱ��ֵ����Ա���ֵ�Ľڵ������const���Ժ�LeftRightValue����
  // ��������ʹ�ó�ʼ���б�
  // ��Ҫ�����ñ���ֵ�Ľڵ㣨SetNodeToBeAssigned��
  // �������ֵ�Ľڵ���������ֵ�Ľڵ�������ȫ��ͬ��operator==()����
  // ���ñ���ֵ�Ľڵ��������ָ��ָ��������ֵ���������Խ�ʡ�ڴ�
  AssignableCheckResult SetNodeForAssign(
      const std::shared_ptr<const OperatorNodeInterface>& node_for_assign,
      bool is_announce);
  std::shared_ptr<const OperatorNodeInterface> GetNodeForAssignPointer() const {
    return node_for_assign_;
  }
  const OperatorNodeInterface& GetNodeForAssignReference() const {
    return *node_for_assign_;
  }

  // �������ڵ�������Ƿ���Ը�ֵ
  // ������ģ�������ʾ�Ƿ�Ϊ����ʱ��ֵ
  // �Զ����AssignableCheckResut::kMayBeZeroToPointer�ľ���������ı䷵�ؽ��
  // ��is_announce == trueʱ
  // ����const��Ǻ�LeftRightValue��ǵļ��������ʹ�ó�ʼ���б�
  // ���ʹ���Զ������С�ƶ�����������С��Υ��constԭ��
  // �������ֵ�Ľڵ���������ֵ�Ľڵ�������ȫ��ͬ��operator==()����Ϊ��������
  // �����ñ���ֵ�Ľڵ��������ָ��ָ��������ֵ���������Խ�ʡ�ڴ�(Υ��constԭ��)
  static AssignableCheckResult CheckAssignable(
      const OperatorNodeInterface& node_to_be_assigned,
      const OperatorNodeInterface& node_for_assign, bool is_announce);

 private:
  // CheckAssignable���ӹ��̣�����ʹ�ó�ʼ�������б��ʼ�����������
  // �������ʼ���ı��������õĳ�ʼ���б�Ҫ���Ѿ���֤Ϊ����ʱ
  // ���᷵��AssignableCheckResult::kInitializeList
  // ���ʹ���Զ������С�ƶ�����������С��Υ��constԭ��
  static AssignableCheckResult VarietyAssignableByInitializeList(
      const VarietyOperatorNode& variety_node,
      const ListInitializeOperatorNode& list_initialize_operator_node);

  // ��Ҫ����ֵ�Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_be_assigned_;
  // ������ֵ�Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> node_for_assign_;
};

class MathematicalOperatorNode : public OperatorNodeInterface {
 public:
  using DeclineMathematicalComputeTypeResult =
      c_parser_frontend::type_system::DeclineMathematicalComputeTypeResult;

  MathematicalOperatorNode(MathematicalOperation mathematical_operation)
      : OperatorNodeInterface(GeneralOperationType::kMathematicalOperation),
        mathematical_operation_(mathematical_operation) {}
  MathematicalOperatorNode(const MathematicalOperatorNode&) = delete;

  MathematicalOperatorNode& operator=(const MathematicalOperatorNode&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetComputeResultNodeReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetComputeResultNodePointer();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  void SetMathematicalOperation(MathematicalOperation mathematical_operation) {
    mathematical_operation_ = mathematical_operation;
  }
  MathematicalOperation GetMathematicalOperation() const {
    return mathematical_operation_;
  }
  // �������������ͺ����ýڵ�
  bool SetLeftOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& left_operator_node);
  std::shared_ptr<const OperatorNodeInterface> GetLeftOperatorNodePointer()
      const {
    return left_operator_node_;
  }
  const OperatorNodeInterface& GetLeftOperatorNodeReference() const {
    return *left_operator_node_;
  }
  // ��������ڵ�������ҽڵ�
  DeclineMathematicalComputeTypeResult SetRightOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& right_operator_node);
  std::shared_ptr<const OperatorNodeInterface> GetRightOperatorNodePointer()
      const {
    return right_operator_node_;
  }
  const OperatorNodeInterface& GetRightOperatorNodeReference() const {
    return *right_operator_node_;
  }
  std::shared_ptr<const VarietyOperatorNode> GetComputeResultNodePointer()
      const {
    return compute_result_node_;
  }
  const VarietyOperatorNode& GetComputeResultNodeReference() const {
    return *compute_result_node_;
  }

  // ������������������������������ľ������
  // ������������򷵻�nullptr
  static std::pair<std::shared_ptr<const VarietyOperatorNode>,
                   DeclineMathematicalComputeTypeResult>
  DeclineComputeResult(
      MathematicalOperation mathematical_operation,
      const std::shared_ptr<const OperatorNodeInterface>& left_operator_node,
      const std::shared_ptr<const OperatorNodeInterface>& right_operator_node);

 private:
  void SetComputeResultNode(
      const std::shared_ptr<const VarietyOperatorNode>& compute_result_node) {
    compute_result_node_ = compute_result_node;
  }

  // ��ѧ��������
  MathematicalOperation mathematical_operation_;
  // ������ڵ�
  std::shared_ptr<const OperatorNodeInterface> left_operator_node_;
  // ������ڵ㣨���û�����ÿգ�
  std::shared_ptr<const OperatorNodeInterface> right_operator_node_;
  // ����õ�������
  std::shared_ptr<const VarietyOperatorNode> compute_result_node_;
};

class LogicalOperationOperatorNode : public OperatorNodeInterface {
 public:
  LogicalOperationOperatorNode(LogicalOperation logical_operation)
      : OperatorNodeInterface(GeneralOperationType::kLogicalOperation),
        logical_operation_(logical_operation) {}
  LogicalOperationOperatorNode(const LogicalOperationOperatorNode&) = delete;

  LogicalOperationOperatorNode& operator=(const LogicalOperationOperatorNode&) =
      delete;

  virtual ConstTag GetResultConstTag() const override {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetComputeResultNodeReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetComputeResultNodePointer();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  LogicalOperation GetLogicalOperation() const { return logical_operation_; }
  bool SetLeftOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& left_operator_node);
  std::shared_ptr<const OperatorNodeInterface> GetLeftOperatorNodePointer()
      const {
    return left_operator_node_;
  }
  const OperatorNodeInterface& GetLeftOperatorNodeReference() const {
    return *left_operator_node_;
  }
  // �����Ƿ�Ϊ�������߼���������ͣ����������߼�����������
  // �ɹ����ú󴴽��߼��������ڵ�
  bool SetRightOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& right_operator_node);
  std::shared_ptr<const OperatorNodeInterface> GetRightOperatorNodePointer()
      const {
    return right_operator_node_;
  }
  const OperatorNodeInterface& GetRightOperatorNodeReference() const {
    return *right_operator_node_;
  }
  std::shared_ptr<const VarietyOperatorNode> GetComputeResultNodePointer()
      const {
    return compute_result_node_;
  }
  const VarietyOperatorNode& GetComputeResultNodeReference() const {
    return *compute_result_node_;
  }

  // ����Ƿ���Բ����߼�����
  static bool CheckLogicalTypeValid(const TypeInterface& type_interface);

 private:
  // ���������ý���ڵ�
  void CreateAndSetResultNode() {
    // �����߼���������Ϊbool
    auto compute_result_node = std::make_shared<VarietyOperatorNode>(
        nullptr, ConstTag::kNonConst, LeftRightValueTag::kRightValue);
    compute_result_node->SetVarietyType(
        CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt1,
                                                SignTag::kUnsigned>());
    compute_result_node_ = compute_result_node;
  }

  // �߼���������
  LogicalOperation logical_operation_;
  // ��ڵ�
  std::shared_ptr<const OperatorNodeInterface> left_operator_node_;
  // �ҽڵ�
  std::shared_ptr<const OperatorNodeInterface> right_operator_node_;
  // ������
  std::shared_ptr<const VarietyOperatorNode> compute_result_node_;
};

// ��ָ�������
class DereferenceOperatorNode : public OperatorNodeInterface {
 public:
  DereferenceOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kDeReference) {}
  DereferenceOperatorNode(const DereferenceOperatorNode&) = delete;

  DereferenceOperatorNode& operator=(const DereferenceOperatorNode&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    return GetDereferencedObjectConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetDereferencedNodeReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetDereferencedNodePointer();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // �趨�������õĽڵ�
  // ��������Ϊ�������õĽڵ�
  // ���������Ƿ�ɹ�����
  bool SetNodeToDereference(
      const std::shared_ptr<const OperatorNodeInterface>& node_to_dereference);

  std::shared_ptr<const OperatorNodeInterface> GetNodeToDereferencePointer()
      const {
    return node_to_dereference_;
  }
  const OperatorNodeInterface& GetNodeToDereferenceReference() const {
    return *node_to_dereference_;
  }
  std::shared_ptr<const VarietyOperatorNode> GetDereferencedNodePointer()
      const {
    return dereferenced_node_;
  }
  const VarietyOperatorNode& GetDereferencedNodeReference() const {
    return *dereferenced_node_;
  }

  ConstTag GetDereferencedObjectConstTag() const {
    return dereferenced_node_->GetConstTag();
  }
  static bool CheckNodeDereferenceAble(
      const OperatorNodeInterface& node_to_dereference);

 private:
  void SetDereferencedNode(
      const std::shared_ptr<VarietyOperatorNode>& dereferenced_node) {
    dereferenced_node_ = dereferenced_node;
  }

  // ��Ҫ�������õĽڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_dereference_;
  // �����ú�õ��Ķ���Ľڵ�
  std::shared_ptr<VarietyOperatorNode> dereferenced_node_;
};

class ObtainAddressOperatorNode : public OperatorNodeInterface {
 public:
  ObtainAddressOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kObtainAddress) {}
  ObtainAddressOperatorNode(const ObtainAddressOperatorNode&) = delete;

  ObtainAddressOperatorNode& operator=(const ObtainAddressOperatorNode&) =
      delete;

  virtual ConstTag GetResultConstTag() const override {
    // ȡ��ַ��õ�ֵ���Ƿ�const
    return ConstTag::kNonConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetObtainedAddressNodeReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetObtainedAddressNodePointer();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // �����Ƿ�������ã�������������򲻻�����
  bool SetNodeToObtainAddress(
      const std::shared_ptr<const VarietyOperatorNode>& node_to_obtain_address);

  std::shared_ptr<const OperatorNodeInterface> GetNodeToBeObtainAddressPointer()
      const {
    return node_to_obtain_address_;
  }
  const OperatorNodeInterface& GetNodeToBeObtainAddressReference() const {
    return *node_to_obtain_address_;
  }
  std::shared_ptr<const OperatorNodeInterface> GetObtainedAddressNodePointer()
      const {
    return obtained_address_node_;
  }
  const OperatorNodeInterface& GetObtainedAddressNodeReference() const {
    return *obtained_address_node_;
  }
  static bool CheckNodeToObtainAddress(
      const OperatorNodeInterface& node_interface);

 private:
  void SetNodeObtainedAddress(
      const std::shared_ptr<const OperatorNodeInterface>&
          node_obtained_address) {
    obtained_address_node_ = node_obtained_address;
  }

  // ��Ҫ��ȡ��ַ�Ľڵ�
  std::shared_ptr<const VarietyOperatorNode> node_to_obtain_address_;
  // ȡ��ַ���õĽڵ�
  std::shared_ptr<const OperatorNodeInterface> obtained_address_node_;
};

class MemberAccessOperatorNode : public OperatorNodeInterface {
 public:
  using MemberIndex =
      c_parser_frontend::type_system::StructureTypeInterface::MemberIndex;
  MemberAccessOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kMemberAccess) {}
  MemberAccessOperatorNode(const MemberAccessOperatorNode&) = delete;

  MemberAccessOperatorNode& operator=(const MemberAccessOperatorNode&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    return GetAccessedNodeReference().GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetAccessedNodeReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetAccessedNodePointer();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // ���ظ����ڵ��Ƿ������Ϊ�����ʳ�Ա�Ľڵ�
  // ����������򲻻�����
  bool SetNodeToAccess(
      const std::shared_ptr<const OperatorNodeInterface>& node_to_access) {
    if (CheckNodeToAccessValid(*node_to_access)) [[likely]] {
      node_to_access_ = node_to_access;
      return true;
    } else {
      return false;
    }
  }
  std::shared_ptr<const OperatorNodeInterface> GetNodeToAccessPointer() {
    return node_to_access_;
  }
  const OperatorNodeInterface& GetNodeToAccessReference() const {
    return *node_to_access_;
  }
  // ���ظ����ڵ��Ƿ��ǳ�Ա��
  // ��������򲻻�����
  // ���������ýڵ㣬������Ҫ���ʵĳ�Ա��
  template <class MemberName>
  bool SetMemberName(MemberName&& member_name) {
    return SetAccessedNodeAndMemberName(
        std::string(std::forward<MemberName>(member_name)));
  }
  // ��ȡҪ���ʵĳ�Ա��
  MemberIndex GetMemberIndex() const { return member_index_; }
  // ��ȡҪ���ʵĳ�Ա����
  // ����ɹ�����Ҫ���ʵĽڵ�ͳ�Ա��
  std::shared_ptr<const OperatorNodeInterface> GetAccessedNodePointer() const {
    return node_accessed_;
  }
  const OperatorNodeInterface& GetAccessedNodeReference() const {
    return *node_accessed_;
  }

  // �������ڵ��Ƿ������Ϊ�����ʵĽڵ�
  static bool CheckNodeToAccessValid(
      const OperatorNodeInterface& node_to_access);

 private:
  // ���ݴ����ʵĽڵ�ͽڵ��Ա�����㲢���÷��ʵĽڵ��Ա���ͺ�const����
  // ����ɹ�����������Ҫ���ʵĽڵ��Ա��
  // ��������ڵ㲻���ڸ����ĳ�Ա���򷵻�false�Ҳ����κθ���
  bool SetAccessedNodeAndMemberName(std::string&& member_name_to_set);
  void SetAccessedNode(
      const std::shared_ptr<OperatorNodeInterface>& node_accessed) {
    node_accessed_ = node_accessed;
  }

  // Ҫ���ʵĽڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_access_;
  // Ҫ���ʵĽڵ��Ա��index��ö�����Ͳ����ø���
  MemberIndex member_index_;
  // ���ʺ�õ��Ľڵ�
  std::shared_ptr<OperatorNodeInterface> node_accessed_;
};

class FunctionCallOperatorNode : public OperatorNodeInterface {
  // �洢��������ʱ����������
  class FunctionCallArgumentsContainer {
   public:
    FunctionCallArgumentsContainer();
    ~FunctionCallArgumentsContainer();

    using ContainerType = std::list<
        std::pair<std::shared_ptr<const OperatorNodeInterface>,
                  std::shared_ptr<std::list<std::unique_ptr<
                      c_parser_frontend::flow_control::FlowInterface>>>>>;
    // ����������Ч�ԣ��������ںϲ���������ʱ����
    void AddFunctionCallArgument(
        const std::shared_ptr<const OperatorNodeInterface>& argument,
        const std::shared_ptr<std::list<
            std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
            flow_control_node_container) {
      function_call_arguments_.emplace_back(
          std::make_pair(argument, flow_control_node_container));
    }
    void AddFunctionCallArgument(
        std::pair<const std::shared_ptr<const OperatorNodeInterface>,
                  const std::shared_ptr<std::list<std::unique_ptr<
                      c_parser_frontend::flow_control::FlowInterface>>>>&&
            argument_data) {
      function_call_arguments_.emplace_back(std::move(argument_data));
    }
    const ContainerType& GetFunctionCallArguments() const {
      return function_call_arguments_;
    }

   private:
    // �������GetFunctionCallArgumentsNotConst()
    friend FunctionCallOperatorNode;

    ContainerType& GetFunctionCallArguments() {
      return function_call_arguments_;
    }

    ContainerType function_call_arguments_;
  };

 public:
  FunctionCallOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kFunctionCall) {}
  FunctionCallOperatorNode(const FunctionCallOperatorNode&) = delete;

  FunctionCallOperatorNode& operator=(const FunctionCallOperatorNode&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetReturnObjectReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetReturnObjectPointer();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  // ����Ҫ���õĶ���
  // �����ò���ǰ����Ҫ���õĶ���
  // ����������ΪStructOrBasicType::kFunction
  bool SetFunctionObjectToCall(
      const std::shared_ptr<const OperatorNodeInterface>&
          function_object_to_call);
  std::shared_ptr<const c_parser_frontend::type_system::FunctionType>
  GetFunctionTypePointer() const {
    return function_type_;
  }
  const c_parser_frontend::type_system::FunctionType& GetFunctionTypeReference()
      const {
    return *function_type_;
  }
  std::shared_ptr<const VarietyOperatorNode> GetReturnObjectPointer() const {
    return return_object_;
  }
  const VarietyOperatorNode& GetReturnObjectReference() const {
    return *return_object_;
  }
  // ��ȡ�������ṩ��ԭʼ��������������д˳������
  const auto& GetFunctionArgumentsOfferred() const {
    return function_arguments_offerred_;
  }
  // ���һ���������������˳�������
  // ��������Ϊ����ӵĲ����ڵ�ͻ�ȡ�����ڵ�Ĳ���
  // ���ش���ӵĲ����Ƿ�ͨ�����飬δͨ�������򲻻����
  // �����ò���ǰ����Ҫ���õĶ���
  AssignableCheckResult AddFunctionCallArgument(
      const std::shared_ptr<const OperatorNodeInterface>& argument_node,
      const std::shared_ptr<std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          sentences_to_get_argument);
  // ���һ�����������в������������˳��Ϊbegin()��end()
  // �����Ƿ�ɹ���ӣ����в���������Ҫ��
  // ������ʧ���򲻻��޸Ĳ���
  // �����ò���ǰ����Ҫ���õĶ���
  bool SetArguments(FunctionCallArgumentsContainer&& container);

  static bool CheckFunctionTypeValid(const TypeInterface& type_interface) {
    return type_interface.GetType() == StructOrBasicType::kFunction;
  }

 private:
  FunctionCallArgumentsContainer& GetFunctionArgumentsOfferred() {
    return function_arguments_offerred_;
  }

  // �������صĶ���
  std::shared_ptr<const VarietyOperatorNode> return_object_;
  // �����õĶ���
  std::shared_ptr<const c_parser_frontend::type_system::FunctionType>
      function_type_;
  // �������ṩ��ԭʼ�����������ȡ�����Ĳ���������д˳������
  FunctionCallArgumentsContainer function_arguments_offerred_;
};

}  // namespace c_parser_frontend::operator_node

#endif  // !CPARSERFRONTEND_OPERATOR_NODE_H_