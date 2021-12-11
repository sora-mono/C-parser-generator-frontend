/// @file operator_node.h
/// @brief �����ڵ�
/// @details
/// ���ļ��ڶ����ʾ���������Ľڵ㣬֧�ֵ���ϸ�����ο�GeneralOperationType
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
/// @brief �������ʹ���
enum class GeneralOperationType {
  kAllocate,               ///< ����ռ�
  kTypeConvert,            ///< ����ת��
  kVariety,                ///< ��������
  kInitValue,              ///< ��ʼ����ֵ
  kTemaryOperator,         ///< ?:����Ŀ�������
  kAssign,                 ///< =����ֵ��
  kMathematicalOperation,  ///< ��ѧ����
  kLogicalOperation,       ///< �߼�����
  kDeReference,            ///< ������
  kObtainAddress,          ///< ȡ��ַ
  kMemberAccess,           ///< ��Ա����
  kFunctionCall            ///< ��������
};
/// @brief ��ѧ�����
enum class MathematicalOperation {
  kOr,                    ///< |����λ��
  kXor,                   ///< ^����λ���
  kAnd,                   ///< &����λ�룩
  kLeftShift,             ///< <<�����ƣ�
  kRightShift,            ///< >>�����ƣ�
  kPlus,                  ///< +���ӣ�
  kMinus,                 ///< -������
  kMultiple,              ///< *���ˣ�
  kDivide,                ///< /������
  kMod,                   ///< %����ģ��
  kLogicalNegative,       ///< ~����λȡ����
  kMathematicalNegative,  ///< -��ȡ����
  kNot,                   ///< !���߼��ǣ�
};
/// @brief ��ѧ�븳ֵ�����
enum class MathematicalAndAssignOperation {
  kOrAssign,          ///< |=
  kXorAssign,         ///< ^=
  kAndAssign,         ///< &=
  kLeftShiftAssign,   ///< <<=
  kRightShiftAssign,  ///< >>=
  kPlusAssign,        ///< +=
  kMinusAssign,       ///< -=
  kMultipleAssign,    ///< *=
  kDivideAssign,      ///< /=
  kModAssign,         ///< %=
};
/// @brief �߼������
enum class LogicalOperation {
  kAndAnd,        ///< &&
  kOrOr,          ///< ||
  kGreater,       ///< >
  kGreaterEqual,  ///< >=
  kLess,          ///< <
  kLessEqual,     ///< <=
  kEqual,         ///< ==
  kNotEqual       ///< !=
};
/// @brief ��ʼ��ֵ����
enum class InitializeType {
  kBasic,     ///< �������ͣ���������ȡ����ֵ�ķ�Χ
  kString,    ///< �ַ������̶�Ϊconst char*����
  kFunction,  ///< ���ڸ�����ָ�븳ֵ��ȫ�ֳ�ʼ����������
  kInitializeList  ///< ��ʼ���б�
};
/// @brief ��ֵ��ֵ���
enum class LeftRightValueTag {
  kLeftValue,  ///< ��ֵ
  kRightValue  ///< ��ֵ
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

/// @brief ����ѧ�븳ֵ�����ת��Ϊ��ѧ�����
/// @param[in] mathematical_and_assign_operation ����ѧ�븳ֵ�����
/// @return ���ض�Ӧ����ѧ�����
MathematicalOperation MathematicalAndAssignOperationToMathematicalOperation(
    MathematicalAndAssignOperation mathematical_and_assign_operation);

/// @class OperatorNodeInterface operator_node.h
/// @brief �����ڵ����
/// @note ���в����ڵ���Ӹ�������
class OperatorNodeInterface {
  /// @brief �����ڵ�ID�ķַ���ǩ
  enum class IdWrapper { kOperatorNodeId };

 public:
  /// @brief �����ڵ�ID
  using OperatorId =
      frontend::common::ExplicitIdWrapper<size_t, IdWrapper,
                                          IdWrapper::kOperatorNodeId>;

  OperatorNodeInterface(GeneralOperationType general_operator_type)
      : general_operator_type_(general_operator_type) {}
  /// @attention ���ƹ��캯�������ƽڵ��Ŷ��������µı��
  OperatorNodeInterface(const OperatorNodeInterface& operator_node)
      : general_operator_type_(operator_node.general_operator_type_) {}
  virtual ~OperatorNodeInterface() {}

  OperatorNodeInterface& operator=(const OperatorNodeInterface& old_interface) {
    general_operator_type_ = old_interface.general_operator_type_;
    return *this;
  }

  /// @brief ��ȡ�ýڵ���������const���
  /// @return ���ز��������const���
  /// @details
  /// 1.��Ա���ʡ������õȲ����ĺ����������Ƕ�ԭ�ṹ��/ԭָ����������ǶԵõ���
  ///   �¶�����в�������ͬ�������Ͳ����Ķ���ͬ
  /// 2.ͨ��������������ȡ���ս���Ĺ����Ժ��Բ����Ĳ�ͬ
  /// 3.�Գ�Ա����Ϊ�����ú�����ȡָ���ĳ�Ա�ڽṹ���е�const��ǣ�����ѧ����
  ///   Ϊ�����ú�����ȡ��������const��ǣ�ConstTag::kNonConst�����Ա�������
  ///   Ϊ�����ú������������ı�����const��ǣ����󲻱䣩
  virtual ConstTag GetResultConstTag() const = 0;
  /// @brief ��ȡ���ս��������
  /// @return ����������ͷ����constָ��
  /// @details
  /// ���˼·ͬGetResultConstTag
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer() const = 0;
  /// @brief ��ȡ���ս���Ľڵ�
  /// @return ����ָ�����ս����ָ��
  /// @retval nullptr �����������սڵ�
  /// @details
  /// 1.�ú������ڳ����ȡ�������ս���Ĺ���
  /// 2.���������ͳ�ʼ���������ղ����ڵ���Ǳ������Է���nullptr
  /// 3.��ֵ���㷵�ر���ֵ�Ķ��󣬽����÷��ؽ����ú�õ��Ķ����Դ�Ϊ��
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const = 0;
  /// @brief �����������ÿ����õ��Ķ�������Ϊnew_type
  /// @param[in] new_type ��������õ��Ķ��������
  /// @param[in] new_const_tag ��������õ��Ķ������const���
  /// @return ����ָ�򿽱���Ķ����ָ��
  /// @retval nullptr �����ó�ʼ������Ϊ��const
  /// @details
  /// �ú�����������ת��
  /// @attention ������Ա�������ʼ��ֵ������ת���ڵ���øú���
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const = 0;

  /// @brief ��ȡ�����ڵ�ID
  /// @return ���ز����ڵ�ID
  OperatorId GetOperatorId() const { return operator_id_; }
  /// @brief ���ò�������
  /// @param[in] operator_type ����������
  void SetGeneralOperatorType(GeneralOperationType operator_type) {
    general_operator_type_ = operator_type;
  }
  /// @brief ��ȡ����������������
  /// @return ���ز���������������
  GeneralOperationType GetGeneralOperatorType() const {
    return general_operator_type_;
  }

 private:
  /// @brief ��ȡһ����δ���������OperatorId
  /// @return ����OperatorId
  /// @note �̼߳�OperatorId����
  static OperatorId GetNewOperatorId() {
    static thread_local OperatorId operator_id(0);
    return operator_id++;
  }

  /// @brief �����ڵ�ID��ͬʱ��Ϊ��Ž����LLVM�ļĴ������
  const OperatorId operator_id_ = GetOperatorId();
  /// @brief ������������
  GeneralOperationType general_operator_type_;
};

/// @class AllocateOperatorNode operator_node.h
/// @brief ����ջ�Ͽռ�
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ����Ҫ�����ڴ�Ľڵ�
  /// @param[in] target_variety ���������ڴ�Ľڵ�
  /// @return �����Ƿ���Է���
  /// @retval true �����óɹ�
  /// @retval false �����ܷ���ñ���
  /// @note ���ܷ����򲻻�����
  /// @attention ֻ��GeneralOperatorType::kVariety���ͽڵ������Ϊ����
  bool SetTargetVariety(
      const std::shared_ptr<const OperatorNodeInterface>& target_variety);
  /// @brief ��ȡҪ�����ڴ�ı����ڵ�
  /// @return ����ָ������ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetTargetVarietyPointer() const {
    return target_variety_;
  }
  /// @brief ��ȡҪ�����ڴ�ı����ڵ�
  /// @return ����ָ������ڵ��const����
  const OperatorNodeInterface& GetTargetVarietyReference() const {
    return *target_variety_;
  }

  /// @brief �������ڵ��Ƿ���Է���ջ�Ͽռ�
  /// @return �����Ƿ���Է���
  /// @retval true �����Է���
  /// @retval false �������Է���
  /// @attention ��������GeneralOperatorType::kVariety���ͽڵ�
  static bool CheckAllocatable(const OperatorNodeInterface& node_to_allocate) {
    assert(node_to_allocate.GetGeneralOperatorType() ==
           GeneralOperationType::kVariety);
    return true;
  }

 private:
  /// @brief ������ջ�Ͽռ�Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> target_variety_;
};

/// @class TypeConvert operator_node.h
/// @brief ����ת��
/// @note Դ�ڵ���Ҫ֧��GetResultOperatorNode()
class TypeConvert : public OperatorNodeInterface {
 public:
  /// @note ����ʹ���������������Ľڵ���Ϊsource_node
  /// source_node����֧��SelfCopy
  TypeConvert(const std::shared_ptr<const OperatorNodeInterface>& source_node,
              const std::shared_ptr<const TypeInterface>& new_type,
              ConstTag new_const_tag)
      : OperatorNodeInterface(GeneralOperationType::kTypeConvert),
        source_node_(source_node) {
    GenerateDestinationNode(new_type, new_const_tag);
  }
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
  /// @attention ���ܿ�������ת���ڵ㣬Ӧ�ÿ���destination_node_
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ��ȡ��ת���Ľڵ�
  /// @return ����ָ��ת���ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetSourceNodePointer() const {
    return source_node_;
  }
  /// @brief ��ȡ��ת���Ľڵ�
  /// @return ���ر�ת���ڵ��const����
  const OperatorNodeInterface& GetSourceNodeReference() const {
    return *source_node_;
  }
  /// @brief ��ȡת����Ľڵ�
  /// @return ����ָ��ת����ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetDestinationNodePointer()
      const {
    return destination_node_;
  }
  /// @brief ��ȡת����Ľڵ�
  /// @return ����ת����ڵ��const����
  const OperatorNodeInterface& GetDestinationNodeReference() const {
    return *destination_node_;
  }
  /// @brief ���ô�ת���Ľڵ�
  /// @param[in] source_node ����ת���Ľڵ�
  /// @note �����µĴ�ת���ڵ��Ҫ����GenerateDestinationNode������ת����Ľڵ�
  void SetSourceNode(
      const std::shared_ptr<const OperatorNodeInterface>& source_node) {
    source_node_ = source_node;
  }
  /// @brief ����ת����Ľڵ�
  /// @param[in] new_type ���½ڵ������
  /// @param[in] new_const_tag ���½ڵ��const���
  /// @return �����Ƿ����ת��
  /// @retval true ������ת��
  /// @retval false ��ת����ʼ������Ϊ��const
  /// @note ����ת���򲻻�����Ŀ��ڵ�
  bool GenerateDestinationNode(
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag);

 private:
  /// @brief ��ת���Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> source_node_;
  /// @brief ת����Ľڵ�
  std::shared_ptr<OperatorNodeInterface> destination_node_;
};

/// @class VarietyOperatorNode operator_node.h
/// @brief ��ʾ����
class VarietyOperatorNode : public OperatorNodeInterface {
 public:
  VarietyOperatorNode(const std::string& variety_name, ConstTag const_tag,
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override;

  /// @brief ���ñ�����
  /// @param[in] variety_name ��������
  void SetVarietyName(const std::string& variety_name) {
    variety_name_ = variety_name;
  }
  /// @brief ��ȡ������
  /// @return ���ر�������const����
  const std::string& GetVarietyName() const { return variety_name_; }
  /// @brief ���ñ�����const���
  /// @param[in] const_tag ��const���
  void SetConstTag(ConstTag const_tag) { variety_const_tag_ = const_tag; }
  /// @brief ��ȡ������const���
  /// @return ���ر�����const���
  /// @retval ConstTag::kConst �������������޸�
  /// @retval ConstTag::kNonConst �����������޸�
  ConstTag GetConstTag() const { return variety_const_tag_; }
  /// @brief ���ñ�������ֵ���
  /// @param[in] left_right_value_tag ������ֵ���
  void SetLeftRightValueTag(LeftRightValueTag left_right_value_tag) {
    variety_left_right_value_tag_ = left_right_value_tag;
  }
  /// @brief ��ȡ����������ֵ���
  /// @return ��������ֵ���
  /// @retval LeftRightValueTag::kLeftValue ����ֵ
  /// @retval LeftRightValueTag::kRightValue ����ֵ
  LeftRightValueTag GetLeftRightValueTag() const {
    return variety_left_right_value_tag_;
  }
  /// @brief ���ñ���������
  /// @param[in] variety_type ��������ͷ���constָ��
  /// @return �����Ƿ�������ø�������
  /// @retval true ���������ø�������
  /// @retval false �����������ø�������
  /// @note ��������ʹ�õ����Ͳο�CheckVarietyTypeValid
  bool SetVarietyType(const std::shared_ptr<const TypeInterface>& variety_type);
  /// @brief ���ñ��������ͣ�����ʹ�ú�������
  /// @param[in] variety_type ��������ͷ���constָ��
  /// @return �����Ƿ�������ø�������
  /// @retval true ���������ø�������
  /// @retval false �����������ø�������
  /// @details
  /// 1.��������������򲻻�����
  /// 2.�����SetVarietyType������ʹ��FunctionType��Ϊ��������
  /// 3.�ú��������ڳ�ʼ���б�ֵʱ�����������VarietyType�ͽ�����
  bool SetVarietyTypeNoCheckFunctionType(
      const std::shared_ptr<const TypeInterface>& variety_type);
  /// @brief ��ȡ����������
  /// @return ����ָ�����������ͷ����constָ��
  std::shared_ptr<const TypeInterface> GetVarietyTypePointer() const {
    return variety_type_;
  }
  /// @brief ��ȡ����������
  /// @return ���ر���������ͷ����const����
  const TypeInterface& GetVarietyTypeReference() const {
    return *variety_type_;
  }

  /// @brief �����������Ƿ������Ϊ��ͨ����������
  /// @param[in] variety_type ������������
  /// @return ���ظ��������Ƿ������Ϊ��ͨ��������
  /// @retval true ���������Ϳ�����Ϊ��ͨ��������
  /// @retval false ������������StructOrBasicType::kFunction��
  /// StructOrBasicType::kInitializeList
  /// @details
  /// ����������ʹ��
  /// StructOrBasicType::kBasic
  /// StructOrBasicType::kPointer
  /// StructOrBasicType::kStruct
  /// StructOrBasicType::kUnion
  /// StructOrBasicType::kEnum
  static bool CheckVarietyTypeValid(const TypeInterface& variety_type);

 private:
  /// @brief ������
  std::string variety_name_;
  /// @brief ��������
  std::shared_ptr<const TypeInterface> variety_type_;
  /// @brief ���������const���
  ConstTag variety_const_tag_;
  /// @brief ����������ֵ���
  LeftRightValueTag variety_left_right_value_tag_;
};

/// @class InitializeOperatorNodeInterface operator_node.h
/// @brief ���г�ʼ�����ݣ������ڳ������Ļ���
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

  /// @brief ���ó�ʼ����������
  /// @param[in] initialize_type ����ʼ����������
  void SetInitializeType(InitializeType initialize_type) {
    initialize_type_ = initialize_type;
  }
  /// @brief ��ȡ��ʼ��������������
  /// @return ���س�ʼ��������������
  /// @note ���������InitializeType����
  InitializeType GetInitializeType() const { return initialize_type_; }
  /// @brief ���ó�ʼ�����ݵľ�������
  /// @param[in] init_value_type ����ʼ�����ݵ�������ͷ���constָ��
  /// @return �����Ƿ���������
  /// @retval true ����������
  /// @retval false �����������ã�������Ч�ĳ�ʼ����������
  /// @details
  /// �Ϸ��ĳ�ʼ�����Ͳο�CheckInitValueTypeValid
  /// @note ��������������򲻻�����
  bool SetInitValueType(
      const std::shared_ptr<const TypeInterface>& init_value_type);
  /// @brief ��ȡ��ʼ�����ݵ�������
  /// @return ����ָ��������ͷ����constָ��
  const std::shared_ptr<const TypeInterface>& GetInitValueType() const {
    return initialize_value_type_;
  }
  /// @brief �����������Ƿ�Ϊ��Ч�ĳ�ʼ����������
  /// @param[in] init_value_type ��������ͷ����const����
  /// @return �����Ƿ�Ϊ��Ч�ĳ�ʼ����������
  /// @retval true ����Ч�ĳ�ʼ����������
  /// @retval false ����Ч�ĳ�ʼ����������
  static bool CheckInitValueTypeValid(const TypeInterface& init_value_type);

 private:
  /// @brief ��ʼ������������
  std::shared_ptr<const TypeInterface> initialize_value_type_;
  /// @brief ��ʼ��������������
  InitializeType initialize_type_;
};

/// @class BasicTypeInitializeOperatorNode operator_node.h
/// @brief ������ʼ�����ͣ���ֵ/�ַ���/������
/// @note ���溯��ʱvalue����
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override;

  /// @brief ���ó�ʼ������ֵ
  /// @param[in] value ���ַ�����ʽ��ʾ�ĳ�ʼ������
  /// @note ʹ���ַ�����ʽ��ʾ�Է�ֹ������ʧ������
  template <class Value>
  void SetValue(Value&& value) {
    value_ = std::forward<Value>(value);
  }
  /// @brief ��ȡ��ʼ�����ݵ�ֵ
  /// @return ���س�ʼ�������ַ�����const����
  const std::string& GetValue() const { return value_; }
  /// @brief ���ó�ʼ�������ݵ�����
  /// @param[in] data_type ����ʼ������������ͷ���ָ��
  /// @note ��������Ͳο�CheckBasicTypeInitializeValid
  bool SetInitDataType(const std::shared_ptr<const TypeInterface>& data_type);
  /// @brief �����������Ƿ������Ϊ������ʼ�����ݵ�����
  /// @param[in] variety_type ������������
  /// @return ���ظ��������Ƿ������Ϊ������ʼ�����ݵ�����
  /// @retval true ����Ч�Ļ�����ʼ����������
  /// @retval false ����Ч�Ļ�����ʼ����������
  /// @note ����ʹ�û�������/const char*/��������
  static bool CheckBasicTypeInitializeValid(const TypeInterface& variety_type);

 private:
  /// @note
  /// 1.�ⲿ����ʱ��Ӧʹ�øú����������ͣ�ʹ��SetInitDataType����
  /// 2.���øú������ó�ʼ���������ͻᵼ������CheckBasicTypeInitializeValid���
  /// 3.����private���ڲ���������
  bool SetInitValueType(
      const std::shared_ptr<const TypeInterface>& init_value_type) {
    return InitializeOperatorNodeInterface::SetInitValueType(init_value_type);
  }

  /// @brief ��ʼ��ֵ
  /// @note ʹ���ַ�����ʽ���⾫����ʧ
  /// �������ͳ�ʼ�����ݸ���ά��ӦĬ��ֵ
  std::string value_;
};

/// @class ListInitializeOperatorNode operator_node.h
/// @brief ��ʼ���б�
class ListInitializeOperatorNode : public InitializeOperatorNodeInterface {
 public:
  ListInitializeOperatorNode()
      : InitializeOperatorNodeInterface(InitializeType::kInitializeList) {}
  ListInitializeOperatorNode(const ListInitializeOperatorNode&) = default;

  ListInitializeOperatorNode& operator=(const ListInitializeOperatorNode&) =
      default;

  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    /// ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ���ó�ʼ���б��к��е�ȫ����ʼ��ֵ
  /// @param[in] list_values ����ʼ���б���ȫ����ʼ��ֵ
  /// @return �����Ƿ�����ֵ��Ϊ�Ϸ��ĳ�ʼ��ֵ
  /// @retval true ��ȫ��ֵ��Ϊ�Ϸ��ĳ�ʼ��ֵ
  /// @retval false �����ڲ��ǺϷ���ʼ��ֵ���͵Ķ���
  /// @note list_values�г�ʼ��ֵ��˳��Ϊ��д˳��
  bool SetListValues(
      std::list<std::shared_ptr<const InitializeOperatorNodeInterface>>&&
          list_values);
  /// @brief ��ȡ��ʼ���б������еĳ�ʼ��ֵ
  /// @return ���ش洢��ʼ���б��г�ʼ��ֵ��������const����
  const std::list<std::shared_ptr<const InitializeOperatorNodeInterface>>&
  GetListValues() const {
    return list_values_;
  }

 private:
  /// @brief ��ʼ���б������еĳ�ʼ��ֵ
  /// @note �����ʼ���б�Ƕ��
  std::list<std::shared_ptr<const InitializeOperatorNodeInterface>>
      list_values_;
};

/// @class TemaryOperatorNode operator_node.h
/// @brief ��Ŀ�����
/// @details
/// ֧��constexpr��Ŀ���������
class TemaryOperatorNode : public OperatorNodeInterface {
 public:
  TemaryOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kTemaryOperator) {}

  virtual ConstTag GetResultConstTag() const override {
    return GetResultReference().GetResultConstTag();
  }
  virtual std::shared_ptr<const TypeInterface> GetResultTypePointer()
      const override {
    /// ��ֵ���ȼ��ڱ���ֵ�ı���
    return GetResultReference().GetResultTypePointer();
  }
  virtual std::shared_ptr<const OperatorNodeInterface> GetResultOperatorNode()
      const override {
    return GetResultReference().GetResultOperatorNode();
  }
  virtual std::shared_ptr<OperatorNodeInterface> SelfCopy(
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    /// ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ���÷�֧����
  /// @param[in] branch_condition ����֧����
  /// @param[in] flow_control_node_container ����ȡ��֧���������в���
  /// @return �����Ƿ��������
  /// @retval true ����������
  /// @retval false ��branch_condition�޷���Ϊ��֧����
  /// @details
  /// 1.��֧������Ҫ��ο�CheckBranchConditionValid
  /// 2.���÷�֧�����������Ҫ�Ķ�����Ҫ��������һ����ٷ�֧����ʱ�Կ���ʹ��Get
  /// ������ȡ֮ǰ�洢�ķ�֧��Ϣ
  /// @note �����ڳ����ķ�֧������������valueΪ"0"��"1"
  /// @attention �����÷�֧���������÷�֧����
  bool SetBranchCondition(
      const std::shared_ptr<const OperatorNodeInterface>& branch_condition,
      const std::shared_ptr<const std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          flow_control_node_container);
  /// @brief ��ȡ��֧����
  /// @return ����ָ���֧�����ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetBranchConditionPointer()
      const {
    return branch_condition_;
  }
  /// @brief ��ȡ��֧����
  /// @return ���ط�֧�����ڵ��const����
  const OperatorNodeInterface& GetBranchConditionReference() const {
    return *branch_condition_;
  }
  /// @brief ��ȡ�õ���֧������ȫ������
  /// @return ����ָ���ȡ��֧����������������constָ��
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
  GetFlowControlNodeToGetConditionPointer() const {
    return condition_flow_control_node_container_;
  }
  /// @brief ��ȡ�õ���֧������ȫ������
  /// @return ���ػ�ȡ��֧����������������const����
  const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
  GetFlowControlNodeToGetConditionReference() const;
  /// @brief ��������Ϊ��ʱ�ķ�֧
  /// @param[in] true_branch �����֧���
  /// @param[in] flow_control_node_container ����ȡ���֧����Ĳ���
  /// @retval true ����������
  /// @retval false ��true_branch�޷���Ϊ���֧���
  /// @details
  /// 1.true_branch��Ҫ��ο�CheckBranchValid
  /// 2.���÷�֧�����������Ҫ�Ķ�����Ҫ��������һ����ٷ�֧
  ///   ��ʱ�Կ���ʹ��Get������ȡ֮ǰ�洢�ķ�֧��Ϣ
  /// @attention �����÷�֧���������÷�֧����
  bool SetTrueBranch(
      const std::shared_ptr<const OperatorNodeInterface>& true_branch,
      const std::shared_ptr<const std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          flow_control_node_container);
  /// @brief ��ȡ���֧���
  /// @return ����ָ�����֧�����constָ��
  std::shared_ptr<const OperatorNodeInterface> GetTrueBranchPointer() const {
    return true_branch_;
  }
  /// @brief ��ȡ���֧���
  /// @return �������֧�����const����
  const OperatorNodeInterface& GetTrueBranchReference() const {
    return *true_branch_;
  }
  /// @brief ��ȡ�õ����֧����Ĳ���
  /// @return ����ָ��洢������������constָ��
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
  GetFlowControlNodeToGetTrueBranchPointer() const {
    return true_branch_flow_control_node_container_;
  }
  /// @brief ��ȡ�õ����֧����Ĳ���
  /// @return ���ش洢������������const����
  const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
  GetFlowControlNodeToGetTrueBranchReference() const {
    return *true_branch_flow_control_node_container_;
  }
  /// @brief ��������Ϊ��ʱ�ķ�֧
  /// @param[in] false_branch ���ٷ�֧���
  /// @param[in] flow_control_node_container ����ȡ�ٷ�֧����Ĳ���
  /// @retval true ����������
  /// @retval false ��false_branch�޷���Ϊ�ٷ�֧���
  /// @details
  /// 1.false_branch��Ҫ��ο�CheckBranchValid
  /// 2.���÷�֧�����������Ҫ�Ķ�����Ҫ��������һ����ٷ�֧
  ///   ��ʱ�Կ���ʹ��Get������ȡ֮ǰ�洢�ķ�֧��Ϣ
  /// @attention �����÷�֧���������÷�֧����
  bool SetFalseBranch(
      const std::shared_ptr<const OperatorNodeInterface>& false_branch,
      const std::shared_ptr<const std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          flow_control_node_container);
  /// @brief ��ȡ�ٷ�֧���
  /// @return ����ָ��ٷ�֧�����constָ��
  std::shared_ptr<const OperatorNodeInterface> GetFalseBranchPointer() const {
    return false_branch_;
  }
  /// @brief ��ȡ�ٷ�֧���
  /// @return ���ؼٷ�֧�����const����
  const OperatorNodeInterface& GetFalseBranchReference() const {
    return *false_branch_;
  }
  /// @brief ��ȡ�õ��ٷ�֧����Ĳ���
  /// @return ����ָ��洢������������constָ��
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
  GetFlowControlNodeToGetFalseBranchPointer() const {
    return false_branch_flow_control_node_container_;
  }
  /// @brief ��ȡ�õ��ٷ�֧����Ĳ���
  /// @return ���ش洢������������const����
  const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&
  GetFlowControlNodeToGetFalseBranchReference() const;
  std::shared_ptr<const OperatorNodeInterface> GetResultPointer() const {
    return result_;
  }
  /// @brief ��ȡ���
  /// @return ���ؽ���ڵ��const����
  const OperatorNodeInterface& GetResultReference() const { return *result_; }

  /// @brief ����֧�����Ƿ���Ч
  /// @param[in] branch_condition ����֧����
  /// @return ���ط�֧�����Ƿ���Ч
  /// @retval true ����֧������Ч
  /// @retval false ����֧������Ч
  /// @note ֻ��StructOrBasicType::kBasic��StructOrBasicType::kPointer������Ϊ
  /// ��֧����
  static bool CheckBranchConditionValid(
      const OperatorNodeInterface& branch_condition);
  /// @brief ���������֧����Ƿ���Ч
  /// @param[in] branch ��������֧���
  /// @return ����������֧����Ƿ���Ч
  /// @retval true ����֧���������Ч
  /// @retval false ����֧���������Ч
  /// @note ֻ��StructOrBasicType::kBasic��StructOrBasicType::kPointer������Ϊ
  /// ��֧����
  static bool CheckBranchValid(const OperatorNodeInterface& branch) {
    return CheckBranchConditionValid(branch);
  }

 private:
  /// @brief �������ؽ���Ľڵ�
  /// @return ���ش�������Ƿ�ɹ�
  /// @retval true �������ɹ�
  /// @details
  /// 1.���÷�֧������������֧�����
  /// 2.���������֧���ܻ���ת���򷵻�false
  /// 3.�����֧����Ϊ�����ڳ��������κβ�������true
  /// 4.��֧�ַǱ����ڳ���������ʹ�ó�ʼ���б�
  bool ConstructResultNode();

  /// @brief ��֧����
  std::shared_ptr<const OperatorNodeInterface> branch_condition_;
  /// @brief ��ȡ��֧�����Ĳ���
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
      condition_flow_control_node_container_;
  /// @brief ����Ϊ��ʱ���ʽ�Ľ��
  std::shared_ptr<const OperatorNodeInterface> true_branch_ = nullptr;
  /// @brief ��ȡ���֧����Ĳ���
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
      true_branch_flow_control_node_container_;
  /// @brief ����Ϊ��ʱ���ʽ�Ľ��
  std::shared_ptr<const OperatorNodeInterface> false_branch_ = nullptr;
  /// @brief ��ȡ�ٷ�֧����Ĳ���
  std::shared_ptr<const std::list<
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>
      false_branch_flow_control_node_container_;
  /// @brief �洢���ؽ���Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> result_;
};

/// @class AssignOperatorNode operator_node.h
/// @brief ��ֵ�ڵ�
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ���ô�����ֵ�Ľڵ�
  /// @param[in] node_to_be_assigned ��������ֵ�Ľڵ�
  /// @note �����ñ���ֵ�Ľڵ㣬������������ֵ�Ľڵ�
  void SetNodeToBeAssigned(
      const std::shared_ptr<const OperatorNodeInterface>& node_to_be_assigned) {
    node_to_be_assigned_ = node_to_be_assigned;
  }
  /// @brief ��ȡ������ֵ�Ľڵ�
  /// @return ����ָ�������ֵ�Ľڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetNodeToBeAssignedPointer()
      const {
    return node_to_be_assigned_;
  }
  /// @brief ��ȡ������ֵ�Ľڵ�
  /// @return ���ش�����ֵ�Ľڵ��const����
  const OperatorNodeInterface& GetNodeToBeAssignedReference() const {
    return *node_to_be_assigned_;
  }
  /// @brief ����������ֵ�Ľڵ�
  /// @param[in] node_for_assign ��������ֵ�Ľڵ�
  /// @param[in] is_announce ���Ƿ�Ϊ��������
  /// @return ���ؼ�������������������
  /// @details
  /// 1.������ǿ��Ը�ֵ������򲻻�����
  /// 2.����ʱ��ֵ����鱻��ֵ�Ľڵ������const���Ժ�LeftRightValue����
  ///   ��������ʹ�ó�ʼ���б�
  /// 3.��Ҫ�����ñ���ֵ�Ľڵ㣨����SetNodeToBeAssigned��
  /// 4.�������ֵ�Ľڵ���������ֵ�Ľڵ�������ȫ��ͬ��operator==()����
  ///   ���ñ���ֵ�Ľڵ��������ָ��ָ��������ֵ���������Խ�ʡ�ڴ�
  AssignableCheckResult SetNodeForAssign(
      const std::shared_ptr<const OperatorNodeInterface>& node_for_assign,
      bool is_announce);
  /// @brief ��ȡ������ֵ�Ľڵ�
  /// @return ����ָ��������ֵ�Ľڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetNodeForAssignPointer() const {
    return node_for_assign_;
  }
  /// @brief ��ȡ������ֵ�Ľڵ�
  /// @return ����������ֵ�Ľڵ��const����
  const OperatorNodeInterface& GetNodeForAssignReference() const {
    return *node_for_assign_;
  }

  /// @brief ��������ڵ��Ƿ���Ը�ֵ
  /// @param[in] node_to_be_assigned ������ֵ�Ľڵ�
  /// @param[in] node_for_assign ��������ֵ�Ľڵ�
  /// @param[in] is_announce
  /// @return ���ؼ���������������
  /// @details
  /// 1.�Զ����AssignableCheckResut::kMayBeZeroToPointer�ľ���������ı䷵�ؽ��
  /// 2.��is_announce == trueʱ
  ///   ����const��Ǻ�LeftRightValue��ǵļ��������ʹ�ó�ʼ���б�
  /// 3.���ʹ���Զ������С�ƶ�����������С��Υ��constԭ��
  /// 4.�������ֵ�Ľڵ���������ֵ�Ľڵ�������ȫ��ͬ��operator==()����Ϊ��������
  ///   �����ñ���ֵ�Ľڵ��������ָ��ָ��������ֵ���������Խ�ʡ�ڴ�
  ///   ��Υ��constԭ��
  static AssignableCheckResult CheckAssignable(
      const OperatorNodeInterface& node_to_be_assigned,
      const OperatorNodeInterface& node_for_assign, bool is_announce);

 private:
  /// @brief ���ʹ�ó�ʼ���б��ʼ�����������
  /// @param[in] variety_node ������ֵ�ı���
  /// @param[in] list_initialize_operator_node ��������ֵ�ĳ�ʼ���б�
  /// @return ���ؼ���������������
  /// @details
  /// 1.CheckAssignable���ӹ��̣�����ʹ�ó�ʼ�������б��ʼ�����������
  /// 2.���᷵��AssignableCheckResult::kInitializeList
  /// 3.���ʹ���Զ������С�ƶ�����������С��Υ��constԭ��
  static AssignableCheckResult VarietyAssignableByInitializeList(
      const VarietyOperatorNode& variety_node,
      const ListInitializeOperatorNode& list_initialize_operator_node);

  /// @brief ��Ҫ����ֵ�Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_be_assigned_;
  /// @brief ������ֵ�Ľڵ�
  std::shared_ptr<const OperatorNodeInterface> node_for_assign_;
};

/// @class MathematicalOperatorNode operator_node.h
/// @brief ��ѧ����
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    // ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ������ѧ����ķ���
  /// @param[in] mathematical_operation ����ѧ�������
  void SetMathematicalOperation(MathematicalOperation mathematical_operation) {
    mathematical_operation_ = mathematical_operation;
  }
  /// @brief ��ȡ��ѧ�������
  /// @return ������ѧ�������
  MathematicalOperation GetMathematicalOperation() const {
    return mathematical_operation_;
  }
  /// @brief �������������/��Ŀ������������ڵ�
  /// @param[in] left_operator_node ������������ڵ��Ŀ�������������
  /// @return �����Ƿ��������
  /// @retval true ����������ܸ�����������
  /// @retval false ������������ܸ�����������
  /// @note ����ǵ�Ŀ�������ᴴ������ڵ�
  /// @attention �������������ͺ����ýڵ�
  bool SetLeftOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& left_operator_node);
  /// @brief ��ȡ���������/��Ŀ�������������
  /// @return ����ָ���������ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetLeftOperatorNodePointer()
      const {
    return left_operator_node_;
  }
  /// @brief ��ȡ���������/��Ŀ�������������
  /// @return �����������ڵ��const����
  const OperatorNodeInterface& GetLeftOperatorNodeReference() const {
    return *left_operator_node_;
  }
  /// @brief ����˫Ŀ������Ҳ�������
  /// @param[in] right_operator_node ���Ҳ�������
  /// @return �������������������������������
  /// @note ��������ڵ�������ҽڵ㣬������������򲻻������Ҳ�������
  /// @attention ��ֹ�Ե�Ŀ��������øú���
  DeclineMathematicalComputeTypeResult SetRightOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& right_operator_node);
  /// @brief ��ȡ���������/��Ŀ�������������
  /// @return ����ָ���������ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetRightOperatorNodePointer()
      const {
    return right_operator_node_;
  }
  /// @brief ��ȡ���������/��Ŀ�������������
  /// @return �����������ڵ��const����
  const OperatorNodeInterface& GetRightOperatorNodeReference() const {
    return *right_operator_node_;
  }
  /// @brief ��ȡ������
  /// @return ����ָ���������ڵ��constָ��
  std::shared_ptr<const VarietyOperatorNode> GetComputeResultNodePointer()
      const {
    return compute_result_node_;
  }
  /// @brief ��ȡ������
  /// @return �����������ڵ��const����
  const VarietyOperatorNode& GetComputeResultNodeReference() const {
    return *compute_result_node_;
  }

  /// @brief �����������
  /// @param[in] mathematical_operation �������
  /// @param[in] left_operator_node �����������
  /// @param[in] right_operator_node ���Ҳ�������
  /// @return ǰ�벿��Ϊ�������ڵ㣬��벿��Ϊ����ľ������
  /// @retval (nullptr,...) ����������
  /// @note ��Ŀ�����������øú���������������ʱ�ʹ����˽���ڵ�
  static std::pair<std::shared_ptr<VarietyOperatorNode>,
                   DeclineMathematicalComputeTypeResult>
  DeclineComputeResult(
      MathematicalOperation mathematical_operation,
      const std::shared_ptr<const OperatorNodeInterface>& left_operator_node,
      const std::shared_ptr<const OperatorNodeInterface>& right_operator_node);

 private:
  /// @brief �����������ڵ�
  /// @param[in] compute_result_node ���������ڵ��constָ��
  void SetComputeResultNode(
      const std::shared_ptr<const VarietyOperatorNode>& compute_result_node) {
    compute_result_node_ = compute_result_node;
  }

  /// @brief ��ѧ��������
  MathematicalOperation mathematical_operation_;
  /// @brief ������ڵ�
  std::shared_ptr<const OperatorNodeInterface> left_operator_node_;
  /// @brief ������ڵ�
  std::shared_ptr<const OperatorNodeInterface> right_operator_node_;
  /// @brief ����õ�������
  std::shared_ptr<const VarietyOperatorNode> compute_result_node_;
};

/// @class LogicalOperationOperatorNode operator_node.h
/// @brief �߼�����
class LogicalOperationOperatorNode : public OperatorNodeInterface {
 public:
  LogicalOperationOperatorNode(LogicalOperation logical_operation)
      : OperatorNodeInterface(GeneralOperationType::kLogicalOperation),
        logical_operation_(logical_operation) {
    CreateAndSetResultNode();
  }
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    /// ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ��ȡ�߼������
  /// @return �����߼������
  LogicalOperation GetLogicalOperation() const { return logical_operation_; }
  /// @brief �������������/��Ŀ�����������
  /// @param[in] left_operator_node �����������/��Ŀ�����������
  /// @return �����Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false ��left_operator_node��������������Ҫ��
  /// @note ���������Ҫ��ο�CheckLogicalTypeValid
  bool SetLeftOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& left_operator_node);
  /// @brief ��ȡ���������/��Ŀ�����������
  /// @return ����ָ�����������/��Ŀ������������ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetLeftOperatorNodePointer()
      const {
    return left_operator_node_;
  }
  /// @brief ��ȡ���������/��Ŀ�����������
  /// @return �������������/��Ŀ������������ڵ��const����
  const OperatorNodeInterface& GetLeftOperatorNodeReference() const {
    return *left_operator_node_;
  }
  /// @brief �����Ҳ�������
  /// @param[in] right_operator_node ���Ҳ�������
  /// @return �����Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false �������������������߼�����
  bool SetRightOperatorNode(
      const std::shared_ptr<const OperatorNodeInterface>& right_operator_node);
  std::shared_ptr<const OperatorNodeInterface> GetRightOperatorNodePointer()
      const {
    return right_operator_node_;
  }
  /// @brief ��ȡ�Ҳ�������
  /// @return ����ָ���Ҳ��������ڵ��constָ��
  const OperatorNodeInterface& GetRightOperatorNodeReference() const {
    return *right_operator_node_;
  }
  /// @brief ��ȡ�Ҳ�������
  /// @return �����Ҳ��������ڵ��const����
  std::shared_ptr<const VarietyOperatorNode> GetComputeResultNodePointer()
      const {
    return compute_result_node_;
  }
  /// @brief ��ȡ������
  /// @return �����������ڵ��const����
  const VarietyOperatorNode& GetComputeResultNodeReference() const {
    return *compute_result_node_;
  }

  /// @brief ����������Ƿ���Բ����߼�����
  /// @return �����������Ƿ���Բ����߼�����
  /// @retval true �����������Բ����߼�����
  /// @retval false �������������Բ����߼�����
  static bool CheckLogicalTypeValid(const TypeInterface& type_interface);

 private:
  /// @brief ���������ý���ڵ�
  void CreateAndSetResultNode() {
    // �����߼���������Ϊbool
    compute_result_node_ = std::make_shared<VarietyOperatorNode>(
        std::string(), ConstTag::kNonConst, LeftRightValueTag::kRightValue);
    compute_result_node_->SetVarietyType(
        CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt1,
                                                SignTag::kUnsigned>());
  }

  /// @brief �߼���������
  LogicalOperation logical_operation_;
  /// @brief ��������
  std::shared_ptr<const OperatorNodeInterface> left_operator_node_;
  /// @brief ��������
  std::shared_ptr<const OperatorNodeInterface> right_operator_node_;
  /// @brief ������
  std::shared_ptr<VarietyOperatorNode> compute_result_node_;
};

/// @class DereferenceOperatorNode operator_node.h
/// @brief ������
class DereferenceOperatorNode : public OperatorNodeInterface {
 public:
  DereferenceOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kDeReference) {}
  DereferenceOperatorNode(const DereferenceOperatorNode&) = delete;

  DereferenceOperatorNode& operator=(const DereferenceOperatorNode&) = delete;

  virtual ConstTag GetResultConstTag() const override {
    return GetDereferencedNodeReference().GetConstTag();
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    /// ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief �趨�������õĽڵ�
  /// @param[in] node_to_dereference ���������õĽڵ�
  /// @return �����Ƿ�ɹ�����
  /// @retval true �����óɹ�
  /// @retval false ���ýڵ㲻�ܽ�����
  /// @note �ڵ�Ҫ��ο�CheckNodeDereferenceAble
  bool SetNodeToDereference(
      const std::shared_ptr<const OperatorNodeInterface>& node_to_dereference);
  /// @brief ��ȡ�������õĽڵ�
  /// @return ����ָ�򱻽����õĽڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetNodeToDereferencePointer()
      const {
    return node_to_dereference_;
  }
  /// @brief ��ȡ�������õĽڵ�
  /// @return ���ر������õĽڵ��const����
  const OperatorNodeInterface& GetNodeToDereferenceReference() const {
    return *node_to_dereference_;
  }
  /// @brief ��ȡ�����õõ��Ľڵ�
  /// @return ����ָ������õõ��Ľڵ��constָ��
  std::shared_ptr<const VarietyOperatorNode> GetDereferencedNodePointer()
      const {
    return dereferenced_node_;
  }
  /// @brief ��ȡ�����õõ��Ľڵ�
  /// @return ���ؽ����õĽڵ��const����
  const VarietyOperatorNode& GetDereferencedNodeReference() const {
    return *dereferenced_node_;
  }

  /// @brief ���ڵ��Ƿ���Խ�����
  /// @return ���ؽڵ��Ƿ���Խ�����
  /// @retval true ���ڵ���Խ�����
  /// @retval false ���ڵ㲻�ܽ�����
  /// @note ֻ��ָ�����Ͳſ��Խ�����
  static bool CheckNodeDereferenceAble(
      const OperatorNodeInterface& node_to_dereference);

 private:
  /// @brief ���ý����ú�õ��Ľڵ�
  /// @param[in] dereferenced_node �������ú�õ��Ľڵ�
  void SetDereferencedNode(
      const std::shared_ptr<VarietyOperatorNode>& dereferenced_node) {
    dereferenced_node_ = dereferenced_node;
  }

  /// @brief �������õĽڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_dereference_;
  /// @brief �����ú�õ��Ľڵ�
  std::shared_ptr<VarietyOperatorNode> dereferenced_node_;
};

/// @class ObtainAddressOperatorNode operator_node.h
/// @brief ȡ��ַ
class ObtainAddressOperatorNode : public OperatorNodeInterface {
 public:
  ObtainAddressOperatorNode()
      : OperatorNodeInterface(GeneralOperationType::kObtainAddress) {}
  ObtainAddressOperatorNode(const ObtainAddressOperatorNode&) = delete;

  ObtainAddressOperatorNode& operator=(const ObtainAddressOperatorNode&) =
      delete;

  virtual ConstTag GetResultConstTag() const override {
    /// ȡ��ַ��õ�ֵ���Ƿ�const
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    /// ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ���ñ�ȡ��ַ�Ľڵ�
  /// @param[in] node_to_obtain_address ����ȡ��ַ�Ľڵ�
  /// @return �����Ƿ�ɹ�����
  /// @retval true �����óɹ�
  /// @retval false ���ýڵ㲻��ȡ��ַ
  /// @note �ڵ�Ҫ��ο�CheckNodeToObtainAddress
  bool SetNodeToObtainAddress(
      const std::shared_ptr<const VarietyOperatorNode>& node_to_obtain_address);
  /// @brief ��ȡ��ȡ��ַ�Ľڵ�
  /// @return ����ָ��ȡ��ַ�Ľڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetNodeToBeObtainAddressPointer()
      const {
    return node_to_obtain_address_;
  }
  /// @brief ��ȡ��ȡ��ַ�Ľڵ�
  /// @return ���ر�ȡ��ַ�Ľڵ��const����
  const OperatorNodeInterface& GetNodeToBeObtainAddressReference() const {
    return *node_to_obtain_address_;
  }
  /// @brief ��ȡȡ��ַ��õ��Ľڵ�
  /// @return ����ָ��ȡ��ַ��õ��Ľڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetObtainedAddressNodePointer()
      const {
    return obtained_address_node_;
  }
  /// @brief ��ȡȡ��ַ��õ��Ľڵ�
  /// @return ����ȡ��ַ��õ��Ľڵ��const����
  const OperatorNodeInterface& GetObtainedAddressNodeReference() const {
    return *obtained_address_node_;
  }
  /// @brief ���ڵ��Ƿ����ȡ��ַ
  /// @param[in] node_interface �������Ľڵ�
  /// @return �����Ƿ����ȡ��ַ
  /// @retval true �������ڵ����ȡ��ַ
  /// @retval false �������ڵ㲻����ȡ��ַ
  static bool CheckNodeToObtainAddress(
      const OperatorNodeInterface& node_interface);

 private:
  /// @brief ����ȡ��ַ��õ��Ľڵ�
  /// @param[in] node_obtained_address ��ȡ��ַ��õ��Ľڵ�
  void SetNodeObtainedAddress(
      const std::shared_ptr<const OperatorNodeInterface>&
          node_obtained_address) {
    obtained_address_node_ = node_obtained_address;
  }

  /// @brief ��Ҫ��ȡ��ַ�Ľڵ�
  std::shared_ptr<const VarietyOperatorNode> node_to_obtain_address_;
  /// @brief ȡ��ַ���õĽڵ�
  std::shared_ptr<const OperatorNodeInterface> obtained_address_node_;
};

/// @class MemberAccessOperatorNode operator_node.h
/// @brief ��Ա���ʽڵ�
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    /// ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ����Ҫ���ʵĽṹ�����ݽڵ�
  /// @param[in] node_to_access �������ʵĽṹ�����ݽڵ�
  /// @return ���ظ����ڵ��Ƿ������Ϊ�����ʳ�Ա�Ľڵ�
  /// @retval true ���ɹ�����
  /// @retval false ���ýڵ㲻�ǽṹ�����ݽڵ�
  /// @note ����������򲻻�����
  bool SetNodeToAccess(
      const std::shared_ptr<const OperatorNodeInterface>& node_to_access) {
    if (CheckNodeToAccessValid(*node_to_access)) [[likely]] {
      node_to_access_ = node_to_access;
      return true;
    } else {
      return false;
    }
  }
  /// @brief ��ȡ�����ʳ�Ա�Ľڵ�
  /// @return ����ָ�򱻷��ʳ�Ա�Ľڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetNodeToAccessPointer() {
    return node_to_access_;
  }
  /// @brief ��ȡ�����ʳ�Ա�Ľڵ�
  /// @return ���ر����ʳ�Ա�Ľڵ��const����
  const OperatorNodeInterface& GetNodeToAccessReference() const {
    return *node_to_access_;
  }
  /// @brief ���÷��ʵĳ�Ա��
  /// @param[in] member_name ���ʵĳ�Ա��
  /// @return ���ظ����ڵ��Ƿ��ǳ�Ա��
  /// @retval true ���ɹ�����
  /// @retval false ����Ա��������
  /// @note ���������Ա���������򲻻�����
  /// @attention ���������ýڵ㣬������Ҫ���ʵĳ�Ա��
  template <class MemberName>
  bool SetMemberName(MemberName&& member_name) {
    return SetAccessedNodeAndMemberName(
        std::string(std::forward<MemberName>(member_name)));
  }
  /// @brief ��ȡ���ʵĳ�Ա�ڽṹ�е��±�
  /// @return ���س�Ա�ڽṹ�е��±�
  MemberIndex GetMemberIndex() const { return member_index_; }
  /// @brief ��ȡ���ʵĳ�Ա�Ľڵ�
  /// @return ���ط��ʵĳ�Ա�ڵ��constָ��
  /// @attention ����ɹ�����Ҫ���ʵĽڵ�ͳ�Ա��
  std::shared_ptr<const OperatorNodeInterface> GetAccessedNodePointer() const {
    return node_accessed_;
  }
  /// @brief ��ȡ���ʵĳ�Ա�Ľڵ�
  /// @return ���ط��ʵĳ�Ա�ڵ��constָ��
  /// @attention ����ɹ�����Ҫ���ʵĽڵ�ͳ�Ա��
  const OperatorNodeInterface& GetAccessedNodeReference() const {
    return *node_accessed_;
  }

  /// @brief �������ڵ��Ƿ������Ϊ�����ʳ�Ա�Ľڵ�
  /// @param[in] node_to_access �������Ľڵ�
  /// @return ���ظ����ڵ��Ƿ������Ϊ�����ʳ�Ա�Ľڵ�
  /// @retval true ��������Ա���Ա����ʳ�Ա���ǽṹ���������ͣ�
  /// @retval false ��������Ա���ܱ����ʳ�Ա�����ǽṹ���������ͣ�
  static bool CheckNodeToAccessValid(
      const OperatorNodeInterface& node_to_access);

 private:
  /// @brief ���÷��ʵĳ�Ա��
  /// @param[in] member_name_to_set �����ʵĳ�Ա��
  /// @return �����Ƿ����óɹ�
  /// @retval true ���ɹ�����
  /// @retval false �������ڸ����ĳ�Ա
  /// @details
  /// ���ݴ����ʵĽڵ�ͽڵ��Ա�����ó�Ա���±꣬���������÷��ʵĳ�Ա���±�
  /// @note ��������ڸ����ĳ�Ա������
  bool SetAccessedNodeAndMemberName(std::string&& member_name_to_set);
  /// @brief ���÷��ʺ�õ��Ľڵ�
  /// @param[in] node_accessed �����ʺ�õ��Ľڵ�
  void SetAccessedNode(
      const std::shared_ptr<OperatorNodeInterface>& node_accessed) {
    node_accessed_ = node_accessed;
  }

  /// @brief Ҫ���ʵĽڵ�
  std::shared_ptr<const OperatorNodeInterface> node_to_access_;
  /// @brief ���ʵĽڵ��Ա��index
  /// @note ö�����Ͳ����ø���
  MemberIndex member_index_;
  /// @brief ���ʺ�õ��Ľڵ�
  std::shared_ptr<OperatorNodeInterface> node_accessed_;
};

/// @brief FunctionCallOperatorNode operator_node.h
/// @brief ��������
class FunctionCallOperatorNode : public OperatorNodeInterface {
  /// @class FunctionCallArgumentsContainer operator_node.h
  /// @brief �洢��������ʱ����������
  class FunctionCallArgumentsContainer {
   public:
    FunctionCallArgumentsContainer();
    ~FunctionCallArgumentsContainer();

    /// @brief �洢���ò���������
    using ContainerType = std::list<
        std::pair<std::shared_ptr<const OperatorNodeInterface>,
                  std::shared_ptr<std::list<std::unique_ptr<
                      c_parser_frontend::flow_control::FlowInterface>>>>>;
    /// @brief ��Ӻ������ò���
    /// @param[in] argument �������ڵ�
    /// @param[in] flow_control_node_container ����ȡ�����ڵ�Ĳ���
    /// @details
    /// ÿ����Ӷ���ӵ����в�����β��
    /// ʹ��ʱ����д˳�����в���
    /// @note ����������Ч��
    void AddFunctionCallArgument(
        const std::shared_ptr<const OperatorNodeInterface>& argument,
        const std::shared_ptr<std::list<
            std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
            flow_control_node_container) {
      function_call_arguments_.emplace_back(
          std::make_pair(argument, flow_control_node_container));
    }
    /// @brief ��ȡȫ���������ò���
    /// @return ���ش洢�������ò�����������const����
    const ContainerType& GetFunctionCallArguments() const {
      return function_call_arguments_;
    }

   private:
    // �������GetFunctionCallArgumentsNotConst()
    friend FunctionCallOperatorNode;

    /// @brief ��ȡȫ���������ò���
    /// @return ���ش洢�������ò���������������
    ContainerType& GetFunctionCallArguments() {
      return function_call_arguments_;
    }

    /// @brief �洢�������ò���������
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
      const std::shared_ptr<const TypeInterface>& new_type,
      ConstTag new_const_tag) const override {
    assert(false);
    /// ��ֹ����
    return std::shared_ptr<OperatorNodeInterface>();
  }

  /// @brief ����Ҫ���õĶ���
  /// @param[in] function_object_to_call ���������õĶ���
  /// @note �������Ҫ��ο�CheckFunctionTypeValid
  /// @attention �����ò���ǰ����Ҫ���õĶ���
  bool SetFunctionType(const std::shared_ptr<const OperatorNodeInterface>&
                           function_object_to_call);
  /// @brief ��ȡ�����õĺ���
  /// @return ����ָ�򱻵��õĺ������͵�constָ��
  std::shared_ptr<const c_parser_frontend::type_system::FunctionType>
  GetFunctionTypePointer() const {
    return function_type_;
  }
  /// @brief ��ȡ�����õĺ���
  /// @return ���ر����õĺ������͵�const����
  const c_parser_frontend::type_system::FunctionType& GetFunctionTypeReference()
      const {
    return *function_type_;
  }
  /// @brief ��ȡ�������ض���
  /// @return ����ָ�������ض����constָ��
  std::shared_ptr<const VarietyOperatorNode> GetReturnObjectPointer() const {
    return return_object_;
  }
  /// @brief ��ȡ�������ض���
  /// @return ���غ������ض����const����
  const VarietyOperatorNode& GetReturnObjectReference() const {
    return *return_object_;
  }
  /// @brief ��ȡԭʼ���ò���
  /// @return ���ش洢���ò�����������const����
  /// @note ���ò�������д˳������
  const auto& GetFunctionArgumentsOfferred() const {
    return function_arguments_offerred_;
  }
  /// @brief ��ӵ��ò���
  /// @param[in] argument_node �����ò���
  /// @param[in] sentences_to_get_argument ����ȡ���ò����Ĳ���
  /// @return ���ز����ļ���������������
  /// @details
  /// ���ش���ӵĲ����Ƿ�ͨ�����飬δͨ�������򲻻����
  /// ÿ�ε��øú����������еĲ���������µĲ���
  /// @attention �����ò���ǰ����Ҫ���õĶ���
  AssignableCheckResult AddFunctionCallArgument(
      const std::shared_ptr<const OperatorNodeInterface>& argument_node,
      const std::shared_ptr<std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
          sentences_to_get_argument);
  /// @brief ���������õ��ò���
  /// @param[in] container ���洢���ò���������
  /// @return �����Ƿ�ɹ���ӣ����в���������Ҫ��
  /// @retval true �����óɹ�
  /// @retval false �������д����޷���ʽת��Ϊ��������Ĳ������͵Ĳ���
  /// @note ������ʧ���򲻻��޸Ĳ���
  /// ��������д˳������
  /// @attention �����ò���ǰ����Ҫ���õĶ���
  bool SetArguments(FunctionCallArgumentsContainer&& container);
  /// @brief ��������Ƿ�������ڵ���
  /// @param[in] type_interface ������������
  /// @return �����Ƿ�������ڵ���
  /// @retval true ���������ڵ���
  /// @retval false �����Ǻ�������
  static bool CheckFunctionTypeValid(const TypeInterface& type_interface) {
    return type_interface.GetType() == StructOrBasicType::kFunction;
  }

 private:
  /// @brief ��ȡԭʼ���ò���
  /// @return ���ش洢���ò���������������
  /// @note ���ò�������д˳������
  FunctionCallArgumentsContainer& GetFunctionArgumentsOfferred() {
    return function_arguments_offerred_;
  }

  /// @brief �������صĶ���
  std::shared_ptr<const VarietyOperatorNode> return_object_;
  /// @brief �����õĶ���
  std::shared_ptr<const c_parser_frontend::type_system::FunctionType>
      function_type_;
  /// @brief ԭʼ���ò������ȡ�����Ĳ���������д˳������
  FunctionCallArgumentsContainer function_arguments_offerred_;
};

}  // namespace c_parser_frontend::operator_node

#endif  /// !CPARSERFRONTEND_OPERATOR_NODE_H_