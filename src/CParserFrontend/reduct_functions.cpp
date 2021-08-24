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
IdOrEquivenceReturnData IdOrEquivenceConstTagId(
    std::vector<WordDataToUser>&& word_data) {
  // ����Consttag�Ƿ�չ�Լ��Ӧ����2���ڵ�
  assert(word_data.size() == 2);
  auto& const_tag_data = word_data[0].GetNonTerminalWordData().user_data_;
  ConstTag const_tag;
  if (!const_tag_data.has_value()) {
    // Const��ǿչ�Լ��ʹ�÷�const���
    const_tag = ConstTag::kNonConst;
  } else {
    // ����const_tagΪ֮ǰ��Լ�õ���Tag
    const_tag = std::any_cast<ConstTag>(const_tag_data);
  }
  auto iter = parser_front_end.AnnounceVariety(
      std::string(word_data[1].GetTerminalWordData().word));
  // ���������ڵ�
  std::shared_ptr<VarietyOperatorNode> variety_node =
      std::make_shared<VarietyOperatorNode>(&iter->first, const_tag,
                                            LeftRightValueTag::kLeftValue);
  // �����ڱ��ڵ㣬�����ж�type_chain�Ƿ�Ϊ��
  std::shared_ptr<TypeInterface> type_chain = std::make_shared<EndType>();
  return IdOrEquivenceReturnData(variety_node, type_chain, type_chain);
}
IdOrEquivenceReturnData IdOrEquivenceConstTag(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& const_tag_data = word_data.front().GetNonTerminalWordData().user_data_;
  ConstTag const_tag;
  if (!const_tag_data.has_value()) {
    // Const��ǿչ�Լ��ʹ�÷�const���
    const_tag = ConstTag::kNonConst;
  } else {
    // ����const_tagΪ֮ǰ��Լ�õ���Tag
    const_tag = std::any_cast<ConstTag>(const_tag_data);
  }
  // ���������ڵ�
  std::shared_ptr<VarietyOperatorNode> variety_node =
      std::make_shared<VarietyOperatorNode>(nullptr, const_tag,
                                            LeftRightValueTag::kLeftValue);
  // �����ڱ��ڵ㣬�����ж�type_chain�Ƿ�Ϊ��
  std::shared_ptr<TypeInterface> type_chain = std::make_shared<EndType>();
  return IdOrEquivenceReturnData(variety_node, type_chain, type_chain);
}
std::any IdOrEquivenceNumAddressing(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  // ��ȡ֮ǰ����������Ϣ
  IdOrEquivenceReturnData&& data_before =
      std::any_cast<IdOrEquivenceReturnData>(
          word_data[0].GetNonTerminalWordData().user_data_);
  // ת��Ϊlong long���ַ���
  size_t chars_converted_to_longlong;
  std::string& array_size_or_index_string =
      word_data[2].GetTerminalWordData().word;
  // ���һ��ָ���ָ��ָ��������С
  data_before.GetTypeChainTailReference().SetNextNodePointer(
      std::make_shared<PointerType>(ConstTag::kNonConst,
                                    std::stoll(array_size_or_index_string,
                                               &chars_converted_to_longlong)));
  if (chars_converted_to_longlong != array_size_or_index_string.size())
      [[unlikely]] {
    // �������е����ֶ�������ת����˵��ʹ���˸�����
    std::cerr
        << std::format(
               "������{:} "
               "{:}�޷���Ϊ�����С��ƫ������ԭ�����Ϊ������������ֵ����",
               word_data[2].GetTerminalWordData().line,
               array_size_or_index_string)
        << std::endl;
    exit(-1);
  }
  // �����µ�β�ڵ�ָ��
  data_before.SetTypeChainTail(
      data_before.GetTypeChainTailReference().GetNextNodePointer());
  return std::move(word_data[0].GetNonTerminalWordData().user_data_);
}
std::any IdOrEquivenceAnonymousAddressing(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // ��ȡ֮ǰ����������Ϣ
  IdOrEquivenceReturnData& data_before =
      *std::any_cast<IdOrEquivenceReturnData>(
          &word_data[0].GetNonTerminalWordData().user_data_);
  // ���һ��ָ���ָ��ָ��������С
  // -1������ƶϴ�С
  data_before.GetTypeChainTailReference().SetNextNodePointer(
      std::make_shared<PointerType>(ConstTag::kNonConst, -1));
  // �����µ�β�ڵ�ָ��
  data_before.SetTypeChainTail(
      data_before.GetTypeChainTailReference().GetNextNodePointer());
  return std::move(word_data[0].GetNonTerminalWordData().user_data_);
}
std::any IdOrEquivencePointerAnnounce(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // ��ȡ֮ǰ����������Ϣ
  IdOrEquivenceReturnData& data_before =
      *std::any_cast<IdOrEquivenceReturnData>(
          &word_data[2].GetNonTerminalWordData().user_data_);
  // ���һ��ָ���ָ��ָ��������С
  // 0����ָ��
  data_before.GetTypeChainTailReference().SetNextNodePointer(
      std::make_shared<PointerType>(ConstTag::kNonConst, 0));
  // �����µ�β�ڵ�ָ��
  data_before.SetTypeChainTail(
      data_before.GetTypeChainTailReference().GetNextNodePointer());
  return std::move(word_data[2].GetNonTerminalWordData());
}
std::any IdOrEquivenceInBrackets(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  return std::move(word_data[1].GetNonTerminalWordData().user_data_);
}
EnumReturnData NotEmptyEnumArgumentsIdInit(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::string& id = word_data.front().GetTerminalWordData().word;
  EnumReturnData enum_return_data;
  // ��һ��ö��ֵĬ�ϴ�0��ʼ
  enum_return_data.GetContainer().emplace(std::move(id), 0);
  // ���������Ϣ
  enum_return_data.SetLastValue(0);
  enum_return_data.SetMaxValue(0);
  enum_return_data.SetMinValue(0);
  return enum_return_data;
}
EnumReturnData NotEmptyEnumArgumentsIdAssignNumInit(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  std::string& id = word_data[0].GetTerminalWordData().word;
  std::string& num = word_data[2].GetTerminalWordData().word;
  EnumReturnData enum_return_data;
  size_t chars_converted_to_longlong;
  // ö�����ֵ
  long long id_value = std::stoll(num, &chars_converted_to_longlong);
  if (chars_converted_to_longlong != num.size()) [[unlikely]] {
    // ���ڸ��������������е�ֵ��������Ϊö�����ֵ
    std::cerr << std::format(
                     "������{:} {:}������Ϊö�����ֵ��ԭ�����Ϊ������",
                     word_data[2].GetTerminalWordData().line, num)
              << std::endl;
    exit(-1);
  }
  // ��Ӹ���ֵ��ö��ֵ
  enum_return_data.GetContainer().emplace(std::move(id), id_value);
  // �趨�����Ϣ
  enum_return_data.SetLastValue(id_value);
  enum_return_data.SetMaxValue(id_value);
  enum_return_data.SetMinValue(id_value);
  return enum_return_data;
}
std::any NotEmptyEnumArgumentsId(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  EnumReturnData& data_before = *std::any_cast<EnumReturnData>(
      &word_data[0].GetNonTerminalWordData().user_data_);
  std::string& id = word_data[2].GetTerminalWordData().word;
  // δָ�����ֵ��˳����һ����������ֵ
  long long id_value = data_before.GetLastValue() + 1;
  auto [iter, inserted] =
      data_before.GetContainer().emplace(std::move(id), id_value);
  if (!inserted) [[unlikely]] {
    // ����ӵ����ID�Ѿ�����
    std::cerr << std::format("������{:} ö��������{:}�Ѷ���",
                             word_data[2].GetTerminalWordData().line, id)
              << std::endl;
    exit(-1);
  }
  // ���������Ϣ
  data_before.SetLastValue(id_value);
  if (data_before.GetMaxValue() < id_value) {
    data_before.SetMaxValue(id_value);
  } else if (data_before.GetMinValue() > id_value) {
    data_before.SetMinValue(id_value);
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_data_);
}
std::any NotEmptyEnumArgumentsIdAssignNum(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 5);
  EnumReturnData& data_before = *std::any_cast<EnumReturnData>(
      &word_data[0].GetNonTerminalWordData().user_data_);
  std::string& id = word_data[2].GetTerminalWordData().word;
  std::string& num = word_data[4].GetTerminalWordData().word;
  size_t chars_converted_to_longlong;
  // δָ�����ֵ��˳����һ����������ֵ
  long long id_value = std::stoll(num, &chars_converted_to_longlong);
  if (chars_converted_to_longlong != num.size()) [[unlikely]] {
    // �������ֵ������ȫת��Ϊlong long
    std::cerr << std::format(
                     "������{0:} ö����{1:} = {2:}�� "
                     "{2:}������Ϊö�����ֵ������ԭ��Ϊ������",
                     word_data[4].GetTerminalWordData().line, id, num)
              << std::endl;
    exit(-1);
  }
  auto [iter, inserted] =
      data_before.GetContainer().emplace(std::move(id), id_value);
  if (!inserted) [[unlikely]] {
    // ����ӵ����ID�Ѿ�����
    std::cerr << std::format(
                     "������{0:} ö����{1:} = {2:}�� ö��������{1:}�Ѷ���",
                     word_data[2].GetTerminalWordData().line, id, num)
              << std::endl;
    exit(-1);
  }
  // ���������Ϣ
  data_before.SetLastValue(id_value);
  if (data_before.GetMaxValue() < id_value) {
    data_before.SetMaxValue(id_value);
  } else if (data_before.GetMinValue() > id_value) {
    data_before.SetMinValue(id_value);
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_data_);
}
}  // namespace c_parser_frontend::parse_functions
