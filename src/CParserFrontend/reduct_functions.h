#ifndef CPARSERFRONTEND_PARSE_FUNCTIONS_H_
#define CPARSERFRONTEND_PARSE_FUNCTIONS_H_
#include <format>
#include <iostream>
#include <limits>

#include "c_parser_frontend.h"
#include "flow_control.h"
#include "operator_node.h"
#include "type_system.h"
namespace c_parser_frontend {
// �����߳�ȫ�ֱ�����C���Ա�����ǰ�˵������
extern thread_local CParserFrontend c_parser_controller;
}  // namespace c_parser_frontend

namespace c_parser_frontend::parse_functions {
using c_parser_frontend::c_parser_controller;
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
// �����ṹ����ʱʹ�õ�ȫ�ֱ����������Ż�ִ���߼�
// �������ڹ����Ľṹ��������
static thread_local std::shared_ptr<StructureTypeInterface>
    structure_type_constructuring;
// ������������ʱʹ�õ�ȫ�ֱ����������Ż�ִ���߼�
// ������������ݵ���
static thread_local std::shared_ptr<ObjectConstructData>
    function_type_construct_data;
// �����������ýڵ�ʱʹ�õ�ȫ�ֱ����������Ż�ִ���߼�
static thread_local std::shared_ptr<FunctionCallOperatorNode>
    function_call_operator_node;

// ������������/��������ʱʹ�õ�����
class ObjectConstructData {
 public:
  // ���������������̵ļ����
  enum class CheckResult {
    // �ɹ������
    kSuccess,
    // ʧ�ܵ����
    kAttachToTerminalType,  // β�ڵ��Ѿ����ս����ͣ��ṹ��/�������ͣ�
                            // ����������һ���ڵ�
    kPointerEnd,      // ������β��Ϊָ��������ս�����
    kReturnFunction,  // ������ͼ���غ������Ǻ���ָ��
    kEmptyChain,      // ���������κ�����
    kConstTagNotSame  // ����POD����ʱ��������������Ҳ��ConstTag���Ͳ�ͬ
  };

  // ��ֹʹ��EndType::GetEndType()����ֹ�޸�ȫ�ֹ���Ľڵ�
  template <class ObjectName>
  ObjectConstructData(ObjectName&& object_name)
      : object_name_(std::forward<ObjectName>(object_name)),
        type_chain_head_(std::make_shared<EndType>()),
        type_chain_tail_(type_chain_head_) {}

  // ����ָ������
  // �����ܹ���VarietyOperatorNode�������ڵ㣩
  // ��FunctionDefine������ͷ��
  // �����Ķ���д��objec_����ԭ��ָ��
  template <class BasicObjectType, class... Args>
    requires std::is_same_v<BasicObjectType, VarietyOperatorNode> ||
             std::is_same_v<BasicObjectType,
                            c_parser_frontend::flow_control::FunctionDefine>
  CheckResult ConstructBasicObjectPart(Args... args);

  // ��β�ڵ������һ���ڵ㣬���ú��Զ�����β�ڵ�ָ������ӵĽڵ�
  // ���һ��FunctionType���ٴε��øú���ʱ�Ὣ�ڵ�嵽�����ķ������Ͳ���
  // ����һ��ֻ�����һ���ڵ�
  // ���ò����еĽڵ����δ����next_node������ᱻ����
  // ���յĽڵ�Ӧ��ConstructObject���
  // ��ô�����Ծ�ȷ����ָ��/����/��������ֵ��const���
  template <class NextNodeType, class... Args>
  CheckResult AttachSingleNodeToTailNodeEmplace(Args&&... args) {
    auto new_node = std::make_shared<NextNodeType>(std::forward<Args>(args)...);
    return AttachSingleNodeToTailNodePointer(std::move(new_node));
  }
  // ����ͬAttachSingleNodeToTailNodeEmplace������ʹ���Ѿ������õĽڵ�
  CheckResult AttachSingleNodeToTailNodePointer(
      std::shared_ptr<const TypeInterface>&& next_node);
  // ��ɹ������̣��������̶���
  // �����������ս�������ߵ�ConstTag��������ӵĽڵ�
  // ������������׼ȷ�����ú�������ֵ/����/ָ���const���
  // ������β�������ڱ��ڵ㣨Ӧ���ս���ڵ㹹��ʱ�Զ�������һ���ڵ�ΪEndType��
  // �Զ������ȡ������ָ��Ĺ���
  // ֻ����Announce������������Define������Define��������λ����������ε���
  std::pair<std::unique_ptr<FlowInterface>, CheckResult> ConstructObject(
      ConstTag const_tag_before_final_type,
      std::shared_ptr<const TypeInterface>&& final_node_to_attach);
  // ��β���ڵ���Ӻ���������Ҫ��β���ڵ�Ϊ��������
  void AddFunctionTypeArgument(
      const std::shared_ptr<const VarietyOperatorNode>& argument) {
    assert(type_chain_tail_->GetType() == StructOrBasicType::kFunction);
    return static_cast<FunctionType&>(*type_chain_tail_)
        .AddFunctionCallArgument(argument);
  }
  // ����ɹ�Լǰ��ȡ����Ķ������Ա���ӵ�ӳ�����
  // ������ֵ���ã���������ӳ���ڵ������ƶ�����
  std::string&& GetObjectName() { return std::move(object_name_); }
  // ��ȡ�����������ͣ�������ͷ�������ͣ�
  StructOrBasicType GetMainType() const {
    return type_chain_head_->GetNextNodeReference().GetType();
  }

 private:
  // �����Ķ���
  std::unique_ptr<FlowInterface> object_;
  // �����Ķ�������������ʱ�洢
  std::string object_name_;
  // ָ����������ͷ���
  // ȫ��ָ��ͷ���ڱ��ڵ�
  const std::shared_ptr<TypeInterface> type_chain_head_;
  // ָ����������β�ڵ�
  std::shared_ptr<TypeInterface> type_chain_tail_;
};

// ö�ٲ�����Լʱ�õ�������
class EnumReturnData {
 public:
  EnumType::EnumContainerType& GetContainer() { return enum_container_; }
  // ���ز���λ�õĵ��������Ƿ����
  std::pair<EnumType::EnumContainerType::iterator, bool> AddMember(
      std::string&& member_name, long long value);
  long long GetMaxValue() const { return max_value_; }
  long long GetMinValue() const { return min_value_; }
  long long GetLastValue() const { return last_value_; }

 private:
  void SetMaxValue(long long max_value) { max_value_ = max_value; }
  void SetMinValue(long long min_value) { min_value_ = min_value; }
  void SetLastValue(long long last_value) { last_value_ = last_value; }

  // ö������ֵ�Ĺ�������
  EnumType::EnumContainerType enum_container_;
  // ����ö��ֵ
  long long max_value_ = LLONG_MIN;
  // ��С��ö��ֵ
  long long min_value_ = LLONG_MAX;
  // �ϴ���ӵ�ö��ֵ
  long long last_value_;
};

// ����/�����������̱�����
// �����������͹����Ķ�����
void VarietyOrFunctionConstructError(
    ObjectConstructData::CheckResult check_result,
    const std::string& object_name);

// ��������ʱ�ĳ�ʼ���ͻ�ȡһ�������зǵ�һ������������
std::shared_ptr<const TypeInterface> GetExtendAnnounceType(
    const std::shared_ptr<const TypeInterface>& source_type);
// ��鸳ֵʱ���ͼ�����������Ӧ�Ĵ�����Ϣ�������error�򲻷���
void CheckAssignableCheckResult(AssignableCheckResult assignable_check_result);
// ���������ѧ����ڵ�Ľ���������Ӧ������Ϣ�������error�򲻷���
void CheckMathematicalComputeTypeResult(
    DeclineMathematicalComputeTypeResult
        decline_mathematical_compute_type_result);
// �������/��������ʱ�Ľ���������Ӧ������Ϣ�������error�򲻷���
void CheckAddTypeResult(AddTypeResult add_type_result);
// ���������Ϣ
void OutputError(const std::string& error);
// ���������Ϣ
void OutputWarning(const std::string& warning);
// ���info��Ϣ
void OutputInfo(const std::string& info);

// ��������ִ����������������������ʼֵ�ı���ע��ͻ�ȡ��չ���������͵Ĳ���
// ������չ����ʱ���������ͣ���ȥ������ά����ָ�룬Ȼ�������Ϊ��ָ��������ȥ��
// һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʱʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarietyAnnounceNoAssign(std::shared_ptr<VarietyOperatorNode>&& variety_node);
// ��������ִ�����������������Ҹ���ʼֵ�ı���ע��ͻ�ȡ��չ���������Ͳ���
// ������չ����ʱ���������ͣ���ȥ������ά����ָ�룬Ȼ�������Ϊ��ָ��������ȥ��
// һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʱʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarietyAnnounceWithAssign(
    std::shared_ptr<VarietyOperatorNode>&& variety_node,
    const std::shared_ptr<const OperatorNodeInterface>& node_for_assign);
// ����ǰ׺++/--�����
// ����ʹ�õ������������++/--����������Ľڵ�ʹ洢��ȡ����Ĳ���������
// �������������Ӳ����ڵ�
// ���������Ŀ�����ڵ�
std::shared_ptr<const OperatorNodeInterface> PrefixPlusOrMinus(
    MathematicalOperation mathematical_operation,
    const std::shared_ptr<const OperatorNodeInterface>& node_to_operate,
    std::list<std::unique_ptr<FlowInterface>>* flow_control_node_container);
// �����׺++/--�����
// ����ʹ�õ������������++/--����������Ľڵ�ʹ洢��ȡ����Ĳ���������
// �������������Ӳ����ڵ�
// ���������õ��Ŀ�����ڵ�
std::shared_ptr<const OperatorNodeInterface> SuffixPlusOrMinus(
    MathematicalOperation mathematical_operation,
    const std::shared_ptr<const OperatorNodeInterface>& node_to_operate,
    std::list<std::unique_ptr<FlowInterface>>* flow_control_node_container);

// ����ʽ��Լʱʹ�õĺ���
// ��ʹ��std::unique_ptr��Ϊ�޷����ƣ��޷����ڹ���std::any&&
// �������⺯�����붨����.cpp�ļ��У�������뱨��LNK2005�ظ�����

// SingleConstexprValue -> Char
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueChar(
    std::string&& word_data);
// SingleConstexprValue -> Str "[" Num "]"
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::shared_ptr<BasicTypeInitializeOperatorNode>
SingleConstexprValueIndexedString(std::string&& str,
                                  std::string&& left_square_bracket,
                                  std::string&& num,
                                  std::string&& right_square_bracket);
// SingleConstexprValue -> Num
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueNum(
    std::string&& num);
// SingleConstexprValue -> Str
// ���ԣ�InitializeType::String��TypeInterface:: const char*
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueString(
    std::string&& str);
// FundamentalType -> "char"
BuiltInType FundamentalTypeChar(std::string&& str);
// FundamentalType -> "short"
BuiltInType FundamentalTypeShort(std::string&& str);
// FundamentalType -> "int"
BuiltInType FundamentalTypeInt(std::string&& str);
// FundamentalType -> "long"
BuiltInType FundamentalTypeLong(std::string&& str);
// FundamentalType -> "float"
BuiltInType FundamentalTypeFloat(std::string&& str);
// FundamentalType -> "double"
BuiltInType FundamentalTypeDouble(std::string&& str);
// FundamentalType -> "void"
BuiltInType FundamentalTypeVoid(std::string&& str);
// SignTag  -> "signed"
SignTag SignTagSigned(std::string&& str);
// SignTag -> "unsigned"
SignTag SignTagUnSigned(std::string&& str);
// ConstTag -> "const"
ConstTag ConstTagConst(std::string&& str);
// IdOrEquivence -> ConstTag Id
// ʹ��std::shared_ptr��װ��std::any���ܴ洢��֧�ָ��Ƶ�����
std::shared_ptr<ObjectConstructData> IdOrEquivenceConstTagId(ConstTag const_tag,
                                                             std::string&& id);
// IdOrEquivence -> IdOrEquivence "[" Num "]"
std::shared_ptr<ObjectConstructData>&& IdOrEquivenceNumAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& num,
    std::string&& right_square_bracket);
// IdOrEquivence -> IdOrEquivence "[" "]"
// ��������ӵ�ָ������Ӧ�����СΪ-1����Ǵ˴������С��Ҫ���ݸ�ֵ����ƶ�
std::shared_ptr<ObjectConstructData>&& IdOrEquivenceAnonymousAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& right_square_bracket);
// IdOrEquivence -> ConstTag "*" IdOrEquivence
std::shared_ptr<ObjectConstructData>&& IdOrEquivencePointerAnnounce(
    ConstTag const_tag, std::string&& operator_pointer,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result);
// IdOrEquivence -> "(" IdOrEquivence ")"
std::shared_ptr<ObjectConstructData>&& IdOrEquivenceInBrackets(
    std::string&& left_bracket,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& right_bracket);
// AnonymousIdOrEquivence -> "const"
// ʹ��std::shared_ptr��std::any���ܴ洢��֧�ָ��Ƶ�����
std::shared_ptr<ObjectConstructData> AnonymousIdOrEquivenceConst(
    std::string&& str_const);
// AnonymousIdOrEquivence -> AnonymousIdOrEquivence "[" Num "]"
std::shared_ptr<ObjectConstructData>&& AnonymousIdOrEquivenceNumAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& num,
    std::string&& right_square_bracket);
// AnonymousIdOrEquivence -> AnonymousIdOrEquivence "[" "]"
std::shared_ptr<ObjectConstructData>&&
AnonymousIdOrEquivenceAnonymousAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& right_square_bracket);
// AnonymousIdOrEquivence -> ConstTag "*" AnonymousIdOrEquivence
std::shared_ptr<ObjectConstructData>&& AnonymousIdOrEquivencePointerAnnounce(
    ConstTag const_tag, std::string&& operator_pointer,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result);
// AnonymousIdOrEquivence -> "(" AnonymousIdOrEquivence ")"
std::shared_ptr<ObjectConstructData>&& AnonymousIdOrEquivenceInBrackets(
    std::string&& left_bracket,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& right_bracket);
// NotEmptyEnumArguments -> Id
std::shared_ptr<EnumReturnData> NotEmptyEnumArgumentsIdBase(std::string&& id);
// NotEmptyEnumArguments -> Id "=" Num
std::shared_ptr<EnumReturnData> NotEmptyEnumArgumentsIdAssignNumBase(
    std::string&& id, std::string&& operator_assign, std::string&& num);
// NotEmptyEnumArguments -> NotEmptyEnumArguments "," Id
std::shared_ptr<EnumReturnData>&& NotEmptyEnumArgumentsIdExtend(
    std::shared_ptr<EnumReturnData>&& enum_data, std::string&& str_comma,
    std::string&& id);
// NotEmptyEnumArguments -> NotEmptyEnumArguments "," Id "=" Num
std::shared_ptr<EnumReturnData>&& NotEmptyEnumArgumentsIdAssignNumExtend(
    std::shared_ptr<EnumReturnData>&& enum_data, std::string&& str_comma,
    std::string&& id, std::string&& operator_assign, std::string&& num);
// EnumArguments -> NotEmptyEnumArguments
std::shared_ptr<EnumReturnData>&& EnumArgumentsNotEmptyEnumArguments(
    std::shared_ptr<EnumReturnData>&& enum_data);
// Enum -> "enum" Id "{" EnumArguments "}"
std::shared_ptr<EnumType> EnumDefine(
    std::string&& str_enum, std::string&& id, std::string&& left_curly_bracket,
    std::shared_ptr<EnumReturnData>&& enum_data,
    std::string&& right_curly_bracket);
// Enum -> "enum" "{" EnumArguments "}"
std::shared_ptr<EnumType> EnumAnonymousDefine(
    std::string&& str_enum, std::string&& left_curly_bracket,
    std::shared_ptr<EnumReturnData>&& enum_data,
    std::string&& right_curly_bracket);
// EnumAnnounce -> "enum" Id
// ���������Ľṹ����ṹ���ͣ�kStruct��
// ���ⷵ������Ϊ����ṹ��͹����������������ͱ���һ��
std::pair<std::string, StructOrBasicType> EnumAnnounce(std::string&& str_enum,
                                                       std::string&& id);
// StructureAnnounce -> "struct" Id
// ���������Ľṹ����ṹ���ͣ�kStruct��
std::pair<std::string, StructOrBasicType> StructureAnnounceStructId(
    std::string&& str_struct, std::string&& id);
// StructureAnnounce -> "union" Id
// ���������Ľṹ����ṹ���ͣ�kUnion��
std::pair<std::string, StructOrBasicType> StructureAnnounceUnionId(
    std::string&& str_union, std::string&& id);
// StructureDefineHead -> "struct"
std::pair<std::string, StructOrBasicType> StructureDefineHeadStruct(
    std::string&& str_struct);
// StructureDefineHead -> "union"
std::pair<std::string, StructOrBasicType> StructureDefineHeadUnion(
    std::string&& str_union);
// StructureDefineHead -> StructureAnnounce
// ����ֵ�����StructureAnnounce
std::pair<std::string, StructOrBasicType>&&
StructureDefineHeadStructureAnnounce(
    std::pair<std::string, StructOrBasicType>&& struct_data);
// StructureDefineInitHead -> StructureDefineHead "{"
// ִ��һЩ��ʼ������
// ���ؽṹ�������ͽڵ�
std::shared_ptr<StructureTypeInterface> StructureDefineInitHead(
    std::pair<std::string, StructOrBasicType>&& struct_data,
    std::string&& left_purly_bracket);
// StructureDefine -> StructureDefineInitHead StructureBody "}"
// ����ֵ���ͣ�std::shared_ptr<StructureTypeInterface>
// ���ؽṹ�������ͽڵ�
std::shared_ptr<StructureTypeInterface>&& StructureDefine(
    std::shared_ptr<StructureTypeInterface>&& struct_data, std::nullptr_t,
    std::string&& right_curly_bracket);
// StructType -> StructureDefine
std::shared_ptr<const StructureTypeInterface> StructTypeStructDefine(
    std::shared_ptr<StructureTypeInterface>&& struct_data);
// StructType -> StructAnnounce
// ���ػ�ȡ���Ľṹ����������
std::shared_ptr<const StructureTypeInterface> StructTypeStructAnnounce(
    std::pair<std::string, StructOrBasicType>&& struct_data);
// BasicType -> ConstTag SignTag FundamentalType
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeFundamental(
    ConstTag const_tag, SignTag sign_tag, BuiltInType builtin_type);
// BasicType -> ConstTag StructType
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeStructType(
    ConstTag const_tag,
    std::shared_ptr<const StructureTypeInterface>&& struct_data);
//// BasicType -> ConstTag Id
//// ���ػ�ȡ����������ConstTag
// std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeId(
//    std::vector<WordDataToUser>&& word_data);
// BasicType -> ConstTag EnumAnnounce
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeEnumAnnounce(
    ConstTag const_tag, std::pair<std::string, StructOrBasicType>&& enum_data);
// FunctionRelaventBasePartFunctionInit -> IdOrEquivence "("
// ��һЩ��ʼ������
std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartFunctionInitBase(
    std::shared_ptr<ObjectConstructData>&& construct_data,
    std::string&& left_bracket);
// FunctionRelaventBasePartFunctionInit -> FunctionRelaventBasePart "("
// ��һЩ��������/����ָ��ʱ�ĳ�ʼ������
std::shared_ptr<ObjectConstructData>&&
FunctionRelaventBasePartFunctionInitExtend(
    std::shared_ptr<ObjectConstructData>&& construct_data,
    std::string&& left_bracket);
// FunctionRelaventBasePart -> FunctionRelaventBasePartFunctionInit
// FunctionRelaventArguments ")"
std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartFunction(
    std::shared_ptr<ObjectConstructData>&& construct_data, std::nullptr_t&&,
    std::string&& right_bracket);
// FunctionRelaventBasePart -> ConstTag "*" FunctionRelaventBasePart
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartPointer(
    ConstTag const_tag, std::string&& str_pointer,
    std::shared_ptr<ObjectConstructData>&& construct_data);
// FunctionRelaventBasePart -> "(" FunctionRelaventBasePart ")"
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartBranckets(
    std::string&& left_bracket,
    std::shared_ptr<ObjectConstructData>&& construct_data,
    std::string&& right_bracket);
// FunctionRelavent -> BasicType FunctionRelaventBasePart
// ����ֵ��װ��ָ��ֻ����Ϊstd::shared_ptr<FunctionType>������������
// ��std::shared_ptr<VarietyOperatorNode>������������
// Define����������λ��������
std::shared_ptr<FlowInterface> FunctionRelavent(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& basic_type_data,
    std::shared_ptr<ObjectConstructData>&& construct_data);
// SingleAnnounceNoAssign -> BasicType IdOrEquivence
// AnonymousSingleAnnounceNoAssign -> BasicType AnonymousIdOrEquivence
// ��������ʽ���øù�Լ������ͨ��is_anonymous����ϸ΢���
// is_anonymous�����Ƿ������������������������������ͣ�
// ����ֵ��װ��ָ��ֻ����Ϊstd::shared_ptr<FunctionType>������������
// ��std::shared_ptr<VarietyOperatorNode>������������
// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
template <bool is_anonymous>
std::shared_ptr<FlowInterface> SingleAnnounceNoAssignVariety(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& basic_type_data,
    std::shared_ptr<ObjectConstructData>&& construct_data);
// SingleAnnounceNoAssign -> ConstTag Id IdOrEquivence
// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
std::shared_ptr<FlowInterface> SingleAnnounceNoAssignNotPodVariety(
    ConstTag const_tag, std::string&& id,
    std::shared_ptr<ObjectConstructData>&& construct_data);
// SingleAnnounceNoAssign -> FunctionRelavent
// AnonymousSingleAnnounceNoAssign -> FunctionRelavent
// ��������ʽ���øù�Լ������ͨ��is_anonymous����ϸ΢���
// is_anonymous�����Ƿ������������������������������ͣ�
// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
template <bool is_anonymous>
std::shared_ptr<FlowInterface>&& SingleAnnounceNoAssignFunctionRelavent(
    std::shared_ptr<FlowInterface>&& flow_control_node);
// TypeDef -> "typedef" SingleAnnounceNoAssign
// ����������
std::nullptr_t TypeDef(std::string&& str_typedef,
                       std::shared_ptr<FlowInterface>&& flow_control_node);
// NotEmptyFunctionRelaventArguments -> SingleAnnounceNoAssign
std::nullptr_t NotEmptyFunctionRelaventArgumentsBase(
    std::shared_ptr<FlowInterface>&& flow_control_node);
// NotEmptyFunctionRelaventArguments -> AnonymousSingleAnnounceNoAssign
std::nullptr_t NotEmptyFunctionRelaventArgumentsAnonymousBase(
    std::shared_ptr<FlowInterface>&& flow_control_node);
// NotEmptyFunctionRelaventArguments -> NotEmptyFunctionRelaventArguments ","
// SingleAnnounceNoAssign
std::nullptr_t NotEmptyFunctionRelaventArgumentsExtend(
    std::nullptr_t&&, std::string&& str_comma,
    std::shared_ptr<FlowInterface>&& flow_control_node);
// NotEmptyFunctionRelaventArguments -> NotEmptyFunctionRelaventArguments ","
// AnonymousSingleAnnounceNoAssign
std::nullptr_t NotEmptyFunctionRelaventArgumentsAnonymousExtend(
    std::nullptr_t&&, std::string&& str_comma,
    std::shared_ptr<FlowInterface>&& flow_control_node);
// FunctionRelaventArguments -> NotEmptyFunctionRelaventArguments
std::nullptr_t FunctionRelaventArguments(std::nullptr_t);
// FunctionDefineHead -> FunctionRelavent "{"
// �Ժ��������ÿ������ִ��Define��ע�ắ�����ͺͺ�����Ӧ�ı��������ڲ��ң�
std::shared_ptr<c_parser_frontend::flow_control::FunctionDefine>
FunctionDefineHead(std::shared_ptr<FlowInterface>&& function_head,
                   std::string&& left_curly_bracket);
// FunctionDefine -> FunctionDefineHead Statements "}"
// ����ֵ���ͣ�std::shared_ptr<FunctionDefine>
// �����ӵĺ������Ƿ�Ϊ�գ����Ϊ��������޷���ֵ����Error
// �������һ�����������õ�ǰ��Ծ����
// �������κ�����
std::nullptr_t FunctionDefine(
    std::shared_ptr<c_parser_frontend::flow_control::FunctionDefine>&&,
    std::nullptr_t, std::string&& right_curly_bracket);
// SingleStructureBody -> SingleAnnounceNoAssign
// ������չ������IDʹ�õ����ͺͱ��������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
SingleStructureBodyBase(std::shared_ptr<FlowInterface>&& flow_control_node);
// SingleStructureBody -> SingleStructureBody "," Id
// ����ֵ���ͣ�std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
// ������չ������IDʹ�õ����ͺͱ��������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&&
SingleStructureBodyExtend(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& struct_data,
    std::string&& str_comma, std::string&& new_member_name);
// NotEmptyStructureBody -> SingleStructureBody
std::nullptr_t NotEmptyStructureBodyBase(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&&);
// NotEmptyStructureBody -> NotEmptyStructureBody SingleStructureBody ";"
std::nullptr_t NotEmptyStructureBodyExtend(
    std::nullptr_t, std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&&,
    std::string&& semicolon);
// StructureBody -> NotEmptyStructureBody
std::nullptr_t StructureBody(std::nullptr_t);
// InitializeList -> "{" InitializeListArguments "}"
std::shared_ptr<ListInitializeOperatorNode> InitializeList(
    std::string&& left_curly_bracket,
    std::shared_ptr<
        std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>&&
        list_arguments,
    std::string&& right_curly_bracket);
// SingleInitializeListArgument -> SingleConstexprValue
std::shared_ptr<InitializeOperatorNodeInterface>&&
SingleInitializeListArgumentConstexprValue(
    std::shared_ptr<BasicTypeInitializeOperatorNode>&& value);
// SingleInitializeListArgument -> InitializeList
std::shared_ptr<InitializeOperatorNodeInterface>&&
SingleInitializeListArgumentList(
    std::shared_ptr<ListInitializeOperatorNode>&& value);
// InitializeListArguments -> SingleInitializeListArgument
std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>
InitializeListArgumentsBase(
    std::shared_ptr<InitializeOperatorNodeInterface>&& init_data_pointer);
// InitializeListArguments -> InitializeListArguments ","
// SingleInitializeListArgument
std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>&&
InitializeListArgumentsExtend(
    std::shared_ptr<std::list<
        std::shared_ptr<InitializeOperatorNodeInterface>>>&& list_pointer,
    std::string&& str_comma,
    std::shared_ptr<InitializeOperatorNodeInterface>&& init_data_pointer);
// AnnounceAssignable -> Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AnnounceAssignableAssignable(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value);
// AnnounceAssignable -> InitializeList
// ���ؿ�����
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AnnounceAssignableInitializeList(
    std::shared_ptr<ListInitializeOperatorNode>&& initialize_list);
// SingleAnnounceAndAssign -> SingleAnnounceNoAssign
// ������չ����ʱ���������ͣ���ȥ������ά����ָ�룬
// �����Ϊ��ָ��������ȥ��һ��ָ�룩
// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignNoAssignBase(
    std::shared_ptr<FlowInterface>&& flow_control_node);
// SingleAnnounceAndAssign -> SingleAnnounceNoAssign "=" AnnounceAssignable
// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignWithAssignBase(
    std::shared_ptr<FlowInterface>&& flow_control_node,
    std::string&& str_assign,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        container);
// SingleAnnounceAndAssign -> SingleAnnounceAndAssign "," Id
// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
SingleAnnounceAndAssignNoAssignExtend(
    std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
               std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        container,
    std::string&& str_comma, std::string&& variety_name);
// SingleAnnounceAndAssign -> SingleAnnounceAndAssign "," Id "="
// AnnounceAssignable
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
SingleAnnounceAndAssignWithAssignExtend(
    std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
               std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        container,
    std::string&& str_comma, std::string&& variety_name,
    std::string&& str_assign,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        assigned_container);
// Type -> BasicType
// �������ͺͱ�����ConstTag
// ����ֵ���ͣ�std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& TypeBasicType(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& type_data);
// Type -> FunctionRelavent
// �������ͺͱ�����ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> TypeFunctionRelavent(
    std::shared_ptr<FlowInterface>&& flow_control_node);
// MathematicalOperator -> "+"
// ������ѧ�����
MathematicalOperation MathematicalOperatorPlus(std::string&& str_operator);
// MathematicalOperator -> "-"
// ������ѧ�����
MathematicalOperation MathematicalOperatorMinus(std::string&& str_operator);
// MathematicalOperator -> "*"
// ������ѧ�����
MathematicalOperation MathematicalOperatorMultiple(std::string&& str_operator);
// MathematicalOperator -> "/"
// ������ѧ�����
MathematicalOperation MathematicalOperatorDivide(std::string&& str_operator);
// MathematicalOperator -> "%"
// ������ѧ�����
MathematicalOperation MathematicalOperatorMod(std::string&& str_operator);
// MathematicalOperator -> "<<"
// ������ѧ�����
MathematicalOperation MathematicalOperatorLeftShift(std::string&& str_operator);
// MathematicalOperator -> ">>"
// ������ѧ�����
MathematicalOperation MathematicalOperatorRightShift(
    std::string&& str_operator);
// MathematicalOperator -> "&"
// ������ѧ�����
MathematicalOperation MathematicalOperatorAnd(std::string&& str_operator);
// MathematicalOperator -> "|"
// ������ѧ�����
MathematicalOperation MathematicalOperatorOr(std::string&& str_operator);
// MathematicalOperator -> "^"
// ������ѧ�����
MathematicalOperation MathematicalOperatorXor(std::string&& str_operator);
// MathematicalOperator -> "!"
// ������ѧ�����
MathematicalOperation MathematicalOperatorNot(std::string&& str_operator);
// MathematicalAndAssignOperator -> "+="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorPlusAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "-="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorMinusAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "*="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorMultipleAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "/="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorDivideAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "%="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorModAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "<<="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorLeftShiftAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> ">>="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorRightShiftAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "&="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorAndAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "|="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorOrAssign(
    std::string&& str_operator);
// MathematicalAndAssignOperator -> "^="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorXorAssign(
    std::string&& str_operator);
// LogicalOperator -> "&&"
// �����߼������
LogicalOperation LogicalOperatorAndAnd(std::string&& str_operator);
// LogicalOperator -> "||"
// �����߼������
LogicalOperation LogicalOperatorOrOr(std::string&& str_operator);
// LogicalOperator -> ">"
// �����߼������
LogicalOperation LogicalOperatorGreater(std::string&& str_operator);
// LogicalOperator -> ">="
// �����߼������
LogicalOperation LogicalOperatorGreaterEqual(std::string&& str_operator);
// LogicalOperator -> "<"
// �����߼������
LogicalOperation LogicalOperatorLess(std::string&& str_operator);
// LogicalOperator -> "<="
// �����߼������
LogicalOperation LogicalOperatorLessEqual(std::string&& str_operator);
// LogicalOperator -> "=="
// �����߼������
LogicalOperation LogicalOperatorEqual(std::string&& str_operator);
// LogicalOperator -> "!="
// �����߼������
LogicalOperation LogicalOperatorNotEqual(std::string&& str_operator);
// Assignable -> SingleConstexprValue
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableConstexprValue(
    std::shared_ptr<BasicTypeInitializeOperatorNode>&& value);
// Assignable -> Id
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableId(std::string&& variety_name);
// Assignable -> TemaryOperator
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AssignableTemaryOperator(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value);
// Assignable -> FunctionCall
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AssignableFunctionCall(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value);
// Assignable -> "sizeof" "(" Type ")"
// ������һ���õ������տ�����ڵ�Ϳ�������sizeof���壩
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfType(
    std::string&& str_sizeof, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& type_data,
    std::string&& str_right_bracket);
// Assignable -> "sizeof" "(" Assignable ")"
// ������һ���õ������տ�����ڵ�Ϳ�������sizeof���壩
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfAssignable(
    std::string&& str_sizeof, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_right_bracket);
// Assignable -> Assignable "." Id
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMemberAccess(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_member_access, std::string&& member_name);
// Assignable -> Assignable "->" Id
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePointerMemberAccess(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_member_access, std::string&& member_name);
// Assignable -> "(" Assignable ")"
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AssignableBracket(
    std::string&& left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value,
    std::string&& right_bracket);
// Assignable -> "(" Type ")" Assignable
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableTypeConvert(
    std::string&& left_bracket,
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& type_data,
    std::string&& right_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        old_variety_data);
// Assignable -> Assignable "=" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableAssign(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        destination_variety_data,
    std::string&& str_assign,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        source_variety_data);
// Assignable -> Assignable MathematicalOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalOperate(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        destination_variety_data,
    MathematicalOperation mathematical_operation,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        source_variety_data);
// Assignable -> Assignable MathematicalAndAssignOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalAndAssignOperate(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        destination_variety_data,
    MathematicalAndAssignOperation mathematical_and_assign_operation,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        source_variety_data);
// Assignable -> Assignable LogicalOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableLogicalOperate(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&& lhr,
    LogicalOperation logical_operation,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        rhr);
// Assignable -> "!" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableNot(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data);
// Assignable -> "~" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableLogicalNegative(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data);
// Assignable -> "-" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalNegative(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data);
// Assignable -> "&" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableObtainAddress(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data);
// Assignable -> "*" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableDereference(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data);
// Assignable -> Assignable "[" Assignable "]"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableArrayAccess(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        array_data,
    std::string&& left_square_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        index_data,
    std::string&& right_square_bracket);
// Assignable -> "++" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixPlus(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data);
// Assignable -> "--" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixMinus(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data);
// Assignable -> Assignable "++"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixPlus(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_operator);
// Assignable -> Assignable "--"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixMinus(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_operator);
// Return -> "return" Assignable ";"
std::nullptr_t ReturnWithValue(
    std::string&& str_return,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_semicolon);
// Return -> "return" ";"
std::nullptr_t ReturnWithoutValue(std::string&& str_return,
                                  std::string&& str_semicolon);
// TemaryOperator -> Assignable "?" Assignable ":" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
TemaryOperator(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_question_mark,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        true_value,
    std::string&& str_colon,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        false_value);
// NotEmptyFunctionCallArguments -> Assignable
std::nullptr_t NotEmptyFunctionCallArgumentsBase(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data);
// NotEmptyFunctionCallArguments -> NotEmptyFunctionCallArguments "," Assignable
std::nullptr_t NotEmptyFunctionCallArgumentsExtend(
    std::nullptr_t, std::string&& str_comma,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data);
// FunctionCallArguments -> NotEmptyFunctionCallArguments
std::nullptr_t FunctionCallArguments(std::nullptr_t);
// FunctionCallInit -> Assignable "("
// ��һЩ��ʼ������
// ���غ������ö���ͻ�ȡ�ɵ��ö���Ĳ�����ͬʱ����ȫ�ֱ���
std::pair<std::shared_ptr<FunctionCallOperatorNode>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCallInitAssignable(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        call_target,
    std::string&& left_bracket);
// FunctionCallInit -> Id "("
// ��һЩ��ʼ������
// ���غ������ö���ͻ�ȡ�ɵ��ö���Ĳ�����ͬʱ����ȫ�ֱ���
std::pair<std::shared_ptr<FunctionCallOperatorNode>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCallInitId(std::string&& function_name, std::string&& str_left_bracket);
// FunctionCall -> FunctionCallInit FunctionCallArguments ")"
// ���غ������ö���
// �ڶ��������洢��ȡ�ɵ��ö�������ͶԿɵ��ö���ĵ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCall(
    std::pair<std::shared_ptr<FunctionCallOperatorNode>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        function_call_data,
    std::nullptr_t, std::string&& right_bracket);
// Assignables -> Assignable
// ���ر���������̵Ľڵ�����
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& AssignablesBase(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data);
// Assignables -> Assignables "," Assignable
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& AssignablesExtend(
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&&
        main_control_node_container,
    std::string&& str_comma,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data);
// Break -> "break" ";"
// ������ת��䣨ʹ��shared_ptr��װ��Ϊstd::any&&��֧�ִ洢���ɸ��Ƶ�ֵ��
std::shared_ptr<std::unique_ptr<Jmp>> Break(std::string&& str_break,
                                            std::string&& str_semicolon);
// Continue -> "continue" ";"
// ������ת��䣨ʹ��shared_ptr��װ��Ϊstd::any&&��֧�ִ洢���ɸ��Ƶ�ֵ��
std::shared_ptr<std::unique_ptr<Jmp>> Continue(std::string&& str_continue,
                                               std::string&& str_semicolon);
// SingleStatement -> If
// �����κβ���
std::nullptr_t SingleStatementIf(std::nullptr_t);
// SingleStatement -> DoWhile
// �����κβ���
std::nullptr_t SingleStatementDoWhile(std::nullptr_t);
// SingleStatement -> While
// �����κβ���
std::nullptr_t SingleStatementWhile(std::nullptr_t);
// SingleStatement -> For
// �����κβ���
std::nullptr_t SingleStatementFor(std::nullptr_t);
// SingleStatement -> Switch
// �����κβ���
std::nullptr_t SingleStatementSwitch(std::nullptr_t);
// SingleStatement -> Assignable ";"
// ��ӻ�ȡAssignable�����̿������
std::nullptr_t SingleStatementAssignable(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        expression,
    std::string&& str_semicolon);
// SingleStatement -> SingleAnnounceAndAssign ";"
// ����������������е����̿������
// �������κ�ֵ
std::nullptr_t SingleStatementAnnounce(
    std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
               std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        announce_statement,
    std::string&& str_semicolon);
// SingleStatement -> Return
// �����κβ���
std::nullptr_t SingleStatementReturn(std::nullptr_t);
// SingleStatement -> Break
std::nullptr_t SingleStatementBreak(
    std::shared_ptr<std::unique_ptr<Jmp>>&& jmp_sentence);
// SingleStatement -> Continue
std::nullptr_t SingleStatementContinue(
    std::shared_ptr<std::unique_ptr<Jmp>>&& jmp_sentence);
// SingleStatement -> ";"
// �����κβ���
std::nullptr_t SingleStatementEmptyStatement(std::string&& str_semicolon);
// IfCondition -> "if" "(" Assignable ")"
// ����if�����ڵ�ͻ�ȡif����������ʹ�õĲ���
// ���������parser_frontend��ע�����̿������
// �������κ�ֵ
std::nullptr_t IfCondition(
    std::string&& str_if, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket);
// IfWithElse -> IfCondition ProcessControlSentenceBody "else"
// ת����if-else���
std::nullptr_t IfWithElse(std::nullptr_t, std::nullptr_t, std::string str_else);
// If->IfWithElse ProcessControlSentenceBody
std::nullptr_t IfElseSence(std::nullptr_t, std::nullptr_t);
// If -> IfCondition ProcessControlSentenceBody
std::nullptr_t IfIfSentence(std::nullptr_t, std::nullptr_t);
// ForRenewSentence -> Assignables
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& ForRenewSentence(
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& expression);
// ForInitSentence -> Assignables
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&&
ForInitSentenceAssignables(
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& expression);
// ForInitSentence -> SingleAnnounceAndAssign
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
ForInitSentenceAnnounce(
    std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
               std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        announce_data);
// ForInitHead -> "for"
// ��һЩ׼������
std::nullptr_t ForInitHead(std::string&& str_for);
// ForHead -> ForInitHead "(" ForInitSentence ";"
//            Assignable ";" ForRenewSentence ")"
// ��Լfor������Ҫ��
std::nullptr_t ForHead(
    std::nullptr_t, std::string&& str_left_bracket,
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&&
        for_init_sentences,
    std::string&& str_semicolon1,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_semicolon2,
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&&
        for_renew_sentences,
    std::string&& str_right_bracket);
// For -> ForHead ProcessControlSentenceBody
// ����������
std::nullptr_t For(std::nullptr_t, std::nullptr_t);
// WhileInitHead -> "while" "(" Assignable ")"
std::nullptr_t WhileInitHead(
    std::string&& str_while, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket);
// While -> WhileInitHead ProcessControlSentenceBody
// ����������
std::nullptr_t While(std::nullptr_t, std::nullptr_t);
// DoWhileInitHead -> "do"
// ��һЩ׼������
std::nullptr_t DoWhileInitHead(std::string&& str_do);
// DoWhile -> DoWhileInitHead ProcessControlSentenceBody
// "while" "(" Assignable ")" ";"
std::nullptr_t DoWhile(
    std::nullptr_t, std::nullptr_t, std::string&& str_while,
    std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket, std::string&& str_semicolon);
// SwitchCase -> "case" SingleConstexprValue ":"
std::nullptr_t SwitchCaseSimple(
    std::string&& str_case,
    std::shared_ptr<BasicTypeInitializeOperatorNode>&& case_data,
    std::string&& str_colon);
// SwitchCase -> "default" ":"
std::nullptr_t SwitchCaseDefault(std::string&& str_default,
                                 std::string&& str_colon);
// SingleSwitchStatement -> SwitchCase
// �����κβ���
std::nullptr_t SingleSwitchStatementCase(std::nullptr_t);
// SingleSwitchStatement -> Statements
// �����κβ���
std::nullptr_t SingleSwitchStatementStatements(std::nullptr_t);
// SwitchStatements -> SwitchStatements SingleSwitchStatement
// �����κβ���
std::nullptr_t SwitchStatements(std::nullptr_t, std::nullptr_t);
// SwitchCondition -> "switch" "(" Assignable ")"
std::nullptr_t SwitchCondition(
    std::string&& str_switch, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket);
// Switch -> SwitchCondition "{" SwitchStatements "}"
// �����κβ���
std::nullptr_t Switch(std::nullptr_t, std::string&& str_left_curly_bracket,
                      std::nullptr_t, std::string&& str_right_curly_bracket);
// Statements -> Statements SingleStatement
// �����κβ���
std::nullptr_t StatementsSingleStatement(std::nullptr_t, std::nullptr_t);
// StatementsLeftBrace -> Statements "{"
// ����������ȼ�
std::nullptr_t StatementsLeftBrace(std::nullptr_t,
                                   std::string&& left_curly_bracket);
// Statements -> StatementsLeftBrace Statements "}"
// ����������
std::nullptr_t StatementsBrace(std::nullptr_t, std::nullptr_t,
                               std::string&& right_curly_bracket);
// ProcessControlSentenceBody -> SingleStatement
// ������������
std::nullptr_t ProcessControlSentenceBodySingleStatement(std::nullptr_t);
// ProcessControlSentenceBody -> "{" Statements "}"
// ������������
// �����κβ���
std::nullptr_t ProcessControlSentenceBodyStatements(
    std::string&& str_left_curly_bracket, std::nullptr_t,
    std::string&& str_right_curly_bracket);
// ������ʽ
// Root -> Root FunctionDefine
// �����κβ���
std::nullptr_t RootFunctionDefine(std::nullptr_t, std::nullptr_t);
// ������ʽ
// Root -> Root SingleAnnounceNoAssign ";"
std::nullptr_t RootAnnounce(std::nullptr_t,
                            std::shared_ptr<FlowInterface>&& flow_control_node,
                            std::string&& str_colon);

template <class BasicObjectType, class... Args>
  requires std::is_same_v<BasicObjectType, VarietyOperatorNode> ||
           std::is_same_v<BasicObjectType,
                          c_parser_frontend::flow_control::FunctionDefine>
ObjectConstructData::CheckResult ObjectConstructData::ConstructBasicObjectPart(
    Args... args) {
  if constexpr (std::is_same_v<BasicObjectType, VarietyOperatorNode>) {
    // ������������
    auto object = std::make_unique<SimpleSentence>();
    bool result = object->SetSentenceOperateNode(
        std::make_shared<VarietyOperatorNode>(std::forward<Args>(args)...));
    assert(result);
    object_ = std::move(object);
  } else {
    // ������������
    object_ = std::make_unique<c_parser_frontend::flow_control::FunctionDefine>(
        std::forward<Args>(args)...);
  }
  return CheckResult::kSuccess;
}
template <bool is_anonymous>
std::shared_ptr<FlowInterface> SingleAnnounceNoAssignVariety(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& basic_type_data,
    std::shared_ptr<ObjectConstructData>&& construct_data) {
  auto& [final_type, const_tag_before_final_type] = basic_type_data;
  if constexpr (is_anonymous) {
    if (construct_data == nullptr) [[likely]] {
      // ��������ָ��������ʡ�Բ�����
      construct_data = std::make_shared<ObjectConstructData>(std::string());
      construct_data->ConstructBasicObjectPart<VarietyOperatorNode>(
          std::string(), const_tag_before_final_type,
          LeftRightValueTag::kLeftValue);
    } else {
      assert(construct_data->GetObjectName().empty());
    }
  } else {
    if (construct_data->GetObjectName().empty()) [[unlikely]] {
      OutputError(std::format("�����ı�����������"));
      exit(-1);
    }
  }
  auto [flow_control_node, construct_result] = construct_data->ConstructObject(
      const_tag_before_final_type, std::move(final_type));
  // ����Ƿ񹹽��ɹ�
  if (construct_result != ObjectConstructData::CheckResult::kSuccess)
      [[unlikely]] {
    VarietyOrFunctionConstructError(construct_result,
                                    construct_data->GetObjectName());
  }
  return std::move(flow_control_node);
}

// SingleAnnounceNoAssign -> FunctionRelavent
// AnonymousSingleAnnounceNoAssign -> FunctionRelavent
// ��������ʽ���øù�Լ������ͨ��is_anonymous����ϸ΢���
// is_anonymous�����Ƿ������������������������������ͣ�
// ����ֵ���ͣ�std::shared_ptr<FlowInterface>
// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
template <bool is_anonymous>
std::shared_ptr<FlowInterface>&& SingleAnnounceNoAssignFunctionRelavent(
    std::shared_ptr<FlowInterface>&& flow_control_node) {
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
  return std::move(flow_control_node);
}
// ����ͬAttachSingleNodeToTailNodeEmplace������ʹ���Ѿ������õĽڵ�

}  // namespace c_parser_frontend::parse_functions
#endif