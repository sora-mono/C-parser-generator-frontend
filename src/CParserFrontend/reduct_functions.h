#ifndef CPARSERFRONTEND_PARSE_FUNCTIONS_H_
#define CPARSERFRONTEND_PARSE_FUNCTIONS_H_
#include <format>
#include <iostream>
#include <limits>

#include "Generator/SyntaxGenerator/process_function_interface.h"
#include "c_parser_frontend.h"
#include "operator_node.h"
#include "type_system.h"
namespace c_parser_frontend::parse_functions {
using WordDataToUser = frontend::generator::syntaxgenerator::
    ProcessFunctionInterface::WordDataToUser;
using c_parser_frontend::operator_node::BasicTypeInitializeOperatorNode;
using c_parser_frontend::operator_node::InitializeType;
using c_parser_frontend::operator_node::LeftRightValueTag;
using c_parser_frontend::operator_node::OperatorNodeInterface;
using c_parser_frontend::operator_node::VarietyOperatorNode;
using c_parser_frontend::type_system::BuiltInType;
using c_parser_frontend::type_system::CommonlyUsedTypeGenerator;
using c_parser_frontend::type_system::ConstTag;
using c_parser_frontend::type_system::EndType;
using c_parser_frontend::type_system::EnumType;
using c_parser_frontend::type_system::PointerType;
using c_parser_frontend::type_system::SignTag;
using c_parser_frontend::type_system::TypeInterface;

// ����ȫ�ֱ�����C���Ա�����ǰ�˵������
c_parser_frontend::CParserFrontend parser_front_end;

// IdOrEquivence����ʽ��Լʱ�õ�������
class IdOrEquivenceReturnData {
 public:
  IdOrEquivenceReturnData(std::shared_ptr<VarietyOperatorNode> variety_node,
                          std::shared_ptr<TypeInterface> type_chain_head,
                          std::shared_ptr<TypeInterface> type_chain_tail)
      : variety_node_(std::move(variety_node)),
        type_chain_head_(std::move(type_chain_head)),
        type_chain_tail_(std::move(type_chain_tail)) {}
  std::shared_ptr<VarietyOperatorNode> GetVarietyNodePoitner() const {
    return variety_node_;
  }
  VarietyOperatorNode& GetVarietyNodeReference() const {
    return *variety_node_;
  }
  void SetVarietyNode(std::shared_ptr<VarietyOperatorNode>&& variety_node) {
    variety_node_ = std::move(variety_node);
  }
  std::shared_ptr<TypeInterface> GetTypeChainHeadPointer() const {
    return type_chain_head_;
  }
  TypeInterface& GetTypeChainHeadReference() const { return *type_chain_head_; }
  void SetTypeChainHead(std::shared_ptr<TypeInterface>&& type_chain_head) {
    type_chain_head_ = std::move(type_chain_head);
  }
  std::shared_ptr<TypeInterface> GetTypeChainTailPointer() const {
    return type_chain_tail_;
  }
  TypeInterface& GetTypeChainTailReference() const { return *type_chain_tail_; }
  void SetTypeChainTail(std::shared_ptr<TypeInterface>&& type_chain_tail) {
    type_chain_tail_ = std::move(type_chain_tail);
  }
  // ��ɱ����Ĺ鲢��ȥ����Ϊ�Ż��ֶ���type_chain_head_���õ��ڱ��ڵ�
  // ��variety_node_�������Ͳ�����ָ��variety_node_��ָ��
  // ���øú�����type_chain_head_ʧЧ
  std::shared_ptr<VarietyOperatorNode> FinishVarietyNodeReduct() {
    type_chain_head_ = type_chain_head_->GetNextNodePointer();
    variety_node_->SetVarietyType(std::move(type_chain_head_));
    return variety_node_;
  }

 private:
  // �����ڵ�
  std::shared_ptr<VarietyOperatorNode> variety_node_;
  // ָ����������ͷ���
  std::shared_ptr<TypeInterface> type_chain_head_;
  // ָ����������β�ڵ�
  std::shared_ptr<TypeInterface> type_chain_tail_;
};
// ö�ٲ�����Լʱ�õ�������
class EnumReturnData {
 public:
  EnumType::EnumContainerType& GetContainer() { return enum_container_; }
  long long GetMaxValue() const { return max_value_; }
  void SetMaxValue(long long max_value) { max_value_ = max_value; }
  long long GetMinValue() const { return min_value_; }
  void SetMinValue(long long min_value) { min_value_ = min_value; }
  long long GetLastValue() const { return last_value_; }
  void SetLastValue(long long last_value) { last_value_ = last_value; }

 private:
  // ö������ֵ�Ĺ�������
  EnumType::EnumContainerType enum_container_;
  // ����ö��ֵ
  long long max_value_ = LLONG_MIN;
  // ��С��ö��ֵ
  long long min_value_ = LLONG_MAX;
  // �ϴ���ӵ�ö��ֵ
  long long last_value_;
};

// ����ʽ��Լʱʹ�õĺ���

// SingleConstexprValue->Char
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueChar(
    std::vector<WordDataToUser>&& word_data);
// SingleConstexprValue->Num
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
  return BuiltInType::kChar;
}
// FundamentalType->"short"
BuiltInType FundamentalTypeShort(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "short");
  return BuiltInType::kShort;
}
// FundamentalType->"int"
BuiltInType FundamentalTypeInt(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "int");
  return BuiltInType::kInt;
}
// FundamentalType->"long"
BuiltInType FundamentalTypeLong(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "long");
  return BuiltInType::kLong;
}
// FundamentalType->"float"
BuiltInType FundamentalTypeFloat(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "float");
  return BuiltInType::kFloat;
}
// FundamentalType->"double"
BuiltInType FundamentalTypeDouble(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  assert(word_data.front().GetTerminalWordData().word == "double");
  return BuiltInType::kDouble;
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
IdOrEquivenceReturnData IdOrEquivenceConstTagId(
    std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->ConstTag
IdOrEquivenceReturnData IdOrEquivenceConstTag(
    std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->IdOrEquivence "[" Num "]"
// ����ֵ���ͣ�IdOrEquivenceReturnData
// ����std::any��ֹ�ƶ�����IdOrEquivenceReturnData
std::any IdOrEquivenceNumAddressing(std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->IdOrEquivence "[" "]"
// ����ֵ���ͣ�IdOrEquivenceReturnData
// ����std::any��ֹ�ƶ�����IdOrEquivenceReturnData
// ��������ӵ�ָ������Ӧ�����СΪ-1����Ǵ˴������С��Ҫ���ݸ�ֵ����ƶ�
std::any IdOrEquivenceAnonymousAddressing(
    std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->Consttag "*" IdOrEquivence
// ����ֵ���ͣ�IdOrEquivenceReturnData
// ����std::any��ֹ�ƶ�����IdOrEquivenceReturnData
std::any IdOrEquivencePointerAnnounce(std::vector<WordDataToUser>&& word_data);
// IdOrEquivence->"(" IdOrEquivence ")"
// ����ֵ���ͣ�IdOrEquivenceReturnData
// ����std::any��ֹ�ƶ�����IdOrEquivenceReturnData
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
  return std::move(word_data.front().GetNonTerminalWordData().user_data_);
}
}  // namespace c_parser_frontend::parse_functions
#endif