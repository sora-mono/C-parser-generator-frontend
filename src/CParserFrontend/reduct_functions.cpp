#include "reduct_functions.h"
namespace c_parser_frontend::parse_functions {
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueChar(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::string& word = word_data.front().GetTerminalWordData().word;
  // ''�м�һ���ַ�
  assert(word.size() == 3);
  // ��ȡ�����ַ���Ϊֵ
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kBasic, std::to_string(word[1]));
  initialize_node->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kChar,
                                              SignTag::kSigned>());
  return initialize_node;
}
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueNum(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::string& word = word_data.front().GetTerminalWordData().word;
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kBasic, std::move(word));
  initialize_node->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kChar,
                                              SignTag::kSigned>());
  return initialize_node;
}
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueString(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::string& word = word_data.front().GetTerminalWordData().word;
  // �ַ���ʹ��""��Ϊ�ָ����
  assert(word.front() == '"');
  assert(word.back() == '"');
  // ɾ��β����"
  word.pop_back();
  // ɾ��ͷ����"
  word.erase(word.begin());
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kString, std::move(word));
  initialize_node->SetInitDataType(
      CommonlyUsedTypeGenerator::GetConstExprStringType());
  return initialize_node;
}
std::shared_ptr<VarietyOperatorNode> IdOrEquivenceConstTagId(
    std::vector<WordDataToUser>&& word_data) {
  // ����Consttag�Ƿ�չ�Լ��Ӧ����2���ڵ�
  assert(word_data.size() == 2);

}
}  // namespace c_parser_frontend::parse_functions