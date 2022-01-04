#ifndef CPARSERFRONTEND_PARSE_FUNCTIONS_H_
#define CPARSERFRONTEND_PARSE_FUNCTIONS_H_
#include <format>
#include <iostream>
#include <limits>

#include "Generator/SyntaxGenerator/process_function_interface.h"
#include "c_parser_frontend.h"
#include "flow_control.h"
#include "operator_node.h"
#include "type_system.h"
namespace c_parser_frontend {
/// �����߳�ȫ�ֱ�����C���Ա�����ǰ�˵������
extern thread_local CParserFrontend c_parser_frontend;
}  // namespace c_parser_frontend

namespace c_parser_frontend::parse_functions {
using WordDataToUser = frontend::generator::syntax_generator::
    ProcessFunctionInterface::WordDataToUser;
using c_parser_frontend::c_parser_frontend;
using c_parser_frontend::flow_control::AllocateOperatorNode;
using c_parser_frontend::flow_control::ConditionBlockInterface;
using c_parser_frontend::flow_control::DoWhileSentence;
using c_parser_frontend::flow_control::FlowInterface;
using c_parser_frontend::flow_control::FlowType;
using c_parser_frontend::flow_control::ForSentence;
using c_parser_frontend::flow_control::IfSentence;
using c_parser_frontend::flow_control::Jmp;
using c_parser_frontend::flow_control::Label;
using c_parser_frontend::flow_control::LoopSentenceInterface;
using c_parser_frontend::flow_control::Return;
using c_parser_frontend::flow_control::SimpleSentence;
using c_parser_frontend::flow_control::SwitchSentence;
using c_parser_frontend::flow_control::WhileSentence;
using c_parser_frontend::operator_node::AssignableCheckResult;
using c_parser_frontend::operator_node::AssignOperatorNode;
using c_parser_frontend::operator_node::BasicTypeInitializeOperatorNode;
using c_parser_frontend::operator_node::DereferenceOperatorNode;
using c_parser_frontend::operator_node::FunctionCallOperatorNode;
using c_parser_frontend::operator_node::GeneralOperationType;
using c_parser_frontend::operator_node::InitializeOperatorNodeInterface;
using c_parser_frontend::operator_node::InitializeType;
using c_parser_frontend::operator_node::LeftRightValueTag;
using c_parser_frontend::operator_node::ListInitializeOperatorNode;
using c_parser_frontend::operator_node::LogicalOperation;
using c_parser_frontend::operator_node::LogicalOperationOperatorNode;
using c_parser_frontend::operator_node::MathematicalAndAssignOperation;
using c_parser_frontend::operator_node::MathematicalOperation;
using c_parser_frontend::operator_node::MathematicalOperatorNode;
using c_parser_frontend::operator_node::MemberAccessOperatorNode;
using c_parser_frontend::operator_node::ObtainAddressOperatorNode;
using c_parser_frontend::operator_node::OperatorNodeInterface;
using c_parser_frontend::operator_node::TemaryOperatorNode;
using c_parser_frontend::operator_node::TypeConvert;
using c_parser_frontend::operator_node::VarietyOperatorNode;
using c_parser_frontend::type_system::AddTypeResult;
using c_parser_frontend::type_system::BasicType;
using c_parser_frontend::type_system::BuiltInType;
using c_parser_frontend::type_system::CommonlyUsedTypeGenerator;
using c_parser_frontend::type_system::ConstTag;
using c_parser_frontend::type_system::EndType;
using c_parser_frontend::type_system::EnumType;
using c_parser_frontend::type_system::FunctionType;
using c_parser_frontend::type_system::GetTypeResult;
using c_parser_frontend::type_system::PointerType;
using c_parser_frontend::type_system::SignTag;
using c_parser_frontend::type_system::StructOrBasicType;
using c_parser_frontend::type_system::StructType;
using c_parser_frontend::type_system::StructureTypeInterface;
using c_parser_frontend::type_system::TypeInterface;
using c_parser_frontend::type_system::UnionType;
using DeclineMathematicalComputeTypeResult = c_parser_frontend::operator_node::
    MathematicalOperatorNode::DeclineMathematicalComputeTypeResult;

class ObjectConstructData;
/// �����ṹ����ʱʹ�õ�ȫ�ֱ����������Ż�ִ���߼�
/// �������ڹ����Ľṹ��������
static thread_local std::shared_ptr<StructureTypeInterface>
    structure_type_constructuring;
/// ������������ʱʹ�õ�ȫ�ֱ����������Ż�ִ���߼�
/// ������������ݵ���
static thread_local std::shared_ptr<ObjectConstructData>
    function_type_construct_data;
/// �����������ýڵ�ʱʹ�õ�ȫ�ֱ����������Ż�ִ���߼�
static thread_local std::shared_ptr<FunctionCallOperatorNode>
    function_call_operator_node;

/// ������������/��������ʱʹ�õ�����
class ObjectConstructData {
 public:
  /// ���������������̵ļ����
  enum class CheckResult {
    /// �ɹ������
    kSuccess,
    /// ʧ�ܵ����
    kAttachToTerminalType,  /// β�ڵ��Ѿ����ս����ͣ��ṹ��/�������ͣ�
                            /// ����������һ���ڵ�
    kPointerEnd,      /// ������β��Ϊָ��������ս�����
    kReturnFunction,  /// ������ͼ���غ������Ǻ���ָ��
    kEmptyChain,      /// ���������κ�����
    kConstTagNotSame  /// ����POD����ʱ��������������Ҳ��ConstTag���Ͳ�ͬ
  };

  /// ��ֹʹ��EndType::GetEndType()����ֹ�޸�ȫ�ֹ���Ľڵ�
  template <class ObjectName>
  ObjectConstructData(ObjectName&& object_name)
      : object_name_(std::forward<ObjectName>(object_name)),
        type_chain_head_(std::make_shared<EndType>()),
        type_chain_tail_(type_chain_head_) {}

  /// ����ָ������
  /// �����ܹ���VarietyOperatorNode�������ڵ㣩
  /// ��FunctionDefine������ͷ��
  /// �����Ķ���д��objec_����ԭ��ָ��
  template <class BasicObjectType, class... Args>
  requires std::is_same_v<BasicObjectType, VarietyOperatorNode> ||
      std::is_same_v<BasicObjectType,
                     c_parser_frontend::flow_control::FunctionDefine>
          CheckResult ConstructBasicObjectPart(Args... args);

  /// ��β�ڵ������һ���ڵ㣬���ú��Զ�����β�ڵ�ָ������ӵĽڵ�
  /// ���һ��FunctionType���ٴε��øú���ʱ�Ὣ�ڵ�嵽�����ķ������Ͳ���
  /// ����һ��ֻ�����һ���ڵ�
  /// ���ò����еĽڵ����δ����next_node������ᱻ����
  /// ���յĽڵ�Ӧ��ConstructObject���
  /// ��ô�����Ծ�ȷ����ָ��/����/��������ֵ��const���
  template <class NextNodeType, class... Args>
  CheckResult AttachSingleNodeToTailNodeEmplace(Args&&... args) {
    auto new_node = std::make_shared<NextNodeType>(std::forward<Args>(args)...);
    return AttachSingleNodeToTailNodePointer(std::move(new_node));
  }
  /// ����ͬAttachSingleNodeToTailNodeEmplace������ʹ���Ѿ������õĽڵ�
  CheckResult AttachSingleNodeToTailNodePointer(
      std::shared_ptr<const TypeInterface>&& next_node);
  /// ��ɹ������̣��������̶���
  /// �����������ս�������ߵ�ConstTag��������ӵĽڵ�
  /// ������������׼ȷ�����ú�������ֵ/����/ָ���const���
  /// ������β�������ڱ��ڵ㣨Ӧ���ս���ڵ㹹��ʱ�Զ�������һ���ڵ�ΪEndType��
  /// �Զ������ȡ������ָ��Ĺ���
  /// ֻ����Announce������������Define������Define��������λ����������ε���
  std::pair<std::unique_ptr<FlowInterface>, CheckResult> ConstructObject(
      ConstTag const_tag_before_final_type,
      std::shared_ptr<const TypeInterface>&& final_node_to_attach);
  /// ��β���ڵ���Ӻ���������Ҫ��β���ڵ�Ϊ��������
  void AddFunctionTypeArgument(
      const std::shared_ptr<const VarietyOperatorNode>& argument) {
    assert(type_chain_tail_->GetType() == StructOrBasicType::kFunction);
    return static_cast<FunctionType&>(*type_chain_tail_)
        .AddFunctionCallArgument(argument);
  }
  /// ����ɹ�Լǰ��ȡ����Ķ������Ա���ӵ�ӳ�����
  /// ������ֵ���ã���������ӳ���ڵ������ƶ�����
  std::string&& GetObjectName() { return std::move(object_name_); }
  /// ��ȡ�����������ͣ�������ͷ�������ͣ�
  StructOrBasicType GetMainType() const {
    return type_chain_head_->GetNextNodeReference().GetType();
  }

 private:
  /// �����Ķ���
  std::unique_ptr<FlowInterface> object_;
  /// �����Ķ�������������ʱ�洢
  std::string object_name_;
  /// ָ����������ͷ���
  /// ȫ��ָ��ͷ���ڱ��ڵ�
  const std::shared_ptr<TypeInterface> type_chain_head_;
  /// ָ����������β�ڵ�
  std::shared_ptr<TypeInterface> type_chain_tail_;
};

/// ö�ٲ�����Լʱ�õ�������
class EnumReturnData {
 public:
  EnumType::EnumContainerType& GetContainer() { return enum_container_; }
  /// ���ز���λ�õĵ��������Ƿ����
  std::pair<EnumType::EnumContainerType::iterator, bool> AddMember(
      std::string&& member_name, long long value);
  long long GetMaxValue() const { return max_value_; }
  long long GetMinValue() const { return min_value_; }
  long long GetLastValue() const { return last_value_; }

 private:
  void SetMaxValue(long long max_value) { max_value_ = max_value; }
  void SetMinValue(long long min_value) { min_value_ = min_value; }
  void SetLastValue(long long last_value) { last_value_ = last_value; }

  /// ö������ֵ�Ĺ�������
  EnumType::EnumContainerType enum_container_;
  /// ����ö��ֵ
  long long max_value_ = LLONG_MIN;
  /// ��С��ö��ֵ
  long long min_value_ = LLONG_MAX;
  /// �ϴ���ӵ�ö��ֵ
  long long last_value_;
};

/// ����/�����������̱�����
/// �����������͹����Ķ�����
void VarietyOrFunctionConstructError(
    ObjectConstructData::CheckResult check_result,
    const std::string& object_name);

/// ��ConstTag�Ĺ�Լ��Ϣ�л�ȡConstTag����
/// ��Ҫ��֤����Ϊ���ս����ʽConstTag��Լ�õ������ݣ������Ƿ�Ϊ�չ�Լ��
ConstTag GetConstTag(const WordDataToUser& raw_const_tag_data);
/// ��SignTag�Ĺ�Լ��Ϣ�л�ȡSignTag����
/// ��Ҫ��֤����Ϊ���ս����ʽSignTag��Լ�õ������ݣ������Ƿ�Ϊ�չ�Լ��
SignTag GetSignTag(const WordDataToUser& raw_sign_tag_data);
/// ��������ʱ�ĳ�ʼ���ͻ�ȡһ�������зǵ�һ������������
std::shared_ptr<const TypeInterface> GetExtendAnnounceType(
    const std::shared_ptr<const TypeInterface>& source_type);
/// ��鸳ֵʱ���ͼ�����������Ӧ�Ĵ�����Ϣ�������error�򲻷���
void CheckAssignableCheckResult(AssignableCheckResult assignable_check_result);
/// ���������ѧ����ڵ�Ľ���������Ӧ������Ϣ�������error�򲻷���
void CheckMathematicalComputeTypeResult(
    DeclineMathematicalComputeTypeResult
        decline_mathematical_compute_type_result);
/// �������/��������ʱ�Ľ���������Ӧ������Ϣ�������error�򲻷���
void CheckAddTypeResult(AddTypeResult add_type_result);
/// ���������Ϣ
void OutputError(const std::string& error);
/// ���������Ϣ
void OutputWarning(const std::string& warning);
/// ���info��Ϣ
void OutputInfo(const std::string& info);

/// ��������ִ����������������������ʼֵ�ı���ע��ͻ�ȡ��չ���������͵Ĳ���
/// ������չ����ʱ���������ͣ���ȥ������ά����ָ�룬Ȼ�������Ϊ��ָ��������ȥ��
/// һ��ָ�룬���򲻱䣩
/// ������ConstTag�ͻ�ȡ����ʱʹ�õĲ���
/// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarietyAnnounceNoAssign(std::shared_ptr<VarietyOperatorNode>&& variety_node);
/// ��������ִ�����������������Ҹ���ʼֵ�ı���ע��ͻ�ȡ��չ���������Ͳ���
/// ������չ����ʱ���������ͣ���ȥ������ά����ָ�룬Ȼ�������Ϊ��ָ��������ȥ��
/// һ��ָ�룬���򲻱䣩
/// ������ConstTag�ͻ�ȡ����ʱʹ�õĲ���
/// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarietyAnnounceWithAssign(
    std::shared_ptr<VarietyOperatorNode>&& variety_node,
    const std::shared_ptr<const OperatorNodeInterface>& node_for_assign);
/// ����ǰ׺++/--�����
/// ����ʹ�õ������������++/--����������Ľڵ�ʹ洢��ȡ����Ĳ���������
/// �������������Ӳ����ڵ�
/// ���������Ŀ�����ڵ�
std::shared_ptr<const OperatorNodeInterface> PrefixPlusOrMinus(
    MathematicalOperation mathematical_operation,
    const std::shared_ptr<const OperatorNodeInterface>& node_to_operate,
    std::list<std::unique_ptr<FlowInterface>>* flow_control_node_container);
/// �����׺++/--�����
/// ����ʹ�õ������������++/--����������Ľڵ�ʹ洢��ȡ����Ĳ���������
/// �������������Ӳ����ڵ�
/// ���������õ��Ŀ�����ڵ�
std::shared_ptr<const OperatorNodeInterface> SuffixPlusOrMinus(
    MathematicalOperation mathematical_operation,
    const std::shared_ptr<const OperatorNodeInterface>& node_to_operate,
    std::list<std::unique_ptr<FlowInterface>>* flow_control_node_container);

/// ����ʽ��Լʱʹ�õĺ���
/// ��ʹ��std::unique_ptr��Ϊ�޷����ƣ��޷����ڹ���std::any&&
/// �������⺯�����붨����.cpp�ļ��У�������뱨��LNK2005�ظ�����

/// SingleConstexprValue -> Char
/// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueChar(
    std::vector<WordDataToUser>&& word_data);
/// SingleConstexprValue -> Str "[" Num "]"
/// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::shared_ptr<BasicTypeInitializeOperatorNode>
SingleConstexprValueIndexedString(std::vector<WordDataToUser>&& word_data);
/// SingleConstexprValue -> Num
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueNum(
    std::vector<WordDataToUser>&& word_data);
/// SingleConstexprValue -> Str
/// ���ԣ�InitializeType::String��TypeInterface:: const char*
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueString(
    std::vector<WordDataToUser>&& word_data);
/// FundamentalType -> "char"
BuiltInType FundamentalTypeChar(std::vector<WordDataToUser>&& word_data);
/// FundamentalType -> "short"
BuiltInType FundamentalTypeShort(std::vector<WordDataToUser>&& word_data);
/// FundamentalType -> "int"
BuiltInType FundamentalTypeInt(std::vector<WordDataToUser>&& word_data);
/// FundamentalType -> "long"
BuiltInType FundamentalTypeLong(std::vector<WordDataToUser>&& word_data);
/// FundamentalType -> "float"
BuiltInType FundamentalTypeFloat(std::vector<WordDataToUser>&& word_data);
/// FundamentalType -> "double"
BuiltInType FundamentalTypeDouble(std::vector<WordDataToUser>&& word_data);
/// FundamentalType -> "void"
BuiltInType FundamentalTypeVoid(std::vector<WordDataToUser>&& word_data);
/// SignTag  -> "signed"
SignTag SignTagSigned(std::vector<WordDataToUser>&& word_data);
/// SignTag -> "unsigned"
SignTag SignTagUnSigned(std::vector<WordDataToUser>&& word_data);
/// ConstTag -> "const"
ConstTag ConstTagConst(std::vector<WordDataToUser>&& word_data);
/// IdOrEquivence -> ConstTag Id
/// �����ڶ��ϣ�����any�����и��ƶ���
/// ʹ��std::shared_ptr����std::shared_ptr��std::any&&���ܴ洢��֧�ָ��Ƶ�����
std::shared_ptr<ObjectConstructData> IdOrEquivenceConstTagId(
    std::vector<WordDataToUser>&& word_data);
/// IdOrEquivence -> IdOrEquivence "[" Num "]"
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
/// ����std::any&&��ֹ�ƶ�����VarietyConstructData
std::any&& IdOrEquivenceNumAddressing(std::vector<WordDataToUser>&& word_data);
/// IdOrEquivence -> IdOrEquivence "[" "]"
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
/// ����std::any&&��ֹ�ƶ�����VarietyConstructData
/// ��������ӵ�ָ������Ӧ�����СΪ-1����Ǵ˴������С��Ҫ���ݸ�ֵ����ƶ�
std::any&& IdOrEquivenceAnonymousAddressing(
    std::vector<WordDataToUser>&& word_data);
/// IdOrEquivence -> ConstTag "*" IdOrEquivence
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
/// ����std::any&&��ֹ�ƶ�����VarietyConstructData
std::any&& IdOrEquivencePointerAnnounce(
    std::vector<WordDataToUser>&& word_data);
/// IdOrEquivence -> "(" IdOrEquivence ")"
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
/// ����std::any&&��ֹ�ƶ�����VarietyConstructData
std::any&& IdOrEquivenceInBrackets(std::vector<WordDataToUser>&& word_data);
/// AnonymousIdOrEquivence -> "const"
/// �����ڶ��ϣ�����any�����и��ƶ���
/// ʹ��std::shared_ptr����std::shared_ptr��std::any&&���ܴ洢��֧�ָ��Ƶ�����
std::shared_ptr<ObjectConstructData> AnonymousIdOrEquivenceConst(
    std::vector<WordDataToUser>&& word_data);
/// AnonymousIdOrEquivence -> AnonymousIdOrEquivence "[" Num "]"
extern std::any&& AnonymousIdOrEquivenceNumAddressing(
    std::vector<WordDataToUser>&& word_data);
/// AnonymousIdOrEquivence -> AnonymousIdOrEquivence "[" "]"
extern std::any&& AnonymousIdOrEquivenceAnonymousAddressing(
    std::vector<WordDataToUser>&& word_data);
/// AnonymousIdOrEquivence -> ConstTag "*" AnonymousIdOrEquivence
extern std::any&& AnonymousIdOrEquivencePointerAnnounce(
    std::vector<WordDataToUser>&& word_data);
/// AnonymousIdOrEquivence -> "(" AnonymousIdOrEquivence ")"
extern std::any&& AnonymousIdOrEquivenceInBrackets(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyEnumArguments -> Id
EnumReturnData NotEmptyEnumArgumentsIdBase(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyEnumArguments -> Id "=" Num
EnumReturnData NotEmptyEnumArgumentsIdAssignNumBase(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyEnumArguments -> NotEmptyEnumArguments "," Id
/// ����ֵ���ͣ�EnumReturnData
std::any&& NotEmptyEnumArgumentsIdExtend(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyEnumArguments -> NotEmptyEnumArguments "," Id "=" Num
/// ����ֵ���ͣ�EnumReturnData
std::any&& NotEmptyEnumArgumentsIdAssignNumExtend(
    std::vector<WordDataToUser>&& word_data);
/// EnumArguments -> NotEmptyEnumArguments
/// ����ֵ���ͣ�EnumReturnData
std::any&& EnumArgumentsNotEmptyEnumArguments(
    std::vector<WordDataToUser>&& word_data);
/// Enum -> "enum" Id "{" EnumArguments "}"
std::shared_ptr<EnumType> EnumDefine(std::vector<WordDataToUser>&& word_data);
/// Enum -> "enum" "{" EnumArguments "}"
std::shared_ptr<EnumType> EnumAnonymousDefine(
    std::vector<WordDataToUser>&& word_data);
/// EnumAnnounce -> "enum" Id
/// ���������Ľṹ����ṹ���ͣ�kStruct��
/// ���ⷵ������Ϊ����ṹ��͹����������������ͱ���һ��
std::pair<std::string, StructOrBasicType> EnumAnnounce(
    std::vector<WordDataToUser>&& word_data);
/// StructureAnnounce -> "struct" Id
/// ���������Ľṹ����ṹ���ͣ�kStruct��
std::pair<std::string, StructOrBasicType> StructureAnnounceStructId(
    std::vector<WordDataToUser>&& word_data);
/// StructureAnnounce -> "union" Id
/// ���������Ľṹ����ṹ���ͣ�kUnion��
std::pair<std::string, StructOrBasicType> StructureAnnounceUnionId(
    std::vector<WordDataToUser>&& word_data);
/// StructureDefineHead -> "struct"
std::pair<std::string, StructOrBasicType> StructureDefineHeadStruct(
    std::vector<WordDataToUser>&& word_data);
/// StructureDefineHead -> "union"
std::pair<std::string, StructOrBasicType> StructureDefineHeadUnion(
    std::vector<WordDataToUser>&& word_data);
/// StructureDefineHead -> StructureAnnounce
/// ����ֵ���ͣ�std::pair<std::string, StructOrBasicType>
/// ����ֵ�����StructureAnnounce
std::any&& StructureDefineHeadStructureAnnounce(
    std::vector<WordDataToUser>&& word_data);
/// StructureDefineInitHead -> StructureDefineHead "{"
/// ִ��һЩ��ʼ������
/// ���ؽṹ�������ͽڵ�
std::shared_ptr<StructureTypeInterface> StructureDefineInitHead(
    std::vector<WordDataToUser>&& word_data);
/// StructureDefine -> StructureDefineInitHead StructureBody "}"
/// ����ֵ���ͣ�std::shared_ptr<StructureTypeInterface>
/// ���ؽṹ�������ͽڵ�
std::any&& StructureDefine(std::vector<WordDataToUser>&& word_data);
/// StructType -> StructureDefine
std::shared_ptr<const StructureTypeInterface> StructTypeStructDefine(
    std::vector<WordDataToUser>&& word_data);
/// StructType -> StructAnnounce
/// ���ػ�ȡ���Ľṹ����������
std::shared_ptr<const StructureTypeInterface> StructTypeStructAnnounce(
    std::vector<WordDataToUser>&& word_data);
/// BasicType -> ConstTag SignTag FundamentalType
/// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeFundamental(
    std::vector<WordDataToUser>&& word_data);
/// BasicType -> ConstTag StructType
/// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeStructType(
    std::vector<WordDataToUser>&& word_data);
///// BasicType -> ConstTag Id
///// ���ػ�ȡ����������ConstTag
/// std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeId(
///    std::vector<WordDataToUser>&& word_data);
/// BasicType -> ConstTag EnumAnnounce
/// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeEnumAnnounce(
    std::vector<WordDataToUser>&& word_data);
/// FunctionRelaventBasePartFunctionInit -> IdOrEquivence "("
/// ��һЩ��ʼ������
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any&& FunctionRelaventBasePartFunctionInitBase(
    std::vector<WordDataToUser>&& word_data);
/// FunctionRelaventBasePartFunctionInit -> FunctionRelaventBasePart "("
/// ��һЩ��������/����ָ��ʱ�ĳ�ʼ������
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any&& FunctionRelaventBasePartFunctionInitExtend(
    std::vector<WordDataToUser>&& word_data);
/// FunctionRelaventBasePart -> FunctionRelaventBasePartFunctionInit
/// FunctionRelaventArguments ")"
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any&& FunctionRelaventBasePartFunction(
    std::vector<WordDataToUser>&& word_data);
/// FunctionRelaventBasePart -> ConstTag "*" FunctionRelaventBasePart
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any&& FunctionRelaventBasePartPointer(
    std::vector<WordDataToUser>&& word_data);
/// FunctionRelaventBasePart -> "(" FunctionRelaventBasePart ")"
/// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any&& FunctionRelaventBasePartBranckets(
    std::vector<WordDataToUser>&& word_data);
/// FunctionRelavent -> BasicType FunctionRelaventBasePart
/// ����ֵ��װ��ָ��ֻ����Ϊstd::shared_ptr<FunctionType>������������
/// ��std::shared_ptr<VarietyOperatorNode>������������
/// Define����������λ��������
std::shared_ptr<FlowInterface> FunctionRelavent(
    std::vector<WordDataToUser>&& word_data);
/// SingleAnnounceNoAssign -> BasicType IdOrEquivence
/// AnonymousSingleAnnounceNoAssign -> BasicType AnonymousIdOrEquivence
/// ��������ʽ���øù�Լ������ͨ��is_anonymous����ϸ΢���
/// is_anonymous�����Ƿ������������������������������ͣ�
/// ����ֵ��װ��ָ��ֻ����Ϊstd::shared_ptr<FunctionType>������������
/// ��std::shared_ptr<VarietyOperatorNode>������������
/// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
template <bool is_anonymous>
std::shared_ptr<FlowInterface> SingleAnnounceNoAssignVariety(
    std::vector<WordDataToUser>&& word_data);
/// SingleAnnounceNoAssign -> ConstTag Id IdOrEquivence
/// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
std::shared_ptr<FlowInterface> SingleAnnounceNoAssignNotPodVariety(
    std::vector<WordDataToUser>&& word_data);
/// SingleAnnounceNoAssign -> FunctionRelavent
/// AnonymousSingleAnnounceNoAssign -> FunctionRelavent
/// ��������ʽ���øù�Լ������ͨ��is_anonymous����ϸ΢���
/// is_anonymous�����Ƿ������������������������������ͣ�
/// ����ֵ���ͣ�std::shared_ptr<FlowInterface>
/// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
template <bool is_anonymous>
std::any&& SingleAnnounceNoAssignFunctionRelavent(
    std::vector<WordDataToUser>&& word_data);
/// TypeDef -> "typedef" SingleAnnounceNoAssign
/// ����������
std::any TypeDef(std::vector<WordDataToUser>&& word_data);
/// NotEmptyFunctionRelaventArguments -> SingleAnnounceNoAssign
/// ����������
std::any NotEmptyFunctionRelaventArgumentsBase(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyFunctionRelaventArguments -> AnonymousSingleAnnounceNoAssign
/// ����������
extern std::any NotEmptyFunctionRelaventArgumentsAnonymousBase(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyFunctionRelaventArguments -> NotEmptyFunctionRelaventArguments ","
/// SingleAnnounceNoAssign
/// ����ֵ���ͣ�std::shared_ptr<FunctionType::ArgumentInfoContainer>
std::any NotEmptyFunctionRelaventArgumentsExtend(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyFunctionRelaventArguments -> NotEmptyFunctionRelaventArguments ","
/// AnonymousSingleAnnounceNoAssign
/// ����ֵ���ͣ�std::shared_ptr<FunctionType::ArgumentInfoContainer>
extern std::any NotEmptyFunctionRelaventArgumentsAnonymousExtend(
    std::vector<WordDataToUser>&& word_data);
/// FunctionRelaventArguments -> NotEmptyFunctionRelaventArguments
/// ����ֵ���ͣ�std::shared_ptr<FunctionType:ArgumentInfoContainer>
std::any&& FunctionRelaventArguments(std::vector<WordDataToUser>&& word_data);
/// FunctionDefineHead -> FunctionRelavent "{"
/// �Ժ��������ÿ������ִ��Define��ע�ắ�����ͺͺ�����Ӧ�ı��������ڲ��ң�
std::shared_ptr<c_parser_frontend::flow_control::FunctionDefine>
FunctionDefineHead(std::vector<WordDataToUser>&& word_data);
/// FunctionDefine -> FunctionDefineHead Statements "}"
/// ����ֵ���ͣ�std::shared_ptr<FunctionDefine>
/// �����ӵĺ������Ƿ�Ϊ�գ����Ϊ��������޷���ֵ����Error
/// �������һ�����������õ�ǰ��Ծ����
/// �������κ�����
std::any FunctionDefine(std::vector<WordDataToUser>&& word_data);
/// SingleStructureBody -> SingleAnnounceNoAssign
/// ������չ������IDʹ�õ����ͺͱ��������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
SingleStructureBodyBase(std::vector<WordDataToUser>&& word_data);
/// SingleStructureBody -> SingleStructureBody "," Id
/// ����ֵ���ͣ�std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
/// ������չ������IDʹ�õ����ͺͱ��������ConstTag
std::any&& SingleStructureBodyExtend(std::vector<WordDataToUser>&& word_data);
/// NotEmptyStructureBody -> SingleStructureBody
/// �������κ�����
std::any NotEmptyStructureBodyBase(std::vector<WordDataToUser>&& word_data);
/// NotEmptyStructureBody -> NotEmptyStructureBody SingleStructureBody ";"
/// �������κ�����
std::any NotEmptyStructureBodyExtend(std::vector<WordDataToUser>&& word_data);
/// StructureBody -> NotEmptyStructureBody
/// �������κ�����
std::any StructureBody(std::vector<WordDataToUser>&& word_data);
/// InitializeList -> "{" InitializeListArguments "}"
std::shared_ptr<ListInitializeOperatorNode> InitializeList(
    std::vector<WordDataToUser>&& word_data);
/// SingleInitializeListArgument -> SingleConstexprValue
/// ����ֵ���ͣ�std::shared_ptr<InitializeOperatorNodeInterface>
std::shared_ptr<InitializeOperatorNodeInterface>
SingleInitializeListArgumentConstexprValue(
    std::vector<WordDataToUser>&& word_data);
/// SingleInitializeListArgument -> InitializeList
/// ����ֵ���ͣ�std::shared_ptr<InitializeOperatorNodeInterface>
std::any&& SingleInitializeListArgumentList(
    std::vector<WordDataToUser>&& word_data);
/// InitializeListArguments -> SingleInitializeListArgument
/// ����ֵ���ͣ�
std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>
InitializeListArgumentsBase(std::vector<WordDataToUser>&& word_data);
/// InitializeListArguments -> InitializeListArguments ","
/// SingleInitializeListArgument
/// ����ֵ���ͣ�
/// std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>
std::any&& InitializeListArgumentsExtend(
    std::vector<WordDataToUser>&& word_data);
/// AnnounceAssignable -> Assignable
/// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
///                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any&& AnnounceAssignableAssignable(
    std::vector<WordDataToUser>&& word_data);
/// AnnounceAssignable -> InitializeList
/// ���ؿ�����
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AnnounceAssignableInitializeList(std::vector<WordDataToUser>&& word_data);
/// SingleAnnounceAndAssign -> SingleAnnounceNoAssign
/// ������չ����ʱ���������ͣ���ȥ������ά����ָ�룬
/// �����Ϊ��ָ��������ȥ��һ��ָ�룩
/// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
/// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignNoAssignBase(std::vector<WordDataToUser>&& word_data);
/// SingleAnnounceAndAssign -> SingleAnnounceNoAssign "=" AnnounceAssignable
/// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
/// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
/// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignWithAssignBase(std::vector<WordDataToUser>&& word_data);
/// SingleAnnounceAndAssign -> SingleAnnounceAndAssign "," Id
/// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
/// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
/// ����ֵ���ͣ�std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
///                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any&& SingleAnnounceAndAssignNoAssignExtend(
    std::vector<WordDataToUser>&& word_data);
/// SingleAnnounceAndAssign -> SingleAnnounceAndAssign "," Id "="
/// AnnounceAssignable
/// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
/// ����ֵ���ͣ�std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
///                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any&& SingleAnnounceAndAssignWithAssignExtend(
    std::vector<WordDataToUser>&& word_data);
/// Type -> BasicType
/// �������ͺͱ�����ConstTag
/// ����ֵ���ͣ�std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
std::any&& TypeBasicType(std::vector<WordDataToUser>&& word_data);
/// Type -> FunctionRelavent
/// �������ͺͱ�����ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> TypeFunctionRelavent(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "+"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorPlus(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "-"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorMinus(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "*"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorMultiple(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "/"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorDivide(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "%"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorMod(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "<<"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorLeftShift(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> ">>"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorRightShift(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "&"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorAnd(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "|"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorOr(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "^"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorXor(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalOperator -> "!"
/// ������ѧ�����
MathematicalOperation MathematicalOperatorNot(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "+="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorPlusAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "-="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorMinusAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "*="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorMultipleAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "/="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorDivideAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "%="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorModAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "<<="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorLeftShiftAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> ">>="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorRightShiftAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "&="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorAndAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "|="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorOrAssign(
    std::vector<WordDataToUser>&& word_data);
/// MathematicalAndAssignOperator -> "^="
/// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorXorAssign(
    std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> "&&"
/// �����߼������
LogicalOperation LogicalOperatorAndAnd(std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> "||"
/// �����߼������
LogicalOperation LogicalOperatorOrOr(std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> ">"
/// �����߼������
LogicalOperation LogicalOperatorGreater(
    std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> ">="
/// �����߼������
LogicalOperation LogicalOperatorGreaterEqual(
    std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> "<"
/// �����߼������
LogicalOperation LogicalOperatorLess(std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> "<="
/// �����߼������
LogicalOperation LogicalOperatorLessEqual(
    std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> "=="
/// �����߼������
LogicalOperation LogicalOperatorEqual(std::vector<WordDataToUser>&& word_data);
/// LogicalOperator -> "!="
/// �����߼������
LogicalOperation LogicalOperatorNotEqual(
    std::vector<WordDataToUser>&& word_data);
/// Assignable -> SingleConstexprValue
/// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableConstexprValue(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Id
/// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableId(std::vector<WordDataToUser>&& word_data);
/// Assignable -> TemaryOperator
/// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
/// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
///                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any&& AssignableTemaryOperator(std::vector<WordDataToUser>&& word_data);
/// Assignable -> FunctionCall
/// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
///                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any&& AssignableFunctionCall(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "sizeof" "(" GetType ")"
/// ������һ���õ������տ�����ڵ�Ϳ�������sizeof���壩
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfType(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "sizeof" "(" Assignable ")"
/// ������һ���õ������տ�����ڵ�Ϳ�������sizeof���壩
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfAssignable(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable "." Id
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMemberAccess(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable "->" Id
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePointerMemberAccess(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "(" Assignable ")"
/// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
/// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
///                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any&& AssignableBracket(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "(" GetType ")" Assignable
/// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableTypeConvert(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable "=" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableAssign(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable MathematicalOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalOperate(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable MathematicalAndAssignOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalAndAssignOperate(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable LogicalOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableLogicalOperate(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "!" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableNot(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "~" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableLogicalNegative(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "-" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalNegative(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "&" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableObtainAddress(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "*" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableDereference(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable "[" Assignable "]"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableArrayAccess(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "++" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixPlus(std::vector<WordDataToUser>&& word_data);
/// Assignable -> "--" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixMinus(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable "++"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixPlus(std::vector<WordDataToUser>&& word_data);
/// Assignable -> Assignable "--"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixMinus(std::vector<WordDataToUser>&& word_data);
/// Return -> "return" Assignable ";"
/// �������κ�ֵ
std::any ReturnWithValue(std::vector<WordDataToUser>&& word_data);
/// Return -> "return" ";"
/// �������κ�ֵ
std::any ReturnWithoutValue(std::vector<WordDataToUser>&& word_data);
/// TemaryOperator -> Assignable "?" Assignable ":" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
TemaryOperator(std::vector<WordDataToUser>&& word_data);
/// NotEmptyFunctionCallArguments -> Assignable
/// �������κ�����
std::any NotEmptyFunctionCallArgumentsBase(
    std::vector<WordDataToUser>&& word_data);
/// NotEmptyFunctionCallArguments -> NotEmptyFunctionCallArguments ","
/// Assignable ����ֵ���ͣ�
/// std::shared_ptr<FunctionCallOperatorNode::FunctionCallArgumentsContainer>
std::any NotEmptyFunctionCallArgumentsExtend(
    std::vector<WordDataToUser>&& word_data);
/// FunctionCallArguments -> NotEmptyFunctionCallArguments
/// ����ֵ���ͣ�
/// std::shared_ptr<FunctionCallOperatorNode::FunctionCallArgumentsContainer>
std::any&& FunctionCallArguments(std::vector<WordDataToUser>&& word_data);
/// FunctionCallInit -> Assignable "("
/// ��һЩ��ʼ������
/// ���غ������ö���ͻ�ȡ�ɵ��ö���Ĳ�����ͬʱ����ȫ�ֱ���
std::pair<std::shared_ptr<FunctionCallOperatorNode>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCallInit(std::vector<WordDataToUser>&& word_data);
/// FunctionCall -> FunctionCallInit FunctionCallArguments ")"
/// ���غ������ö���
/// �ڶ��������洢��ȡ�ɵ��ö�������ͶԿɵ��ö���ĵ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCall(std::vector<WordDataToUser>&& word_data);
/// Assignables -> Assignable
/// ���ر���������̵Ľڵ�����
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>> AssignablesBase(
    std::vector<WordDataToUser>&& word_data);
/// Assignables -> Assignables "," Assignable
/// ����ֵ���ͣ�std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
std::any&& AssignablesExtend(std::vector<WordDataToUser>&& word_data);
/// Break -> "break" ";"
/// ������ת��䣨ʹ��shared_ptr��װ��Ϊstd::any&&��֧�ִ洢���ɸ��Ƶ�ֵ��
std::shared_ptr<std::unique_ptr<Jmp>> Break(
    std::vector<WordDataToUser>&& word_data);
/// Continue -> "continue" ";"
/// ������ת��䣨ʹ��shared_ptr��װ��Ϊstd::any&&��֧�ִ洢���ɸ��Ƶ�ֵ��
std::shared_ptr<std::unique_ptr<Jmp>> Continue(
    std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> If
/// �����κβ���
std::any SingleStatementIf(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> DoWhile
/// �����κβ���
std::any SingleStatementDoWhile(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> While
/// �����κβ���
std::any SingleStatementWhile(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> For
/// �����κβ���
std::any SingleStatementFor(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> Switch
/// �����κβ���
std::any SingleStatementSwitch(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> Assignable ";"
/// ��ӻ�ȡAssignable�����̿������
/// �������κ�ֵ
std::any SingleStatementAssignable(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> SingleAnnounceAndAssign ";"
/// ����������������е����̿������
/// �������κ�ֵ
std::any SingleStatementAnnounce(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> Return
/// �����κβ���
std::any SingleStatementReturn(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> Break
/// �������κ�ֵ
std::any SingleStatementBreak(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> Continue
/// �������κ�ֵ
std::any SingleStatementContinue(std::vector<WordDataToUser>&& word_data);
/// SingleStatement -> ";"
/// �����κβ���
std::any SingleStatementEmptyStatement(std::vector<WordDataToUser>&& word_data);
/// IfCondition -> "if" "(" Assignable ")"
/// ����if�����ڵ�ͻ�ȡif����������ʹ�õĲ���
/// ���������parser_frontend��ע�����̿������
/// �������κ�ֵ
std::any IfCondition(std::vector<WordDataToUser>&& word_data);
/// IfWithElse -> IfCondition ProcessControlSentenceBody "else"
/// ת����if-else���
/// �������κ�ֵ
std::any IfWithElse(std::vector<WordDataToUser>&& word_data);
/// If->IfWithElse ProcessControlSentenceBody
/// �������κ�ֵ
std::any IfElseSence(std::vector<WordDataToUser>&& word_data);
/// If -> IfCondition ProcessControlSentenceBody
/// �������κ�ֵ
std::any IfIfSentence(std::vector<WordDataToUser>&& word_data);
/// ForRenewSentence -> Assignables
/// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
///                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any&& ForRenewSentence(std::vector<WordDataToUser>&& word_data);
/// ForInitSentence -> Assignables
/// ����ֵ���ͣ�std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
std::any&& ForInitSentenceAssignables(std::vector<WordDataToUser>&& word_data);
/// ForInitSentence -> SingleAnnounceAndAssign
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
ForInitSentenceAnnounce(std::vector<WordDataToUser>&& word_data);
/// ForInitHead -> "for"
/// ��һЩ׼������
/// �������κ�ֵ
std::any ForInitHead(std::vector<WordDataToUser>&& word_data);
/// ForHead -> ForInitHead "(" ForInitSentence ";"
///            Assignable ";" ForRenewSentence ")"
/// ��Լfor������Ҫ��
/// �������κ�ֵ
std::any ForHead(std::vector<WordDataToUser>&& word_data);
/// For -> ForHead ProcessControlSentenceBody
/// ����������
/// �������κ�ֵ
std::any For(std::vector<WordDataToUser>&& word_data);
/// WhileInitHead -> "while" "(" Assignable ")"
/// �������κ�ֵ
std::any WhileInitHead(std::vector<WordDataToUser>&& word_data);
/// While -> WhileInitHead ProcessControlSentenceBody
/// ����������
/// �������κ�ֵ
std::any While(std::vector<WordDataToUser>&& word_data);
/// DoWhileInitHead -> "do"
/// ��һЩ׼������
/// �������κ�����
std::any DoWhileInitHead(std::vector<WordDataToUser>&& word_data);
/// DoWhile -> DoWhileInitHead ProcessControlSentenceBody
/// while "(" Assignable ")" ";"
/// �������κ�����
std::any DoWhile(std::vector<WordDataToUser>&& word_data);
/// SwitchCase -> "case" SingleConstexprValue ":"
/// �������κ�����
std::any SwitchCaseSimple(std::vector<WordDataToUser>&& word_data);
/// SwitchCase -> "default" ":"
/// �������κ�����
std::any SwitchCaseDefault(std::vector<WordDataToUser>&& word_data);
/// SingleSwitchStatement -> SwitchCase
/// �����κβ���
std::any SingleSwitchStatementCase(std::vector<WordDataToUser>&& word_data);
/// SingleSwitchStatement -> Statements
/// �����κβ���
std::any SingleSwitchStatementStatements(
    std::vector<WordDataToUser>&& word_data);
/// SwitchStatements -> SwitchStatements SingleSwitchStatement
/// �����κβ���
std::any SwitchStatements(std::vector<WordDataToUser>&& word_data);
/// SwitchCondition -> "switch" "(" Assignable ")"
/// �������κ�ֵ
std::any SwitchCondition(std::vector<WordDataToUser>&& word_data);
/// Switch -> SwitchCondition "{" SwitchStatements "}"
/// �����κβ���
std::any Switch(std::vector<WordDataToUser>&& word_data);
/// Statements -> Statements SingleStatement
/// �����κβ���
std::any StatementsSingleStatement(std::vector<WordDataToUser>&& word_data);
/// StatementsLeftBrace -> Statements "{"
/// ����������ȼ�
/// �������κ�ֵ
std::any StatementsLeftBrace(std::vector<WordDataToUser>&& word_data);
/// Statements -> StatementsLeftBrace Statements "}"
/// ����������
/// �������κ�ֵ
std::any StatementsBrace(std::vector<WordDataToUser>&& word_data);
/// ProcessControlSentenceBody -> SingleStatement
/// ������������
/// �����κβ���
std::any ProcessControlSentenceBodySingleStatement(
    std::vector<WordDataToUser>&& word_data);
/// ProcessControlSentenceBody -> "{" Statements "}"
/// ������������
/// �����κβ���
std::any ProcessControlSentenceBodyStatements(
    std::vector<WordDataToUser>&& word_data);
/// ������ʽ
/// Root -> Root FunctionDefine
/// �����κβ���
std::any RootFunctionDefine(std::vector<WordDataToUser>&& word_data);
/// ������ʽ
/// Root -> Root SingleAnnounceNoAssign ";"
/// �������κ�ֵ
std::any RootAnnounce(std::vector<WordDataToUser>&& word_data);

template <class BasicObjectType, class... Args>
requires std::is_same_v<BasicObjectType, VarietyOperatorNode> ||
    std::is_same_v<BasicObjectType,
                   c_parser_frontend::flow_control::FunctionDefine>
        ObjectConstructData::CheckResult
        ObjectConstructData::ConstructBasicObjectPart(Args... args) {
  if constexpr (std::is_same_v<BasicObjectType, VarietyOperatorNode>) {
    /// ������������
    auto object = std::make_unique<SimpleSentence>();
    bool result = object->SetSentenceOperateNode(
        std::make_shared<VarietyOperatorNode>(std::forward<Args>(args)...));
    assert(result);
    object_ = std::move(object);
  } else {
    /// ������������
    object_ = std::make_unique<c_parser_frontend::flow_control::FunctionDefine>(
        std::forward<Args>(args)...);
  }
  return CheckResult::kSuccess;
}
template <bool is_anonymous>
std::shared_ptr<FlowInterface> SingleAnnounceNoAssignVariety(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto [final_type, const_tag_before_final_type] =
      std::any_cast<std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  std::shared_ptr<ObjectConstructData> construct_data;
  if constexpr (is_anonymous) {
    if (word_data[1].Empty()) [[likely]] {
      /// ��������ָ��������ʡ�Բ�����
      construct_data = std::make_shared<ObjectConstructData>(std::string());
      construct_data->ConstructBasicObjectPart<VarietyOperatorNode>(
          std::string(), const_tag_before_final_type,
          LeftRightValueTag::kLeftValue);
    } else {
      construct_data = std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[1].GetNonTerminalWordData().user_returned_data);
      assert(construct_data->GetObjectName().empty());
    }
  } else {
    construct_data = std::any_cast<std::shared_ptr<ObjectConstructData>&>(
        word_data[1].GetNonTerminalWordData().user_returned_data);
    if (construct_data->GetObjectName().empty()) [[unlikely]] {
      OutputError(std::format("�����ı�����������"));
      exit(-1);
    }
  }
  auto [flow_control_node, construct_result] = construct_data->ConstructObject(
      const_tag_before_final_type, std::move(final_type));
  /// ����Ƿ񹹽��ɹ�
  if (construct_result != ObjectConstructData::CheckResult::kSuccess)
      [[unlikely]] {
    VarietyOrFunctionConstructError(construct_result,
                                    construct_data->GetObjectName());
  }
  return std::move(flow_control_node);
}

/// SingleAnnounceNoAssign -> FunctionRelavent
/// AnonymousSingleAnnounceNoAssign -> FunctionRelavent
/// ��������ʽ���øù�Լ������ͨ��is_anonymous����ϸ΢���
/// is_anonymous�����Ƿ������������������������������ͣ�
/// ����ֵ���ͣ�std::shared_ptr<FlowInterface>
/// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
template <bool is_anonymous>
std::any&& SingleAnnounceNoAssignFunctionRelavent(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& flow_control_node = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  if constexpr (is_anonymous) {
    auto& variety_node = static_cast<const VarietyOperatorNode&>(
        static_cast<SimpleSentence&>(*flow_control_node)
            .GetSentenceOperateNodeReference());
    assert(variety_node.GetVarietyName().empty());
  } else {
    const std::string* variety_name;
    switch (flow_control_node->GetFlowType()) {
      case FlowType::kSimpleSentence:
        variety_name = &static_cast<const VarietyOperatorNode&>(
                            static_cast<SimpleSentence&>(*flow_control_node)
                                .GetSentenceOperateNodeReference())
                            .GetVarietyName();
        break;
      case FlowType::kFunctionDefine:
        variety_name = &static_cast<const flow_control::FunctionDefine&>(
                            *flow_control_node)
                            .GetFunctionTypeReference()
                            .GetFunctionName();
        break;
      default:
        assert(false);
        break;
    }
    if (variety_name->empty()) [[unlikely]] {
      OutputError(std::format("�����ı�����������"));
      exit(-1);
    }
  }
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
/// ����ͬAttachSingleNodeToTailNodeEmplace������ʹ���Ѿ������õĽڵ�

}  // namespace c_parser_frontend::parse_functions
#endif