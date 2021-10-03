#include "reduct_functions.h"

#include <optional>
#include <tuple>
namespace c_parser_frontend::parse_functions {

std::pair<EnumType::EnumContainerType::iterator, bool>
EnumReturnData::AddMember(std::string&& member_name, long long value) {
  auto return_result = enum_container_.emplace(std::move(member_name), value);
  if (return_result.second == true) {
    // �ɹ����룬�����������
    SetLastValue(value);
    if (GetMaxValue() < value) {
      SetMaxValue(value);
    } else if (GetMinValue() > value) {
      SetMinValue(value);
    }
  }
  return return_result;
}

void VarietyOrFunctionConstructError(
    ObjectConstructData::CheckResult check_result,
    const std::string& object_name) {
  switch (check_result) {
    [[likely]] case ObjectConstructData::CheckResult::kSuccess : break;
    case ObjectConstructData::CheckResult::kAttachToTerminalType:
      std::cerr
          << std::format(
                 "����{:} ����������{:}�Ѵ����ս����ͣ����������������ͽṹ",
                 GetLine(), object_name)
          << std::endl;
      exit(-1);
      break;
    case ObjectConstructData::CheckResult::kReturnFunction:
      std::cerr << std::format("����{:} ����{:}����ֵ����Ϊ����", GetLine(),
                               object_name);
      exit(-1);
      break;
    case ObjectConstructData::CheckResult::kPointerEnd:
      std::cerr << std::format("����{:} ����������{:}ȱ�پ�������", GetLine(),
                               object_name)
                << std::endl;
      exit(-1);
      break;
    case ObjectConstructData::CheckResult::kEmptyChain:
      std::cerr << std::format("����{:} û��Ϊ����������{:}�ṩ�κ�����",
                               GetLine(), object_name)
                << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
}

// ��ConstTag�Ĺ�Լ��Ϣ�л�ȡConstTag����
// ��Ҫ��֤����Ϊ���ս����ʽConstTag��Լ�õ������ݣ������Ƿ�Ϊ�չ�Լ��
ConstTag GetConstTag(const WordDataToUser& raw_const_tag_data) {
  auto& const_tag_data =
      raw_const_tag_data.GetNonTerminalWordData().user_returned_data;
  ConstTag const_tag = ConstTag::kNonConst;
  if (const_tag_data.has_value()) [[unlikely]] {
    const_tag = ConstTag::kConst;
  }
  return const_tag;
}

// ��������ʱ�ĳ�ʼ���ͻ�ȡһ�������зǵ�һ������������
std::shared_ptr<const TypeInterface> GetExtendAnnounceType(
    const std::shared_ptr<const TypeInterface>& source_type) {
  if (source_type->GetType() == StructOrBasicType::kPointer) [[unlikely]] {
    // ��һ�������ı���ʹ����ȫ���ͣ�����ı���ʹ��ȥ��һ��ָ�������
    return source_type->GetNextNodePointer();
  } else {
    // ��һ�������ı�������ָ�룬��������������һ����ͬ
    return source_type;
  }
}

void CheckAssignableCheckResult(AssignableCheckResult assignable_check_result) {
  switch (assignable_check_result) {
    case AssignableCheckResult::kNonConvert:
    case AssignableCheckResult::kUpperConvert:
    case AssignableCheckResult::kConvertToVoidPointer:
    case AssignableCheckResult::kZeroConvertToPointer:
      // �������Ը�ֵ�����
      break;
    case AssignableCheckResult::kUnsignedToSigned:
      // ʹ�þ���
      std::cerr << std::format("����{:} ���棺���޷������͸�ֵ���з�������",
                               GetLine())
                << std::endl;
      break;
    case AssignableCheckResult::kSignedToUnsigned:
      // ʹ�þ���
      std::cerr << std::format("����{:} ���棺���з������͸�ֵ���޷�������",
                               GetLine())
                << std::endl;
      break;
    case AssignableCheckResult::kLowerConvert:
      std::cerr << std::format("����{:} �ڸ�ֵʱ������խת��", GetLine())
                << std::endl;
      exit(-1);
      break;
    case AssignableCheckResult::kCanNotConvert:
      std::cerr << std::format("����{:} �޷�ת������", GetLine()) << std::endl;
      exit(-1);
      break;
    case AssignableCheckResult::kAssignedNodeIsConst:
      std::cerr << std::format("����{:} ������ʱ�޷���const����ֵ", GetLine())
                << std::endl;
      exit(-1);
      break;
    case AssignableCheckResult::kAssignToRightValue:
      std::cerr << std::format("����{:} �޷�����ֵ����ֵ", GetLine())
                << std::endl;
      exit(-1);
      break;
    case AssignableCheckResult::kArgumentsFull:
      // TODO ֧�ֿɱ�����������޸ĸñ�����и�������ж�
      std::cerr << std::format("����{:} ʵ�����������β�����", GetLine())
                << std::endl;
      exit(-1);
      break;
    case AssignableCheckResult::kMayBeZeroToPointer:
      // �������ڷ���ǰӦ������ȷ�ϲ��滻�ɸ���ȷ�Ľ��
    default:
      assert(false);
      break;
  }
}

void CheckMathematicalComputeTypeResult(
    DeclineMathematicalComputeTypeResult
        decline_mathematical_compute_type_result) {
  switch (decline_mathematical_compute_type_result) {
    case DeclineMathematicalComputeTypeResult::kComputable:
    case DeclineMathematicalComputeTypeResult::kConvertToLeft:
    case DeclineMathematicalComputeTypeResult::kConvertToRight:
    case DeclineMathematicalComputeTypeResult::kLeftOffsetRightPointer:
    case DeclineMathematicalComputeTypeResult::kLeftPointerRightOffset:
      // ��������
      break;
    case DeclineMathematicalComputeTypeResult::kLeftNotComputableType:
      std::cerr << std::format("����{:} �����������ǿ���������", GetLine())
                << std::endl;
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kLeftNotIntger:
      std::cerr << std::format("����{:} ����������Ϊָ��ƫ����ʱ��������",
                               GetLine())
                << std::endl;
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kLeftRightBothPointer:
      std::cerr << std::format("����{:} ����ָ�������޷�������ѧ����",
                               GetLine())
                << std::endl;
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kRightNotComputableType:
      std::cerr << std::format("����{:} �����������ǿ���������", GetLine())
                << std::endl;
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kRightNotIntger:
      std::cerr << std::format("����{:} ����������Ϊָ��ƫ����ʱ��������",
                               GetLine())
                << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
}

void CheckAddTypeResult(AddTypeResult add_type_result) {
  switch (add_type_result) {
    case c_parser_frontend::type_system::AddTypeResult::kAbleToAdd:
    case c_parser_frontend::type_system::AddTypeResult::kNew:
    case c_parser_frontend::type_system::AddTypeResult::kFunctionDefine:
    case c_parser_frontend::type_system::AddTypeResult::kShiftToVector:
    case c_parser_frontend::type_system::AddTypeResult::kAddToVector:
      break;
    case c_parser_frontend::type_system::AddTypeResult::
        kAnnounceOrDefineBeforeFunctionAnnounce:
      std::cerr << std::format("����{:} ����{:} Warning: �ظ���������",
                               GetLine(), GetColumn())
                << std::endl;
      break;
    case c_parser_frontend::type_system::AddTypeResult::kTypeAlreadyIn:
      std::cerr << std::format("����{:} ����{:} �Ѵ���ͬ������", GetLine(),
                               GetColumn())
                << std::endl;
      exit(-1);
      break;
    case c_parser_frontend::type_system::AddTypeResult::kRedefineFunction:
      std::cerr << std::format("����{:} ����{:} �ض��庯��", GetLine(),
                               GetColumn())
                << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
}

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
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt8,
                                              SignTag::kSigned>());
  return initialize_node;
}
std::shared_ptr<BasicTypeInitializeOperatorNode>
SingleConstexprValueIndexedString(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  size_t converted_num;
  std::string& index_str = word_data[2].GetTerminalWordData().word;
  std::string& string_str = word_data[0].GetTerminalWordData().word;
  size_t index = std::stoull(index_str, &converted_num);
  // ���ʹ�õ��±��Ƿ�ȫ��ת��
  if (converted_num != index_str.size()) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} �޷�ת���±꣬����Ϊ�������򳬳������±귶Χ",
                     GetLine())
              << std::endl;
    exit(-1);
  }
  // ����±��Ƿ�Խ��
  char result;
  if (index > string_str.size()) [[unlikely]] {
    std::cerr << std::format("����{:} ʹ�õ��±�����ַ�����С", GetLine())
              << std::endl;
    exit(-1);
  } else if (index == string_str.size()) [[unlikely]] {
    result = '\0';
  } else {
    result = string_str[index];
  }
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kBasic, std::to_string(result));
  initialize_node->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt8,
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
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt8,
                                              SignTag::kSigned>());
  return initialize_node;
}
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueString(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::string& str = word_data.front().GetTerminalWordData().word;
  // �ַ���ʹ��""��Ϊ�ָ����
  assert(str.front() == '"');
  assert(str.back() == '"');
  // ɾ��β����"
  str.pop_back();
  // ɾ��ͷ����"
  str.erase(str.begin());
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kString, std::move(str));
  initialize_node->SetInitDataType(
      CommonlyUsedTypeGenerator::GetConstExprStringType());
  return initialize_node;
}
std::shared_ptr<ObjectConstructData> IdOrEquivenceConstTagId(
    std::vector<WordDataToUser>&& word_data) {
  // ����Consttag�Ƿ�չ�Լ��Ӧ����2���ڵ�
  assert(word_data.size() == 2);
  ConstTag const_tag = GetConstTag(word_data[0]);
  std::string& name = word_data[1].GetTerminalWordData().word;
  std::shared_ptr<ObjectConstructData> construct_data =
      std::make_shared<ObjectConstructData>(std::move(name));
  // �Ȳ���������ָ�룬��Ϊ�޷�ȷ�����ջ�ú���ͷ���Ǳ���
  construct_data->ConstructBasicObjectPart<VarietyOperatorNode>(
      nullptr, const_tag, LeftRightValueTag::kLeftValue);
  return construct_data;
}
std::shared_ptr<ObjectConstructData> IdOrEquivenceConstTag(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  ConstTag const_tag = GetConstTag(word_data.front());
  std::shared_ptr<ObjectConstructData> construct_data =
      std::make_shared<ObjectConstructData>(std::string());
  construct_data->ConstructBasicObjectPart<VarietyOperatorNode>(
      nullptr, const_tag, LeftRightValueTag::kLeftValue);
  return construct_data;
}
std::any IdOrEquivenceNumAddressing(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  // ��ȡ֮ǰ����������Ϣ
  auto data_before =
      std::move(std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data));
  // ת��Ϊlong long���ַ���
  size_t chars_converted_to_longlong;
  std::string& array_size_or_index_string =
      word_data[2].GetTerminalWordData().word;
  size_t array_size =
      std::stoull(array_size_or_index_string, &chars_converted_to_longlong);
  // ����Ƿ����е����־�����ת��
  if (chars_converted_to_longlong != array_size_or_index_string.size())
      [[unlikely]] {
    // �������е����ֶ�������ת����˵��ʹ���˸�����
    std::cerr
        << std::format(
               "������{:} "
               "{:}�޷���Ϊ�����С��ƫ������ԭ�����Ϊ����������ֵ���󡢸���",
               GetLine(), array_size_or_index_string)
        << std::endl;
    exit(-1);
  }
  // ���һ��ָ���ָ��ָ��������С
  auto result = data_before->AttachSingleNodeToTailNodeEmplace<PointerType>(
      ConstTag::kNonConst, array_size);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result, data_before->GetObjectName());
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}
std::any IdOrEquivenceAnonymousAddressing(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // ��ȡ֮ǰ����������Ϣ
  std::shared_ptr<ObjectConstructData>& data_before =
      std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  // ���һ��ָ���ָ��ָ��������С
  // -1������ƶϴ�С
  auto result = data_before->AttachSingleNodeToTailNodeEmplace<PointerType>(
      ConstTag::kNonConst, -1);
  // ����Ƿ�ɹ����
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result,

                                    data_before->GetObjectName());
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}
std::any IdOrEquivencePointerAnnounce(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // ��ȡ֮ǰ����������Ϣ
  std::shared_ptr<ObjectConstructData>& data_before =
      std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  // ���һ��ָ���ָ��ָ��������С
  // 0����ָ��
  auto result = data_before->AttachSingleNodeToTailNodeEmplace<PointerType>(
      ConstTag::kNonConst, 0);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result,

                                    data_before->GetObjectName());
  }
  return std::move(word_data[2].GetNonTerminalWordData());
}
std::any IdOrEquivenceInBrackets(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  return std::move(word_data[1].GetNonTerminalWordData().user_returned_data);
}
EnumReturnData NotEmptyEnumArgumentsIdBase(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::string& enum_member_name = word_data.front().GetTerminalWordData().word;
  EnumReturnData enum_return_data;
  // ��һ��ö��ֵĬ�ϴ�0��ʼ
  auto [iter, inserted] =
      enum_return_data.AddMember(std::move(enum_member_name), 0);
  assert(inserted);
  return enum_return_data;
}
EnumReturnData NotEmptyEnumArgumentsIdAssignNumBase(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  std::string& enum_member_name = word_data[0].GetTerminalWordData().word;
  std::string& num = word_data[2].GetTerminalWordData().word;
  EnumReturnData enum_return_data;
  size_t chars_converted_to_longlong;
  // ö�����ֵ
  long long enum_member_value = std::stoll(num, &chars_converted_to_longlong);
  if (chars_converted_to_longlong != num.size()) [[unlikely]] {
    // ���ڸ��������������е�ֵ��������Ϊö�����ֵ
    std::cerr << std::format(
                     "������{:} {:}������Ϊö�����ֵ��ԭ�����Ϊ������",
                     GetLine(), num)
              << std::endl;
    exit(-1);
  }
  // ��Ӹ���ֵ��ö��ֵ
  auto [iter, inserted] = enum_return_data.AddMember(
      std::move(enum_member_name), enum_member_value);
  if (!inserted) [[unlikely]] {
    // ����ʧ�ܣ�����ͬ����Ա
    std::cerr << std::format("������{:} ö���{:}�Ѷ���", GetLine(),
                             enum_member_name)
              << std::endl;
    exit(-1);
  }
  return enum_return_data;
}
std::any NotEmptyEnumArgumentsIdExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  EnumReturnData& data_before = std::any_cast<EnumReturnData&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  std::string& enum_member_name = word_data[2].GetTerminalWordData().word;
  // δָ�����ֵ��˳����һ����������ֵ
  long long enum_member_value = data_before.GetLastValue() + 1;
  auto [iter, inserted] =
      data_before.AddMember(std::move(enum_member_name), enum_member_value);
  if (!inserted) [[unlikely]] {
    // ����ӵ����ID�Ѿ�����
    std::cerr << std::format("������{:} ö���{:}�Ѷ���", GetLine(),
                             enum_member_name)
              << std::endl;
    exit(-1);
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}
std::any NotEmptyEnumArgumentsIdAssignNumExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 5);
  EnumReturnData& data_before = std::any_cast<EnumReturnData&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  std::string& enum_member_name = word_data[2].GetTerminalWordData().word;
  std::string& num = word_data[4].GetTerminalWordData().word;
  size_t chars_converted_to_longlong;
  // δָ�����ֵ��˳����һ����������ֵ
  long long enum_member_value = std::stoll(num, &chars_converted_to_longlong);
  if (chars_converted_to_longlong != num.size()) [[unlikely]] {
    // �������ֵ������ȫת��Ϊlong long
    std::cerr << std::format(
                     "������{0:} ö����{1:} = {2:}�� "
                     "{2:}������Ϊö�����ֵ������ԭ��Ϊ������",
                     GetLine(), enum_member_name, num)
              << std::endl;
    exit(-1);
  }
  auto [iter, inserted] =
      data_before.AddMember(std::move(enum_member_name), enum_member_value);
  if (!inserted) [[unlikely]] {
    // ����ӵ����ID�Ѿ�����
    std::cerr << std::format(
                     "������{0:} ö����{1:} = {2:}�� ö���{1:}�Ѷ���",
                     GetLine(), enum_member_name, num)
              << std::endl;
    exit(-1);
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}
std::shared_ptr<EnumType> EnumDefine(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 5);
  std::any& enum_return_data =
      word_data[3].GetNonTerminalWordData().user_returned_data;
  std::string& enum_name = word_data[1].GetTerminalWordData().word;
  // ������ö���Ի�ȡָ�����ֵ�ָ��
  auto iter = parser_frontend.AnnounceTypeName(std::move(enum_name));
  std::shared_ptr<EnumType> enum_type;
  if (enum_return_data.has_value()) {
    // ��ö���г�Ա
    enum_type = std::make_shared<EnumType>(
        &iter->first,
        std::move(
            std::any_cast<EnumReturnData&>(enum_return_data).GetContainer()));
  } else {
    // ��ö��û�г�Ա
    enum_type = std::make_shared<EnumType>(&iter->first);
  }
  // ��Ӹ�ö�ٵĶ���
  auto [insert_iter, result] =
      parser_frontend.DefineType(std::move(enum_name), enum_type);
  CheckAddTypeResult(result);
  return enum_type;
}
std::shared_ptr<EnumType> EnumAnonymousDefine(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  std::any& enum_return_data =
      word_data[3].GetNonTerminalWordData().user_returned_data;
  std::shared_ptr<EnumType> enum_type;
  if (enum_return_data.has_value()) {
    // ��ö���г�Ա
    enum_type = std::make_shared<EnumType>(
        nullptr,
        std::move(
            std::any_cast<EnumReturnData&>(enum_return_data).GetContainer()));
  } else {
    // ��ö��û�г�Ա
    enum_type = std::make_shared<EnumType>(nullptr);
  }
  // ����ö�����趨��
  return enum_type;
}
std::shared_ptr<StructureTypeInterface> StructureDefineInitHead(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  // �����ṹ���Ա�洢����
  // std::any�޷�ʹ��std::unique_ptr���죬ֻ��ʹ��std::shared_ptr
  auto& [struct_name, struct_type] =
      std::any_cast<std::pair<std::string, StructOrBasicType>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  // ��ȡָ��ṹ����ָ��
  const std::string* structure_name_pointer = nullptr;
  if (!struct_name.empty()) {
    // ��������Ľṹ������������������
    auto iter = parser_frontend.AnnounceTypeName(std::move(struct_name));
    structure_name_pointer = &iter->first;
  }
  switch (struct_type) {
    case StructOrBasicType::kStruct:
      structure_type_constructuring =
          std::make_shared<StructType>(structure_name_pointer);
      break;
    case StructOrBasicType::kUnion:
      structure_type_constructuring =
          std::make_shared<UnionType>(structure_name_pointer);
      break;
    default:
      assert(false);
      break;
  }
  // ����Ƿ������ṹ��ע��ṹ��/����������
  // ��ʹ��struct_name��Ϊ�Ѿ����ƶ�����
  if (structure_name_pointer != nullptr) [[likely]] {
    auto [iter, result] = parser_frontend.DefineType(
        *structure_name_pointer, structure_type_constructuring);
    CheckAddTypeResult(result);
  }
  return structure_type_constructuring;
}
std::any StructureDefine(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}
std::shared_ptr<const StructureTypeInterface> StructTypeStructAnnounce(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& struct_data = std::any_cast<std::pair<std::string, StructOrBasicType>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  std::string& struct_name = struct_data.first;
  StructOrBasicType struct_type = struct_data.second;
  auto [type_pointer, get_type_result] =
      parser_frontend.GetType(struct_name, struct_type);
  switch (get_type_result) {
    case GetTypeResult::kSuccess:
      // �ɹ���ȡ
      break;
    case GetTypeResult::kTypeNameNotFound:
    case GetTypeResult::kNoMatchTypePrefer: {
      // �Ҳ����������Ƶ�����
      const char* type_name =
          struct_type == StructOrBasicType::kStruct ? "�ṹ��" : "������";
      std::cerr << std::format("����{:} {:}{:}δ����", GetLine(), type_name,
                               struct_name)
                << std::endl;
    } break;
    case GetTypeResult::kSeveralSameLevelMatches:
      // ���������������ѡ��ƫ��ʱ��Ч
    default:
      assert(false);
      break;
  }
  return std::static_pointer_cast<const StructureTypeInterface>(type_pointer);
}
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeFundamental(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // ��ȡConstTag
  ConstTag const_tag = GetConstTag(word_data[0]);
  // ��ȡSignTag
  auto& sign_tag_data =
      word_data[1].GetNonTerminalWordData().user_returned_data;
  SignTag sign_tag = SignTag::kSigned;
  if (!sign_tag_data.has_value()) {
    sign_tag = std::any_cast<SignTag&>(sign_tag_data);
  }
  BuiltInType builtin_type = std::any_cast<BuiltInType&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  switch (builtin_type) {
    case c_parser_frontend::type_system::BuiltInType::kInt1:
    case c_parser_frontend::type_system::BuiltInType::kVoid:
      // ����������ֻ��ʹ���޷���
      sign_tag = SignTag::kUnsigned;
      break;
    default:
      break;
  }
  return std::make_pair(CommonlyUsedTypeGenerator::GetBasicTypeNotTemplate(
                            builtin_type, sign_tag),
                        const_tag);
}

// BasicType->ConstTag StructType
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeStructType(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  ConstTag const_tag = GetConstTag(word_data[0]);
  return std::make_pair(
      std::move(std::any_cast<std::shared_ptr<StructureTypeInterface>&>(
          word_data[1].GetNonTerminalWordData().user_returned_data)),
      const_tag);
}

// BasicType->ConstTag Id
// ���ػ�ȡ����������ConstTag
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeId(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  ConstTag const_tag = GetConstTag(word_data[0]);
  std::string& type_name = word_data[1].GetTerminalWordData().word;
  assert(!type_name.empty());
  // ��ȡID��Ӧ������
  auto [type_pointer, result] =
      parser_frontend.GetType(type_name, StructOrBasicType::kNotSpecified);
  switch (result) {
    case GetTypeResult::kSuccess:
      // �ɹ���ȡ��������
      break;
    case GetTypeResult::kSeveralSameLevelMatches:
      std::cerr << std::format("����{:} ������{:}��Ӧ���ͬ������", GetLine(),
                               type_name);
      exit(-1);
      break;
    case GetTypeResult::kTypeNameNotFound:
      std::cerr << std::format("����{:} ����{:}������", GetLine(), type_name)
                << std::endl;
      exit(-1);
      break;
    case GetTypeResult::kNoMatchTypePrefer:
      // ������ѡ������ʱ���ܷ��ظý��
    default:
      assert(false);
      break;
  }
  return std::make_pair(std::move(type_pointer), const_tag);
}
std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeEnumAnnounce(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  ConstTag const_tag = GetConstTag(word_data[0]);
  std::string& enum_name =
      std::any_cast<std::pair<std::string, StructOrBasicType>&>(
          word_data[1].GetNonTerminalWordData().user_returned_data)
          .first;
  auto [type_pointer, result] =
      parser_frontend.GetType(enum_name, StructOrBasicType::kNotSpecified);
  switch (result) {
    case GetTypeResult::kSuccess:
      // �ɹ���ȡ��������
      break;
    case GetTypeResult::kSeveralSameLevelMatches:
      std::cerr << std::format("����{:} ������{:}��Ӧ���ͬ������", GetLine(),
                               enum_name);
      exit(-1);
      break;
    case GetTypeResult::kTypeNameNotFound:
      std::cerr << std::format("����{:} ����{:}������", GetLine(), enum_name)
                << std::endl;
      exit(-1);
      break;
    case GetTypeResult::kNoMatchTypePrefer:
      // ������ѡ������ʱ���ܷ��ظý��
    default:
      assert(false);
      break;
  }
  return std::make_pair(std::move(type_pointer), const_tag);
}

std::any FunctionRelaventBasePartFunctionInitBase(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  // ����������������ָ����Ϊ�����β�
  if (!word_data[0].GetNonTerminalWordData().user_returned_data.has_value())
      [[unlikely]] {
    std::cerr << std::format("����{:} �޷�������������", GetLine())
              << std::endl;
    exit(-1);
  } else {
    // IdOrEquivence����ʽ��Լ�õ�������
    std::shared_ptr<ObjectConstructData>& construct_data =
        std::any_cast<std::shared_ptr<ObjectConstructData>&>(
            word_data[0].GetNonTerminalWordData().user_returned_data);
    // ���ù������ݣ��ڹ�����������ʱʹ��
    function_type_construct_data = construct_data;
  }
  // ���������������Ͳ����ú�������
  auto result = function_type_construct_data
                    ->AttachSingleNodeToTailNodeEmplace<FunctionType>(nullptr);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(
        result, function_type_construct_data->GetObjectName());
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}

std::any FunctionRelaventBasePartFunctionInitExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  std::shared_ptr<ObjectConstructData>& construct_data =
      std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  // ���ù������ݣ��ڹ�����������ʱʹ��
  function_type_construct_data = construct_data;
  // ���������������Ͳ����ú�������
  auto result = function_type_construct_data
                    ->AttachSingleNodeToTailNodeEmplace<FunctionType>(nullptr);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(
        result, function_type_construct_data->GetObjectName());
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}

std::any FunctionRelaventBasePartPointer(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  ConstTag const_tag = GetConstTag(word_data[0]);
  std::shared_ptr<ObjectConstructData>& construct_data =
      std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  // ���һ��ָ��ڵ�
  auto result = construct_data->AttachSingleNodeToTailNodeEmplace<PointerType>(
      const_tag, 0);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result, construct_data->GetObjectName());
  }
  return std::move(word_data[2].GetNonTerminalWordData().user_returned_data);
}

std::shared_ptr<FlowInterface> FunctionRelavent(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto [final_type, const_tag_before_final_type] =
      std::any_cast<std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  std::shared_ptr<ObjectConstructData>& construct_data =
      std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[1].GetNonTerminalWordData().user_returned_data);
  auto [flow_control_node, construct_result] = construct_data->ConstructObject(
      const_tag_before_final_type, std::move(final_type));
  // ���ڵ㹹���Ƿ�ɹ�
  if (construct_result != ObjectConstructData::CheckResult::kSuccess)
      [[unlikely]] {
    VarietyOrFunctionConstructError(construct_result,
                                    construct_data->GetObjectName());
  }
  return std::move(flow_control_node);
}

std::shared_ptr<FlowInterface> SingleAnnounceNoAssignVariety(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  if (!word_data[1].GetNonTerminalWordData().user_returned_data.has_value())
      [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �﷨����", GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  std::shared_ptr<ObjectConstructData>& construct_data =
      std::any_cast<std::shared_ptr<ObjectConstructData>&>(
          word_data[1].GetNonTerminalWordData().user_returned_data);
  if (construct_data->GetObjectName().empty()) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �����ı�����������", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto [final_type, const_tag_before_final_type] =
      std::any_cast<std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  auto [flow_control_node, construct_result] = construct_data->ConstructObject(
      const_tag_before_final_type, std::move(final_type));
  // ����Ƿ񹹽��ɹ�
  if (construct_result != ObjectConstructData::CheckResult::kSuccess) {
    VarietyOrFunctionConstructError(construct_result,
                                    construct_data->GetObjectName());
  }
  return std::move(flow_control_node);
}

std::any SingleAnnounceNoAssignFunctionRelavent(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}

std::any TypeDef(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& announced_object = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  switch (announced_object->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      // ���ܸ����������
      std::cerr << std::format("����{:} �޷������������", GetLine())
                << std::endl;
      exit(-1);
      break;
    default:
      break;
  }
  auto variety_pointer = static_pointer_cast<VarietyOperatorNode>(
      static_cast<SimpleSentence&>(*announced_object)
          .GetSentenceOperateNodePointer());
  auto type_pointer = variety_pointer->GetVarietyTypePointer();
  const std::string* type_name = variety_pointer->GetVarietyNamePointer();
  // ע��������
  auto iter = parser_frontend.AnnounceTypeName(*type_name);
  auto [ignore_iter, result] =
      parser_frontend.DefineType(*type_name, type_pointer);
  if (result == AddTypeResult::kTypeAlreadyIn) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} "
                     "ʹ��typedef�������ʱʹ�õ�����{:}�Ѵ�����ͬ���͵Ķ���",
                     GetLine(), GetColumn(), *type_name)
              << std::endl;
    exit(-1);
  }
  // ��������յ����ͽڵ�
  parser_frontend.RemoveVarietySystemEmptyNode(*type_name);
  return std::any();
}

std::any NotEmptyFunctionPointerArgumentsBase(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::shared_ptr<FlowInterface>& flow_control_node =
      std::any_cast<std::shared_ptr<FlowInterface>&>(
          word_data.front().GetNonTerminalWordData().user_returned_data);
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      // ֻ�б���������Ϊ��������
      break;
    case FlowType::kFunctionDefine:
      // ����������Ϊ��������ʱ�Ĳ���
      std::cerr
          << std::format(
                 "����{:} ����������Ϊ����ʱ��������,"
                 "��ʹ�ú���ָ��\n����ϳ����ں���{:}��",
                 GetLine(),
                 static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
                     *flow_control_node)
                     .GetFunctionTypeReference()
                     .GetFunctionName())
          << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
  auto variety_pointer = std::static_pointer_cast<VarietyOperatorNode>(
      static_cast<SimpleSentence&>(*flow_control_node)
          .GetSentenceOperateNodePointer());
  // ��Ӻ�������
  function_type_construct_data->AddFunctionTypeArgument(variety_pointer);
  return std::any();
}

std::any NotEmptyFunctionPointerArgumentsExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& flow_control_node = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      // ֻ�б���������Ϊ��������
      break;
    case FlowType::kFunctionDefine:
      // ����������Ϊ�����Ĳ���
      std::cerr
          << std::format(
                 "����{:} ����������Ϊ����ʱ��������,"
                 "��ʹ�ú���ָ��\n����ϳ����ں���{:}��",
                 GetLine(),
                 static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
                     *flow_control_node)
                     .GetFunctionTypeReference()
                     .GetFunctionName())
          << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
  auto variety_pointer = std::static_pointer_cast<VarietyOperatorNode>(
      static_cast<SimpleSentence&>(*flow_control_node)
          .GetSentenceOperateNodePointer());
  // ��Ӻ�������
  function_type_construct_data->AddFunctionTypeArgument(variety_pointer);
  return std::any();
}

std::any FunctionRelaventArguments(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(
      word_data.front().GetNonTerminalWordData().user_returned_data);
}

std::shared_ptr<c_parser_frontend::flow_control::FunctionDefine>
FunctionDefineHead(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& function_head = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  // ����Ƿ�Ϊ����ͷ
  switch (function_head->GetFlowType()) {
    case FlowType::kFunctionDefine:
      break;
    case FlowType::kSimpleSentence:
      std::cerr << std::format("����{:} ���������﷨����", GetLine())
                << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
  std::shared_ptr<FunctionType> function_type =
      static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
          *function_head)
          .GetFunctionTypePointer();
  // ���õ�ǰ����������
  parser_frontend.SetFunctionToConstruct(
      std::shared_ptr<FunctionType>(function_type));
  return std::static_pointer_cast<
      c_parser_frontend::flow_control::FunctionDefine>(function_head);
}

std::any FunctionDefine(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // ��Ӻ���������Ѿ���Sentences�������������
  // ֻ������������
  parser_frontend.PopActionScope();
  return std::any();
}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
SingleStructureBodyBase(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& flow_control_node = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  // ����Ƿ�Ϊ��������
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      std::cerr << std::format("����{:} ��������������Ϊ�ṹ���ݳ�Ա",
                               GetLine())
                << std::endl;
      exit(-1);
      break;
    default:
      break;
  }
  std::shared_ptr<VarietyOperatorNode> variety_pointer =
      std::static_pointer_cast<VarietyOperatorNode>(
          static_cast<SimpleSentence&>(*flow_control_node)
              .GetSentenceOperateNodePointer());
  // ��չ����ʱʹ�õ����ͣ��������ȥ����һ��ָ�룩
  std::shared_ptr<const TypeInterface> extend_type =
      GetExtendAnnounceType(variety_pointer->GetVarietyTypePointer());
  // ��ȡ��չ����ʱ�ı���ConstTag
  ConstTag extend_const_tag = variety_pointer->GetConstTag();
  // ��ṹ���ݽڵ�����ӳ�Ա
  auto structure_member_index =
      structure_type_constructuring->AddStructureMember(
          *variety_pointer->GetVarietyNamePointer(),
          variety_pointer->GetVarietyTypePointer(),
          variety_pointer->GetConstTag());
  assert(structure_member_index.IsValid());
  return std::make_pair(std::move(extend_type), extend_const_tag);
}

std::any SingleStructureBodyExtend(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [extend_type, extend_const_tag] =
      std::any_cast<std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  std::string& new_member_name = word_data[2].GetTerminalWordData().word;
  auto structure_member_index =
      structure_type_constructuring->AddStructureMember(
          std::move(new_member_name), extend_type, extend_const_tag);
  // ������ӳ�Ա���Ƿ��ظ�
  if (!structure_member_index.IsValid()) {
    std::cerr << std::format("����{:} �ض����Ա{:}", GetLine(),
                             new_member_name)
              << std::endl;
    exit(-1);
  }
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}

std::shared_ptr<ListInitializeOperatorNode> InitializeList(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto initialize_list = std::make_shared<ListInitializeOperatorNode>();
  // ��ȡ��ʼ���б����
  auto& list_argument_pointer = std::any_cast<std::shared_ptr<
      std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  // ��ÿһ��������ӵ�������
  for (auto& single_argument_pointer : *list_argument_pointer) {
    // ����Ƿ�Ϊ��Ч�ĳ�ʼ������
    bool result =
        initialize_list->AddListValue(std::move(single_argument_pointer));
    assert(result);
  }
  return std::move(initialize_list);
}

std::any SingleInitializeListArgumentList(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::move(word_data.front());
}

std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>
InitializeListArgumentsBase(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& init_data_pointer =
      std::any_cast<std::shared_ptr<InitializeOperatorNodeInterface>&>(
          word_data.front().GetNonTerminalWordData().user_returned_data);
  auto list_pointer = std::make_shared<
      std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>();
  list_pointer->emplace_back(std::move(init_data_pointer));
  return std::move(list_pointer);
}

std::any InitializeListArgumentsExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& list_pointer = std::any_cast<std::shared_ptr<
      std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  auto& init_data_pointer =
      std::any_cast<std::shared_ptr<InitializeOperatorNodeInterface>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  list_pointer->emplace_back(std::move(init_data_pointer));
  return std::move(word_data[2].GetNonTerminalWordData().user_returned_data);
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AnnounceAssignableInitializeList(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  return std::make_pair(
      std::move(
          std::any_cast<std::shared_ptr<InitializeOperatorNodeInterface>&>(
              word_data.front().GetNonTerminalWordData().user_returned_data)),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarietyAnnounceNoAssign(std::shared_ptr<VarietyOperatorNode>&& variety_node) {
  // ��ȡԭʼ����ʹ�õ�����
  auto raw_variety_type = variety_node->GetVarietyTypePointer();
  // ��ȡԭʼ������ConstTag
  ConstTag extend_const_tag = variety_node->GetConstTag();
  // �洢��ȡ�������̲���������
  auto flow_control_node_container =
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>();
  // ��������ӿռ����ڵ�
  auto allocate_space_node = std::make_shared<AllocateOperatorNode>();
  bool result = allocate_space_node->SetTargetVariety(
      std::shared_ptr<VarietyOperatorNode>(variety_node));
  assert(result);
  auto allocate_flow_control_node = std::make_unique<SimpleSentence>();
  result = allocate_flow_control_node->SetSentenceOperateNode(
      std::move(allocate_space_node));
  assert(result);
  flow_control_node_container->emplace_back(
      std::move(allocate_flow_control_node));
  // �������
  parser_frontend.DefineVariety(*variety_node->GetVarietyNamePointer(),
                                std::move(variety_node));
  // ��ȡ��չ����ʱ������
  if (raw_variety_type->GetType() == StructOrBasicType::kPointer) {
    // �����ָ����������ԭʼ�������ͻ�����ȥ��һ��ָ��
    return std::make_tuple(raw_variety_type->GetNextNodePointer(),
                           extend_const_tag,
                           std::move(flow_control_node_container));
  } else {
    // ����ָ�����ͣ�ֱ�ӷ���
    return std::make_tuple(raw_variety_type, extend_const_tag,
                           std::move(flow_control_node_container));
  }
}

std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarieytAnnounceWithAssign(
    std::shared_ptr<VarietyOperatorNode>&& variety_node,
    std::shared_ptr<OperatorNodeInterface>&& node_for_assign) {
  // ��ȡԭʼ����ʹ�õ�����
  auto raw_variety_type = variety_node->GetVarietyTypePointer();
  // ��ȡԭʼ������ConstTag
  ConstTag extend_const_tag = variety_node->GetConstTag();
  // �洢��ȡ�������̲���������
  auto flow_control_node_container =
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>();
  // ��������ӿռ����ڵ�
  auto allocate_node = std::make_shared<AllocateOperatorNode>();
  // ����Ҫ����ռ�Ľڵ�
  bool result = allocate_node->SetTargetVariety(variety_node);
  assert(result);
  // ������ռ�Ľڵ��װ�����̽ڵ�
  auto allocate_flow_control_node = std::make_unique<SimpleSentence>();
  result = allocate_flow_control_node->SetSentenceOperateNode(allocate_node);
  assert(result);
  // ����������
  flow_control_node_container->emplace_back(
      std::move(allocate_flow_control_node));
  // ��������Ӹ�ֵ�ڵ�
  auto assign_node = std::make_shared<AssignOperatorNode>();
  // ���ñ���ֵ�Ľڵ�
  assign_node->SetNodeToBeAssigned(std::move(variety_node));
  // ����������ֵ�Ľڵ�
  // ��ǰ��������״̬
  auto assignable_check_result =
      assign_node->SetNodeForAssign(node_for_assign, true);
  CheckAssignableCheckResult(assignable_check_result);
  // ����ֵ�ڵ��װ�����̽ڵ�
  auto assign_flow_control_node = std::make_unique<SimpleSentence>();
  result =
      assign_flow_control_node->SetSentenceOperateNode(std::move(assign_node));
  assert(result);
  // ����������
  flow_control_node_container->emplace_back(
      std::move(assign_flow_control_node));
  // �������
  parser_frontend.DefineVariety(*variety_node->GetVarietyNamePointer(),
                                std::move(variety_node));
  // ��ȡ��չ����ʱ������
  if (raw_variety_type->GetType() == StructOrBasicType::kPointer) {
    // �����ָ����������ԭʼ�������ͻ�����ȥ��һ��ָ��
    return std::make_tuple(raw_variety_type->GetNextNodePointer(),
                           extend_const_tag,
                           std::move(flow_control_node_container));
  } else {
    // ����ָ�����ͣ�ֱ�ӷ���
    return std::make_tuple(raw_variety_type, extend_const_tag,
                           std::move(flow_control_node_container));
  }
}

std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignNoAssignBase(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& flow_control_node = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      std::cerr << std::format("����{:} ��֧�ֵ���չ����������������",
                               GetLine())
                << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
  return VarietyAnnounceNoAssign(std::static_pointer_cast<VarietyOperatorNode>(
      static_cast<SimpleSentence&>(*flow_control_node)
          .GetSentenceOperateNodePointer()));
}

std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
SingleAnnounceAndAssignWithAssignBase(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& flow_control_node = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  // ����Ƿ�Ϊ��������
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      std::cerr << std::format("����{:} ��֧�ֵ���չ����������������",
                               GetLine())
                << std::endl;
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
  return VarieytAnnounceWithAssign(
      std::static_pointer_cast<VarietyOperatorNode>(
          static_cast<SimpleSentence&>(*flow_control_node)
              .GetSentenceOperateNodePointer()),
      std::any_cast<std::shared_ptr<OperatorNodeInterface>>(
          word_data[2].GetNonTerminalWordData().user_returned_data));
}

std::any SingleAnnounceAndAssignNoAssignExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [extend_announce_type, extend_const_tag, flow_control_node_container] =
      std::any_cast<std::tuple<
          std::shared_ptr<const TypeInterface>, ConstTag,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  std::string& variety_name = word_data[2].GetTerminalWordData().word;
  // ���������ڵ�
  // ����������
  auto iter = parser_frontend.AnnounceVarietyName(variety_name);
  auto variety_node = std::make_shared<VarietyOperatorNode>(
      &iter->first, extend_const_tag, LeftRightValueTag::kLeftValue);
  variety_node->SetVarietyType(
      std::shared_ptr<const TypeInterface>(extend_announce_type));
  auto [ignore_type, ignore_const_tag, sub_flow_control_node_container] =
      VarietyAnnounceNoAssign(std::move(variety_node));
  // ����ȡ�����Ĳ����ϲ�����������
  flow_control_node_container->merge(
      std::move(*sub_flow_control_node_container));
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}

std::any SingleAnnounceAndAssignWithAssignExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 5);
  auto& [extend_announce_type, extend_const_tag, flow_control_node_container] =
      std::any_cast<std::tuple<
          std::shared_ptr<const TypeInterface>, ConstTag,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  auto& node_for_assign =
      std::any_cast<std::shared_ptr<OperatorNodeInterface>&>(
          word_data[4].GetNonTerminalWordData().user_returned_data);
  std::string& variety_name = word_data[2].GetTerminalWordData().word;
  // ���������ڵ�
  // ����������
  auto iter = parser_frontend.AnnounceVarietyName(variety_name);
  auto variety_node = std::make_shared<VarietyOperatorNode>(
      &iter->first, extend_const_tag, LeftRightValueTag::kLeftValue);
  variety_node->SetVarietyType(
      std::shared_ptr<const TypeInterface>(extend_announce_type));
  auto [ignore_type, ignore_const_tag, sub_flow_control_node_container] =
      VarieytAnnounceWithAssign(std::move(variety_node),
                                std::move(node_for_assign));
  // ����ȡ�����Ĳ����ϲ�����������
  flow_control_node_container->merge(
      std::move(*sub_flow_control_node_container));
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag> TypeFunctionRelavent(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& flow_control_node = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  // ��������Ƿ�Ϊ��������
  if (flow_control_node->GetFlowType() == FlowType::kFunctionDefine)
      [[unlikely]] {
    // ����������Ϊ����ʹ��
    std::cerr << std::format("����{:} ����������Ϊ����", GetLine())
              << std::endl;
    exit(-1);
  }
  auto operator_node = std::static_pointer_cast<VarietyOperatorNode>(
      static_cast<SimpleSentence&>(*flow_control_node)
          .GetSentenceOperateNodePointer());
  // ����Ƿ�Ϊ����ָ������
  if (operator_node->GetVarietyNamePointer() != nullptr) [[unlikely]] {
    std::cerr << std::format("����{:} �˴�����ʹ�ñ�������", GetLine())
              << std::endl;
    exit(-1);
  }
  return std::make_pair(operator_node->GetVarietyTypePointer(),
                        operator_node->GetConstTag());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableConstexprValue(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  // �����洢��ȡ����/�����Ĳ���������
  return std::make_pair(
      std::move(
          std::any_cast<std::shared_ptr<InitializeOperatorNodeInterface>&>(
              word_data.front().GetNonTerminalWordData().user_returned_data)),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableId(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  std::string& variety_name = word_data.front().GetTerminalWordData().word;
  auto [variety_operator_node, found] =
      parser_frontend.GetVariety(variety_name);
  if (!found) [[unlikely]] {
    // δ�ҵ��������Ƶı���
    std::cerr << std::format("����{:} �Ҳ�������{:}", GetLine(), variety_name)
              << std::endl;
    exit(-1);
  }
  return std::make_pair(
      std::move(variety_operator_node),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableTypeConvert(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  auto [new_type, new_const_tag] =
      std::any_cast<std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&>(
          word_data[1].GetNonTerminalWordData().user_returned_data);
  auto& [old_operator_node, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[3].GetNonTerminalWordData().user_returned_data);
  // ת���ڵ�
  auto convert_operator_node =
      std::make_shared<TypeConvert>(old_operator_node, new_type);
  // ��������ת���õ��Ľڵ�
  auto converted_operator_node =
      convert_operator_node->GetDestinationNodePointer();
  // �����µ�const����
  if (converted_operator_node->GetGeneralOperatorType() ==
      GeneralOperationType::kVariety) [[likely]] {
    // ��������ΪconstҲ����Ϊ��const
    std::static_pointer_cast<VarietyOperatorNode>(converted_operator_node)
        ->SetConstTag(new_const_tag);
  } else if (new_const_tag != ConstTag::kConst) [[unlikely]] {
    // ��������Ķ�����ֵ�������޸�const����Ϊ�ɱ�
    std::cerr << std::format("����{:} �޷�ת��Ϊ��const", GetLine())
              << std::endl;
    exit(-1);
  }
  // ���ת�����̿��ƽڵ�
  auto flow_control_node = std::make_unique<SimpleSentence>();
  bool result =
      flow_control_node->SetSentenceOperateNode(convert_operator_node);
  assert(result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ���ؾ���ת���Ľڵ������ת���ڵ�
  return std::make_pair(std::move(converted_operator_node),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfType(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  auto [type_pointer, variety_const_tag] =
      std::any_cast<std::pair<std::shared_ptr<TypeInterface>, ConstTag>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  size_t type_size = type_pointer->TypeSizeOf();
  auto return_constexpr_value =
      std::make_shared<BasicTypeInitializeOperatorNode>(
          InitializeType::kBasic, std::to_string(type_size));
  BuiltInType return_constexpr_value_type =
      c_parser_frontend::type_system::CalculateBuiltInType(
          return_constexpr_value->GetValue());
  assert(return_constexpr_value_type < BuiltInType::kFloat32);
  bool result = return_constexpr_value->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicTypeNotTemplate(
          return_constexpr_value_type, SignTag::kSigned));
  assert(result);
  // ���ؿ��������̳е������Զ��ͷ�
  return std::make_pair(
      std::move(return_constexpr_value),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfAssignable(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  auto& [assignable_node, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  size_t type_size = assignable_node->GetResultTypePointer()->TypeSizeOf();
  auto return_constexpr_value =
      std::make_shared<BasicTypeInitializeOperatorNode>(
          InitializeType::kBasic, std::to_string(type_size));
  BuiltInType return_constexpr_value_type =
      c_parser_frontend::type_system::CalculateBuiltInType(
          return_constexpr_value->GetValue());
  assert(return_constexpr_value_type < BuiltInType::kFloat32);
  bool result = return_constexpr_value->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicTypeNotTemplate(
          return_constexpr_value_type, SignTag::kSigned));
  assert(result);
  // ���ؿ��������̳е������Զ��ͷ�
  return std::make_pair(
      std::move(return_constexpr_value),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMemberAccess(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [assignable, sentences_to_get_assignable] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  auto member_access_node = std::make_shared<MemberAccessOperatorNode>();
  bool result = member_access_node->SetNodeToAccess(assignable);
  if (!result) [[unlikely]] {
    std::cout << std::format(
                     "����{:} ����{:} �޷��Էǽṹ���ݻ�ö�����ͷ��ʳ�Ա",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  result = member_access_node->SetMemberName(
      std::move(word_data[2].GetTerminalWordData().word));
  if (!result) [[unlikely]] {
    std::cout << std::format("����{:} ����{:} �����ṹ�岻���ڳ�Ա{:}",
                             GetLine(), GetColumn(),
                             word_data[2].GetTerminalWordData().word)
              << std::endl;
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  result = flow_control_node->SetSentenceOperateNode(member_access_node);
  assert(result);
  sentences_to_get_assignable->emplace_back(std::move(flow_control_node));
  return std::make_pair(member_access_node->GetResultOperatorNode(),
                        std::move(sentences_to_get_assignable));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePointerMemberAccess(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [assignable, sentences_to_get_assignable] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  // ��ָ������ú���ʳ�Ա
  auto dereference_node = std::make_shared<DereferenceOperatorNode>();
  bool result = dereference_node->SetNodeToDereference(assignable);
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} ���ܶԷ�ָ�����ʹ��\"->\"�����",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  auto dereferenced_node = dereference_node->GetDereferencedNodePointer();
  auto member_access_node = std::make_shared<MemberAccessOperatorNode>();
  result = member_access_node->SetNodeToAccess(dereferenced_node);
  if (!result) [[unlikely]] {
    std::cout << std::format(
                     "����{:} ����{:} �޷��Էǽṹ���ݻ�ö�����ͷ��ʳ�Ա",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  result = member_access_node->SetMemberName(
      std::move(word_data[2].GetTerminalWordData().word));
  if (!result) [[unlikely]] {
    std::cout << std::format("����{:} ����{:} �����ṹ���ݲ����ڳ�Ա{:}",
                             GetLine(), GetColumn(),
                             word_data[2].GetTerminalWordData().word)
              << std::endl;
    exit(-1);
  }
  // ��ӽ����ýڵ�ͳ�Ա���ʽڵ�
  auto flow_control_node = std::make_unique<SimpleSentence>();
  result = flow_control_node->SetSentenceOperateNode(dereference_node);
  assert(result);
  sentences_to_get_assignable->emplace_back(std::move(flow_control_node));
  flow_control_node = std::make_unique<SimpleSentence>();
  result = flow_control_node->SetSentenceOperateNode(member_access_node);
  assert(result);
  sentences_to_get_assignable->emplace_back(std::move(flow_control_node));
  return std::make_pair(member_access_node->GetResultOperatorNode(),
                        std::move(sentences_to_get_assignable));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalOperate(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [left_operator_node, left_flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  auto mathematical_operation = std::any_cast<MathematicalOperation>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto& [right_operator_node, right_flow_control_node_container] =
      std::any_cast<std::pair<
          std::shared_ptr<OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  auto mathematical_operator_node =
      std::make_shared<MathematicalOperatorNode>(mathematical_operation);
  bool left_operator_node_check_result =
      mathematical_operator_node->SetLeftOperatorNode(left_operator_node);
  if (!left_operator_node_check_result) [[unlikely]] {
    std::cerr << std::format("����{:}������{:} ���������޷���������", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  DeclineMathematicalComputeTypeResult right_operator_node_check_result =
      mathematical_operator_node->SetRightOperatorNode(right_operator_node);
  CheckMathematicalComputeTypeResult(right_operator_node_check_result);
  // ������̿��ƽڵ�
  auto flow_control_node = std::make_unique<SimpleSentence>();
  bool result =
      flow_control_node->SetSentenceOperateNode(mathematical_operator_node);
  assert(result);
  // ������������ڵ�Ĳ����ϲ�����������
  left_flow_control_node_container->merge(
      std::move(*right_flow_control_node_container));
  // �����ѧ����ڵ�
  left_flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(
      mathematical_operator_node->GetComputeResultNodePointer(),
      std::move(left_flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalAndAssignOperate(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [left_operator_node, left_flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  auto mathematical_and_assign_operation =
      std::any_cast<MathematicalAndAssignOperation>(
          word_data[1].GetNonTerminalWordData().user_returned_data);
  auto& [right_operator_node, right_flow_control_node_container] =
      std::any_cast<std::pair<
          std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  // ��ȡ��ѧ�����
  MathematicalOperation mathematical_operation = c_parser_frontend::
      operator_node::MathematicalAndAssignOperationToMathematicalOperation(
          mathematical_and_assign_operation);
  // ��������ڵ�
  auto mathematical_operator_node =
      std::make_shared<MathematicalOperatorNode>(mathematical_operation);
  bool left_operator_node_check_result =
      mathematical_operator_node->SetLeftOperatorNode(left_operator_node);
  if (!left_operator_node_check_result) [[unlikely]] {
    std::cerr << std::format("����{:}������{:} ���������޷���������", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  DeclineMathematicalComputeTypeResult right_operator_node_check_result =
      mathematical_operator_node->SetRightOperatorNode(right_operator_node);
  CheckMathematicalComputeTypeResult(right_operator_node_check_result);
  // ���ɸ�ֵ�ڵ�
  auto assign_operator_node = std::make_shared<AssignOperatorNode>();
  assign_operator_node->SetNodeToBeAssigned(left_operator_node);
  AssignableCheckResult assignable_check_result =
      assign_operator_node->SetNodeForAssign(right_operator_node, false);
  CheckAssignableCheckResult(assignable_check_result);
  // ������������ڵ�Ĳ����ϲ�����������
  left_flow_control_node_container->merge(
      std::move(*right_flow_control_node_container));
  // ����������̿��ƽڵ�͸�ֵ���̿��ƽڵ�
  auto flow_control_node_mathematical = std::make_unique<SimpleSentence>();
  bool result = flow_control_node_mathematical->SetSentenceOperateNode(
      mathematical_operator_node);
  assert(result);
  left_flow_control_node_container->emplace_back(
      std::move(flow_control_node_mathematical));
  auto flow_control_node_assign = std::make_unique<SimpleSentence>();
  result =
      flow_control_node_assign->SetSentenceOperateNode(assign_operator_node);
  assert(result);
  left_flow_control_node_container->emplace_back(
      std::move(flow_control_node_assign));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(assign_operator_node->GetResultOperatorNode(),
                        std::move(left_flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableLogicalOperate(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [left_operator_node, left_flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  auto logical_operation = std::any_cast<LogicalOperation>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto& [right_operator_node, right_flow_control_node_container] =
      std::any_cast<std::pair<
          std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  // �����߼�����ڵ�
  auto logical_operator_node =
      std::make_shared<LogicalOperationOperatorNode>(logical_operation);
  bool check_result =
      logical_operator_node->SetLeftOperatorNode(left_operator_node);
  if (!check_result) [[unlikely]] {
    std::cerr << std::format("����{:}������{:} ��������޷���Ϊ�߼�����ڵ�",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  check_result =
      logical_operator_node->SetRightOperatorNode(right_operator_node);
  if (!check_result) [[unlikely]] {
    std::cerr << std::format("����{:}������{:} �Ҳ������޷���Ϊ�߼�����ڵ�",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  // ������������ڵ�Ĳ����ϲ�����������
  left_flow_control_node_container->merge(
      std::move(*right_flow_control_node_container));
  // ����������̽ڵ�
  auto flow_control_node = std::make_unique<SimpleSentence>();
  check_result =
      flow_control_node->SetSentenceOperateNode(logical_operator_node);
  assert(check_result);
  left_flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(logical_operator_node->GetResultOperatorNode(),
                        std::move(left_flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableNot(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [sub_assignable_node, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto not_operator_node =
      std::make_shared<MathematicalOperatorNode>(MathematicalOperation::kNot);
  bool check_result =
      not_operator_node->SetLeftOperatorNode(sub_assignable_node);
  if (!check_result) [[unlikely]] {
    std::cerr << std::format("����{:}������{:} �޷������߼�������", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  check_result = flow_control_node->SetSentenceOperateNode(not_operator_node);
  assert(check_result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(not_operator_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableLogicalNegative(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [sub_assignable_node, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto not_operator_node = std::make_shared<MathematicalOperatorNode>(
      MathematicalOperation::kLogicalNegative);
  bool check_result =
      not_operator_node->SetLeftOperatorNode(sub_assignable_node);
  if (!check_result) [[unlikely]] {
    std::cerr << std::format("����{:}������{:} �޷����а�λȡ������", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  check_result = flow_control_node->SetSentenceOperateNode(not_operator_node);
  assert(check_result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(not_operator_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalNegative(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [sub_assignable_node, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto not_operator_node = std::make_shared<MathematicalOperatorNode>(
      MathematicalOperation::kMathematicalNegative);
  bool check_result =
      not_operator_node->SetLeftOperatorNode(sub_assignable_node);
  if (!check_result) [[unlikely]] {
    std::cerr << std::format("����{:}������{:} �޷�����ȡ������", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  check_result = flow_control_node->SetSentenceOperateNode(not_operator_node);
  assert(check_result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(not_operator_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableObtainAddress(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [sub_assignable_node, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto node_to_obtain_address =
      std::static_pointer_cast<const VarietyOperatorNode>(sub_assignable_node);
  // ��鱻ȡ��ַ�Ľڵ�
  if (node_to_obtain_address->GetGeneralOperatorType() !=
      GeneralOperationType::kVariety) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �޷��ԷǱ�������ȡ��ַ",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  } else if (node_to_obtain_address->GetLeftRightValueTag() !=
             LeftRightValueTag::kLeftValue) [[unlikely]] {
    // ��ȡ��ַ�Ľڵ�����ֵ
    std::cerr << std::format("����{:} ����{:} �޷�����ֵȡ��ַ", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto obtain_address_node = std::make_shared<ObtainAddressOperatorNode>();
  bool result =
      obtain_address_node->SetNodeToObtainAddress(node_to_obtain_address);
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �޷�ȡ��ַ", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  result = flow_control_node->SetSentenceOperateNode(obtain_address_node);
  assert(result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(obtain_address_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableDereference(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [sub_assignable_node, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto dereference_node = std::make_shared<DereferenceOperatorNode>();
  bool result = dereference_node->SetNodeToDereference(sub_assignable_node);
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �޷�������", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  result = flow_control_node->SetSentenceOperateNode(dereference_node);
  assert(result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(dereference_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableArrayAccess(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  auto& [node_to_dereference, main_flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  auto& [index_node, sub_flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  // ������ָ���ַ��ƫ������ӵĽڵ�
  auto plus_operator_node =
      std::make_shared<MathematicalOperatorNode>(MathematicalOperation::kPlus);
  bool left_node_check_result =
      plus_operator_node->SetLeftOperatorNode(node_to_dereference);
  if (!left_node_check_result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �޷����Ѱַ", GetLine(),
                             GetColumn())
              << std::endl;
    exit(-1);
  }
  auto right_node_check_result =
      plus_operator_node->SetRightOperatorNode(index_node);
  CheckMathematicalComputeTypeResult(right_node_check_result);
  // ���������յ�ַ�����õĽڵ�
  auto dereference_operator_node = std::make_shared<DereferenceOperatorNode>();
  bool dereference_result = dereference_operator_node->SetNodeToDereference(
      plus_operator_node->GetResultOperatorNode());
  if (!dereference_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} �޷��������飬����ʹ���˸������±�",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  // �ϲ���ȡindex�Ĳ�������������
  main_flow_control_node_container->merge(
      std::move(*sub_flow_control_node_container));
  // �������̽ڵ�
  auto plus_flow_control_node = std::make_unique<SimpleSentence>();
  bool result =
      plus_flow_control_node->SetSentenceOperateNode(plus_operator_node);
  assert(result);
  main_flow_control_node_container->emplace_back(
      std::move(plus_flow_control_node));
  auto dereference_flow_control_node = std::make_unique<SimpleSentence>();
  result = dereference_flow_control_node->SetSentenceOperateNode(
      dereference_operator_node);
  assert(result);
  main_flow_control_node_container->emplace_back(
      std::move(dereference_flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(dereference_operator_node->GetResultOperatorNode(),
                        std::move(main_flow_control_node_container));
}

std::shared_ptr<const OperatorNodeInterface> PrefixPlusOrMinus(
    MathematicalOperation mathematical_operation,
    const std::shared_ptr<const OperatorNodeInterface>& node_to_operate,
    std::list<std::unique_ptr<FlowInterface>>* flow_control_node_container) {
  assert(mathematical_operation == MathematicalOperation::kPlus ||
         mathematical_operation == MathematicalOperation::kMinus);
  // ��������ڵ�
  auto mathematical_operator_node =
      std::make_shared<MathematicalOperatorNode>(mathematical_operation);
  // ����++/--����ʹ�õ���ֵ1
  auto constexpr_num_for_operate =
      std::make_shared<BasicTypeInitializeOperatorNode>(InitializeType::kBasic,
                                                        "1");
  constexpr_num_for_operate->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt1,
                                              SignTag::kSigned>());
  bool left_operator_node_check_result =
      mathematical_operator_node->SetLeftOperatorNode(node_to_operate);
  if (!left_operator_node_check_result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �޷��Ը�����ʹ��++�����",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  auto right_operator_node_check_result =
      mathematical_operator_node->SetRightOperatorNode(
          constexpr_num_for_operate);
  CheckMathematicalComputeTypeResult(right_operator_node_check_result);
  // ���ɸ�ֵ�ڵ�
  auto assign_operator_node = std::make_shared<AssignOperatorNode>();
  assign_operator_node->SetNodeToBeAssigned(node_to_operate);
  auto assignable_check_result = assign_operator_node->SetNodeForAssign(
      mathematical_operator_node->GetResultOperatorNode(), false);
  CheckAssignableCheckResult(assignable_check_result);
  // �����������̿��ƽڵ�
  auto operate_flow_control_node = std::make_unique<SimpleSentence>();
  bool result = operate_flow_control_node->SetSentenceOperateNode(
      mathematical_operator_node);
  assert(result);
  flow_control_node_container->emplace_back(
      std::move(operate_flow_control_node));
  // ���ɸ�ֵ���̿��ƽڵ�
  auto assign_flow_control_node = std::make_unique<SimpleSentence>();
  result =
      assign_flow_control_node->SetSentenceOperateNode(assign_operator_node);
  assert(result);
  flow_control_node_container->emplace_back(
      std::move(assign_flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return mathematical_operator_node->GetResultOperatorNode();
}

std::shared_ptr<const OperatorNodeInterface> SuffixPlusOrMinus(
    MathematicalOperation mathematical_operation,
    const std::shared_ptr<const OperatorNodeInterface>& node_to_operate,
    std::list<std::unique_ptr<FlowInterface>>* flow_control_node_container) {
  // �����м�����ڵ�͸�ֵ�ڵ�
  // ����һ�ݱ�����ԭ����+1/-1���ȼ���ִ��ǰ׺++/--��
  auto temp_variety_node = std::make_shared<VarietyOperatorNode>(
      nullptr, ConstTag::kNonConst, LeftRightValueTag::kRightValue);
  bool set_temp_variety_type_result = temp_variety_node->SetVarietyType(
      node_to_operate->GetResultTypePointer());
  assert(set_temp_variety_type_result);
  // �������ƽڵ㣬����Դ�ڵ㵽��ʱ����
  auto copy_assign_operator_node = std::make_shared<AssignOperatorNode>();
  copy_assign_operator_node->SetNodeToBeAssigned(temp_variety_node);
  auto copy_assign_node_check_result =
      copy_assign_operator_node->SetNodeForAssign(node_to_operate, false);
  CheckAssignableCheckResult(copy_assign_node_check_result);
  auto copy_flow_control_node = std::make_unique<SimpleSentence>();
  bool result =
      copy_flow_control_node->SetSentenceOperateNode(copy_assign_operator_node);
  assert(result);
  flow_control_node_container->emplace_back(std::move(copy_flow_control_node));
  // ��Դ�ڵ�ִ��ǰ׺++/--����
  PrefixPlusOrMinus(mathematical_operation, node_to_operate,
                    flow_control_node_container);
  // ���ظ��Ƶõ��Ľڵ�
  return copy_assign_operator_node->GetResultOperatorNode();
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixPlus(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [node_to_plus, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  return std::make_pair(
      PrefixPlusOrMinus(MathematicalOperation::kPlus, node_to_plus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixMinus(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [node_to_minus, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  return std::make_pair(
      PrefixPlusOrMinus(MathematicalOperation::kMinus, node_to_minus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixPlus(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [node_to_plus, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  return std::make_pair(
      SuffixPlusOrMinus(MathematicalOperation::kPlus, node_to_plus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixMinus(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [node_to_plus, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  return std::make_pair(
      SuffixPlusOrMinus(MathematicalOperation::kMinus, node_to_plus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>> ReturnWithValue(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [return_target, sentences_to_get_assignable] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  auto return_flow_control_node = std::make_unique<Return>();
  auto active_function = parser_frontend.GetActiveFunctionPointer();
  if (active_function == nullptr) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} ��ǰ�����ں����ڣ��޷�����",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  bool result =
      return_flow_control_node->SetReturnTarget(active_function, return_target);
  if (!result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} "
                     "�޷����ظ����͵�ֵ�����������޷�ת��Ϊ����{:}���ص�����",
                     GetLine(), GetColumn(), active_function->GetFunctionName())
              << std::endl;
    exit(-1);
  }
  sentences_to_get_assignable->emplace_back(
      std::move(return_flow_control_node));
  return std::move(sentences_to_get_assignable);
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>> ReturnWithoutValue(
    std::vector<WordDataToUser>&& word_data) {
  auto sentences_to_get_assignable =
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>();
  auto return_flow_control_node = std::make_unique<Return>();
  auto active_function = parser_frontend.GetActiveFunctionPointer();
  if (active_function == nullptr) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} ��ǰ�����ں����ڣ��޷�����",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  bool result =
      return_flow_control_node->SetReturnTarget(active_function, nullptr);
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} ����{:}�޷����ؿ�ֵ", GetLine(),
                             GetColumn(), active_function->GetFunctionName())
              << std::endl;
    exit(-1);
  }
  sentences_to_get_assignable->emplace_back(
      std::move(return_flow_control_node));
  return std::move(sentences_to_get_assignable);
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
TemaryOperator(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 5);
  auto& [temary_condition, condition_flow_control_node_container] =
      std::any_cast<std::pair<
          std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  auto& [temary_true_branch, true_branch_flow_control_node_container] =
      std::any_cast<std::pair<
          std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[2].GetNonTerminalWordData().user_returned_data);
  auto& [temary_false_branch, false_branch_flow_control_node_container] =
      std::any_cast<std::pair<
          std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[4].GetNonTerminalWordData().user_returned_data);
  // ������Ŀ������ڵ�
  auto temary_operator_node = std::make_shared<TemaryOperatorNode>();
  bool condition_check_result = temary_operator_node->SetBranchCondition(
      temary_condition, condition_flow_control_node_container);
  if (!condition_check_result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �޷�������������Ϊ��Ŀ���������",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  bool true_branch_check_result = temary_operator_node->SetTrueBranch(
      temary_true_branch, true_branch_flow_control_node_container);
  if (!true_branch_check_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} �޷�������������Ϊ��Ŀ��������֧���",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  bool false_branch_check_result = temary_operator_node->SetFalseBranch(
      temary_false_branch, false_branch_flow_control_node_container);
  if (!false_branch_check_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} �޷�������������Ϊ��Ŀ������ٷ�֧���",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  // �����Ŀ��������̿��ƽڵ�
  auto temary_flow_control_node = std::make_unique<SimpleSentence>();
  bool set_sentence_check_result =
      temary_flow_control_node->SetSentenceOperateNode(temary_operator_node);
  assert(set_sentence_check_result);
  auto flow_control_node_container =
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>();
  flow_control_node_container->emplace_back(
      std::move(temary_flow_control_node));
  // ������Ŀ�������������յõ��Ľڵ�
  return std::make_pair(temary_operator_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::any NotEmptyFunctionCallArgumentsBase(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& [argument_node, sentences_to_get_argument] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  AssignableCheckResult check_result =
      function_call_operator_node->AddFunctionCallArgument(
          argument_node, sentences_to_get_argument);
  CheckAssignableCheckResult(check_result);
  return std::any();
}

std::any NotEmptyFunctionCallArgumentsExtend(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [argument_node, sentences_to_get_argument] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  AssignableCheckResult check_result =
      function_call_operator_node->AddFunctionCallArgument(
          argument_node, sentences_to_get_argument);
  CheckAssignableCheckResult(check_result);
  return std::any();
}

std::pair<std::shared_ptr<FunctionCallOperatorNode>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCallInit(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto function_call_node = std::make_shared<FunctionCallOperatorNode>();
  // ����ȫ�ֱ������Ա���Ӻ�������ʵ��ʱʹ��
  function_call_operator_node = function_call_node;
  auto& [node_to_call, sentences_to_get_node_to_call] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  // ����Ƿ�Ϊ�������ͻ�һ�غ���ָ�룬�����ָ������һ�ν�����
  // ͬʱ���ú������ýڵ��������õĶ���
  if (node_to_call->GetResultTypePointer()->GetType() !=
      StructOrBasicType::kFunction) {
    auto dereference_operator_node =
        std::make_shared<DereferenceOperatorNode>();
    bool result = dereference_operator_node->SetNodeToDereference(node_to_call);
    if (!result ||
        dereference_operator_node->GetResultTypePointer()->GetType() !=
            StructOrBasicType::kFunction) [[unlikely]] {
      std::cerr << std::format(
                       "����{:} ����{:} "
                       "�������ú����Ķ���Ȳ��Ǻ�����Ҳ����һ�غ���ָ��",
                       GetLine(), GetColumn())
                << std::endl;
      exit(-1);
    }
    // ��װ�����ýڵ�
    auto dereference_flow_control_node = std::make_unique<SimpleSentence>();
    result = dereference_flow_control_node->SetSentenceOperateNode(
        dereference_operator_node);
    assert(result);
    // ��ӽ����ýڵ�
    sentences_to_get_node_to_call->emplace_back(
        std::move(dereference_flow_control_node));
    // ���õ��õĽڵ�Ϊ�����ú�õ��Ľڵ�
    result = function_call_operator_node->SetFunctionObjectToCall(
        dereference_operator_node->GetResultOperatorNode());
    assert(result);
  } else {
    // �����ڵ����ֱ�ӵ���
    bool result =
        function_call_operator_node->SetFunctionObjectToCall(node_to_call);
    assert(result);
  }

  return std::make_pair(std::move(function_call_node),
                        std::move(sentences_to_get_node_to_call));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCall(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& [node_to_call, sentences_to_get_node_to_call] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  // �������̿��ƽڵ㲢���
  auto function_call_flow_control_node = std::make_unique<SimpleSentence>();
  bool result = function_call_flow_control_node->SetSentenceOperateNode(
      function_call_operator_node);
  assert(result);
  sentences_to_get_node_to_call->emplace_back(
      std::move(function_call_flow_control_node));
  // ���غ������õõ��Ľڵ�
  return std::make_pair(function_call_operator_node->GetResultOperatorNode(),
                        std::move(sentences_to_get_node_to_call));
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>> AssignablesBase(
    std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& [ignore_assignable, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  return std::move(flow_control_node_container);
}

std::any AssignablesExtend(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // �洢���̿��ƽڵ��������
  auto& main_control_node_container = std::any_cast<
      std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  // ��ǰAssignable�����Ŀ��ƽڵ������
  auto& [ignore_assignable, now_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  // �ϲ�����������
  main_control_node_container->merge(std::move(*now_control_node_container));
  return std::move(word_data[0].GetNonTerminalWordData().user_returned_data);
}

std::shared_ptr<std::unique_ptr<Jmp>> Break(
    std::vector<WordDataToUser>&& word_data) {
  auto& top_flow_control_sentence = static_cast<ConditionBlockInterface&>(
      parser_frontend.GetTopFlowControlSentence());
  switch (top_flow_control_sentence.GetFlowType()) {
    case FlowType::kDoWhileSentence:
    case FlowType::kWhileSentence:
    case FlowType::kForSentence:
    case FlowType::kSwitchSentence:
      return std::make_shared<std::unique_ptr<Jmp>>(std::make_unique<Jmp>(
          top_flow_control_sentence.GetSentenceEndLabel()));
      break;
    default:
      std::cerr
          << std::format(
                 "����{:} ����{:} �޷�������for/while/do-while/switch���",
                 GetLine(), GetColumn())
          << std::endl;
      exit(-1);
      // ��ֹ����
      return std::shared_ptr<std::unique_ptr<Jmp>>();
      break;
  }
}

std::shared_ptr<std::unique_ptr<Jmp>> Continue(
    std::vector<WordDataToUser>&& word_data) {
  auto& top_flow_control_sentence = static_cast<LoopSentenceInterface&>(
      parser_frontend.GetTopFlowControlSentence());
  switch (top_flow_control_sentence.GetFlowType()) {
    case FlowType::kDoWhileSentence:
    case FlowType::kWhileSentence:
    case FlowType::kForSentence:
      return std::make_shared<std::unique_ptr<Jmp>>(std::make_unique<Jmp>(
          top_flow_control_sentence.GetLoopMainBlockEndLabel()));
      break;
    default:
      std::cerr
          << std::format(
                 "����{:} ����{:} �޷��ڷ�for/while/do-while�����ʹ��continue",
                 GetLine(), GetColumn())
          << std::endl;
      exit(-1);
      // ��ֹ����
      return std::shared_ptr<std::unique_ptr<Jmp>>();
      break;
  }
}

std::any SingleStatementAssignable(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [ignore_assignable, flow_control_node_container] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[0].GetNonTerminalWordData().user_returned_data);
  bool result =
      parser_frontend.AddSentences(std::move(*flow_control_node_container));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} ����䲻Ӧ�����ڸ÷�Χ��",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return std::any();
}

std::any SingleStatementAnnounce(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto [ignore_type, ignore_const_tag, flow_control_node_container] =
      std::any_cast<std::tuple<
          std::shared_ptr<const TypeInterface>, ConstTag,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data[0].GetNonTerminalWordData().user_returned_data);
  bool result =
      parser_frontend.AddSentences(std::move(*flow_control_node_container));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} ����䲻Ӧ�����ڸ÷�Χ��",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return std::any();
}

std::any SingleStatementReturn(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& sentences_to_get_return_node = *std::any_cast<
      std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  bool result =
      parser_frontend.AddSentences(std::move(sentences_to_get_return_node));
  // ���б���Ӧ���ⲽ��Լǰ����
  assert(result);
  return std::any();
}

std::any SingleStatementBreak(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& jmp_sentence = *std::any_cast<std::shared_ptr<std::unique_ptr<Jmp>>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  bool result = parser_frontend.AddSentence(std::move(jmp_sentence));
  // ���б���Ӧ���ⲽ��Լǰ����
  assert(result);
  return std::any();
}

std::any SingleStatementContinue(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 1);
  auto& jmp_sentence = *std::any_cast<std::shared_ptr<std::unique_ptr<Jmp>>&>(
      word_data.front().GetNonTerminalWordData().user_returned_data);
  bool result = parser_frontend.AddSentence(std::move(jmp_sentence));
  // ���б���Ӧ���ⲽ��Լǰ����
  assert(result);
  return std::any();
}

std::any IfCondition(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  // if�����ͻ�ȡ�����Ĳ���
  auto& [if_condition, sentences_to_get_if_condition] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  auto if_flow_control_node = std::make_unique<IfSentence>();
  bool result = if_flow_control_node->SetCondition(
      if_condition, std::move(*sentences_to_get_if_condition));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} �������޷���Ϊif�������",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  parser_frontend.PushFlowControlSentence(std::move(if_flow_control_node));
  return std::any();
}

std::any IfWithElse(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  // ת��Ϊif-else���
  parser_frontend.ConvertIfSentenceToIfElseSentence();
  return std::any();
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
ForInitSentenceAnnounce(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  auto& [ignore_extend_type, ignore_extend_const_tag,
         flow_control_node_container] =
      std::any_cast<std::tuple<
          std::shared_ptr<const TypeInterface>, ConstTag,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
          word_data.front().GetNonTerminalWordData().user_returned_data);
  return std::move(flow_control_node_container);
}

std::any ForHead(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 6);
  auto& for_sentence =
      static_cast<ForSentence&>(parser_frontend.GetTopFlowControlSentence());
  assert(for_sentence.GetFlowType() == FlowType::kForSentence);
  auto& for_init_sentences = std::any_cast<
      std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  auto& [for_condition, sentences_to_get_for_condition] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[3].GetNonTerminalWordData().user_returned_data);
  auto& for_renew_sentences = std::any_cast<
      std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&>(
      word_data[4].GetNonTerminalWordData().user_returned_data);
  bool result =
      for_sentence.AddForInitSentences(std::move(*for_init_sentences));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ��������޷���Ϊfor����ʼ������",
                             GetLine())
              << std::endl;
    exit(-1);
  }
  result = for_sentence.SetCondition(
      for_condition, std::move(*sentences_to_get_for_condition));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ��������޷���Ϊfor���ѭ������",
                             GetLine())
              << std::endl;
    exit(-1);
  }
  result = for_sentence.AddForRenewSentences(std::move(*for_renew_sentences));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ��������޷�����for����и���ѭ������",
                             GetLine())
              << std::endl;
    exit(-1);
  }
  return std::any();
}

std::any WhileInitHead(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  auto& [while_condition, sentences_to_get_condition] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  auto pointer_to_while_sentence =
      std::make_shared<std::unique_ptr<WhileSentence>>(
          std::make_unique<WhileSentence>());
  auto& while_sentence = *pointer_to_while_sentence;
  parser_frontend.AddActionScopeLevel();
  bool result = while_sentence->SetCondition(
      while_condition, std::move(*sentences_to_get_condition));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ���������޷���Ϊwhileѭ���������",
                             GetLine())
              << std::endl;
    exit(-1);
  }
  parser_frontend.PushFlowControlSentence(std::move(while_sentence));
  return std::any();
}

std::any DoWhile(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 7);
  auto& [assignable, sentences_to_get_assignable] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[4].GetNonTerminalWordData().user_returned_data);
  DoWhileSentence& do_while_sentence = static_cast<DoWhileSentence&>(
      parser_frontend.GetTopFlowControlSentence());
  assert(do_while_sentence.GetFlowType() == FlowType::kDoWhileSentence);
  // ����do-while�������
  do_while_sentence.SetCondition(assignable,
                                 std::move(*sentences_to_get_assignable));
  parser_frontend.PopActionScope();
  return std::any();
}

std::any SwitchCaseSimple(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto& case_data =
      std::any_cast<std::shared_ptr<BasicTypeInitializeOperatorNode>&>(
          word_data[1].GetNonTerminalWordData().user_returned_data);
  bool result = parser_frontend.AddSwitchSimpleCase(case_data);
  if (!result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} "
                     "�޷���Ӹ�����caseѡ������ǲ�λ��switch����ڻ�case��"
                     "���Ѵ���",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return std::any();
}

std::any SwitchCaseDefault(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 2);
  bool result = parser_frontend.AddSwitchDefaultCase();
  if (!result) [[unlikely]] {
    std::cerr
        << std::format(
               "����{:} ����{:} "
               "�޷����default��ǩ�����ܲ�λ��switch����ڻ��Ѵ���default��ǩ",
               GetLine(), GetColumn())
        << std::endl;
    exit(-1);
  }
  return std::any();
}

std::any SwitchCondition(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 4);
  auto& [assignable, sentences_to_get_assignable] = std::any_cast<
      std::pair<std::shared_ptr<const OperatorNodeInterface>,
                std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&>(
      word_data[2].GetNonTerminalWordData().user_returned_data);
  auto switch_sentence = std::make_unique<SwitchSentence>();
  bool result = switch_sentence->SetCondition(
      assignable, std::move(*sentences_to_get_assignable));
  if (!result) [[unlikely]] {
    std::cerr << std::format("����{:} ����{:} ���������޷���Ϊswitch��֧����",
                             GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  parser_frontend.PushFlowControlSentence(std::move(switch_sentence));
  return std::any();
}

std::any RootAnnounce(std::vector<WordDataToUser>&& word_data) {
  assert(word_data.size() == 3);
  auto flow_control_node = std::any_cast<std::shared_ptr<FlowInterface>&>(
      word_data[1].GetNonTerminalWordData().user_returned_data);
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence: {
      // ȫ�ֱ�������
      auto variety_operator_node =
          std::static_pointer_cast<VarietyOperatorNode>(
              static_cast<SimpleSentence&>(*flow_control_node)
                  .GetSentenceOperateNodePointer());
      // ��ӱ�������
      parser_frontend.DefineVariety(
          *variety_operator_node->GetVarietyNamePointer(),
          variety_operator_node);
    } break;
    case FlowType::kFunctionDefine: {
      // ��������
      auto [ignore_iter, announce_result] =
          parser_frontend.AnnounceFunctionType(
              static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
                  *flow_control_node)
                  .GetFunctionTypePointer());
      CheckAddTypeResult(announce_result);
    } break;
    default:
      assert(false);
      break;
  }
  return std::any();
}

// ��ɹ������̣��������̶���
// �������յĺ�����ָ��/������ָ��

std::pair<std::unique_ptr<FlowInterface>, ObjectConstructData::CheckResult>
ObjectConstructData::ConstructObject(
    ConstTag const_tag_before_final_type,
    std::shared_ptr<const TypeInterface>&& final_node_to_attach) {
  switch (type_chain_tail_->GetType()) {
    case StructOrBasicType::kPointer:
      // ������ĩ��Ϊָ�룬���ø�ָ���const_tag
      std::static_pointer_cast<PointerType>(type_chain_tail_)
          ->SetConstTag(const_tag_before_final_type);
      break;
    case StructOrBasicType::kFunction:
      // ������ĩ��Ϊ���������ú�������ֵ��const_tag
      std::static_pointer_cast<FunctionType>(type_chain_tail_)
          ->SetReturnTypeConstTag(const_tag_before_final_type);
      break;
    case StructOrBasicType::kEnd:
      // ֮ǰδ�����κ����ͽڵ�
      switch (final_node_to_attach->GetType()) {
        case StructOrBasicType::kBasic:
          // ����POD����
          // ��������void���͵ı���
          if (static_cast<const BasicType&>(*final_node_to_attach)
                  .GetBuiltInType() == BuiltInType::kVoid) [[unlikely]] {
            std::cerr << std::format("����{:} ����{:}��������Ϊ\"void\"",
                                     GetLine(), GetObjectName())
                      << std::endl;
            exit(-1);
          }
          break;
        case StructOrBasicType::kFunction:
          std::cerr << std::format("����{:} ����{:}δ��������ֵ", GetLine(),
                                   GetObjectName())
                    << std::endl;
          exit(-1);
          break;
        case StructOrBasicType::kStruct:
        case StructOrBasicType::kUnion:
        case StructOrBasicType::kEnum:
          break;
          [[unlikely]] default : assert(false);
          break;
      }
      static_cast<VarietyOperatorNode&>(static_cast<SimpleSentence&>(*object_)
                                            .GetSentenceOperateNodeReference())
          .SetConstTag(const_tag_before_final_type);
      break;
    default:
      // �����������ʽ����������һ���ڵ�ʱӦ����
      break;
  }
  AttachSingleNodeToTailNodePointer(std::move(final_node_to_attach));
  switch (type_chain_tail_->GetType()) {
    [[unlikely]] case StructOrBasicType::kPointer :
        // δ��ȫ����������������������ָ���β
        return std::make_pair(std::unique_ptr<FlowInterface>(),
                              CheckResult::kPointerEnd);
    break;
    [[unlikely]] case StructOrBasicType::kEnd :
        // ��������
        return std::make_pair(std::unique_ptr<FlowInterface>(),
                              CheckResult::kEmptyChain);
    break;
    default:
      break;
  }
  // �����ڱ��ڵ�
  auto final_type_chain = type_chain_head_->GetNextNodePointer();
  // �ж��Ƿ�Ϊ����ͷ
  if (final_type_chain->GetType() == StructOrBasicType::kFunction)
      [[unlikely]] {
    // Ҫ����������Ϊ��������object_���������͡�������������
    // ת��Ϊ�������͡�����������

    // ��������ӵĺ���
    auto iter = parser_frontend.AnnounceTypeName(GetObjectName());
    // ��ȡ����������
    // Ϊ�˹���������Υ��constԭ��
    auto function_type_chain = std::const_pointer_cast<FunctionType>(
        std::static_pointer_cast<const FunctionType>(final_type_chain));
    // ��ȡ������ָ������ú�����
    function_type_chain->SetFunctionName(&iter->first);
    // �����������壨FunctionDefine���̣�
    ConstructBasicObjectPart<c_parser_frontend::flow_control::FunctionDefine>(
        std::move(function_type_chain));
  } else {
    // Ҫ����������Ϊ�������������ת��
    // ��������ӱ���
    auto iter = parser_frontend.AnnounceVarietyName(GetObjectName());
    auto& variety_node = static_cast<VarietyOperatorNode&>(
        static_cast<SimpleSentence&>(*object_)
            .GetSentenceOperateNodeReference());
    // ���ñ���������
    variety_node.SetVarietyType(
        std::move(type_chain_head_->GetNextNodePointer()));
    // ���ñ�����
    variety_node.SetVarietyName(&iter->first);
  }
  return std::make_pair(std::move(object_), CheckResult::kSuccess);
}

}  // namespace c_parser_frontend::parse_functions