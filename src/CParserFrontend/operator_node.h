#ifndef CPARSERFRONTEND_OPERATOR_NODE_H_
#define CPARSERFRONTEND_OPERATOR_NODE_H_
#include "Common/id_wrapper.h"
#include "type_system.h"
// �����ڵ�
namespace c_parser_frontend::operator_node {
// ����ڵ����
enum class GeneralOperationType {
  kAllocate,               // ����ռ�
  kVariety,                // ��������
  kInitValue,              // ��ʼ����ֵ
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
  kPlus,        // +���ӣ�
  kMinus,       // -������
  kMultiple,    // *���ˣ�
  kDivide,      // /������
  kMod,         // %����ģ��
  kLeftShift,   // <<
  kRightShift,  // >>
  kAnd,         // &
  kOr,          // |
  kNot,         // !
  kXor,         // ^
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

class OperatorNodeInterface {
  enum class IdWrapper { kOperatorNodeId };

 public:
  // �����ڵ�ID
  using OperatorId =
      frontend::common::ExplicitIdWrapper<size_t, IdWrapper,
                                          IdWrapper::kOperatorNodeId>;
  OperatorNodeInterface() : operator_id_(GetOperatorId()) {}
  OperatorNodeInterface(GeneralOperationType general_operator_type)
      : operator_id_(GetNewOperatorId()),
        general_operator_type_(general_operator_type) {}
  virtual ~OperatorNodeInterface();

  // ��ȡ�ýڵ�����ConstTag
  virtual ConstTag GetResultConstTag() const = 0;
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer() const = 0;

  void SetOperatorId(OperatorId operator_id) { operator_id_ = operator_id; }
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
    static OperatorId operator_id(0);
    return operator_id++;
  }

  // �����ڵ�ID��ͬʱ��Ϊ��Ž����LLVM�ļĴ������
  OperatorId operator_id_;
  // ��ģ�������������
  GeneralOperationType general_operator_type_;
};

class AllocateOperatorNode : public OperatorNodeInterface {
 public:
  AllocateOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kAllocate) {}

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

  // ����Ҫ�����ڴ�Ľڵ�
  // �����Ƿ���Է��䣬���ܷ����򲻻�����
  bool SetTargetVariety(
      std::shared_ptr<const OperatorNodeInterface>&& target_variety);
  // ��������ΪҪ��ӵ�ά���Ĵ�С
  // ������ά����Сǰ��������Ҫ�����ڴ�Ľڵ�
  // ���˳��ͬ����˳�����������������
  // ����������Ͳ�֧�ֶ����һά�򷵻�false
  // �����鲻Ҫ���ã���ʱnum_to_allocate_.size() == 0
  bool AddNumToAllocate(size_t num);
  const std::list<size_t>& GetNumToAllocate() const { return num_to_allocate_; }
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
  // ������ռ�Ĵ�С������ѹ�뿿��������һ�˵Ĵ�С
  std::list<size_t> num_to_allocate_;
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

  virtual ConstTag GetResultConstTag() const override { return GetConstTag(); }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return variety_type_;
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
  bool SetVarietyType(std::shared_ptr<const TypeInterface>&& variety_type);

  // �����������Ƿ������Ϊ����������
  static bool CheckVarietyTypeValid(const TypeInterface& variety_type);

 private:
  // ������
  const std::string* variety_name_;
  // ��������
  std::shared_ptr<const TypeInterface> variety_type_;
  // ���������const���
  ConstTag variety_const_tag_;
  // ����������ֵ���
  LeftRightValueTag variety_left_right_value_tag_;
};

// �洢��ʼ�����ݣ������ڳ�����
class InitializeOperatorNode : public OperatorNodeInterface {
 public:
  InitializeOperatorNode(InitializeType initialize_type)
      : OperatorNodeInterface(GeneralOperationType::kInitValue),
        initialize_type_(initialize_type) {}

  virtual ConstTag GetResultConstTag() const override {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return initialize_value_type_;
  }

  void SetInitlizeType(InitializeType initialize_type) {
    initialize_type_ = initialize_type;
  }
  InitializeType GetInitializeType() const { return initialize_type_; }
  // ���øýڵ������
  // �����Ƿ�Ϊ��Ч�ĳ�ʼ���������ͣ������Ч�����
  bool SetInitValueType(std::shared_ptr<const TypeInterface>&& init_value_type);
  // �����Ƿ�Ϊ��Ч�ĳ�ʼ����������
  static bool CheckInitValueTypeValid(const TypeInterface& init_value_type);

 private:
  // ��ʼ����������
  std::shared_ptr<const TypeInterface> initialize_value_type_;
  // ��ʼ����������
  InitializeType initialize_type_;
};

// �������ͣ���ֵ/�ַ�����
class BasicTypeInitializeOperatorNode : public InitializeOperatorNode {
 public:
  template <class Value>
  BasicTypeInitializeOperatorNode(InitializeType initialize_type, Value&& value)
      : InitializeOperatorNode(initialize_type),
        value_(std::forward<Value>(value)) {
    assert(initialize_type == InitializeType::kBasic ||
           initialize_type == InitializeType::kString);
  }

  virtual ConstTag GetResultConstTag() const override {
    return InitializeOperatorNode::GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return InitializeOperatorNode::GetResultTypePointer();
  }

  template <class Value>
  void SetValue(Value&& value) {
    value_ = std::forward<Value>(value);
  }
  const std::string& GetValue() const { return value_; }
  // ���ó�ʼ�����������ͣ���������/const char*��
  bool SetInitDataType(std::shared_ptr<const TypeInterface>&& data_type);
  // �ö����Ƿ����ʹ�ø���������
  // ��Ч���ͷ���true����Ч���ͷ���false
  static bool CheckBasicTypeInitializeValid(const TypeInterface& variety_type);

 private:
  // �ڸǺ�������ֹ����
  bool SetInitValueType(std::shared_ptr<const TypeInterface>&& init_value_type);
  // �ַ�����ʽ�洢�Է�������ʧ
  std::string value_;
};

// ��ʼ���б�
class ListInitializeOperatorNode : public InitializeOperatorNode {
  ListInitializeOperatorNode()
      : InitializeOperatorNode(InitializeType::kInitializeList) {}

  virtual ConstTag GetResultConstTag() const override {
    return InitializeOperatorNode::GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return InitializeOperatorNode::GetResultTypePointer();
  }

  // ���ʼ���б�����ӳ�ʼ��ֵ
  // ����ֵ�Ƿ�Ϊ��Ч�ĳ�ʼ���б�������ͣ����������Ч��ִ�в������
  // ֵ������˳�����
  bool AddListValue(std::shared_ptr<const InitializeOperatorNode>&& list_value);
  const std::vector<std::shared_ptr<const InitializeOperatorNode>>&
  GetListValues() const {
    return list_values_;
  }
  bool SetInitListType(std::shared_ptr<const TypeInterface>&& list_type);
  // �������ĳ�ʼ���б������Ƿ�Ϸ�
  static bool CheckInitListTypeValid(const TypeInterface& list_type) {
    return list_type.GetType() == StructOrBasicType::kInitializeList;
  }
  // �������ĳ�ʼ��ֵ�Ƿ�Ϸ�
  static bool CheckInitListValueTypeValid(
      const TypeInterface& list_value_type) {
    return InitializeOperatorNode::CheckInitValueTypeValid(list_value_type);
  }

 private:
  // ��ʼ���б��е�ֵ
  std::vector<std::shared_ptr<const InitializeOperatorNode>> list_values_;
};

// ��ֵ�ڵ�
class AssignOperatorNode : public OperatorNodeInterface {
 public:
  AssignOperatorNode() : OperatorNodeInterface(GeneralOperationType::kAssign) {}

  virtual ConstTag GetResultConstTag() const override {
    return GetNodeToBeAssignedReference().GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    // ��ֵ���ȼ��ڱ���ֵ�ı���
    return GetNodeToBeAssignedReference().GetResultTypePointer();
  }

  void SetNodeToBeAssigned(
      std::shared_ptr<const OperatorNodeInterface>&& node_to_be_assigned) {
    node_to_be_assigned_ = std::move(node_to_be_assigned);
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
  // ����ʱ��ֵ����Ա���ֵ�Ľڵ������const����
  // ��Ҫ�����ñ���ֵ�Ľڵ�
  AssignableCheckResult SetNodeForAssign(
      std::shared_ptr<const OperatorNodeInterface>&& node_for_assign,
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
  // ��is_announce == trueʱ����node_to_be_assigned��const���
  static AssignableCheckResult CheckAssignable(
      const OperatorNodeInterface& node_to_be_assigned,
      const OperatorNodeInterface& node_for_assign, bool is_announce);

 private:
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

  virtual ConstTag GetResultConstTag() const override {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetComputeResultTypeReference().GetResultTypePointer();
  }

  void SetMathematicalOperation(MathematicalOperation mathematical_operation) {
    mathematical_operation_ = mathematical_operation;
  }
  MathematicalOperation GetMathematicalOperation() const {
    return mathematical_operation_;
  }
  // �������������ͺ����ýڵ�
  bool SetLeftOperatorNode(
      std::shared_ptr<const OperatorNodeInterface>&& left_operator_node);
  std::shared_ptr<const OperatorNodeInterface> GetLeftOperatorNodePointer()
      const {
    return left_operator_node_;
  }
  const OperatorNodeInterface& GetLeftOperatorNodeReference() const {
    return *left_operator_node_;
  }
  // ��������ڵ�������ҽڵ�
  DeclineMathematicalComputeTypeResult SetRightOperatorNode(
      std::shared_ptr<const OperatorNodeInterface>&& right_operator_node);
  std::shared_ptr<const OperatorNodeInterface> GetRightOperatorNodePointer()
      const {
    return right_operator_node_;
  }
  const OperatorNodeInterface& GetRightOperatorNodeReference() const {
    return *right_operator_node_;
  }
  std::shared_ptr<const VarietyOperatorNode> GetComputeResultTypePointer()
      const {
    return compute_result_node_;
  }
  const VarietyOperatorNode& GetComputeResultTypeReference() const {
    return *compute_result_node_;
  }

  // ������������������������������ľ������
  // ������������򷵻�nullptr
  static std::pair<std::shared_ptr<const VarietyOperatorNode>,
                   DeclineMathematicalComputeTypeResult>
  DeclineComputeResult(
      MathematicalOperation mathematical_operation,
      std::shared_ptr<const OperatorNodeInterface>&& left_operator_node,
      std::shared_ptr<const OperatorNodeInterface>&& right_operator_node);

 private:
  void SetComputeResultNode(
      std::shared_ptr<const VarietyOperatorNode>&& compute_result_node) {
    compute_result_node_ = std::move(compute_result_node);
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

  virtual ConstTag GetResultConstTag() const override {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetComputeResultNodeReference().GetResultTypePointer();
  }

  LogicalOperation GetLogicalOperation() const { return logical_operation_; }
  bool SetLeftOperatorNode(
      std::shared_ptr<const OperatorNodeInterface>&& left_operator_node);
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
      std::shared_ptr<const OperatorNodeInterface>&& right_operator_node);
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
        CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kBool,
                                                SignTag::kUnsigned>());
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
  DereferenceOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kDeReference) {}

  virtual ConstTag GetResultConstTag() const override {
    return GetDereferencedObjectConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetDereferencedNodeReference().GetResultTypePointer();
  }

  // �趨�������õĽڵ�
  // ��������Ϊ�������õĽڵ�
  // ���������Ƿ�ɹ�����
  bool SetNodeToDereference(
      std::shared_ptr<const OperatorNodeInterface>&& node_to_dereference);

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
      const OperatorNodeInterface& node_to_dereference) {
    // ���ҽ���Ϊָ��ʱ���Խ�����
    return node_to_dereference.GetResultTypePointer()->GetType() ==
           StructOrBasicType::kPointer;
  }

 private:
  void SetDereferencedNode(
      std::shared_ptr<const VarietyOperatorNode>&& dereferenced_node) {
    dereferenced_node_ = std::move(dereferenced_node);
  }

  // ��Ҫ�������õĽڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_dereference_;
  // �����ú�õ��Ķ���Ľڵ�
  std::shared_ptr<const VarietyOperatorNode> dereferenced_node_;
};

class ObtainAddress : public OperatorNodeInterface {
  ObtainAddress()
      : OperatorNodeInterface(GeneralOperationType::kObtainAddress) {}

  virtual ConstTag GetResultConstTag() const override {
    // ȡ��ַ��õ�ֵ���Ƿ�const
    return ConstTag::kNonConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetObtainedAddressNodeReference().GetResultTypePointer();
  }

  // �����Ƿ�������ã�������������򲻻�����
  bool SetNodeToObtainAddress(
      std::shared_ptr<const OperatorNodeInterface>&& node_to_obtain_address);

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
      std::shared_ptr<const OperatorNodeInterface>&& node_obtained_address) {
    obtained_address_node_ = std::move(node_obtained_address);
  }

  // ��Ҫ��ȡ��ַ�Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_obtain_address_;
  // ȡ��ַ���õĽڵ�
  std::shared_ptr<const OperatorNodeInterface> obtained_address_node_;
};

class MemberAccessOperatorNode : public OperatorNodeInterface {
  MemberAccessOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kMemberAccess) {}

  virtual ConstTag GetResultConstTag() const override {
    return GetAccessedNodeReference().GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetAccessedNodeReference().GetResultTypePointer();
  }

  // ���ظ����ڵ��Ƿ������Ϊ�����ʳ�Ա�Ľڵ�
  // ����������򲻻�����
  bool SetNodeToAccess(
      std::shared_ptr<const OperatorNodeInterface>&& node_to_access) {
    if (CheckNodeToAccessValid(*node_to_access)) [[likely]] {
      node_to_access_ = std::move(node_to_access);
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
  const std::string& GetMemberName() const { return member_name_; }
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
      std::shared_ptr<const OperatorNodeInterface>&& node_accessed) {
    node_accessed_ = std::move(node_accessed);
  }

  // Ҫ���ʵĽڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_access_;
  // Ҫ���ʵĽڵ��Ա��
  std::string member_name_;
  // ���ʺ�õ��Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> node_accessed_;
};

class FunctionCallOperatorNode : public OperatorNodeInterface {
  FunctionCallOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kFunctionCall) {}

  virtual ConstTag GetResultConstTag() const override {
    return ConstTag::kConst;
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    return GetReturnObjectReference().GetResultTypePointer();
  }

  bool SetFunctionType(std::shared_ptr<const TypeInterface>&& type_pointer);
  std::shared_ptr<const TypeInterface> GetFunctionTypePointer() const {
    return function_type_;
  }
  const TypeInterface& GetFunctionTypeReference() const {
    return *function_type_;
  }
  std::shared_ptr<const VarietyOperatorNode> GetReturnObjectPointer() const {
    return return_object_;
  }
  const VarietyOperatorNode& GetReturnObjectReference() const {
    return *return_object_;
  }
  // ��ȡ�����ṩ�������Ĳ����������뺯���������������ȫƥ�䣩������д˳������
  const auto& GetFunctionArgumentsForCall() const {
    return function_arguments_for_call_;
  }
  // ��ȡ�������ṩ��ԭʼ��������������д˳������
  const auto& GetFunctionArgumentsOfferred() const {
    return function_arguments_offerred_;
  }
  // ���һ���������������˳�������
  // ���ش���ӵĲ����Ƿ�ͨ�����飬δͨ�������򲻻����
  AssignableCheckResult AddArgument(
      std::shared_ptr<const OperatorNodeInterface>&& argument_node);
  static bool CheckFunctionTypeValid(const TypeInterface& type_interface) {
    return type_interface.GetType() == StructOrBasicType::kFunction;
  }

 private:
  std::vector<std::shared_ptr<const OperatorNodeInterface>>&
  GetFunctionArgumentsForCall() {
    return function_arguments_for_call_;
  }
  std::vector<std::shared_ptr<const OperatorNodeInterface>>&
  GetFunctionArgumentsOfferred() {
    return function_arguments_offerred_;
  }

  // �������صĶ���
  std::shared_ptr<const VarietyOperatorNode> return_object_;
  // �ú���������
  std::shared_ptr<const c_parser_frontend::type_system::FunctionType>
      function_type_;
  // �������ṩ��ԭʼ��������������д˳������
  std::vector<std::shared_ptr<const OperatorNodeInterface>>
      function_arguments_offerred_;
  // �����ṩ�������Ĳ����������뺯���������������ȫƥ�䣩������д˳������
  std::vector<std::shared_ptr<const OperatorNodeInterface>>
      function_arguments_for_call_;
};

}  // namespace c_parser_frontend::operator_node

#endif  // !CPARSERFRONTEND_OPERATOR_NODE_H_