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
// �����߳�ȫ�ֱ�����C���Ա�����ǰ�˵������
extern thread_local CParserFrontend parser_frontend;
}  // namespace c_parser_frontend

namespace c_parser_frontend::parse_functions {
using WordDataToUser = frontend::generator::syntaxgenerator::
    ProcessFunctionInterface::WordDataToUser;
using c_parser_frontend::parser_frontend;
using c_parser_frontend::flow_control::AllocateOperatorNode;
using c_parser_frontend::flow_control::DoWhileSentence;
using c_parser_frontend::flow_control::FlowInterface;
using c_parser_frontend::flow_control::FlowType;
using c_parser_frontend::flow_control::ForSentence;
using c_parser_frontend::flow_control::FunctionDefine;
using c_parser_frontend::flow_control::IfSentence;
using c_parser_frontend::flow_control::Label;
using c_parser_frontend::flow_control::LoopSentenceInterface;
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
    kEmptyChain       // ���������κ�����
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
      std::is_same_v<BasicObjectType, FunctionDefine>
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

// ��ConstTag�Ĺ�Լ��Ϣ�л�ȡConstTag����
// ��Ҫ��֤����Ϊ���ս����ʽConstTag��Լ�õ������ݣ������Ƿ�Ϊ�չ�Լ��
ConstTag GetConstTag(const WordDataToUser& raw_const_tag_data);
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

// ����ʽ��Լʱʹ�õĺ���
// ��ʹ��std::unique_ptr��Ϊ�޷����ƣ��޷����ڹ���std::any

// SingleConstexprValue-> Char
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueChar(
    std::vector<WordDataToUser>&& word_data);
// SingleConstexprValue-> Str "[" Num "]"
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::shared_ptr<BasicTypeInitializeOperatorNode>
SingleConstexprValueIndexedString(std::vector<WordDataToUser>&& word_data);
// SingleConstexprValue-> Num
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
// TODO ��ȷ��ȡ������ֵ��Ӧ������
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueNum(
    std::vector<WordDataToUser>&& word_data);
// SingleConstexprValue->Str
// ���ԣ�InitializeType::String��TypeInterface:: const char*
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueString(
    std::vector<WordDataToUser>&& word_data);
// FundamentalType->"char"
BuiltInType FundamentalTypeChar(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "char");
  return BuiltInType::kInt8;
}
// FundamentalType->"short"
BuiltInType FundamentalTypeShort(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "short");
  return BuiltInType::kInt16;
}
// FundamentalType->"int"
BuiltInType FundamentalTypeInt(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "int");
  return BuiltInType::kInt32;
}
// FundamentalType->"long"
BuiltInType FundamentalTypeLong(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "long");
  return BuiltInType::kInt32;
}
// FundamentalType->"float"
BuiltInType FundamentalTypeFloat(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "float");
  return BuiltInType::kFloat32;
}
// FundamentalType->"double"
BuiltInType FundamentalTypeDouble(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "double");
  return BuiltInType::kFloat64;
}
// FundamentalType->"void"
BuiltInType FundamentalTypeVoid(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "void");
  return BuiltInType::kVoid;
}
// SignTag->"signed"
SignTag SignTagSigned(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "signed");
  return SignTag::kSigned;
}
// SignTag->"unsigned"
SignTag SignTagUnSigned(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "unsigned");
  return SignTag::kUnsigned;
}
// ConstTag->"const"
ConstTag ConstTagConst(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "const");
  return ConstTag::kConst;
}
// IdOrEquivence->ConstTag Id
// �����ڶ��ϣ�����any�����и��ƶ���
// ʹ��std::shared_ptr����std::shared_ptr��std::any���ܴ洢��֧�ָ��Ƶ�����
std::shared_ptr<ObjectConstructData> IdOrEquivenceConstTagId(
    std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->ConstTag
// �����ڶ��ϣ�����any�����и��ƶ���
// ʹ��std::shared_ptr����std::shared_ptr��std::any���ܴ洢��֧�ָ��Ƶ�����
std::shared_ptr<ObjectConstructData> IdOrEquivenceConstTag(
    std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->IdOrEquivence "[" Num "]"
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
// ����std::any��ֹ�ƶ�����VarietyConstructData
std::any IdOrEquivenceNumAddressing(std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->IdOrEquivence "[" "]"
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
// ����std::any��ֹ�ƶ�����VarietyConstructData
// ��������ӵ�ָ������Ӧ�����СΪ-1����Ǵ˴������С��Ҫ���ݸ�ֵ����ƶ�
std::any IdOrEquivenceAnonymousAddressing(
    std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->Consttag "*" IdOrEquivence
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
// ����std::any��ֹ�ƶ�����VarietyConstructData
std::any IdOrEquivencePointerAnnounce(std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->"(" IdOrEquivence ")"
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
// ����std::any��ֹ�ƶ�����VarietyConstructData
std::any IdOrEquivenceInBrackets(std::vector<WordDataToUser>&& word_data);
// NotEmptyEnumArguments-> Id
EnumReturnData NotEmptyEnumArgumentsIdInit(
    std::vector<WordDataToUser>&& word_data);
// NotEmptyEnumArguments-> Id "=" Num
EnumReturnData NotEmptyEnumArgumentsIdAssignNumInit(
    std::vector<WordDataToUser>&& word_data);
// NotEmptyEnumArguments-> NotEmptyEnumArguments "," Id
// ����ֵ���ͣ�EnumReturnData
std::any NotEmptyEnumArgumentsId(std::vector<WordDataToUser>&& word_data);
// NotEmptyEnumArguments-> NotEmptyEnumArguments "," Id "=" Num
// ����ֵ���ͣ�EnumReturnData
std::any NotEmptyEnumArgumentsIdAssignNum(
    std::vector<WordDataToUser>&& word_data);
// EnumArguments->NotEmptyEnumArguments
// ����ֵ���ͣ�EnumReturnData
std::any EnumArgumentsNotEmptyEnumArguments(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// Enum->"enum" Id "{" EnumArguments "}"
std::shared_ptr<EnumType> EnumDefine(std::vector<WordDataToUser>&& word_data);
// Enum->"enum" "{" EnumArguments "}"
std::shared_ptr<EnumType> EnumAnonymousDefine(
    std::vector<WordDataToUser>&& word_data);
// EnumAnnounce->"enum" Id
// ���������Ľṹ����ṹ���ͣ�kStruct��
// ���ⷵ������Ϊ����ṹ��͹����������������ͱ���һ��
inline std::pair<std::string, StructOrBasicType> EnumAnnounce(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  std::string& enum_name = word_data[1].GetTerminalWordData().word;
  return std::make_pair(std::move(enum_name), StructOrBasicType::kEnum);
}
// StructureAnnounce->"struct" Id
// ���������Ľṹ����ṹ���ͣ�kStruct��
inline std::pair<std::string, StructOrBasicType> StructureAnnounceStructId(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  return std::make_pair(std::move(word_data[1].GetTerminalWordData().word),
                        StructOrBasicType::kStruct);
}
// StructureAnnounce->"union" Id
// ���������Ľṹ����ṹ���ͣ�kUnion��
inline std::pair<std::string, StructOrBasicType> StructureAnnounceUnionId(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  return std::make_pair(std::move(word_data[1].GetTerminalWordData().word),
                        StructOrBasicType::kUnion);
}
// StructureDefineHead->"struct"
inline std::pair<std::string, StructOrBasicType> StructureDefineHeadStruct(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::make_pair(std::string(), StructOrBasicType::kStruct);
}
// StructureDefineHead->"union"
std::pair<std::string, StructOrBasicType> StructureDefineHeadUnion(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::make_pair(std::string(), StructOrBasicType::kUnion);
}
// StructureDefineHead->StructureAnnounce
// ����ֵ���ͣ�std::pair<std::string, StructOrBasicType>
// ����ֵ�����StructureAnnounce
std::any StructureDefineHeadStructureAnnounce(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// StructureDefineInitHead-> StructureDefineHead "{"
// ִ��һЩ��ʼ������
// ���ؽṹ�������ͽڵ�
std::shared_ptr<StructureTypeInterface> StructureDefineInitHead(
    std::vector<WordDataToUser>&& word_data);
// StructureDefine-> StructureDefineInitHead StructureBody "}"
// ����ֵ���ͣ�std::shared_ptr<StructureTypeInterface>
// ���ؽṹ�������ͽڵ�
std::any StructureDefine(std::vector<WordDataToUser>&& word_data);
// StructType-> StructureDefine
std::shared_ptr<const StructureTypeInterface> StructTypeStructDefine(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any_cast<std::shared_ptr<StructureTypeInterface>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// StructType-> StructAnnounce
// ���ػ�ȡ���Ľṹ����������
std::shared_ptr<const StructureTypeInterface> StructTypeStructAnnounce(
    std::vector<WordDataToUser>&& word_data);
// BasicType-> ConstTag SignTag FundamentalType
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeFundamental(
    std::vector<WordDataToUser>&& word_data);
// BasicType-> ConstTag StructType
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeStructType(
    std::vector<WordDataToUser>&& word_data);
// BasicType-> ConstTag Id
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeId(
    std::vector<WordDataToUser>&& word_data);
// BasicType-> ConstTag EnumAnnounce
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeEnumAnnounce(
    std::vector<WordDataToUser>&& word_data);
// FunctionRelaventBasePartFunctionInit-> IdOrEquivence "("
// ��һЩ��ʼ������
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any FunctionRelaventBasePartFunctionInit(
    std::vector<WordDataToUser>&& word_data);
// FunctionRelaventBasePart-> FunctionRelaventBasePartFunctionInit
//  FunctionRelaventArguments ")"
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any FunctionRelaventBasePartFunction(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}
// FunctionRelaventBasePart-> ConstTag "*" FunctionRelaventBasePart
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any FunctionRelaventBasePartPointer(
    std::vector<WordDataToUser>&& word_data);
// FunctionRelaventBasePart-> "(" FunctionRelaventBasePart ")"
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any FunctionRelaventBasePartBranckets(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  return std::move(word_data[1].GetNonTerminalWordData().user_returned_data);
}
// FunctionRelaventBasePartFunctionInit-> FunctionRelaventBasePart "("
// ��һЩ��������ָ��ʱ�ĳ�ʼ������
// ����ֵ���ͣ�std::shared_ptr<ObjectConstructData>
std::any FunctionRelaventBasePartFunctionInit(
    std::vector<WordDataToUser>&& word_data);
// FunctionRelavent-> BasicType FunctionRelaventBasePart
// ����ֵ��װ��ָ��ֻ����Ϊstd::shared_ptr<FunctionType>������������
// ��std::shared_ptr<VarietyOperatorNode>������������
// Define����������λ��������
std::shared_ptr<FlowInterface> FunctionRelavent(
    std::vector<WordDataToUser>&& word_data);
// SingleAnnounceNoAssign-> BasicType IdOrEquivence
// ����ֵ��װ��ָ��ֻ����Ϊstd::shared_ptr<FunctionType>������������
// ��std::shared_ptr<VarietyOperatorNode>������������
// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
std::shared_ptr<FlowInterface> SingleAnnounceNoAssignVariety(
    std::vector<WordDataToUser>&& word_data);
// SingleAnnounceNoAssign-> FunctionRelavent
// ����ֵ���ͣ�std::shared_ptr<FlowInterface>
// ��ִ��DefineVariety/DefineTypeҲ����ӿռ����ڵ�
std::any SingleAnnounceNoAssignFunctionRelavent(
    std::vector<WordDataToUser>&& word_data);
// TypeDef-> "typedef" SingleAnnounceNoAssign
// ����������
std::any TypeDef(std::vector<WordDataToUser>&& word_data);
// NotEmptyFunctionPointerArguments-> SingleAnnounceNoAssign
// ����������
std::any NotEmptyFunctionPointerArgumentsBase(
    std::vector<WordDataToUser>&& word_data);
// NotEmptyFunctionPointerArguments-> NotEmptyFunctionPointerArguments ","
// SingleAnnounceNoAssign
// ����ֵ���ͣ�std::shared_ptr<FunctionType::ArgumentInfoContainer>
std::any NotEmptyFunctionPointerArgumentsExtend(
    std::vector<WordDataToUser>&& word_data);
// FunctionRelaventArguments-> NotEmptyFunctionPointerArguments
// ����ֵ���ͣ�std::shared_ptr<FunctionType:ArgumentInfoContainer>
std::any FunctionRelaventArguments(std::vector<WordDataToUser>&& word_data);
// FunctionDefineHead-> FunctionRelavent "{"
// �Ժ��������ÿ������ִ��Define��ע�ắ�����ͺͺ�����Ӧ�ı��������ڲ��ң�
std::shared_ptr<FunctionDefine> FunctionDefineHead(
    std::vector<WordDataToUser>&& word_data);
// FunctionDefine-> FunctionDefineHead Sentences "}"
// ����ֵ���ͣ�std::shared_ptr<FunctionDefine>
// �������һ�����������õ�ǰ��Ծ����
// �������κ�����
std::any FunctionDefineMain(std::vector<WordDataToUser>&& word_data);
// SingleStructureBody-> SingleAnnounceNoAssign
// ������չ������IDʹ�õ����ͺͱ��������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
SingleStructureBodyBase(std::vector<WordDataToUser>&& word_data);
// SingleStructureBody-> SingleStructureBody "," Id
// ����ֵ���ͣ�std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
// ������չ������IDʹ�õ����ͺͱ��������ConstTag
std::any SingleStructureBodyExtend(std::vector<WordDataToUser>&& word_data);
// NotEmptyStructureBody-> SingleStructureBody
// �������κ�����
inline std::any NotEmptyStructureBodyBase(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// NotEmptyStructureBody-> NotEmptyStructureBody SingleStructureBody ";"
// �������κ�����
inline std::any NotEmptyStructureBodyExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  return std::any();
}
// StructureBody-> NotEmptyStructureBody
// �������κ�����
// StructureBody-> NotEmptyStructureBody
inline std::any StructureBody(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// InitializeList-> "{" InitializeListArguments "}"
std::shared_ptr<ListInitializeOperatorNode> InitializeList(
    std::vector<WordDataToUser>&& word_data);
// SingleInitializeListArgument-> SingleConstexprValue
// ����ֵ���ͣ�std::shared_ptr<InitializeOperatorNodeInterface>
std::any SingleInitializeListArgumentConstexprValue(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// SingleInitializeListArgument-> InitializeList
// ����ֵ���ͣ�std::shared_ptr<InitializeOperatorNodeInterface>
std::any SingleInitializeListArgumentList(
    std::vector<WordDataToUser>&& word_data);
// InitializeListArguments-> SingleInitializeListArgument
// ����ֵ���ͣ�
std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>
InitializeListArgumentsBase(std::vector<WordDataToUser>&& word_data);
// InitializeListArguments-> InitializeListArguments ","
// SingleInitializeListArgument
// ����ֵ���ͣ�
// std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>
std::any InitializeListArgumentsExtend(std::vector<WordDataToUser>&& word_data);
// AnnounceAssignable-> Assignable
// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
inline std::any AnnounceAssignableAssignable(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// AnnounceAssignable-> InitializeList
// ���ؿ�����
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AnnounceAssignableInitializeList(std::vector<WordDataToUser>&& word_data);
// ��������ִ����������������������ʼֵ�ı���ע��ͻ�ȡ��չ���������͵Ĳ���
// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʱʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarietyAnnounceNoAssign(std::shared_ptr<VarietyOperatorNode>&& variety_node);
// ��������ִ�����������������Ҹ���ʼֵ�ı���ע��ͻ�ȡ��չ���������Ͳ���
// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʱʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarieytAnnounceWithAssign(
    std::shared_ptr<VarietyOperatorNode>&& variety_node,
    std::shared_ptr<OperatorNodeInterface>&& node_for_assign);
// SingleAnnounceAndAssign-> SingleAnnounce
// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignNoAssignBase(std::vector<WordDataToUser>&& word_data);
// SingleAnnounceAndAssign-> SingleAnnounce "=" AnnounceAssignable
// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignWithAssignBase(std::vector<WordDataToUser>&& word_data);
// SingleAnnounceAndAssign-> SingleAnnounceAndAssign "," Id
// ������չ����ʱ���������ͣ�������Ϊ��ָ��������ȥ��һ��ָ�룬���򲻱䣩
// ������ConstTag�ͻ�ȡ����ʹ�õĲ���
// ����ֵ���ͣ�std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any SingleAnnounceAndAssignNoAssignExtend(
    std::vector<WordDataToUser>&& word_data);
// SingleAnnounceAndAssign-> SingleAnnounceAndAssign "," Id "="
// AnnounceAssignable
// ����ǺϷ�����������ӱ������岢�����ռ����ڵ�͸�ֵ�ڵ�
// ����ֵ���ͣ�std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any SingleAnnounceAndAssignWithAssignExtend(
    std::vector<WordDataToUser>&& word_data);
// Type-> BasicType
// �������ͺͱ�����ConstTag
// ����ֵ���ͣ�std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
std::any TypeBasicType(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// Type-> FunctionRelavent
// �������ͺͱ�����ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> TypeFunctionRelavent(
    std::vector<WordDataToUser>&& word_data);
// MathematicalOperator-> "+"
// ������ѧ�����
MathematicalOperation MathematicalOperatorPlus(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "+");
  return MathematicalOperation::kPlus;
}
// MathematicalOperator-> "-"
// ������ѧ�����
MathematicalOperation MathematicalOperatorMinus(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "-");
  return MathematicalOperation::kMinus;
}
// MathematicalOperator-> "*"
// ������ѧ�����
MathematicalOperation MathematicalOperatorMultiple(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "*");
  return MathematicalOperation::kMultiple;
}
// MathematicalOperator-> "/"
// ������ѧ�����
MathematicalOperation MathematicalOperatorDivide(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "/");
  return MathematicalOperation::kDivide;
}
// MathematicalOperator-> "%"
// ������ѧ�����
MathematicalOperation MathematicalOperatorMod(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "%");
  return MathematicalOperation::kMod;
}
// MathematicalOperator-> "<<"
// ������ѧ�����
MathematicalOperation MathematicalOperatorLeftShift(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "<<");
  return MathematicalOperation::kLeftShift;
}
// MathematicalOperator-> ">>"
// ������ѧ�����
MathematicalOperation MathematicalOperatorRightShift(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == ">>");
  return MathematicalOperation::kRightShift;
}
// MathematicalOperator-> "&"
// ������ѧ�����
MathematicalOperation MathematicalOperatorAnd(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "&");
  return MathematicalOperation::kAnd;
}
// MathematicalOperator-> "|"
// ������ѧ�����
MathematicalOperation MathematicalOperatorOr(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "|");
  return MathematicalOperation::kOr;
}
// MathematicalOperator-> "^"
// ������ѧ�����
MathematicalOperation MathematicalOperatorXor(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "^");
  return MathematicalOperation::kXor;
}
// MathematicalOperator-> "!"
// ������ѧ�����
MathematicalOperation MathematicalOperatorNot(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "!");
  return MathematicalOperation::kNot;
}
// MathematicalAndAssignOperator-> "+="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorPlusAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "+=");
  return MathematicalAndAssignOperation::kPlusAssign;
}
// MathematicalAndAssignOperator-> "-="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorMinusAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "-=");
  return MathematicalAndAssignOperation::kMinusAssign;
}
// MathematicalAndAssignOperator-> "*="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorMultipleAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "*=");
  return MathematicalAndAssignOperation::kMultipleAssign;
}
// MathematicalAndAssignOperator-> "/="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorDivideAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "/=");
  return MathematicalAndAssignOperation::kDivideAssign;
}
// MathematicalAndAssignOperator-> "%="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorModAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "%=");
  return MathematicalAndAssignOperation::kModAssign;
}
// MathematicalAndAssignOperator-> "<<="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorLeftShiftAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "<<=");
  return MathematicalAndAssignOperation::kLeftShiftAssign;
}
// MathematicalAndAssignOperator-> ">>="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorRightShiftAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == ">>=");
  return MathematicalAndAssignOperation::kRightShiftAssign;
}
// MathematicalAndAssignOperator-> "&="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorAndAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "&=");
  return MathematicalAndAssignOperation::kAndAssign;
}
// MathematicalAndAssignOperator-> "|="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorOrAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "|=");
  return MathematicalAndAssignOperation::kOrAssign;
}
// MathematicalAndAssignOperator-> "^="
// ������ѧ��ֵ�����
MathematicalAndAssignOperation MathematicalAndAssignOperatorXorAssign(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "^=");
  return MathematicalAndAssignOperation::kXorAssign;
}
// LogicalOperator-> "&&"
// �����߼������
LogicalOperation LogicalOperatorAndAnd(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "&&");
  return LogicalOperation::kAndAnd;
}
// LogicalOperator-> "||"
// �����߼������
LogicalOperation LogicalOperatorOrOr(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "||");
  return LogicalOperation::kOrOr;
}
// LogicalOperator-> ">"
// �����߼������
LogicalOperation LogicalOperatorGreater(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == ">");
  return LogicalOperation::kGreater;
}
// LogicalOperator-> ">="
// �����߼������
LogicalOperation LogicalOperatorGreaterEqual(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == ">=");
  return LogicalOperation::kGreaterEqual;
}
// LogicalOperator-> "<"
// �����߼������
LogicalOperation LogicalOperatorLess(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "<");
  return LogicalOperation::kLess;
}
// LogicalOperator-> "<="
// �����߼������
LogicalOperation LogicalOperatorLessEqual(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "<=");
  return LogicalOperation::kLessEqual;
}
// LogicalOperator-> "=="
// �����߼������
LogicalOperation LogicalOperatorEqual(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "==");
  return LogicalOperation::kEqual;
}
// LogicalOperator-> "!="
// �����߼������
LogicalOperation LogicalOperatorNotEqual(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1 &&
         word_data.front().GetTerminalWordData().word == "!=");
  return LogicalOperation::kNotEqual;
}
// Assignable-> SingleConstexprValue
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableConstexprValue(std::vector<WordDataToUser>&& word_data);
// Assignable-> Id
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableId(std::vector<WordDataToUser>&& word_data);
// Assignable-> TemaryOperator
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any AssignableTypeTemaryOperator(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// Assignable-> FunctionCall
// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any AssignableFunctionCall(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// Assignable-> "sizeof" "(" Type ")"
// ������һ���õ������տ�����ڵ�Ϳ�������sizeof���壩
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfType(std::vector<WordDataToUser>&& word_data);
// Assignable-> "sizeof" "(" Assignable ")"
// ������һ���õ������տ�����ڵ�Ϳ�������sizeof���壩
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfAssignable(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable "." Id
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMemberAccess(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable "->" Id
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePointerMemberAccess(std::vector<WordDataToUser>&& word_data);
// Assignable-> "(" Assignable ")"
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
inline std::any AssignableBracket(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// Assignable-> "(" Type ")" Assignable
// ������һ���õ������տ�����ڵ�ͻ�ȡ���̵Ĳ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableTypeConvert(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable MathematicalOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalOperate(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable MathematicalAndAssignOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalAndAssignOperate(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable LogicalOperator Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableLogicalOperate(std::vector<WordDataToUser>&& word_data);
// Assignable-> "!" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableNot(std::vector<WordDataToUser>&& word_data);
// Assignable-> "~" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableNegative(std::vector<WordDataToUser>&& word_data);
// Assignable-> "&" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableObtainAddress(std::vector<WordDataToUser>&& word_data);
// Assignable-> "*" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableDereference(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable "[" Assignable "]"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableArrayAccess(std::vector<WordDataToUser>&& word_data);
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
// Assignable-> "++" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixPlus(std::vector<WordDataToUser>&& word_data);
// Assignable-> "--" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixMinus(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable "++"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixPlus(std::vector<WordDataToUser>&& word_data);
// Assignable-> Assignable "--"
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixMinus(std::vector<WordDataToUser>&& word_data);
// TemaryOperator-> Assignable "?" Assignable ":" Assignable
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
TemaryOperator(std::vector<WordDataToUser>&& word_data);
// NotEmptyFunctionCallArguments-> Assignable
// �������κ�����
std::any NotEmptyFunctionCallArgumentsBase(
    std::vector<WordDataToUser>&& word_data);
// NotEmptyFunctionCallArguments-> NotEmptyFunctionCallArguments "," Assignable
// ����ֵ���ͣ�
// std::shared_ptr<FunctionCallOperatorNode::FunctionCallArgumentsContainer>
std::any NotEmptyFunctionCallArgumentsExtend(
    std::vector<WordDataToUser>&& word_data);
// FunctionCallArguments-> NotEmptyFunctionCallArguments
// ����ֵ���ͣ�
// std::shared_ptr<FunctionCallOperatorNode::FunctionCallArgumentsContainer>
std::any FunctionCallArguments(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// FunctionCallInit-> Assignable "("
// ��һЩ��ʼ������
// ���غ������ö���ͻ�ȡ�ɵ��ö���Ĳ�����ͬʱ����ȫ�ֱ���
std::pair<std::shared_ptr<FunctionCallOperatorNode>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCallInit(std::vector<WordDataToUser>&& word_data);
// FunctionCall-> FunctionCallInit FunctionCallArguments ")"
// ���غ������ö���
// �ڶ��������洢��ȡ�ɵ��ö�������ͶԿɵ��ö���ĵ���
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCall(std::vector<WordDataToUser>&& word_data);
// Assignables-> Assignable
// ���ر���������̵Ľڵ�����
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>> AssignablesBase(
    std::vector<WordDataToUser>&& word_data);
// Assignables-> Assignables "," Assignable
// ����ֵ���ͣ�std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
std::any AssignablesExtend(std::vector<WordDataToUser>&& word_data);
// SingleStatement-> If
// �����κβ���
std::any SingleStatementIf(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// SingleStatement-> DoWhile
// �����κβ���
std::any SingleStatementDoWhile(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// SingleStatement-> While
// �����κβ���
std::any SingleStatementWhile(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// SingleStatement-> For
// �����κβ���
std::any SingleStatementFor(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// SingleStatement-> Switch
// �����κβ���
std::any SingleStatementSwitch(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// SingleStatement-> Assignable ";"
// ��ӻ�ȡAssignable�����̿������
// �������κ�ֵ
std::any SingleStatementAssignable(std::vector<WordDataToUser>&& word_data);
// SingleStatement-> SingleAnnounceAndAssign ";"
// ����������������е����̿������
// �������κ�ֵ
std::any SingleStatementAnnounce(std::vector<WordDataToUser>&& word_data);
// IfCondition-> "if" "(" Assignable ")"
// ����if�����ڵ�ͻ�ȡif����������ʹ�õĲ���
// ���������parser_frontend��ע�����̿������
// �������κ�ֵ
std::any IfCondition(std::vector<WordDataToUser>&& word_data);
// IfWithElse-> IfCondition ProcessControlSentenceBody "else"
// ת����if-else���
// �������κ�ֵ
std::any IfWithElse(std::vector<WordDataToUser>&& word_data);
// If->IfWithElse ProcessControlSentenceBody
// �������κ�ֵ
std::any IfElseSence(std::vector<WordDataToUser>&& word_data) {
  parser_frontend.PopActionScope();
  return std::any();
}
// If-> IfCondition ProcessControlSentenceBody
// �������κ�ֵ
std::any IfIfSentence(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  parser_frontend.PopActionScope();
  return std::any();
}
// ForRenewSentences-> Assignables ";"
// ����ֵ���ͣ�std::pair<std::shared_ptr<const OperatorNodeInterface>,
//                   std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
std::any ForRenewSentences(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}
// ForInitSentence-> Assignables ";"
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
ForInitSentenceAssignables(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [ignore_assignable, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  return flow_control_node_container;
}
// ForInitSentence-> SingleAnnounceAndAssign ";"
std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
ForInitSentenceAnnounce(std::vector<WordDataToUser>&& word_data);
// ForInitHead-> "for"
// ��һЩ׼������
// �������κ�ֵ
inline std::any ForInitHead(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  parser_frontend.PushFlowControlSentence(std::make_unique<ForSentence>());
  return std::any();
}
// ForHead-> ForInitHead "(" ForInitSentence Assignable ForRenewSentences ")"
// ��Լfor������Ҫ��
// �������κ�ֵ
std::any ForHead(std::vector<WordDataToUser>&& word_data);
// For-> ForHead ProcessControlSentenceBody
// ����������
// �������κ�ֵ
std::any For(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  parser_frontend.PopActionScope();
  return std::any();
}
// WhileInitHead-> "while" "(" Assignable ")"
// �������κ�ֵ
std::any WhileInitHead(std::vector<WordDataToUser>&& word_data);
// While-> WhileInitHead ProcessControlSentenceBody
// ����������
// �������κ�ֵ
std::any While(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  parser_frontend.PopActionScope();
  return std::any();
}
// DoWhileInitHead-> "do"
// ��һЩ׼������
// �������κ�����
std::any DoWhileInitHead(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  parser_frontend.PushFlowControlSentence(std::make_unique<DoWhileSentence>());
  return std::any();
}
// DoWhile-> DoWhileInitHead ProcessControlSentenceBody
// while "(" Assignable ")" ";"
// �������κ�����
std::any DoWhile(std::vector<WordDataToUser>&& word_data);
// SwitchCaseSimple-> "case" SingleConstexprValue ":"
// �������κ�����
std::any SwitchCaseSimple(std::vector<WordDataToUser>&& word_data);
// SwitchCaseSimple-> "default" ":"
// �������κ�����
std::any SwitchCaseDefault(std::vector<WordDataToUser>&& word_data);
// SingleSwitchStatement-> SwitchCase
// �����κβ���
std::any SingleSwitchStatementCase(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// SingleSwitchStatement-> Statements
// �����κβ���
std::any SingleSwitchStatementStatements(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::any();
}
// SwitchStatements-> SwitchStatements SingleSwitchStatement
// �����κβ���
std::any SwitchStatements(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  return std::any();
}
// SwitchCondition-> "switch" "(" Assignable ")"
// �������κ�ֵ
std::any SwitchCondition(std::vector<WordDataToUser>&& word_data);
// Switch-> SwitchCondition "{" SwitchStatements "}"
// �����κβ���
std::any Switch(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  return std::any();
}
// Statements-> Statements SingleStatement
// �����κβ���
std::any Statements(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  return std::any();
}
// ������ʽ
// Root-> Root FunctionDefine
// �����κβ���
std::any RootFunctionDefine(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  return std::any();
}
// ������ʽ
// Root-> Root SingleAnnounceNoAssign ";"
// �������κ�ֵ
std::any RootAnnounce(std::vector<WordDataToUser>&& word_data);

template <class BasicObjectType, class... Args>
requires std::is_same_v<BasicObjectType, VarietyOperatorNode> ||
    std::is_same_v<BasicObjectType, FunctionDefine>
        ObjectConstructData::CheckResult
        ObjectConstructData::ConstructBasicObjectPart(Args... args) {
  if constexpr (std::is_same_v<BasicObjectType, VarietyOperatorNode>) {
    // ������������
    auto object = std::make_unique<SimpleSentence>();
    bool result = object->SetSentenceOperateNode(
        std::make_shared<VarietyOperatorNode>(std::forward<Args>(args)...));
    assert(result);
    object_ = std::move(object);
  } else {
    // ������������
    object_ = std::make_unique<FunctionDefine>(std::forward<Args>(args)...);
  }
  return CheckResult::kSuccess;
}

// ����ͬAttachSingleNodeToTailNodeEmplace������ʹ���Ѿ������õĽڵ�

ObjectConstructData::CheckResult
ObjectConstructData::AttachSingleNodeToTailNodePointer(
    std::shared_ptr<const TypeInterface>&& next_node) {
  switch (type_chain_tail_->GetType()) {
    case StructOrBasicType::kFunction:
      // ����ָ�빹�����壬���²���Ľڵ���ڷ���ֵ��λ��
      if (next_node->GetType() == StructOrBasicType::kFunction) {
        // ������ĵ�һ������ֵ�ڵ�Ϊ����
        // �������ܷ��غ���
        return CheckResult::kReturnFunction;
      }
      static_cast<FunctionType&>(*type_chain_tail_)
          .SetReturnTypePointer(next_node);
      // �˴�Ϊ�˹���������Υ��constԭ��
      type_chain_tail_ = std::const_pointer_cast<TypeInterface>(next_node);
      break;
    case StructOrBasicType::kPointer:
      // ��Ҫ�������Ƿ���������������
      if (std::static_pointer_cast<PointerType>(type_chain_tail_)
              ->GetArraySize() != 0) [[unlikely]] {
        // ��0������������
        std::cerr << std::format("����{:} ��֧������ָ������", GetLine())
                  << std::endl;
        exit(-1);
      }
      [[fallthrough]];
    case StructOrBasicType::kEnd:
      // ��ͨ�ڵ㹹�����壬��ԭ���Ľṹ������
      type_chain_tail_->SetNextNode(next_node);
      // �˴�Ϊ�˹���������Υ��constԭ��
      type_chain_tail_ = std::const_pointer_cast<TypeInterface>(next_node);
      break;
    case StructOrBasicType::kBasic:
    case StructOrBasicType::kStruct:
    case StructOrBasicType::kUnion:
    case StructOrBasicType::kEnum:
      // ��ЩΪ�ս�ڵ㣬����������һ���ڵ�
      return CheckResult::kAttachToTerminalType;
      break;
    default:
      assert(false);
      break;
  }
  return CheckResult::kSuccess;
}

}  // namespace c_parser_frontend::parse_functions
#endif