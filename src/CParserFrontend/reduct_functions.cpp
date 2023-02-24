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
    [[likely]] case ObjectConstructData::CheckResult::kSuccess:
      break;
    case ObjectConstructData::CheckResult::kAttachToTerminalType:
      OutputError(std::format(
          "����������{:}�Ѵ����ս����ͣ����������������ͽṹ", object_name));
      exit(-1);
      break;
    case ObjectConstructData::CheckResult::kReturnFunction:
      OutputError(std::format("����{:}����ֵ����Ϊ����", object_name));
      exit(-1);
      break;
    case ObjectConstructData::CheckResult::kPointerEnd:
      OutputError(std::format("����������{:}ȱ�پ�������", object_name));
      exit(-1);
      break;
    case ObjectConstructData::CheckResult::kEmptyChain:
      OutputError(std::format("û��Ϊ����������{:}�ṩ�κ�����", object_name));
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
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
      OutputWarning(std::format("���޷������͸�ֵ���з�������"));
      break;
    case AssignableCheckResult::kSignedToUnsigned:
      // ʹ�þ���
      OutputWarning(std::format("���з������͸�ֵ���޷�������"));
      break;
    case AssignableCheckResult::kLowerConvert:
      OutputError(std::format("�ڸ�ֵʱ������խת��"));
      exit(-1);
      break;
    case AssignableCheckResult::kCanNotConvert:
      OutputError(std::format("�޷�ת������"));
      exit(-1);
      break;
    case AssignableCheckResult::kAssignedNodeIsConst:
      OutputError(std::format("������ʱ�޷���const����ֵ"));
      exit(-1);
      break;
    case AssignableCheckResult::kAssignToRightValue:
      OutputError(std::format("�޷�����ֵ����ֵ"));
      exit(-1);
      break;
    case AssignableCheckResult::kArgumentsFull:
      // TODO ֧�ֿɱ�����������޸ĸñ�����и�������ж�
      OutputError(std::format("ʵ�����������β�����"));
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
      OutputError(std::format("�����������ǿ���������"));
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kLeftNotIntger:
      OutputError(std::format("����������Ϊָ��ƫ����ʱ��������"));
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kLeftRightBothPointer:
      OutputError(std::format("����ָ�������޷�������ѧ����"));
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kRightNotComputableType:
      OutputError(std::format("�����������ǿ���������"));
      exit(-1);
      break;
    case DeclineMathematicalComputeTypeResult::kRightNotIntger:
      OutputError(std::format("����������Ϊָ��ƫ����ʱ��������"));
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
      OutputWarning(std::format("�ظ���������"));
      break;
    case c_parser_frontend::type_system::AddTypeResult::kTypeAlreadyIn:
      OutputError(std::format("�Ѵ���ͬ������"));
      exit(-1);
      break;
    case c_parser_frontend::type_system::AddTypeResult::kDoubleAnnounceFunction:
      OutputError(std::format("�ض��庯��"));
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
}

void OutputError(const std::string& error) {
  std::cerr << std::format("Compline Error: ����:{:} ����:{:} ", GetLine() + 1,
                           GetColumn() + 1)
            << error << std::endl;
}

void OutputWarning(const std::string& warning) {
  std::cerr << std::format("Compline Warning: ����:{:} ����:{:} ",
                           GetLine() + 1, GetColumn() + 1)
            << warning << std::endl;
}

void OutputInfo(const std::string& info) {
  std::cout << std::format("Compline Info: ����:{:} ����:{:} ", GetLine() + 1,
                           GetColumn() + 1)
            << info << std::endl;
}

std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueChar(
    std::string&& word_data) {
  // ''�м�һ���ַ����һ����ת���ַ����ַ�
  // ��ȡ�����ַ���Ϊֵ
  std::string character;
  if (word_data.size() == 3) [[likely]] {
    // ����ת���ַ�
    character = std::to_string(word_data[1]);
  } else {
    // ��ת���ַ�
    character = std::to_string(word_data[2]);
  }
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kBasic, std::move(character));
  initialize_node->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt8,
                                              SignTag::kSigned>());
  return initialize_node;
}
std::shared_ptr<BasicTypeInitializeOperatorNode>
SingleConstexprValueIndexedString(std::string&& str,
                                  std::string&& left_square_bracket,
                                  std::string&& num,
                                  std::string&& right_square_bracket) {
  assert(left_square_bracket == "[");
  assert(right_square_bracket == "]");

  size_t converted_num;
  for (auto iter = str.begin(); iter < str.end(); iter++) {
    // ɾ��ת���ַ�
    if (*iter == '\\') [[unlikely]] {
      iter = str.erase(iter);
      if (*iter == '\\') [[unlikely]] {
        ++iter;
      }
    }
  }
  size_t index = std::stoull(num, &converted_num);
  // ���ʹ�õ��±��Ƿ�ȫ��ת��
  if (converted_num != num.size()) [[unlikely]] {
    OutputError(std::format("�޷�ת���±꣬����Ϊ�������򳬳������±귶Χ"));
    exit(-1);
  }
  // ����±��Ƿ�Խ��
  char result;
  if (index > str.size()) [[unlikely]] {
    OutputError(std::format("ʹ�õ��±�����ַ�����С"));
    exit(-1);
  } else if (index == str.size()) [[unlikely]] {
    result = '\0';
  } else {
    // +1��������ͷ��"
    result = str[index + 1];
  }
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kBasic, std::to_string(result));
  initialize_node->SetInitDataType(
      CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kInt8,
                                              SignTag::kSigned>());
  return initialize_node;
}
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueNum(
    std::string&& num) {
  auto initialize_node = std::make_shared<BasicTypeInitializeOperatorNode>(
      InitializeType::kBasic, std::move(num));
  initialize_node->SetInitDataType(
      type_system::CommonlyUsedTypeGenerator::GetBasicTypeNotTemplate(
          type_system::CalculateBuiltInType(initialize_node->GetValue()),
          SignTag::kSigned));
  return initialize_node;
}
std::shared_ptr<BasicTypeInitializeOperatorNode> SingleConstexprValueString(
    std::string&& str) {
  // �ַ���ʹ��""��Ϊ�ָ����
  assert(str.front() == '"');
  assert(str.back() == '"');
  // ɾ��ת���ַ�
  for (auto iter = str.begin(); iter < str.end(); iter++) {
    if (*iter == '\\') [[unlikely]] {
      iter = str.erase(iter);
      if (*iter == '\\') [[unlikely]] {
        ++iter;
      }
    }
  }
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

BuiltInType FundamentalTypeChar(std::string&& str) {
  return BuiltInType::kInt8;
}

BuiltInType FundamentalTypeShort(std::string&& str) {
  assert(str == "short");
  return BuiltInType::kInt16;
}

BuiltInType FundamentalTypeInt(std::string&& str) {
  assert(str == "int");
  return BuiltInType::kInt32;
}

BuiltInType FundamentalTypeLong(std::string&& str) {
  assert(str == "long");
  return BuiltInType::kInt32;
}

BuiltInType FundamentalTypeFloat(std::string&& str) {
  assert(str == "float");
  return BuiltInType::kFloat32;
}

BuiltInType FundamentalTypeDouble(std::string&& str) {
  assert(str == "double");
  return BuiltInType::kFloat64;
}

BuiltInType FundamentalTypeVoid(std::string&& str) {
  assert(str == "void");
  return BuiltInType::kVoid;
}

SignTag SignTagSigned(std::string&& str) {
  assert(str == "signed");
  return SignTag::kSigned;
}

SignTag SignTagUnSigned(std::string&& str) {
  assert(str == "unsigned");
  return SignTag::kUnsigned;
}

ConstTag ConstTagConst(std::string&& str) {
  assert(str == "const");
  return ConstTag::kConst;
}
std::shared_ptr<ObjectConstructData> IdOrEquivenceConstTagId(ConstTag const_tag,
                                                             std::string&& id) {
  std::shared_ptr<ObjectConstructData> construct_data =
      std::make_shared<ObjectConstructData>(std::move(id));
  construct_data->ConstructBasicObjectPart<VarietyOperatorNode>(
      std::string(), const_tag, LeftRightValueTag::kLeftValue);
  return construct_data;
}

std::shared_ptr<ObjectConstructData>&& IdOrEquivenceNumAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& num,
    std::string&& right_square_bracket) {
  assert(left_square_bracket == "[");
  assert(right_square_bracket == "]");

  // ת��Ϊlong long���ַ���
  size_t chars_converted_to_longlong;
  size_t array_size = std::stoull(num, &chars_converted_to_longlong);
  // ����Ƿ����е����־�����ת��
  if (chars_converted_to_longlong != num.size()) [[unlikely]] {
    // �������е����ֶ�������ת����˵��ʹ���˸�����
    OutputError(std::format(
        "{:}�޷���Ϊ�����С��ƫ������ԭ�����Ϊ����������ֵ���󡢸���", num));
    exit(-1);
  }
  // ���һ��ָ���ָ��ָ��������С
  auto result =
      sub_reduct_result->AttachSingleNodeToTailNodeEmplace<PointerType>(
          ConstTag::kNonConst, array_size);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result, sub_reduct_result->GetObjectName());
  }
  return std::move(sub_reduct_result);
}
std::shared_ptr<ObjectConstructData>&& IdOrEquivenceAnonymousAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& right_square_bracket) {
  assert(left_square_bracket == "[");
  assert(right_square_bracket == "]");

  // ���һ��ָ���ָ��ָ��������С
  // -1������ƶϴ�С
  auto result =
      sub_reduct_result->AttachSingleNodeToTailNodeEmplace<PointerType>(
          ConstTag::kNonConst, -1);
  // ����Ƿ�ɹ����
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result, sub_reduct_result->GetObjectName());
  }
  return std::move(sub_reduct_result);
}
std::shared_ptr<ObjectConstructData>&& IdOrEquivencePointerAnnounce(
    ConstTag const_tag, std::string&& operator_pointer,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result) {
  assert(operator_pointer == "*");

  // ���һ��ָ���ָ��ָ��������С
  // 0����ָ��
  auto result =
      sub_reduct_result->AttachSingleNodeToTailNodeEmplace<PointerType>(
          const_tag, 0);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result, sub_reduct_result->GetObjectName());
  }
  return std::move(sub_reduct_result);
}
std::shared_ptr<ObjectConstructData>&& IdOrEquivenceInBrackets(
    std::string&& left_bracket,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& right_bracket) {
  assert(left_bracket == "(");
  assert(right_bracket == ")");

  return std::move(sub_reduct_result);
}

std::shared_ptr<ObjectConstructData> AnonymousIdOrEquivenceConst(
    std::string&& str_const) {
  assert(str_const == "const");

  std::shared_ptr<ObjectConstructData> construct_data =
      std::make_shared<ObjectConstructData>(std::string());
  construct_data->ConstructBasicObjectPart<VarietyOperatorNode>(
      std::string(), ConstTag::kConst, LeftRightValueTag::kLeftValue);
  return construct_data;
}

std::shared_ptr<ObjectConstructData>&& AnonymousIdOrEquivenceNumAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& num,
    std::string&& right_square_bracket) {
  return IdOrEquivenceNumAddressing(
      std::move(sub_reduct_result), std::move(left_square_bracket),
      std::move(num), std::move(right_square_bracket));
}

std::shared_ptr<ObjectConstructData>&&
AnonymousIdOrEquivenceAnonymousAddressing(
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& left_square_bracket, std::string&& right_square_bracket) {
  return IdOrEquivenceAnonymousAddressing(std::move(sub_reduct_result),
                                          std::move(left_square_bracket),
                                          std::move(right_square_bracket));
}

std::shared_ptr<ObjectConstructData>&& AnonymousIdOrEquivencePointerAnnounce(
    ConstTag const_tag, std::string&& operator_pointer,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result) {
  return IdOrEquivencePointerAnnounce(std::move(const_tag),
                                      std::move(operator_pointer),
                                      std::move(sub_reduct_result));
}

std::shared_ptr<ObjectConstructData>&& AnonymousIdOrEquivenceInBrackets(
    std::string&& left_bracket,
    std::shared_ptr<ObjectConstructData>&& sub_reduct_result,
    std::string&& right_bracket) {
  return IdOrEquivenceInBrackets(std::move(left_bracket),
                                 std::move(sub_reduct_result),
                                 std::move(right_bracket));
}

std::shared_ptr<EnumReturnData> NotEmptyEnumArgumentsIdBase(std::string&& id) {
  auto enum_return_data = std::make_shared<EnumReturnData>();
  // ��һ��ö��ֵĬ�ϴ�0��ʼ
  auto [iter, inserted] = enum_return_data->AddMember(std::move(id), 0);
  assert(inserted);
  return enum_return_data;
}
std::shared_ptr<EnumReturnData> NotEmptyEnumArgumentsIdAssignNumBase(
    std::string&& id, std::string&& operator_assign, std::string&& num) {
  assert(operator_assign == "=");

  auto enum_return_data = std::make_shared<EnumReturnData>();
  size_t chars_converted_to_longlong;
  // ö�����ֵ
  long long enum_member_value = std::stoll(num, &chars_converted_to_longlong);
  if (chars_converted_to_longlong != num.size()) [[unlikely]] {
    // ���ڸ��������������е�ֵ��������Ϊö�����ֵ
    OutputError(std::format("{:}������Ϊö�����ֵ��ԭ�����Ϊ������", num));
    exit(-1);
  }
  // ��Ӹ���ֵ��ö��ֵ
  auto [iter, inserted] =
      enum_return_data->AddMember(std::move(id), enum_member_value);
  if (!inserted) [[unlikely]] {
    // ����ʧ�ܣ�����ͬ����Ա
    OutputError(std::format("ö���{:}�Ѷ���", id));
    exit(-1);
  }
  return enum_return_data;
}
std::shared_ptr<EnumReturnData>&& NotEmptyEnumArgumentsIdExtend(
    std::shared_ptr<EnumReturnData>&& enum_data, std::string&& str_comma,
    std::string&& id) {
  assert(str_comma == ",");

  // δָ�����ֵ��˳����һ����������ֵ
  long long enum_member_value = enum_data->GetLastValue() + 1;
  auto [iter, inserted] =
      enum_data->AddMember(std::move(id), enum_member_value);
  if (!inserted) [[unlikely]] {
    // ����ӵ����ID�Ѿ�����
    OutputError(std::format("ö���{:}�Ѷ���", id));
    exit(-1);
  }
  return std::move(enum_data);
}
std::shared_ptr<EnumReturnData>&& NotEmptyEnumArgumentsIdAssignNumExtend(
    std::shared_ptr<EnumReturnData>&& enum_data, std::string&& str_comma,
    std::string&& id, std::string&& operator_assign, std::string&& num) {
  assert(str_comma == ",");
  assert(operator_assign == "=");

  size_t chars_converted_to_longlong;
  // δָ�����ֵ��˳����һ����������ֵ
  long long enum_member_value = std::stoll(num, &chars_converted_to_longlong);
  if (chars_converted_to_longlong != num.size()) [[unlikely]] {
    // �������ֵ������ȫת��Ϊlong long
    OutputError(
        std::format("ö����{0:} = {1:}�� "
                    "{1:}������Ϊö�����ֵ������ԭ��Ϊ������",
                    id, num));
    exit(-1);
  }
  auto [iter, inserted] =
      enum_data->AddMember(std::move(id), enum_member_value);
  if (!inserted) [[unlikely]] {
    // ����ӵ����ID�Ѿ�����
    OutputError(std::format("ö����{0:} = {1:}�� ö���{0:}�Ѷ���", id, num));
    exit(-1);
  }
  return std::move(enum_data);
}

std::shared_ptr<EnumReturnData>&& EnumArgumentsNotEmptyEnumArguments(
    std::shared_ptr<EnumReturnData>&& enum_data) {
  return std::move(enum_data);
}
std::shared_ptr<EnumType> EnumDefine(
    std::string&& str_enum, std::string&& id, std::string&& left_curly_bracket,
    std::shared_ptr<EnumReturnData>&& enum_data,
    std::string&& right_curly_bracket) {
  assert(str_enum == "enum");
  assert(left_curly_bracket == "{");
  assert(right_curly_bracket == "}");

  std::shared_ptr<EnumType> enum_type =
      std::make_shared<EnumType>(id, std::move(enum_data->GetContainer()));
  // ��Ӹ�ö�ٵĶ���
  auto [insert_iter, result] =
      c_parser_controller.DefineType(std::move(id), enum_type);
  CheckAddTypeResult(result);
  return enum_type;
}
std::shared_ptr<EnumType> EnumAnonymousDefine(
    std::string&& str_enum, std::string&& left_curly_bracket,
    std::shared_ptr<EnumReturnData>&& enum_data,
    std::string&& right_curly_bracket) {
  assert(str_enum == "enum");
  assert(left_curly_bracket == "{");
  assert(right_curly_bracket == "}");

  std::shared_ptr<EnumType> enum_type = std::make_shared<EnumType>(
      std::string(), std::move(enum_data->GetContainer()));
  ;
  // ����ö�����趨��
  return enum_type;
}

std::pair<std::string, StructOrBasicType> EnumAnnounce(std::string&& str_enum,
                                                       std::string&& id) {
  assert(str_enum == "enum");

  return std::make_pair(std::move(id), StructOrBasicType::kEnum);
}

std::pair<std::string, StructOrBasicType> StructureAnnounceStructId(
    std::string&& str_struct, std::string&& id) {
  assert(str_struct == "struct");

  return std::make_pair(std::move(id), StructOrBasicType::kStruct);
}

std::pair<std::string, StructOrBasicType> StructureAnnounceUnionId(
    std::string&& str_union, std::string&& id) {
  assert(str_union == "union");

  return std::make_pair(std::move(id), StructOrBasicType::kUnion);
}

std::pair<std::string, StructOrBasicType> StructureDefineHeadStruct(
    std::string&& str_struct) {
  assert(str_struct == "struct");

  return std::make_pair(std::string(), StructOrBasicType::kStruct);
}

std::pair<std::string, StructOrBasicType> StructureDefineHeadUnion(
    std::string&& str_union) {
  assert(str_union == "union");

  return std::make_pair(std::string(), StructOrBasicType::kUnion);
}

std::pair<std::string, StructOrBasicType>&&
StructureDefineHeadStructureAnnounce(
    std::pair<std::string, StructOrBasicType>&& struct_data) {
  return std::move(struct_data);
}

std::shared_ptr<StructureTypeInterface> StructureDefineInitHead(
    std::pair<std::string, StructOrBasicType>&& struct_data,
    std::string&& left_purly_bracket) {
  auto& [struct_name, struct_type] = struct_data;
  switch (struct_type) {
    case StructOrBasicType::kStruct:
      structure_type_constructuring = std::make_shared<StructType>(struct_name);
      break;
    case StructOrBasicType::kUnion:
      structure_type_constructuring = std::make_shared<UnionType>(struct_name);
      break;
    default:
      assert(false);
      break;
  }
  // ����Ƿ������ṹ��ע��ṹ��/����������
  // ��ʹ��struct_name��Ϊ�Ѿ����ƶ�����
  if (!struct_name.empty()) [[likely]] {
    auto [iter, result] = c_parser_controller.DefineType(
        std::move(struct_name), structure_type_constructuring);
    CheckAddTypeResult(result);
  }
  return structure_type_constructuring;
}
std::shared_ptr<StructureTypeInterface>&& StructureDefine(
    std::shared_ptr<StructureTypeInterface>&& struct_data, std::nullptr_t,
    std::string&& right_curly_bracket) {
  assert(right_curly_bracket == "}");

  return std::move(struct_data);
}

std::shared_ptr<const StructureTypeInterface> StructTypeStructDefine(
    std::shared_ptr<StructureTypeInterface>&& struct_data) {
  return std::move(struct_data);
}
std::shared_ptr<const StructureTypeInterface> StructTypeStructAnnounce(
    std::pair<std::string, StructOrBasicType>&& struct_data) {
  std::string& struct_name = struct_data.first;
  StructOrBasicType struct_type = struct_data.second;
  auto [type_pointer, get_type_result] =
      c_parser_controller.GetType(struct_name, struct_type);
  switch (get_type_result) {
    case GetTypeResult::kSuccess:
      // �ɹ���ȡ
      break;
    case GetTypeResult::kTypeNameNotFound:
    case GetTypeResult::kNoMatchTypePrefer: {
      // �Ҳ����������Ƶ�����
      const char* type_name =
          struct_type == StructOrBasicType::kStruct ? "�ṹ��" : "������";
      OutputError(std::format("{:}{:}δ����", type_name, struct_name));
      exit(-1);
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
    ConstTag const_tag, SignTag sign_tag, BuiltInType builtin_type) {
  switch (builtin_type) {
      // ����������ֻ��ʹ���޷���
    case c_parser_frontend::type_system::BuiltInType::kVoid:
    case c_parser_frontend::type_system::BuiltInType::kInt1:
      assert(sign_tag == SignTag::kSigned);
      break;
    default:
      break;
  }
  return std::make_pair(CommonlyUsedTypeGenerator::GetBasicTypeNotTemplate(
                            builtin_type, sign_tag),
                        const_tag);
}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeStructType(
    ConstTag const_tag,
    std::shared_ptr<const StructureTypeInterface>&& struct_data) {
  return std::make_pair(std::move(struct_data), const_tag);
}

//// BasicType->ConstTag Id
//// ���ػ�ȡ����������ConstTag
// std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeId(
//    std::vector<WordDataToUser>&& word_data) {
//  assert(word_data.size() == 2);
//  ConstTag const_tag = GetConstTag(word_data[0]);
//  std::string& type_name = word_data[1].GetTerminalWordData().word;
//  assert(!type_name.empty());
//  // ��ȡID��Ӧ������
//  auto [type_pointer, result] =
//      c_parser_frontend.GetType(type_name, StructOrBasicType::kNotSpecified);
//  switch (result) {
//    case GetTypeResult::kSuccess:
//      // �ɹ���ȡ��������
//      break;
//    case GetTypeResult::kSeveralSameLevelMatches:
//      OutputError(std::format("������{:}��Ӧ���ͬ������", type_name));
//      exit(-1);
//      break;
//    case GetTypeResult::kTypeNameNotFound:
//      OutputError(std::format("����{:}������", type_name));
//      exit(-1);
//      break;
//    case GetTypeResult::kNoMatchTypePrefer:
//      // ������ѡ������ʱ���ܷ��ظý��
//    default:
//      assert(false);
//      break;
//  }
//  return std::make_pair(std::move(type_pointer), const_tag);
//}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag> BasicTypeEnumAnnounce(
    ConstTag const_tag, std::pair<std::string, StructOrBasicType>&& enum_data) {
  std::string& enum_name = enum_data.first;
  auto [type_pointer, result] =
      c_parser_controller.GetType(enum_name, StructOrBasicType::kNotSpecified);
  switch (result) {
    case GetTypeResult::kSuccess:
      // �ɹ���ȡ��������
      break;
    case GetTypeResult::kSeveralSameLevelMatches:
      OutputError(std::format("������{:}��Ӧ���ͬ������", enum_name));
      exit(-1);
      break;
    case GetTypeResult::kTypeNameNotFound:
      OutputError(std::format("����{:}������", enum_name));
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

std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartFunctionInitBase(
    std::shared_ptr<ObjectConstructData>&& construct_data,
    std::string&& left_bracket) {
  assert(left_bracket == "(");

  // ���ù������ݣ��ڹ�����������ʱʹ��
  function_type_construct_data = construct_data;
  // ���������������Ͳ����ú�������
  auto result =
      function_type_construct_data
          ->AttachSingleNodeToTailNodeEmplace<FunctionType>(std::string());
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(
        result, function_type_construct_data->GetObjectName());
  }
  return std::move(construct_data);
}

std::shared_ptr<ObjectConstructData>&&
FunctionRelaventBasePartFunctionInitExtend(
    std::shared_ptr<ObjectConstructData>&& construct_data,
    std::string&& left_bracket) {
  return FunctionRelaventBasePartFunctionInitBase(std::move(construct_data),
                                                  std::move(left_bracket));
}

std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartFunction(
    std::shared_ptr<ObjectConstructData>&& construct_data, std::nullptr_t&&,
    std::string&& right_bracket) {
  assert(right_bracket == ")");

  return std::move(construct_data);
}

std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartPointer(
    ConstTag const_tag, std::string&& str_pointer,
    std::shared_ptr<ObjectConstructData>&& construct_data) {
  assert(str_pointer == "*");

  // ���һ��ָ��ڵ�
  auto result = construct_data->AttachSingleNodeToTailNodeEmplace<PointerType>(
      const_tag, 0);
  // �����ӽ��
  if (result != ObjectConstructData::CheckResult::kSuccess) [[unlikely]] {
    VarietyOrFunctionConstructError(result, construct_data->GetObjectName());
  }
  return std::move(construct_data);
}

std::shared_ptr<ObjectConstructData>&& FunctionRelaventBasePartBranckets(
    std::string&& left_bracket,
    std::shared_ptr<ObjectConstructData>&& construct_data,
    std::string&& right_bracket) {
  assert(left_bracket == "(");
  assert(right_bracket == ")");

  return std::move(construct_data);
}

std::shared_ptr<FlowInterface> FunctionRelavent(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& basic_type_data,
    std::shared_ptr<ObjectConstructData>&& construct_data) {
  auto& [final_type, const_tag_before_final_type] = basic_type_data;
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

std::shared_ptr<FlowInterface> SingleAnnounceNoAssignNotPodVariety(
    ConstTag const_tag, std::string&& id,
    std::shared_ptr<ObjectConstructData>&& construct_data) {
  if (construct_data->GetObjectName().empty()) [[unlikely]] {
    OutputError(std::format("�����ı�����������"));
    exit(-1);
  }
  auto [final_type, result] =
      c_parser_controller.GetType(id, StructOrBasicType::kNotSpecified);
  switch (result) {
    case GetTypeResult::kSeveralSameLevelMatches:
      OutputError(
          std::format("������ {:} ��Ӧ�������ͣ��޷�ȷ��Ҫʹ�õ�����", id));
      exit(-1);
      break;
    case GetTypeResult::kTypeNameNotFound:
      OutputError(std::format("���������� {:}", id));
      exit(-1);
      break;
    case GetTypeResult::kSuccess:
      break;
    case GetTypeResult::kNoMatchTypePrefer:
    default:
      assert(false);
      break;
  }
  auto [flow_control_node, construct_result] =
      construct_data->ConstructObject(const_tag, std::move(final_type));
  if (construct_result != ObjectConstructData::CheckResult::kSuccess)
      [[unlikely]] {
    VarietyOrFunctionConstructError(construct_result,
                                    construct_data->GetObjectName());
  }
  return std::move(flow_control_node);
}

std::nullptr_t TypeDef(std::string&& str_typedef,
                       std::shared_ptr<FlowInterface>&& flow_control_node) {
  assert(str_typedef == "typedef");

  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      // ���ܸ����������
      OutputError(std::format("�޷������������"));
      exit(-1);
      break;
    default:
      break;
  }
  auto variety_pointer = static_pointer_cast<VarietyOperatorNode>(
      static_cast<SimpleSentence&>(*flow_control_node)
          .GetSentenceOperateNodePointer());
  auto type_pointer = variety_pointer->GetVarietyTypePointer();
  const std::string& type_name = variety_pointer->GetVarietyName();
  auto [ignore_iter, result] =
      c_parser_controller.DefineType(type_name, type_pointer);
  if (result == AddTypeResult::kTypeAlreadyIn) [[unlikely]] {
    OutputError(std::format(
        "ʹ��typedef�������ʱʹ�õ�����{:}�Ѵ�����ͬ���͵Ķ���", type_name));
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t NotEmptyFunctionRelaventArgumentsBase(
    std::shared_ptr<FlowInterface>&& flow_control_node) {
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      // ֻ�б���������Ϊ��������
      break;
    case FlowType::kFunctionDefine:
      // ����������Ϊ��������ʱ�Ĳ���
      OutputError(std::format("����������Ϊ����ʱ��������,��ʹ�ú���ָ��"));
      OutputError(std::format(
          "����ϳ����ں���{:}��",
          static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
              *flow_control_node)
              .GetFunctionTypeReference()
              .GetFunctionName()));
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
  return nullptr;
}

std::nullptr_t NotEmptyFunctionRelaventArgumentsAnonymousBase(
    std::shared_ptr<FlowInterface>&& flow_control_node) {
  return NotEmptyFunctionRelaventArgumentsBase(std::move(flow_control_node));
}

std::nullptr_t NotEmptyFunctionRelaventArgumentsExtend(
    std::nullptr_t&&, std::string&& str_comma,
    std::shared_ptr<FlowInterface>&& flow_control_node) {
  assert(str_comma == ",");

  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      // ֻ�б���������Ϊ��������
      break;
    case FlowType::kFunctionDefine:
      // ����������Ϊ�����Ĳ���
      OutputError(std::format("����������Ϊ����ʱ��������,��ʹ�ú���ָ��"));
      OutputError(std::format(
          "����ϳ����ں���{:}��",
          static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
              *flow_control_node)
              .GetFunctionTypeReference()
              .GetFunctionName()));
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
  return nullptr;
}

std::nullptr_t NotEmptyFunctionRelaventArgumentsAnonymousExtend(
    std::nullptr_t&&, std::string&& str_comma,
    std::shared_ptr<FlowInterface>&& flow_control_node) {
  return NotEmptyFunctionRelaventArgumentsExtend(nullptr, std::move(str_comma),
                                                 std::move(flow_control_node));
}

std::nullptr_t FunctionRelaventArguments(std::nullptr_t) { return nullptr; }

std::shared_ptr<c_parser_frontend::flow_control::FunctionDefine>
FunctionDefineHead(std::shared_ptr<FlowInterface>&& function_head,
                   std::string&& left_curly_bracket) {
  assert(left_curly_bracket == "{");

  // ����Ƿ�Ϊ����ͷ
  switch (function_head->GetFlowType()) {
    case FlowType::kFunctionDefine:
      break;
    case FlowType::kSimpleSentence:
      OutputError(std::format("���������﷨����"));
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
  std::shared_ptr<const FunctionType> function_type =
      static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
          *function_head)
          .GetFunctionTypePointer();
  // ���õ�ǰ����������
  bool set_result = c_parser_controller.SetFunctionToConstruct(
      std::shared_ptr<const FunctionType>(function_type));
  if (!set_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} "
                     "�޷�����/���庯��������ԭ��Ϊ�������ڲ�����/"
                     "���庯������������������ͬ������ǩ����ͬ�ĺ���",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return std::static_pointer_cast<
      c_parser_frontend::flow_control::FunctionDefine>(
      std::move(function_head));
}

std::nullptr_t FunctionDefine(
    std::shared_ptr<c_parser_frontend::flow_control::FunctionDefine>&&,
    std::nullptr_t, std::string&& right_curly_bracket) {
  assert(right_curly_bracket == "}");

  // ��麯�����Ƿ�Ϊ�գ����Ϊ��������޷���ֵ�������򱨴�
  auto& active_function = c_parser_controller.GetActiveFunctionReference();
  if (active_function.GetSentences().empty()) [[unlikely]] {
    auto& function_return_type =
        active_function.GetFunctionTypeReference().GetReturnTypeReference();
    if (function_return_type ==
        *c_parser_frontend::type_system::CommonlyUsedTypeGenerator::
            GetBasicType<c_parser_frontend::type_system::BuiltInType::kVoid,
                         c_parser_frontend::type_system::SignTag::kUnsigned>())
        [[likely]] {
      // �����޷���ֵ
      bool result = c_parser_controller.AddSentence(std::make_unique<Return>());
      assert(result);
    } else {
      OutputError(std::format("�������뷵��һ��ֵ"));
      exit(-1);
    }
  }
  // ��Ӻ���������Ѿ���Sentences�������������
  // ֻ������������
  c_parser_controller.PopActionScope();
  return nullptr;
}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag>
SingleStructureBodyBase(std::shared_ptr<FlowInterface>&& flow_control_node) {
  // ����Ƿ�Ϊ��������
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      OutputError(std::format("��������������Ϊ�ṹ���ݳ�Ա"));
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
          variety_pointer->GetVarietyName(),
          variety_pointer->GetVarietyTypePointer(),
          variety_pointer->GetConstTag());
  assert(structure_member_index.IsValid());
  return std::make_pair(std::move(extend_type), extend_const_tag);
}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&&
SingleStructureBodyExtend(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& struct_data,
    std::string&& str_comma, std::string&& new_member_name) {
  assert(str_comma == ",");

  auto& [extend_type, extend_const_tag] = struct_data;
  auto structure_member_index =
      structure_type_constructuring->AddStructureMember(
          std::move(new_member_name), extend_type, extend_const_tag);
  // ������ӳ�Ա���Ƿ��ظ�
  if (!structure_member_index.IsValid()) {
    OutputError(std::format("�ض����Ա{:}", new_member_name));
    exit(-1);
  }
  return std::move(struct_data);
}

std::nullptr_t NotEmptyStructureBodyBase(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&&) {
  return nullptr;
}

std::nullptr_t NotEmptyStructureBodyExtend(
    std::nullptr_t, std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&&,
    std::string&& semicolon) {
  assert(semicolon == ";");

  return nullptr;
}

std::nullptr_t StructureBody(std::nullptr_t) { return nullptr; }

std::shared_ptr<ListInitializeOperatorNode> InitializeList(
    std::string&& left_curly_bracket,
    std::shared_ptr<
        std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>&&
        list_arguments,
    std::string&& right_curly_bracket) {
  auto initialize_list = std::make_shared<ListInitializeOperatorNode>();
  // ��ȡ��ʼ���б����
  std::list<std::shared_ptr<const InitializeOperatorNodeInterface>>
      argument_pointers_to_add;
  for (auto& pointer : *list_arguments) {
    argument_pointers_to_add.emplace_back(std::move(pointer));
  }
  // ����ʼ���б�����в�����ӵ�������
  bool result =
      initialize_list->SetListValues(std::move(argument_pointers_to_add));
  return std::move(initialize_list);
}

std::shared_ptr<InitializeOperatorNodeInterface>&&
SingleInitializeListArgumentConstexprValue(
    std::shared_ptr<BasicTypeInitializeOperatorNode>&& value) {
  return std::move(value);
}

std::shared_ptr<InitializeOperatorNodeInterface>&&
SingleInitializeListArgumentList(
    std::shared_ptr<ListInitializeOperatorNode>&& value) {
  return std::move(value);
}

std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>
InitializeListArgumentsBase(
    std::shared_ptr<InitializeOperatorNodeInterface>&& init_data_pointer) {
  auto list_pointer = std::make_shared<
      std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>();
  list_pointer->emplace_back(std::move(init_data_pointer));
  return list_pointer;
}

std::shared_ptr<std::list<std::shared_ptr<InitializeOperatorNodeInterface>>>&&
InitializeListArgumentsExtend(
    std::shared_ptr<std::list<
        std::shared_ptr<InitializeOperatorNodeInterface>>>&& list_pointer,
    std::string&& str_comma,
    std::shared_ptr<InitializeOperatorNodeInterface>&& init_data_pointer) {
  assert(str_comma == ",");

  list_pointer->emplace_back(std::move(init_data_pointer));
  return std::move(list_pointer);
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AnnounceAssignableAssignable(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value) {
  return std::move(value);
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AnnounceAssignableInitializeList(
    std::shared_ptr<ListInitializeOperatorNode>&& initialize_list) {
  return std::make_pair(
      std::move(initialize_list),
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
  c_parser_controller.DefineVariety(variety_node);
  // ��ȡ��չ����ʱ������
  if (raw_variety_type->GetType() == StructOrBasicType::kPointer) {
    // ��������ά����ָ��
    while (raw_variety_type->GetType() == StructOrBasicType::kPointer &&
           std::static_pointer_cast<const PointerType>(raw_variety_type)
                   ->GetArraySize() != 0) {
      raw_variety_type = raw_variety_type->GetNextNodePointer();
    }
    // �����ָ����������ԭʼ�������ͻ�����ȥ��һ��ָ��
    return std::make_tuple(
        raw_variety_type->GetType() == StructOrBasicType::kPointer
            ? raw_variety_type->GetNextNodePointer()
            : raw_variety_type,
        extend_const_tag, std::move(flow_control_node_container));
  } else {
    // ����ָ�����ͣ�ֱ�ӷ���
    return std::make_tuple(raw_variety_type, extend_const_tag,
                           std::move(flow_control_node_container));
  }
}

std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
VarietyAnnounceWithAssign(
    std::shared_ptr<VarietyOperatorNode>&& variety_node,
    const std::shared_ptr<const OperatorNodeInterface>& node_for_assign) {
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
  assign_node->SetNodeToBeAssigned(variety_node);
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
  c_parser_controller.DefineVariety(variety_node);
  // ��ȡ��չ����ʱ������
  if (raw_variety_type->GetType() == StructOrBasicType::kPointer) {
    // ��������ά����ָ��
    while (raw_variety_type->GetType() == StructOrBasicType::kPointer &&
           std::static_pointer_cast<const PointerType>(raw_variety_type)
                   ->GetArraySize() != 0) {
      raw_variety_type = raw_variety_type->GetNextNodePointer();
    }
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
SingleAnnounceAndAssignNoAssignBase(
    std::shared_ptr<FlowInterface>&& flow_control_node) {
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      OutputError(std::format("��֧�ֵ���չ����������������"));
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
SingleAnnounceAndAssignWithAssignBase(
    std::shared_ptr<FlowInterface>&& flow_control_node,
    std::string&& str_assign,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        container) {
  assert(str_assign == "=");

  // ����Ƿ�Ϊ��������
  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence:
      break;
    case FlowType::kFunctionDefine:
      OutputError(std::format("��֧�ֵ���չ����������������"));
      exit(-1);
      break;
    default:
      assert(false);
      break;
  }
  auto& [node_for_assign, statements_to_get_node_for_assign] = container;
  c_parser_controller.AddSentences(
      std::move(*statements_to_get_node_for_assign));
  return VarietyAnnounceWithAssign(
      std::static_pointer_cast<VarietyOperatorNode>(
          static_cast<SimpleSentence&>(*flow_control_node)
              .GetSentenceOperateNodePointer()),
      std::move(node_for_assign));
}

std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
           std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
SingleAnnounceAndAssignNoAssignExtend(
    std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
               std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        container,
    std::string&& str_comma, std::string&& variety_name) {
  assert(str_comma == ",");

  auto& [extend_announce_type, extend_const_tag, flow_control_node_container] =
      container;
  // ���������ڵ�
  auto variety_node = std::make_shared<VarietyOperatorNode>(
      std::move(variety_name), extend_const_tag, LeftRightValueTag::kLeftValue);
  variety_node->SetVarietyType(
      std::shared_ptr<const TypeInterface>(extend_announce_type));
  auto [ignore_type, ignore_const_tag, sub_flow_control_node_container] =
      VarietyAnnounceNoAssign(std::move(variety_node));
  // ����ȡ�����Ĳ����ϲ�����������
  flow_control_node_container->splice(
      flow_control_node_container->end(),
      std::move(*sub_flow_control_node_container));
  return std::move(container);
}

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
        assigned_container) {
  assert(str_comma == ",");
  assert(str_assign == "=");

  auto& [extend_announce_type, extend_const_tag, flow_control_node_container] =
      container;
  auto& [node_for_assign, flow_control_nodes_to_get_node_for_assign] =
      assigned_container;
  // ����ȡ����ֵ�����Ĳ����ϲ�����������
  flow_control_node_container->splice(
      flow_control_node_container->end(),
      std::move(*flow_control_nodes_to_get_node_for_assign));
  // ���������ڵ�
  auto variety_node = std::make_shared<VarietyOperatorNode>(
      std::move(variety_name), extend_const_tag, LeftRightValueTag::kLeftValue);
  variety_node->SetVarietyType(
      std::shared_ptr<const TypeInterface>(extend_announce_type));
  auto [ignore_type, ignore_const_tag, sub_flow_control_node_container] =
      VarietyAnnounceWithAssign(std::move(variety_node),
                                std::move(node_for_assign));
  // ����ʼ�������Ĳ����ϲ�����������
  flow_control_node_container->splice(
      flow_control_node_container->end(),
      std::move(*sub_flow_control_node_container));
  return std::move(container);
}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& TypeBasicType(
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& type_data) {
  return std::move(type_data);
}

std::pair<std::shared_ptr<const TypeInterface>, ConstTag> TypeFunctionRelavent(
    std::shared_ptr<FlowInterface>&& flow_control_node) {
  // ��������Ƿ�Ϊ��������
  if (flow_control_node->GetFlowType() == FlowType::kFunctionDefine)
      [[unlikely]] {
    // ����������Ϊ����ʹ��
    OutputError(std::format("��������/���岻����Ϊ����"));
    exit(-1);
  }
  auto operator_node = std::static_pointer_cast<VarietyOperatorNode>(
      static_cast<SimpleSentence&>(*flow_control_node)
          .GetSentenceOperateNodePointer());
  // ����Ƿ�Ϊ����ָ������
  if (!operator_node->GetVarietyName().empty()) [[unlikely]] {
    OutputError(std::format("����������������"));
    exit(-1);
  }
  return std::make_pair(operator_node->GetVarietyTypePointer(),
                        operator_node->GetConstTag());
}

MathematicalOperation MathematicalOperatorPlus(std::string&& str_operator) {
  assert(str_operator == "+");
  return MathematicalOperation::kPlus;
}

MathematicalOperation MathematicalOperatorMinus(std::string&& str_operator) {
  assert(str_operator == "-");
  return MathematicalOperation::kMinus;
}

MathematicalOperation MathematicalOperatorMultiple(std::string&& str_operator) {
  assert(str_operator == "*");
  return MathematicalOperation::kMultiple;
}

MathematicalOperation MathematicalOperatorDivide(std::string&& str_operator) {
  assert(str_operator == "/");
  return MathematicalOperation::kDivide;
}

MathematicalOperation MathematicalOperatorMod(std::string&& str_operator) {
  assert(str_operator == "%");
  return MathematicalOperation::kMod;
}

MathematicalOperation MathematicalOperatorLeftShift(
    std::string&& str_operator) {
  assert(str_operator == "<<");
  return MathematicalOperation::kLeftShift;
}

MathematicalOperation MathematicalOperatorRightShift(
    std::string&& str_operator) {
  assert(str_operator == ">>");
  return MathematicalOperation::kRightShift;
}

MathematicalOperation MathematicalOperatorAnd(std::string&& str_operator) {
  assert(str_operator == "&");
  return MathematicalOperation::kAnd;
}

MathematicalOperation MathematicalOperatorOr(std::string&& str_operator) {
  assert(str_operator == "|");
  return MathematicalOperation::kOr;
}

MathematicalOperation MathematicalOperatorXor(std::string&& str_operator) {
  assert(str_operator == "^");
  return MathematicalOperation::kXor;
}

MathematicalOperation MathematicalOperatorNot(std::string&& str_operator) {
  assert(str_operator == "!");
  return MathematicalOperation::kNot;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorPlusAssign(
    std::string&& str_operator) {
  assert(str_operator == "+=");
  return MathematicalAndAssignOperation::kPlusAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorMinusAssign(
    std::string&& str_operator) {
  assert(str_operator == "-=");
  return MathematicalAndAssignOperation::kMinusAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorMultipleAssign(
    std::string&& str_operator) {
  assert(str_operator == "*=");
  return MathematicalAndAssignOperation::kMultipleAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorDivideAssign(
    std::string&& str_operator) {
  assert(str_operator == "/=");
  return MathematicalAndAssignOperation::kDivideAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorModAssign(
    std::string&& str_operator) {
  assert(str_operator == "%=");
  return MathematicalAndAssignOperation::kModAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorLeftShiftAssign(
    std::string&& str_operator) {
  assert(str_operator == "<<=");
  return MathematicalAndAssignOperation::kLeftShiftAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorRightShiftAssign(
    std::string&& str_operator) {
  assert(str_operator == ">>=");
  return MathematicalAndAssignOperation::kRightShiftAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorAndAssign(
    std::string&& str_operator) {
  assert(str_operator == "&=");
  return MathematicalAndAssignOperation::kAndAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorOrAssign(
    std::string&& str_operator) {
  assert(str_operator == "|=");
  return MathematicalAndAssignOperation::kOrAssign;
}

MathematicalAndAssignOperation MathematicalAndAssignOperatorXorAssign(
    std::string&& str_operator) {
  assert(str_operator == "^=");
  return MathematicalAndAssignOperation::kXorAssign;
}

LogicalOperation LogicalOperatorAndAnd(std::string&& str_operator) {
  assert(str_operator == "&&");
  return LogicalOperation::kAndAnd;
}

LogicalOperation LogicalOperatorOrOr(std::string&& str_operator) {
  assert(str_operator == "||");
  return LogicalOperation::kOrOr;
}

LogicalOperation LogicalOperatorGreater(std::string&& str_operator) {
  assert(str_operator == ">");
  return LogicalOperation::kGreater;
}

LogicalOperation LogicalOperatorGreaterEqual(std::string&& str_operator) {
  assert(str_operator == ">=");
  return LogicalOperation::kGreaterEqual;
}

LogicalOperation LogicalOperatorLess(std::string&& str_operator) {
  assert(str_operator == "<");
  return LogicalOperation::kLess;
}

LogicalOperation LogicalOperatorLessEqual(std::string&& str_operator) {
  assert(str_operator == "<=");
  return LogicalOperation::kLessEqual;
}

LogicalOperation LogicalOperatorEqual(std::string&& str_operator) {
  assert(str_operator == "==");
  return LogicalOperation::kEqual;
}

LogicalOperation LogicalOperatorNotEqual(std::string&& str_operator) {
  assert(str_operator == "!=");
  return LogicalOperation::kNotEqual;
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableConstexprValue(
    std::shared_ptr<BasicTypeInitializeOperatorNode>&& value) {
  // �����洢��ȡ����/�����Ĳ���������
  return std::make_pair(
      std::move(value),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableId(std::string&& variety_name) {
  auto [variety_operator_node, found] =
      c_parser_controller.GetVarietyOrFunction(variety_name);
  if (!found) [[unlikely]] {
    // δ�ҵ��������Ƶı���
    OutputError(std::format("�Ҳ�������{:}", variety_name));
    exit(-1);
  }
  return std::make_pair(
      std::move(variety_operator_node),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AssignableTemaryOperator(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value) {
  return std::move(value);
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AssignableBracket(
    std::string&& left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value,
    std::string&& right_bracket) {
  assert(left_bracket == "(");
  assert(right_bracket == ")");

  return std::move(value);
}
std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableTypeConvert(
    std::string&& left_bracket,
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& type_data,
    std::string&& right_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        old_variety_data) {
  assert(left_bracket == "(");
  assert(right_bracket == ")");

  auto& [new_type, new_const_tag] = type_data;
  auto& [old_operator_node, flow_control_node_container] = old_variety_data;
  if (old_operator_node->GetGeneralOperatorType() ==
          GeneralOperationType::kInitValue &&
      new_const_tag == ConstTag::kNonConst) [[unlikely]] {
    OutputError("��ʼ�����ݲ���ת��Ϊ��const����");
    exit(-1);
  }
  // ת���ڵ�
  auto convert_operator_node =
      std::make_shared<TypeConvert>(old_operator_node, new_type, new_const_tag);
  // ��������ת���õ��Ľڵ�
  auto converted_operator_node =
      convert_operator_node->GetDestinationNodePointer();
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
AssignableAssign(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        destination_variety_data,
    std::string&& str_assign,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        source_variety_data) {
  assert(str_assign == "=");

  auto& [node_to_be_assigned, statements_to_get_node_to_be_assigned] =
      destination_variety_data;
  auto& [node_for_assign, statements_to_get_node_for_assign] =
      source_variety_data;
  // ��ȡ�����Ĵ���ֵ����
  auto real_node_to_be_assigned = node_to_be_assigned->GetResultOperatorNode();
  if (real_node_to_be_assigned == nullptr) {
    real_node_to_be_assigned = node_to_be_assigned;
  }
  // ��ȡ�����Ĵ���ֵ����
  auto real_node_for_assign = node_for_assign->GetResultOperatorNode();
  if (real_node_for_assign == nullptr) {
    real_node_for_assign = node_for_assign;
  }
  if (real_node_to_be_assigned->GetGeneralOperatorType() !=
      GeneralOperationType::kVariety) [[unlikely]] {
    OutputError(std::format("�޷��ԷǱ�����ֵ"));
    exit(-1);
  } else if (static_cast<const VarietyOperatorNode&>(*real_node_to_be_assigned)
                 .GetLeftRightValueTag() != LeftRightValueTag::kLeftValue)
      [[unlikely]] {
    OutputError(std::format("�޷�����ֵ��ֵ"));
    exit(-1);
  }
  auto assign_node = std::make_shared<AssignOperatorNode>();
  assign_node->SetNodeToBeAssigned(real_node_to_be_assigned);
  auto assignable_check_result =
      assign_node->SetNodeForAssign(real_node_for_assign, false);
  CheckAssignableCheckResult(assignable_check_result);
  // �ϲ���ȡ��ֵ�ýڵ�ͱ���ֵ�Ľڵ�����в��������
  statements_to_get_node_to_be_assigned->splice(
      statements_to_get_node_to_be_assigned->end(),
      std::move(*statements_to_get_node_for_assign));
  auto flow_control_sentence = std::make_unique<SimpleSentence>();
  bool result = flow_control_sentence->SetSentenceOperateNode(assign_node);
  assert(result);
  statements_to_get_node_to_be_assigned->emplace_back(
      std::move(flow_control_sentence));
  assert(statements_to_get_node_for_assign->empty());
  return std::make_pair(std::move(assign_node),
                        std::move(statements_to_get_node_to_be_assigned));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
AssignableFunctionCall(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value) {
  return std::move(value);
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSizeOfType(
    std::string&& str_sizeof, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&& type_data,
    std::string&& str_right_bracket) {
  assert(str_sizeof == "sizeof");
  assert(str_left_bracket == "(");
  assert(str_right_bracket == ")");

  auto& [type_pointer, variety_const_tag] = type_data;
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
AssignableSizeOfAssignable(
    std::string&& str_sizeof, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_right_bracket) {
  assert(str_sizeof == "sizeof");
  assert(str_left_bracket == "(");
  assert(str_right_bracket == ")");

  auto& [assignable_node, flow_control_node_container] = variety_data;
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
AssignableMemberAccess(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_member_access, std::string&& member_name) {
  assert(str_member_access == ".");

  auto& [assignable, sentences_to_get_assignable] = variety_data;
  auto member_access_node = std::make_shared<MemberAccessOperatorNode>();
  bool result = member_access_node->SetNodeToAccess(assignable);
  if (!result) [[unlikely]] {
    OutputError(std::format("�޷��Էǽṹ���ݻ�ö�����ͷ��ʳ�Ա"));
    exit(-1);
  }
  result = member_access_node->SetMemberName(std::move(member_name));
  if (!result) [[unlikely]] {
    OutputError(std::format("�����ṹ�岻���ڳ�Ա{:}", member_name));
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
AssignablePointerMemberAccess(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_member_access, std::string&& member_name) {
  assert(str_member_access == "->");

  auto& [assignable, sentences_to_get_assignable] = variety_data;
  // ��ָ������ú���ʳ�Ա
  auto dereference_node = std::make_shared<DereferenceOperatorNode>();
  bool result = dereference_node->SetNodeToDereference(assignable);
  if (!result) [[unlikely]] {
    OutputError(std::format("���ܶԷ�ָ�����ʹ��\"->\"�����"));
    exit(-1);
  }
  auto dereferenced_node = dereference_node->GetDereferencedNodePointer();
  auto member_access_node = std::make_shared<MemberAccessOperatorNode>();
  result = member_access_node->SetNodeToAccess(dereferenced_node);
  if (!result) [[unlikely]] {
    OutputError(std::format("�޷��Էǽṹ���ݻ�ö�����ͷ��ʳ�Ա"));
    exit(-1);
  }
  result = member_access_node->SetMemberName(std::move(member_name));
  if (!result) [[unlikely]] {
    OutputError(std::format("�����ṹ���ݲ����ڳ�Ա{:}", member_name));
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
AssignableMathematicalOperate(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        destination_variety_data,
    MathematicalOperation mathematical_operation,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        source_variety_data) {
  auto& [left_operator_node, left_flow_control_node_container] =
      destination_variety_data;
  auto& [right_operator_node, right_flow_control_node_container] =
      source_variety_data;
  auto mathematical_operator_node =
      std::make_shared<MathematicalOperatorNode>(mathematical_operation);
  bool left_operator_node_check_result =
      mathematical_operator_node->SetLeftOperatorNode(left_operator_node);
  if (!left_operator_node_check_result) [[unlikely]] {
    OutputError(std::format("���������޷���������"));
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
  left_flow_control_node_container->splice(
      left_flow_control_node_container->end(),
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
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        destination_variety_data,
    MathematicalAndAssignOperation mathematical_and_assign_operation,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        source_variety_data) {
  auto& [left_operator_node, left_flow_control_node_container] =
      destination_variety_data;
  auto& [right_operator_node, right_flow_control_node_container] =
      source_variety_data;
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
    OutputError(std::format("���������޷���������"));
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
  left_flow_control_node_container->splice(
      left_flow_control_node_container->end(),
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
AssignableLogicalOperate(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&& lhr,
    LogicalOperation logical_operation,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        rhr) {
  auto& [left_operator_node, left_flow_control_node_container] = lhr;
  auto& [right_operator_node, right_flow_control_node_container] = rhr;
  // �����߼�����ڵ�
  auto logical_operator_node =
      std::make_shared<LogicalOperationOperatorNode>(logical_operation);
  bool check_result =
      logical_operator_node->SetLeftOperatorNode(left_operator_node);
  if (!check_result) [[unlikely]] {
    OutputError(std::format("��������޷���Ϊ�߼�����ڵ�"));
    exit(-1);
  }
  check_result =
      logical_operator_node->SetRightOperatorNode(right_operator_node);
  if (!check_result) [[unlikely]] {
    OutputError(std::format("�Ҳ������޷���Ϊ�߼�����ڵ�"));
    exit(-1);
  }
  // ������������ڵ�Ĳ����ϲ�����������
  left_flow_control_node_container->splice(
      left_flow_control_node_container->end(),
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
AssignableNot(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data) {
  assert(str_operator == "!");

  auto& [sub_assignable_node, flow_control_node_container] = variety_data;
  auto not_operator_node =
      std::make_shared<MathematicalOperatorNode>(MathematicalOperation::kNot);
  bool check_result =
      not_operator_node->SetLeftOperatorNode(sub_assignable_node);
  if (!check_result) [[unlikely]] {
    OutputError(std::format("�޷������߼�������"));
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
AssignableLogicalNegative(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data) {
  assert(str_operator == "~");

  auto& [sub_assignable_node, flow_control_node_container] = variety_data;
  auto logic_negative_node = std::make_shared<MathematicalOperatorNode>(
      MathematicalOperation::kLogicalNegative);
  bool check_result =
      logic_negative_node->SetLeftOperatorNode(sub_assignable_node);
  if (!check_result) [[unlikely]] {
    OutputError(std::format("�޷����а�λȡ������"));
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  check_result = flow_control_node->SetSentenceOperateNode(logic_negative_node);
  assert(check_result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(logic_negative_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableMathematicalNegative(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data) {
  assert(str_operator == "-");

  auto& [sub_assignable_node, flow_control_node_container] = variety_data;
  auto math_negative_node = std::make_shared<MathematicalOperatorNode>(
      MathematicalOperation::kMathematicalNegative);
  bool check_result =
      math_negative_node->SetLeftOperatorNode(sub_assignable_node);
  if (!check_result) [[unlikely]] {
    OutputError(std::format("�޷�����ȡ������"));
    exit(-1);
  }
  auto flow_control_node = std::make_unique<SimpleSentence>();
  check_result = flow_control_node->SetSentenceOperateNode(math_negative_node);
  assert(check_result);
  flow_control_node_container->emplace_back(std::move(flow_control_node));
  // ��������õ��Ľڵ����������ڵ�
  return std::make_pair(math_negative_node->GetResultOperatorNode(),
                        std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableObtainAddress(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data) {
  assert(str_operator == "&");

  auto& [sub_assignable_node, flow_control_node_container] = variety_data;
  auto node_to_obtain_address =
      std::static_pointer_cast<const VarietyOperatorNode>(sub_assignable_node);
  // ��鱻ȡ��ַ�Ľڵ�
  if (node_to_obtain_address->GetGeneralOperatorType() !=
      GeneralOperationType::kVariety) [[unlikely]] {
    OutputError(std::format("�޷��ԷǱ�������ȡ��ַ"));
    exit(-1);
  } else if (node_to_obtain_address->GetLeftRightValueTag() !=
             LeftRightValueTag::kLeftValue) [[unlikely]] {
    // ��ȡ��ַ�Ľڵ�����ֵ
    OutputError(std::format("�޷�����ֵȡ��ַ"));
    exit(-1);
  }
  auto obtain_address_node = std::make_shared<ObtainAddressOperatorNode>();
  bool result =
      obtain_address_node->SetNodeToObtainAddress(node_to_obtain_address);
  if (!result) [[unlikely]] {
    OutputError(std::format("�޷�ȡ��ַ"));
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
AssignableDereference(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data) {
  assert(str_operator == "*");

  auto& [sub_assignable_node, flow_control_node_container] = variety_data;
  auto dereference_node = std::make_shared<DereferenceOperatorNode>();
  bool result = dereference_node->SetNodeToDereference(sub_assignable_node);
  if (!result) [[unlikely]] {
    OutputError(std::format("�޷�������"));
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
AssignableArrayAccess(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        array_data,
    std::string&& left_square_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        index_data,
    std::string&& right_square_bracket) {
  assert(left_square_bracket == "[");
  assert(right_square_bracket == "]");

  auto& [node_to_dereference, main_flow_control_node_container] = array_data;
  auto& [index_node, sub_flow_control_node_container] = index_data;
  // ������ָ���ַ��ƫ������ӵĽڵ�
  auto plus_operator_node =
      std::make_shared<MathematicalOperatorNode>(MathematicalOperation::kPlus);
  bool left_node_check_result =
      plus_operator_node->SetLeftOperatorNode(node_to_dereference);
  if (!left_node_check_result) [[unlikely]] {
    OutputError(std::format("�޷����Ѱַ"));
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
    OutputError(std::format("�޷��������飬����ʹ���˸������±�"));
    exit(-1);
  }
  // �ϲ���ȡindex�Ĳ�������������
  main_flow_control_node_container->splice(
      main_flow_control_node_container->end(),
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
    OutputError(std::format("�޷��Ը�����ʹ��++�����"));
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
      std::string(), ConstTag::kNonConst, LeftRightValueTag::kRightValue);
  bool set_temp_variety_type_result = temp_variety_node->SetVarietyType(
      node_to_operate->GetResultTypePointer());
  assert(set_temp_variety_type_result);
  // �������ƽڵ㣬����Դ�ڵ㵽��ʱ����
  auto copy_assign_operator_node = std::make_shared<AssignOperatorNode>();
  copy_assign_operator_node->SetNodeToBeAssigned(temp_variety_node);
  auto copy_assign_node_check_result =
      copy_assign_operator_node->SetNodeForAssign(node_to_operate, true);
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
AssignablePrefixPlus(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data) {
  assert(str_operator == "++");

  auto& [node_to_plus, flow_control_node_container] = variety_data;
  return std::make_pair(
      PrefixPlusOrMinus(MathematicalOperation::kPlus, node_to_plus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignablePrefixMinus(
    std::string&& str_operator,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data) {
  assert(str_operator == "--");

  auto& [node_to_minus, flow_control_node_container] = variety_data;
  return std::make_pair(
      PrefixPlusOrMinus(MathematicalOperation::kMinus, node_to_minus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixPlus(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_operator) {
  assert(str_operator == "++");

  auto& [node_to_plus, flow_control_node_container] = variety_data;
  return std::make_pair(
      SuffixPlusOrMinus(MathematicalOperation::kPlus, node_to_plus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
AssignableSuffixMinus(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_operator) {
  assert(str_operator == "--");
  auto& [node_to_plus, flow_control_node_container] = variety_data;
  return std::make_pair(
      SuffixPlusOrMinus(MathematicalOperation::kMinus, node_to_plus,
                        flow_control_node_container.get()),
      std::move(flow_control_node_container));
}

std::nullptr_t ReturnWithValue(
    std::string&& str_return,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        variety_data,
    std::string&& str_semicolon) {
  assert(str_return == "return");
  assert(str_semicolon == ";");

  auto& [return_target, sentences_to_get_assignable] = variety_data;
  auto return_flow_control_node = std::make_unique<Return>();
  auto active_function = c_parser_controller.GetActiveFunctionPointer();
  if (active_function == nullptr) [[unlikely]] {
    OutputError(std::format("��ǰ�����ں����ڣ��޷�����"));
    exit(-1);
  }
  return_flow_control_node->SetReturnValue(return_target);
  bool result =
      c_parser_controller.AddSentences(std::move(*sentences_to_get_assignable));
  if (!result) [[unlikely]] {
    OutputError(std::format("���ɷ���ֵ�����д��ڷǷ�����"));
    exit(-1);
  }
  result = c_parser_controller.AddSentence(std::move(return_flow_control_node));
  if (!result) [[unlikely]] {
    OutputError(std::format(
        "����{:}�����з���ֵ",
        active_function->GetFunctionTypeReference().GetFunctionName()));
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t ReturnWithoutValue(std::string&& str_return,
                                  std::string&& str_semicolon) {
  assert(str_return == "return");
  assert(str_semicolon == ";");

  auto sentences_to_get_assignable =
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>();
  auto return_flow_control_node = std::make_unique<Return>();
  auto active_function = c_parser_controller.GetActiveFunctionPointer();
  if (active_function == nullptr) [[unlikely]] {
    OutputError(std::format("��ǰ�����ں����ڣ��޷�����"));
    exit(-1);
  }
  return_flow_control_node->SetReturnValue(nullptr);
  bool result =
      c_parser_controller.AddSentences(std::move(*sentences_to_get_assignable));
  if (!result) [[unlikely]] {
    OutputError(std::format("���ɷ���ֵ�����д��ڷǷ�����"));
    exit(-1);
  }
  result = c_parser_controller.AddSentence(std::move(return_flow_control_node));
  if (!result) [[unlikely]] {
    OutputError(std::format(
        "����{:}�����з���ֵ",
        active_function->GetFunctionTypeReference().GetFunctionName()));
    exit(-1);
  }
  return nullptr;
}

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
        false_value) {
  assert(str_question_mark == "?");
  assert(str_colon == ":");

  auto& [temary_condition, condition_flow_control_node_container] = condition;
  auto& [temary_true_branch, true_branch_flow_control_node_container] =
      true_value;
  auto& [temary_false_branch, false_branch_flow_control_node_container] =
      false_value;
  // ������Ŀ������ڵ�
  auto temary_operator_node = std::make_shared<TemaryOperatorNode>();
  bool condition_check_result = temary_operator_node->SetBranchCondition(
      temary_condition, condition_flow_control_node_container);
  if (!condition_check_result) [[unlikely]] {
    OutputError(std::format("�޷�������������Ϊ��Ŀ���������"));
    exit(-1);
  }
  bool true_branch_check_result = temary_operator_node->SetTrueBranch(
      temary_true_branch, true_branch_flow_control_node_container);
  if (!true_branch_check_result) [[unlikely]] {
    OutputError(std::format("�޷�������������Ϊ��Ŀ��������֧���"));
    exit(-1);
  }
  bool false_branch_check_result = temary_operator_node->SetFalseBranch(
      temary_false_branch, false_branch_flow_control_node_container);
  if (!false_branch_check_result) [[unlikely]] {
    OutputError(std::format("�޷�������������Ϊ��Ŀ������ٷ�֧���"));
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

std::nullptr_t NotEmptyFunctionCallArgumentsBase(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data) {
  auto& [argument_node, sentences_to_get_argument] = value_data;
  AssignableCheckResult check_result =
      function_call_operator_node->AddFunctionCallArgument(
          argument_node, sentences_to_get_argument);
  CheckAssignableCheckResult(check_result);
  return nullptr;
}

std::nullptr_t NotEmptyFunctionCallArgumentsExtend(
    std::nullptr_t, std::string&& str_comma,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data) {
  assert(str_comma == ",");

  auto& [argument_node, sentences_to_get_argument] = value_data;
  AssignableCheckResult check_result =
      function_call_operator_node->AddFunctionCallArgument(
          argument_node, sentences_to_get_argument);
  CheckAssignableCheckResult(check_result);
  return nullptr;
}

std::nullptr_t FunctionCallArguments(std::nullptr_t) { return nullptr; }

std::pair<std::shared_ptr<FunctionCallOperatorNode>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCallInitAssignable(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        call_target,
    std::string&& left_bracket) {
  assert(left_bracket == "(");

  auto function_call_node = std::make_shared<FunctionCallOperatorNode>();
  // ����ȫ�ֱ������Ա���Ӻ�������ʵ��ʱʹ��
  function_call_operator_node = function_call_node;
  auto& [node_to_call, sentences_to_get_node_to_call] = call_target;
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
      OutputError(
          std::format("�������ú����Ķ���Ȳ��Ǻ�����Ҳ����һ�غ���ָ��"));
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
    result = function_call_operator_node->SetFunctionType(
        dereference_operator_node->GetResultOperatorNode());
    assert(result);
  } else {
    // �����ڵ����ֱ�ӵ���
    bool result = function_call_operator_node->SetFunctionType(node_to_call);
    assert(result);
  }

  return std::make_pair(std::move(function_call_node),
                        std::move(sentences_to_get_node_to_call));
}

std::pair<std::shared_ptr<FunctionCallOperatorNode>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCallInitId(std::string&& function_name,
                   std::string&& str_left_bracket) {
  assert(str_left_bracket == "(");

  auto function_call_node = std::make_shared<FunctionCallOperatorNode>();
  // ����ȫ�ֱ������Ա���Ӻ�������ʵ��ʱʹ��
  function_call_operator_node = function_call_node;
  auto [node_to_call, exist] =
      c_parser_controller.GetVarietyOrFunction(function_name);
  if (!exist) {
    OutputError(
        std::format("��������Ϊ{:}�ĺ�����һ������ָ��", function_name));
    exit(-1);
  }
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
      OutputError(
          std::format("�������ú����Ķ���Ȳ��Ǻ�����Ҳ����һ�غ���ָ��"));
      exit(-1);
    }
    // ��װ�����ýڵ�
    auto dereference_flow_control_node = std::make_unique<SimpleSentence>();
    result = dereference_flow_control_node->SetSentenceOperateNode(
        dereference_operator_node);
    assert(result);
    // ��ӽ����ýڵ�
    result = c_parser_controller.AddSentence(
        std::move(dereference_flow_control_node));
    assert(result);
    // ���õ��õĽڵ�����Ϊ�����ú�õ��ĺ�������
    result = function_call_operator_node->SetFunctionType(
        dereference_operator_node->GetResultOperatorNode());
    assert(result);
  } else {
    // �����ڵ����ֱ�ӵ���
    bool result = function_call_operator_node->SetFunctionType(node_to_call);
    assert(result);
  }

  return std::make_pair(
      std::move(function_call_node),
      std::make_shared<std::list<std::unique_ptr<FlowInterface>>>());
}

std::pair<std::shared_ptr<const OperatorNodeInterface>,
          std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>
FunctionCall(
    std::pair<std::shared_ptr<FunctionCallOperatorNode>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        function_call_data,
    std::nullptr_t, std::string&& right_bracket) {
  assert(right_bracket == ")");

  auto& [node_to_call, sentences_to_get_node_to_call] = function_call_data;
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

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& AssignablesBase(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data) {
  return std::move(value_data.second);
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& AssignablesExtend(
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&&
        main_control_node_container,
    std::string&& str_comma,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        value_data) {
  assert(str_comma == ",");

  // ��ǰAssignable�����Ŀ��ƽڵ������
  auto& [ignore_assignable, now_control_node_container] = value_data;
  // �ϲ�����������
  main_control_node_container->splice(main_control_node_container->end(),
                                      std::move(*now_control_node_container));
  return std::move(main_control_node_container);
}

std::shared_ptr<std::unique_ptr<Jmp>> Break(std::string&& str_break,
                                            std::string&& str_semicolon) {
  assert(str_break == "break");
  assert(str_semicolon == ";");

  auto& top_flow_control_sentence = static_cast<ConditionBlockInterface&>(
      c_parser_controller.GetTopFlowControlSentence());
  switch (top_flow_control_sentence.GetFlowType()) {
    case FlowType::kDoWhileSentence:
    case FlowType::kWhileSentence:
    case FlowType::kForSentence:
    case FlowType::kSwitchSentence:
      return std::make_shared<std::unique_ptr<Jmp>>(std::make_unique<Jmp>(
          top_flow_control_sentence.GetSentenceEndLabel()));
      break;
    default:
      OutputError(std::format("�޷�������for/while/do-while/switch���"));
      exit(-1);
      // ��ֹ����
      return std::shared_ptr<std::unique_ptr<Jmp>>();
      break;
  }
}

std::shared_ptr<std::unique_ptr<Jmp>> Continue(std::string&& str_continue,
                                               std::string&& str_semicolon) {
  assert(str_continue == "continue");
  assert(str_semicolon == ";");

  auto& top_flow_control_sentence = static_cast<LoopSentenceInterface&>(
      c_parser_controller.GetTopFlowControlSentence());
  switch (top_flow_control_sentence.GetFlowType()) {
    case FlowType::kDoWhileSentence:
    case FlowType::kWhileSentence:
    case FlowType::kForSentence:
      return std::make_shared<std::unique_ptr<Jmp>>(std::make_unique<Jmp>(
          top_flow_control_sentence.GetLoopMainBlockEndLabel()));
      break;
    default:
      OutputError(std::format("�޷��ڷ�for/while/do-while�����ʹ��continue"));
      exit(-1);
      // ��ֹ����
      return std::shared_ptr<std::unique_ptr<Jmp>>();
      break;
  }
}

std::nullptr_t SingleStatementIf(std::nullptr_t) { return nullptr; }

std::nullptr_t SingleStatementDoWhile(std::nullptr_t) { return nullptr; }

std::nullptr_t SingleStatementWhile(std::nullptr_t) { return nullptr; }

std::nullptr_t SingleStatementFor(std::nullptr_t) { return nullptr; }

std::nullptr_t SingleStatementSwitch(std::nullptr_t) { return nullptr; }

std::nullptr_t SingleStatementAssignable(
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        expression,
    std::string&& str_semicolon) {
  assert(str_semicolon == ";");

  auto& [ignore_assignable, flow_control_node_container] = expression;
  bool result =
      c_parser_controller.AddSentences(std::move(*flow_control_node_container));
  if (!result) [[unlikely]] {
    OutputError(std::format("����䲻Ӧ�����ڸ÷�Χ��"));
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t SingleStatementAnnounce(
    std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
               std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        announce_statement,
    std::string&& str_semicolon) {
  assert(str_semicolon == ";");

  auto& [ignore_type, ignore_const_tag, flow_control_node_container] =
      announce_statement;
  bool result =
      c_parser_controller.AddSentences(std::move(*flow_control_node_container));
  if (!result) [[unlikely]] {
    OutputError(std::format("����䲻Ӧ�����ڸ÷�Χ��"));
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t SingleStatementReturn(std::nullptr_t) { return nullptr; }

std::nullptr_t SingleStatementBreak(
    std::shared_ptr<std::unique_ptr<Jmp>>&& jmp_sentence) {
  bool result = c_parser_controller.AddSentence(std::move(*jmp_sentence));
  // ���б���Ӧ���ⲽ��Լǰ����
  assert(result);
  return nullptr;
}

std::nullptr_t SingleStatementContinue(
    std::shared_ptr<std::unique_ptr<Jmp>>&& jmp_sentence) {
  bool result = c_parser_controller.AddSentence(std::move(*jmp_sentence));
  // ���б���Ӧ���ⲽ��Լǰ����
  assert(result);
  return nullptr;
}

std::nullptr_t SingleStatementEmptyStatement(std::string&& str_semicolon) {
  assert(str_semicolon == ";");

  return nullptr;
}

std::nullptr_t IfCondition(
    std::string&& str_if, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket) {
  assert(str_if == "if");
  assert(str_left_bracket == "(");
  assert(str_right_bracket == ")");

  // if�����ͻ�ȡ�����Ĳ���
  auto& [if_condition, sentences_to_get_if_condition] = condition;
  auto if_flow_control_node = std::make_unique<IfSentence>();
  bool result = if_flow_control_node->SetCondition(
      if_condition, std::move(*sentences_to_get_if_condition));
  if (!result) [[unlikely]] {
    OutputError(std::format("�������޷���Ϊif�������"));
    exit(-1);
  }
  bool push_result = c_parser_controller.PushFlowControlSentence(
      std::move(if_flow_control_node));
  if (!push_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} ���̿����������ں��������ڲ�ʹ��",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t IfWithElse(std::nullptr_t, std::nullptr_t,
                          std::string str_else) {
  assert(str_else == "else");

  // ת��Ϊif-else���
  c_parser_controller.ConvertIfSentenceToIfElseSentence();
  return nullptr;
}

std::nullptr_t IfElseSence(std::nullptr_t, std::nullptr_t) {
  c_parser_controller.PopActionScope();
  return nullptr;
}

std::nullptr_t IfIfSentence(std::nullptr_t, std::nullptr_t) {
  c_parser_controller.PopActionScope();
  return nullptr;
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& ForRenewSentence(
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& expression) {
  return std::move(expression);
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&&
ForInitSentenceAssignables(
    std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>&& expression) {
  return std::move(expression);
}

std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
ForInitSentenceAnnounce(
    std::tuple<std::shared_ptr<const TypeInterface>, ConstTag,
               std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        announce_data) {
  auto& [ignore_extend_type, ignore_extend_const_tag,
         flow_control_node_container] = announce_data;
  return std::move(flow_control_node_container);
}

std::nullptr_t ForInitHead(std::string&& str_for) {
  assert(str_for == "for");

  bool push_result = c_parser_controller.PushFlowControlSentence(
      std::make_unique<ForSentence>());
  if (!push_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} ���̿����������ں��������ڲ�ʹ��",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return nullptr;
}

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
    std::string&& str_right_bracket) {
  assert(str_left_bracket == "(");
  assert(str_semicolon1 == ";");
  assert(str_semicolon2 == ";");
  assert(str_right_bracket == ")");

  auto& for_sentence = static_cast<ForSentence&>(
      c_parser_controller.GetTopFlowControlSentence());
  assert(for_sentence.GetFlowType() == FlowType::kForSentence);
  auto& [for_condition, sentences_to_get_for_condition] = condition;
  bool result =
      for_sentence.AddForInitSentences(std::move(*for_init_sentences));
  if (!result) [[unlikely]] {
    OutputError(std::format("��������޷���Ϊfor����ʼ������"));
    exit(-1);
  }
  result = for_sentence.SetCondition(
      for_condition, std::move(*sentences_to_get_for_condition));
  if (!result) [[unlikely]] {
    OutputError(std::format("��������޷���Ϊfor���ѭ������"));
    exit(-1);
  }
  result = for_sentence.AddForRenewSentences(std::move(*for_renew_sentences));
  if (!result) [[unlikely]] {
    OutputError(std::format("��������޷�����for����и���ѭ������"));
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t For(std::nullptr_t, std::nullptr_t) {
  c_parser_controller.PopActionScope();
  return nullptr;
}

std::nullptr_t WhileInitHead(
    std::string&& str_while, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket) {
  assert(str_while == "while");
  assert(str_left_bracket == "(");
  assert(str_right_bracket == ")");

  auto& [while_condition, sentences_to_get_condition] = condition;
  auto pointer_to_while_sentence =
      std::make_shared<std::unique_ptr<WhileSentence>>(
          std::make_unique<WhileSentence>());
  auto& while_sentence = *pointer_to_while_sentence;
  bool result = while_sentence->SetCondition(
      while_condition, std::move(*sentences_to_get_condition));
  if (!result) [[unlikely]] {
    OutputError(std::format("���������޷���Ϊwhileѭ���������"));
    exit(-1);
  }
  bool push_result =
      c_parser_controller.PushFlowControlSentence(std::move(while_sentence));
  if (!push_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} ���̿����������ں��������ڲ�ʹ��",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t While(std::nullptr_t, std::nullptr_t) {
  c_parser_controller.PopActionScope();
  return nullptr;
}

std::nullptr_t DoWhileInitHead(std::string&& str_do) {
  assert(str_do == "do");

  bool push_result = c_parser_controller.PushFlowControlSentence(
      std::make_unique<DoWhileSentence>());
  if (!push_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} ���̿����������ں��������ڲ�ʹ��",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t DoWhile(
    std::nullptr_t, std::nullptr_t, std::string&& str_while,
    std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket, std::string&& str_semicolon) {
  assert(str_while == "while");
  assert(str_left_bracket == "(");
  assert(str_right_bracket == ")");
  assert(str_semicolon == ";");

  auto& [assignable, sentences_to_get_assignable] = condition;
  DoWhileSentence& do_while_sentence = static_cast<DoWhileSentence&>(
      c_parser_controller.GetTopFlowControlSentence());
  assert(do_while_sentence.GetFlowType() == FlowType::kDoWhileSentence);
  // ����do-while�������
  do_while_sentence.SetCondition(assignable,
                                 std::move(*sentences_to_get_assignable));
  c_parser_controller.PopActionScope();
  return nullptr;
}

std::nullptr_t SwitchCaseSimple(
    std::string&& str_case,
    std::shared_ptr<BasicTypeInitializeOperatorNode>&& case_data,
    std::string&& str_colon) {
  assert(str_case == "case");
  assert(str_colon == ":");

  bool result = c_parser_controller.AddSwitchSimpleCase(case_data);
  if (!result) [[unlikely]] {
    OutputError(std::format(
        "�޷���Ӹ�����caseѡ������ǲ�λ��switch����ڻ�case�����Ѵ���"));
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t SwitchCaseDefault(std::string&& str_default,
                                 std::string&& str_colon) {
  assert(str_default == "default");
  assert(str_colon == ":");

  bool result = c_parser_controller.AddSwitchDefaultCase();
  if (!result) [[unlikely]] {
    OutputError(std::format(
        "�޷����default��ǩ�����ܲ�λ��switch����ڻ��Ѵ���default��ǩ"));
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t SingleSwitchStatementCase(std::nullptr_t) { return nullptr; }

std::nullptr_t SingleSwitchStatementStatements(std::nullptr_t) {
  return nullptr;
}

std::nullptr_t SwitchStatements(std::nullptr_t, std::nullptr_t) {
  return nullptr;
}

std::nullptr_t SwitchCondition(
    std::string&& str_switch, std::string&& str_left_bracket,
    std::pair<std::shared_ptr<const OperatorNodeInterface>,
              std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>>&&
        condition,
    std::string&& str_right_bracket) {
  assert(str_switch == "switch");
  assert(str_left_bracket == "(");
  assert(str_right_bracket == ")");

  auto& [assignable, sentences_to_get_assignable] = condition;
  auto switch_sentence = std::make_unique<SwitchSentence>();
  bool result = switch_sentence->SetCondition(
      assignable, std::move(*sentences_to_get_assignable));
  if (!result) [[unlikely]] {
    OutputError(std::format("���������޷���Ϊswitch��֧����"));
    exit(-1);
  }
  bool push_result =
      c_parser_controller.PushFlowControlSentence(std::move(switch_sentence));
  if (!push_result) [[unlikely]] {
    std::cerr << std::format(
                     "����{:} ����{:} ���̿����������ں��������ڲ�ʹ��",
                     GetLine(), GetColumn())
              << std::endl;
    exit(-1);
  }
  return nullptr;
}

std::nullptr_t Switch(std::nullptr_t, std::string&& str_left_curly_bracket,
                      std::nullptr_t, std::string&& str_right_curly_bracket) {
  assert(str_left_curly_bracket == "{");
  assert(str_right_curly_bracket == "}");

  return nullptr;
}

std::nullptr_t StatementsSingleStatement(std::nullptr_t, std::nullptr_t) {
  return nullptr;
}

std::nullptr_t StatementsLeftBrace(std::nullptr_t,
                                   std::string&& left_curly_bracket) {
  assert(left_curly_bracket == "{");
  c_parser_controller.AddActionScopeLevel();
  return nullptr;
}

std::nullptr_t StatementsBrace(std::nullptr_t, std::nullptr_t,
                               std::string&& right_curly_bracket) {
  assert(right_curly_bracket == "}");
  c_parser_controller.PopActionScope();
  return nullptr;
}
std::nullptr_t ProcessControlSentenceBodySingleStatement(std::nullptr_t) {
  return nullptr;
}
//
// std::any ProcessControlSentenceBodyLeftBrace(
//    std::vector<WordDataToUser>&& word_data) {
//  assert(word_data.size() == 1);
//  return std::any();
//}

std::nullptr_t ProcessControlSentenceBodyStatements(
    std::string&& str_left_curly_bracket, std::nullptr_t,
    std::string&& str_right_curly_bracket) {
  assert(str_left_curly_bracket == "{");
  assert(str_right_curly_bracket == "}");

  return nullptr;
}

std::nullptr_t RootFunctionDefine(std::nullptr_t, std::nullptr_t) {
  return nullptr;
}

std::nullptr_t RootAnnounce(std::nullptr_t,
                            std::shared_ptr<FlowInterface>&& flow_control_node,
                            std::string&& str_colon) {
  assert(str_colon == ";");

  switch (flow_control_node->GetFlowType()) {
    case FlowType::kSimpleSentence: {
      // ȫ�ֱ�������
      auto variety_operator_node =
          std::static_pointer_cast<VarietyOperatorNode>(
              static_cast<SimpleSentence&>(*flow_control_node)
                  .GetSentenceOperateNodePointer());
      // ��ӱ�������
      c_parser_controller.DefineVariety(variety_operator_node);
    } break;
    case FlowType::kFunctionDefine: {
      // ��������
      auto [ignore_iter, announce_result] =
          c_parser_controller.AnnounceFunction(
              static_cast<c_parser_frontend::flow_control::FunctionDefine&>(
                  *flow_control_node)
                  .GetFunctionTypePointer());
      CheckAddTypeResult(announce_result);
    } break;
    default:
      assert(false);
      break;
  }
  return nullptr;
}

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
                  ->GetArraySize() != 0 &&
          next_node->GetType() == StructOrBasicType::kFunction) [[unlikely]] {
        // ��0������������
        OutputError(std::format("��֧��������������"));
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

std::pair<std::unique_ptr<FlowInterface>, ObjectConstructData::CheckResult>
ObjectConstructData::ConstructObject(
    ConstTag const_tag_before_final_type,
    std::shared_ptr<const TypeInterface>&& final_node_to_attach) {
  switch (type_chain_tail_->GetType()) {
    case StructOrBasicType::kPointer: {
      // ������ĩ��Ϊָ�룬���ø�ָ���const_tag
      auto& pointer_type = static_cast<PointerType&>(*type_chain_tail_);
      if (pointer_type.GetConstTag() == ConstTag::kConst &&
          const_tag_before_final_type == ConstTag::kConst) [[unlikely]] {
        OutputWarning(std::format("�����������ظ�����const"));
      }
      pointer_type.SetConstTag(const_tag_before_final_type);
    } break;
    case StructOrBasicType::kFunction:
      // ��������º�������ֵ����ֻ��һ����Ч���ͽڵ㣨final_node_to_attach��
      // �Ϸ�������������ڵ�Ӧ���ǻ�������
      static_cast<FunctionType&>(*type_chain_tail_)
          .SetReturnTypeConstTag(const_tag_before_final_type);
      break;
    case StructOrBasicType::kEnd:
      // ֮ǰδ�����κ����ͽڵ�
      assert(static_cast<SimpleSentence&>(*object_)
                 .GetSentenceOperateNodeReference()
                 .GetGeneralOperatorType() == GeneralOperationType::kVariety);
      switch (final_node_to_attach->GetType()) {
        case StructOrBasicType::kBasic:
          // ����POD����
          // ��������void���͵ı���
          if (static_cast<const BasicType&>(*final_node_to_attach)
                  .GetBuiltInType() == BuiltInType::kVoid) [[unlikely]] {
            OutputError(
                std::format("����{:}��������Ϊ\"void\"", GetObjectName()));
            exit(-1);
          }
          // ��������ı�����ConstTag�����չ���ʱ��ConstTag�Ƿ���ͬ
          if (const_tag_before_final_type == ConstTag::kConst &&
              static_cast<const VarietyOperatorNode&>(
                  static_cast<SimpleSentence&>(*object_)
                      .GetSentenceOperateNodeReference())
                      .GetConstTag() == const_tag_before_final_type)
              [[unlikely]] {
            OutputWarning(std::format("�����������ظ�����const"));
          }
          break;
        case StructOrBasicType::kFunction:
          OutputError(std::format("����{:}δ��������ֵ", GetObjectName()));
          exit(-1);
          break;
        case StructOrBasicType::kStruct:
        case StructOrBasicType::kUnion:
        case StructOrBasicType::kEnum:
          break;
        [[unlikely]] default:
          assert(false);
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
    [[unlikely]] case StructOrBasicType::kPointer:
      // δ��ȫ����������������������ָ���β
      return std::make_pair(std::unique_ptr<FlowInterface>(),
                            CheckResult::kPointerEnd);
      break;
    [[unlikely]] case StructOrBasicType::kEnd:
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

    // ��ȡ����������
    // Ϊ�˹���������Υ��constԭ��
    auto function_type_chain = std::const_pointer_cast<FunctionType>(
        std::static_pointer_cast<const FunctionType>(final_type_chain));
    // ��ȡ������ָ������ú�����
    function_type_chain->SetFunctionName(std::move(GetObjectName()));
    // �����������壨FunctionDefine���̣�
    ConstructBasicObjectPart<c_parser_frontend::flow_control::FunctionDefine>(
        std::move(function_type_chain));
  } else {
    // Ҫ����������Ϊ�������������ת��
    auto& variety_node = static_cast<VarietyOperatorNode&>(
        static_cast<SimpleSentence&>(*object_)
            .GetSentenceOperateNodeReference());
    // ���ñ���������
    variety_node.SetVarietyType(
        std::move(type_chain_head_->GetNextNodePointer()));
    // ���ñ�����
    variety_node.SetVarietyName(std::move(GetObjectName()));
  }
  return std::make_pair(std::move(object_), CheckResult::kSuccess);
}

}  // namespace c_parser_frontend::parse_functions