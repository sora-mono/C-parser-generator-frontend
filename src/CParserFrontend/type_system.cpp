#include "type_system.h"

namespace c_parser_frontend::type_system {
BuiltInType CalculateBuiltInType(const std::string& value) {
  // ת��������ת�����ַ���
  size_t converted_characters;
  uint64_t integer_value = std::stoull(value, &converted_characters);
  if (converted_characters == value.size()) {
    // ȫ��ת����value��������
    if (integer_value <= UINT16_MAX) {
      if (integer_value <= UINT8_MAX) {
        return BuiltInType::kInt8;
      } else {
        return BuiltInType::kInt16;
      }
    } else {
      if (integer_value <= UINT32_MAX) {
        return BuiltInType::kInt32;
      } else {
        // C���Բ�֧��long long
        return BuiltInType::kVoid;
      }
    }
  } else {
    // δȫ��ת���������Ǹ�����Ҳ�����ǳ������֧����ֵ��С
    long double float_value = std::stold(value, &converted_characters);
    if (converted_characters != value.size()) [[unlikely]] {
      // �������֧����ֵ��С
      return BuiltInType::kVoid;
    }
    if (float_value <= FLT_MAX) {
      return BuiltInType::kFloat32;
    } else if (float_value <= DBL_MAX) {
      return BuiltInType::kFloat64;
    } else {
      // C���Բ�֧��long double
      return BuiltInType::kVoid;
    }
  }
}
bool operator==(const std::shared_ptr<TypeInterface>& type_interface1,
                const std::shared_ptr<TypeInterface>& type_interface2) {
  return *type_interface1 == *type_interface2;
}

std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult>
TypeSystem::GetType(const std::string& type_name,
                    StructOrBasicType type_prefer) {
  auto iter = GetTypeNameToNode().find(type_name);
  if (iter == GetTypeNameToNode().end()) [[unlikely]] {
    return std::make_pair(std::shared_ptr<TypeInterface>(),
                          GetTypeResult::kTypeNameNotFound);
  }
  return iter->second.GetType(type_prefer);
}

bool TypeInterface::operator==(const TypeInterface& type_interface) const {
  if (IsSameObject(type_interface)) [[likely]] {
    // �κ������඼Ӧ�ȵ�����ͼ��̳����IsSameObject������operator==()
    // ���������������StructOrBasicType::kEnd��β
    // ��Ӧ��EndType���operator==()ֱ�ӷ���true���ս������
    return GetNextNodeReference() == type_interface.GetNextNodeReference();
  } else {
    return false;
  }
}
bool BasicType::operator==(const TypeInterface& type_interface) const {
  if (IsSameObject(type_interface)) [[likely]] {
    return GetNextNodeReference() == type_interface.GetNextNodeReference();
  } else {
    return false;
  }
}

inline AssignableCheckResult BasicType::CanBeAssignedBy(
    const TypeInterface& type_interface) const {
  // �������������ֵ������
  switch (type_interface.GetType()) {
    // ��������ֻ���û������͸�ֵ
    // c�����в���ʹ�ó�ʼ���б��ʼ���������ͣ����ɣ�
    case StructOrBasicType::kBasic:
      break;
    case StructOrBasicType::kNotSpecified:
      // �����ͽ��ɳ����ڸ�����������ȡ���Ͳ�����
      assert(false);
      break;
    case StructOrBasicType::kInitializeList:
      // ��ʼ���б�����operator_node����
      return AssignableCheckResult::kInitializeList;
      break;
    default:
      return AssignableCheckResult::kCanNotConvert;
      break;
  }
  // ���ת������
  const BasicType& basic_type = static_cast<const BasicType&>(type_interface);
  // ����������
  if (GetSignTag() != basic_type.GetSignTag()) [[unlikely]] {
    if (GetSignTag() == SignTag::kSigned) {
      return AssignableCheckResult::kUnsignedToSigned;
    } else {
      return AssignableCheckResult::kSignedToUnsigned;
    }
  }
  int differ = static_cast<int>(GetBuiltInType()) -
               static_cast<int>(basic_type.GetBuiltInType());
  if (differ > 0) {
    // �������ȼ��ͣ�Ӧ��ö��������ת��
    return AssignableCheckResult::kUpperConvert;
  } else if (differ == 0) {
    // ������ö���������ͬ������ת��
    return AssignableCheckResult::kNonConvert;
  } else {
    // �������ȼ��ߣ�Ӧ���������ת��
    return AssignableCheckResult::kLowerConvert;
  }
}

inline size_t BasicType::GetTypeStoreSize() const {
  switch (GetBuiltInType()) {
    case BuiltInType::kInt1:
      // x86��bool����ҲҪʹ��1���ֽ�
      return 1;
      break;
    case BuiltInType::kInt8:
      return 1;
      break;
    case BuiltInType::kInt16:
      return 2;
      break;
    case BuiltInType::kInt32:
      return 4;
      break;
    case BuiltInType::kFloat32:
      return 4;
      break;
    case BuiltInType::kFloat64:
      return 8;
      break;
    case BuiltInType::kVoid:
      // ����voidָ��
      return 4;
      break;
    default:
      assert(false);
      // ��ֹ����
      return size_t();
      break;
  }
}

inline bool BasicType::IsSameObject(const TypeInterface& type_interface) const {
  // this == &basic_type���Ż��ֶΣ�����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  const BasicType& basic_type = static_cast<const BasicType&>(type_interface);
  return this == &type_interface ||
         TypeInterface::IsSameObject(type_interface) &&
             GetBuiltInType() == basic_type.GetBuiltInType() &&
             GetSignTag() == basic_type.GetSignTag();
}

bool PointerType::operator==(const TypeInterface& type_interface) const {
  if (IsSameObject(type_interface)) [[likely]] {
    return GetNextNodeReference() == type_interface.GetNextNodeReference();
  } else {
    return false;
  }
};
bool FunctionType::operator==(const TypeInterface& type_interface) const {
  if (IsSameObject(type_interface)) [[likely]] {
    return GetNextNodeReference() == type_interface.GetNextNodeReference();
  } else {
    return false;
  }
}
AssignableCheckResult FunctionType::CanBeAssignedBy(
    const TypeInterface& type_interface) const {
  // �ڸ�ֵ������ָ��Ĺ����е���
  // ��������ֻ��ʹ�ú�����ֵ
  // ���������������Ƿ���ͬ
  if (TypeInterface::IsSameObject(type_interface)) [[likely]] {
    // ��麯��ǩ���Ƿ���ͬ
    if (IsSameSignature(static_cast<const FunctionType&>(type_interface)))
        [[likely]] {
      return AssignableCheckResult::kNonConvert;
    }
  }
  return AssignableCheckResult::kCanNotConvert;
}

// �����Ƿ�Ϊ�������������������в������κ����̿�����䣩

// ����������Ƿ������Ϊ�����ڳ��ֵ����

AssignableCheckResult PointerType::CanBeAssignedBy(
    const TypeInterface& type_interface) const {
  // �������������ֵ������
  switch (type_interface.GetType()) {
    case StructOrBasicType::kBasic:
      // ����Ǹ�������һ������ת��Ϊָ��
      // ����Ƿ�Ϊ����
      if (static_cast<const BasicType&>(type_interface).GetBuiltInType() <
          BuiltInType::kFloat32) [[likely]] {
        // ����ֵΪ0ʱ���Ը�ֵ��ָ��
        return AssignableCheckResult::kMayBeZeroToPointer;
      } else {
        return AssignableCheckResult::kCanNotConvert;
      }
      break;
    case StructOrBasicType::kPointer:
      break;
    case StructOrBasicType::kNotSpecified:
      // �����ͽ��ɳ����ڸ�����������ȡ���Ͳ�����
      assert(false);
      break;
    case StructOrBasicType::kFunction:
      // ����������Ϊһ������ָ��Դ�
      {
        // ����Ƿ���һ���ڵ�Ϊ�����ڵ�
        if (GetNextNodeReference().GetType() == StructOrBasicType::kFunction)
            [[likely]] {
          return GetNextNodeReference().CanBeAssignedBy(type_interface);
        } else {
          // ������ֻ����Ϊһ�غ���ָ��ʹ��
          return AssignableCheckResult::kCanNotConvert;
        }
      }
      break;
    case StructOrBasicType::kInitializeList:
      // ��ʼ���б���operator_node�м��
      return AssignableCheckResult::kInitializeList;
      break;
    default:
      return AssignableCheckResult::kCanNotConvert;
      break;
  }
  // ���const���
  const PointerType& pointer_type =
      static_cast<const PointerType&>(type_interface);
  switch (static_cast<int>(GetConstTag()) -
          static_cast<int>(pointer_type.GetConstTag())) {
    case -1:
      [[unlikely]]
      // ����ֵ������Ϊ��const��������ֵ����const
      // ��������²����Ը�ֵ
      return AssignableCheckResult::kAssignedNodeIsConst;
      break;
    case 0:
    case 1:
      // �������ֿ��Ը�ֵ�����
      break;
    default:
      assert(false);
  }
  auto assignable_check_result = GetNextNodeReference().CanBeAssignedBy(
      type_interface.GetNextNodeReference());
  // ����Ƿ�Ϊ��ָ�븳ֵ��ͬ��ά����voidָ��Ĺ���
  if (assignable_check_result == AssignableCheckResult::kCanNotConvert)
      [[unlikely]] {
    if (GetNextNodeReference() ==
        *CommonlyUsedTypeGenerator::GetBasicType<BuiltInType::kVoid,
                                                 SignTag::kUnsigned>())
        [[likely]] {
      // ����ֵ�Ķ���Ϊvoidָ�룬��ֵ�Ķ���Ϊͬ��ά��ָ��
      assignable_check_result = AssignableCheckResult::kConvertToVoidPointer;
    }
  }
  return assignable_check_result;
}

size_t PointerType::TypeSizeOf() const {
  size_t array_size = GetArraySize();
  assert(array_size != -1);
  if (array_size == 0) {
    // ��ָ�룬ֱ�ӷ���ָ���С
    return PointerType::GetTypeStoreSize();
  } else {
    // ָ�������ָ�룬��ά�ȴ�С��ˣ������Ի�����Ԫ��С
    return array_size * GetNextNodeReference().TypeSizeOf();
  }
}

AssignableCheckResult StructType::CanBeAssignedBy(
    const TypeInterface& type_interface) const {
  if (type_interface.GetType() == StructOrBasicType::kInitializeList)
      [[likely]] {
    // ��ֵ�õ�����Ϊ��ʼ���б�����opreator_node����
    return AssignableCheckResult::kInitializeList;
  } else {
    return *this == type_interface ? AssignableCheckResult::kNonConvert
                                   : AssignableCheckResult::kCanNotConvert;
  }
}

size_t StructType::GetTypeStoreSize() const {
  // ɨ��ṹ���Ի�ȡ���ĳ�Ա
  const auto& struct_members = GetStructureMembers();
  // C���Կսṹ���СΪ0
  // C++�սṹ���СΪ1
  size_t biggest_member_size = 0;
  for (const auto& member : struct_members) {
    size_t member_size = member.first->GetTypeStoreSize();
    if (member_size > biggest_member_size) {
      biggest_member_size = member_size;
    }
  }
  // ��ȡϵͳ�����ֽڵ�λ��ṹ������Ա����Сֵ��Ϊ�ֽڶ���ĵ�λ
  size_t aligen_size = std::min(size_t(4), biggest_member_size);
  // C���Կսṹ���СΪ0
  size_t struct_size = 0;
  // ��д�ṹ���С
  for (const auto& member : struct_members) {
    // �������ʣ��Ŀռ�
    size_t rest_space_to_member_align = struct_size % aligen_size;
    // �洢�ó�Ա��Ҫ�Ŀռ�
    size_t type_store_size = member.first->TypeSizeOf();
    // ����ó�Ա����ռ���ھ������Ŀռ䣬���ⲿ�ֿռ���Ϊ���ռ�
    if (rest_space_to_member_align < type_store_size) {
      struct_size += aligen_size - rest_space_to_member_align;
    }
    // �Ѿ����룬���Ӵ洢����Ŀռ�
    struct_size += type_store_size;
  }
  return struct_size;
}

AssignableCheckResult UnionType::CanBeAssignedBy(
    const TypeInterface& type_interface) const {
  if (type_interface.GetType() == StructOrBasicType::kInitializeList)
      [[unlikely]] {
    return AssignableCheckResult::kInitializeList;
  } else {
    return *this == type_interface ? AssignableCheckResult::kNonConvert
                                   : AssignableCheckResult::kCanNotConvert;
  }
}

size_t UnionType::GetTypeStoreSize() const {
  // ɨ�蹲�����Ի�ȡ���ĳ�Ա
  const auto& union_members = GetStructureMembers();
  // C���Կչ������СΪ0
  // C++�չ������СΪ1
  size_t biggest_member_size = 0;
  for (const auto& member : union_members) {
    size_t member_size = member.first->TypeSizeOf();
    if (member_size > biggest_member_size) {
      biggest_member_size = member_size;
    }
  }
  // ��ȡϵͳ�����ֽڵ�λ��ṹ������Ա����Сֵ��Ϊ�ֽڶ���ĵ�λ
  size_t aligen_size = std::min(size_t(4), biggest_member_size);
  size_t rest_space_to_aligen = biggest_member_size % aligen_size;
  if (rest_space_to_aligen != 0) {
    return biggest_member_size + aligen_size - rest_space_to_aligen;
  } else {
    return biggest_member_size;
  }
}

AssignableCheckResult EnumType::CanBeAssignedBy(
    const TypeInterface& type_interface) const {
  if (type_interface.GetType() == StructOrBasicType::kInitializeList)
      [[unlikely]] {
    return AssignableCheckResult::kInitializeList;
  } else {
    return *this == type_interface ? AssignableCheckResult::kNonConvert
                                   : AssignableCheckResult::kCanNotConvert;
  }
}

bool EnumType::IsSameObject(const TypeInterface& type_interface) const {
  // this == &type_interface���Ż��ֶ�
  // ����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  if (this == &type_interface) [[likely]] {
    // ���߹���һ���ڵ�
    return true;
  } else if (TypeInterface::IsSameObject(type_interface)) [[likely]] {
    const EnumType& enum_type = static_cast<const EnumType&>(type_interface);
    const std::string& enum_name_this = GetEnumName();
    const std::string& enum_name_argument = enum_type.GetEnumName();
    if (!enum_name_this.empty()) [[likely]] {
      if (!enum_name_argument.empty()) [[likely]] {
        // ���߶��Ǿ���ö��
        // C�����о���ö�ٸ�����������
        return enum_name_this == enum_name_argument;
      }
    }
    // ���߶�������ö��
    // �����ͬһ��ö������Ӧ���ڵ�һ��if����
  }
  return false;
}

bool InitializeListType::IsSameObject(
    const TypeInterface& type_interface) const {
  // this == &type_interface���Ż��ֶ�
  // ����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  return this == &type_interface ||
         TypeInterface::IsSameObject(type_interface) &&
             GetListTypes() ==
                 static_cast<const InitializeListType&>(type_interface)
                     .GetListTypes();
}

bool PointerType::IsSameObject(const TypeInterface& type_interface) const {
  // this == &basic_type���Ż��ֶΣ�����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  return this == &type_interface ||
         TypeInterface::IsSameObject(type_interface) &&
             GetConstTag() ==
                 static_cast<const PointerType&>(type_interface).GetConstTag();
}

std::shared_ptr<const BasicType>
CommonlyUsedTypeGenerator::GetBasicTypeNotTemplate(BuiltInType built_in_type,
                                                   SignTag sign_tag) {
  switch (built_in_type) {
    case BuiltInType::kVoid:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kVoid, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kVoid, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kInt1:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kInt1, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kInt1, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kInt8:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kInt8, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kInt8, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kInt16:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kInt16, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kInt16, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kInt32:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kInt32, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kInt32, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kFloat32:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kFloat32, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kFloat32, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kFloat64:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kFloat64, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kFloat64, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    default:
      assert(false);
      break;
  }
  // ��ֹ����
  return GetBasicType<BuiltInType::kVoid, SignTag::kUnsigned>();
}

std::pair<std::shared_ptr<const TypeInterface>,
          DeclineMathematicalComputeTypeResult>
TypeInterface::DeclineMathematicalComputeResult(
    const std::shared_ptr<const TypeInterface>& left_compute_type,
    const std::shared_ptr<const TypeInterface>& right_compute_type) {
  switch (left_compute_type->GetType()) {
    case StructOrBasicType::kBasic: {
      switch (right_compute_type->GetType()) {
        case StructOrBasicType::kBasic:
          if (static_cast<long long>(left_compute_type->GetType()) >=
              static_cast<long long>(right_compute_type->GetType())) {
            // �Ҳ��������������
            return std::make_pair(
                left_compute_type,
                DeclineMathematicalComputeTypeResult::kConvertToLeft);
          } else {
            // ��������������Ҳ�
            return std::make_pair(
                right_compute_type,
                DeclineMathematicalComputeTypeResult::kConvertToRight);
          }
          break;
        case StructOrBasicType::kPointer:
          // ָ����ƫ�������
          if (static_cast<long long>(left_compute_type->GetType()) <
              static_cast<long long>(BuiltInType::kFloat32)) [[likely]] {
            // ƫ���������ͣ�������Ϊָ���ƫ����
            return std::make_pair(
                right_compute_type,
                DeclineMathematicalComputeTypeResult::kLeftOffsetRightPointer);
          } else {
            // ���������͵�ƫ������������ָ��
            return std::make_pair(
                std::shared_ptr<const TypeInterface>(),
                DeclineMathematicalComputeTypeResult::kLeftNotIntger);
          }
          break;
        default:
          return std::make_pair(
              std::shared_ptr<const TypeInterface>(),
              DeclineMathematicalComputeTypeResult::kRightNotComputableType);
          break;
      }
    } break;
    case StructOrBasicType::kPointer: {
      switch (right_compute_type->GetType()) {
        case StructOrBasicType::kBasic:
          // ָ����ƫ�������
          if (static_cast<long long>(right_compute_type->GetType()) <
              static_cast<long long>(BuiltInType::kFloat32)) [[likely]] {
            // ƫ���������ͣ�������Ϊָ���ƫ����
            return std::make_pair(
                left_compute_type,
                DeclineMathematicalComputeTypeResult::kLeftPointerRightOffset);
          } else {
            // ���������͵�ƫ������������ָ��
            return std::make_pair(
                std::shared_ptr<const TypeInterface>(),
                DeclineMathematicalComputeTypeResult::kRightNotIntger);
          }
          break;
        case StructOrBasicType::kPointer:
          // ָ����ָ�����
          return std::make_pair(
              std::shared_ptr<const TypeInterface>(),
              DeclineMathematicalComputeTypeResult::kLeftRightBothPointer);
          break;
        default:
          return std::make_pair(
              std::shared_ptr<const TypeInterface>(),
              DeclineMathematicalComputeTypeResult::kRightNotComputableType);
          break;
      }
    } break;
    default:
      return std::make_pair(
          std::shared_ptr<const TypeInterface>(),
          DeclineMathematicalComputeTypeResult::kLeftNotComputableType);
      break;
  }
}

AddTypeResult TypeSystem::TypeData::AddType(
    const std::shared_ptr<const TypeInterface>& type_to_add) {
  std::monostate* empty_status_pointer =
      std::get_if<std::monostate>(&type_data_);
  if (empty_status_pointer != nullptr) [[likely]] {
    // �ýڵ�δ�����κ�ָ��
    // �����һ��ָ��
    type_data_ = type_to_add;
    return AddTypeResult::kNew;
  } else {
    std::shared_ptr<const TypeInterface>* shared_pointer =
        std::get_if<std::shared_ptr<const TypeInterface>>(&type_data_);
    if (shared_pointer != nullptr) [[likely]] {
      // �ýڵ㱣����һ��ָ�룬����һ����תΪvector�洢
      // ���Ҫ��ӵ������Ƿ������������ظ������ظ����kPointer/kBasicһ��
      if (IsSameKind((*shared_pointer)->GetType(), type_to_add->GetType()))
          [[unlikely]] {
        // ����Ƿ�����Ӻ�������
        if (type_to_add->GetType() == StructOrBasicType::kFunction)
            [[unlikely]] {
          AddTypeResult function_define_add_result =
              CheckFunctionDefineAddResult(
                  static_cast<const FunctionType&>(**shared_pointer),
                  static_cast<const FunctionType&>(*type_to_add));
          if (function_define_add_result == AddTypeResult::kTypeAlreadyIn)
              [[likely]] {
            // �������ǩ����ͬӦ����AddTypeResult::kOverrideFunction
            // ʹ���µ�ָ���滻ԭ��ָ�룬�Ӷ����º���������
            *shared_pointer = type_to_add;
            function_define_add_result = AddTypeResult::kFunctionDefine;
          }
          return function_define_add_result;
        } else {
          // ����ӵ�����������������ͬ��������ͻ
          return AddTypeResult::kTypeAlreadyIn;
        }
      }
      auto pointers =
          std::make_unique<std::list<std::shared_ptr<const TypeInterface>>>();
      // ��StructOrBasicType::kBasic��StructOrBasicType::kPointer���ͷ�����ǰ��
      // �Ӷ��Ż��������͵��ٶ�
      if (IsSameKind(type_to_add->GetType(), StructOrBasicType::kBasic)) {
        pointers->emplace_back(type_to_add);
        pointers->emplace_back(std::move(*shared_pointer));
      } else {
        // ԭ�������Ͳ�һ�����ڸô��࣬������ӵ�����һ�������ڸô���
        pointers->emplace_back(std::move(*shared_pointer));
        pointers->emplace_back(type_to_add);
      }
      type_data_ = std::move(pointers);
      return AddTypeResult::kShiftToVector;
    } else {
      // �ýڵ��Ѿ�ʹ��list�洢
      std::unique_ptr<std::list<std::shared_ptr<const TypeInterface>>>&
          pointers = *std::get_if<
              std::unique_ptr<std::list<std::shared_ptr<const TypeInterface>>>>(
              &type_data_);
      assert(pointers != nullptr);
      // ������ӵ��������Ѵ��ڵ������Ƿ�����ͬһ����
      for (auto& stored_pointer : *pointers) {
        if (IsSameKind(stored_pointer->GetType(), type_to_add->GetType()))
            [[unlikely]] {
          // ����Ƿ�����Ӻ�������
          if (type_to_add->GetType() == StructOrBasicType::kFunction)
              [[unlikely]] {
            AddTypeResult function_define_add_result =
                CheckFunctionDefineAddResult(
                    static_cast<const FunctionType&>(*stored_pointer),
                    static_cast<const FunctionType&>(*type_to_add));
            if (function_define_add_result == AddTypeResult::kTypeAlreadyIn)
                [[likely]] {
              // �������Ϊ�ں�������ǰ����Ӻ�������
              // ʹ���µ�ָ���滻ԭ��ָ��
              stored_pointer = type_to_add;
              return AddTypeResult::kFunctionDefine;
            }
            return function_define_add_result;
          }
          // ����ӵ�����������������ͬ��������ͻ
          return AddTypeResult::kTypeAlreadyIn;
        }
      }
      pointers->emplace_back(type_to_add);
      if (IsSameKind(type_to_add->GetType(), StructOrBasicType::kBasic)) {
        // �²��������ΪStructOrBasicType::kBasic/kPointer
        // �������ͷŵ���һ��λ���Ա��Ż�������������ʱ�Ĳ����߼�
        std::swap(pointers->front(), pointers->back());
      }
      return AddTypeResult::kAddToVector;
    }
  }
}

std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult>
TypeSystem::TypeData::GetType(StructOrBasicType type_prefer) const {
  if (std::get_if<std::monostate>(&type_data_) != nullptr) [[unlikely]] {
    // δ�洢�κνڵ㣬��Ч�ڸ����ƶ�Ӧ�����Ͳ�����
    return std::make_pair(std::shared_ptr<TypeInterface>(),
                          GetTypeResult::kTypeNameNotFound);
  }
  const std::shared_ptr<const TypeInterface>* shared_pointer =
      std::get_if<std::shared_ptr<const TypeInterface>>(&type_data_);
  if (shared_pointer != nullptr) [[likely]] {
    // ���洢1��ָ��
    if (type_prefer == StructOrBasicType::kNotSpecified ||
        (*shared_pointer)->GetType() == type_prefer) [[likely]] {
      // ������ѡ�������������ѡ���������
      return std::make_pair(*shared_pointer, GetTypeResult::kSuccess);
    } else {
      // û��ƥ�������ѡ������
      return std::make_pair(std::shared_ptr<TypeInterface>(),
                            GetTypeResult::kNoMatchTypePrefer);
    }
  } else {
    // �洢���ָ��
    auto& pointers = *std::get_if<
        std::unique_ptr<std::list<std::shared_ptr<const TypeInterface>>>>(
        &type_data_);
    // �ж��Ƿ�ָ������ѡ������
    if (type_prefer == StructOrBasicType::kNotSpecified) [[likely]] {
      if (pointers->front()->GetType() == StructOrBasicType::kBasic) {
        // ����ƥ��kBasic
        return std::make_pair(pointers->front(), GetTypeResult::kSuccess);
      } else {
        // vector��û��kBasic������һ���洢��������ָ�룬��Щָ��ͬ��
        return std::make_pair(std::shared_ptr<const TypeInterface>(),
                              GetTypeResult::kSeveralSameLevelMatches);
      }
    } else {
      // ����vector�������������ѡ��������ͬ��ָ��
      for (auto& pointer : *pointers) {
        if (pointer->GetType() == type_prefer) [[unlikely]] {
          return std::make_pair(pointer, GetTypeResult::kSuccess);
        }
      }
      // û��ƥ�������ѡ������
      return std::make_pair(std::shared_ptr<const TypeInterface>(),
                            GetTypeResult::kNoMatchTypePrefer);
    }
  }
}

bool TypeSystem::TypeData::IsSameKind(StructOrBasicType type1,
                                      StructOrBasicType type2) {
  unsigned long long type1_ = static_cast<unsigned long long>(type1);
  unsigned long long type2_ = static_cast<unsigned long long>(type2);
  constexpr unsigned long long kBasicType =
      static_cast<unsigned long long>(StructOrBasicType::kBasic);
  constexpr unsigned long long kPointerType =
      static_cast<unsigned long long>(StructOrBasicType::kPointer);
  // �ڶ����ֵ�������������Ƿ������kPointer/kBasic
  return type1 == type2 || ((type1_ ^ kBasicType) |
                            (type1_ ^ kPointerType) & (type2_ ^ kBasicType) |
                            (type2_ ^ kPointerType));
}
StructureTypeInterface::StructureMemberContainer::MemberIndex
StructureTypeInterface::StructureMemberContainer::GetMemberIndex(
    const std::string& member_name) const {
  auto iter = member_name_to_index_.find(member_name);
  if (iter != member_name_to_index_.end()) [[likely]] {
    return iter->second;
  } else {
    return MemberIndex::InvalidId();
  }
}

bool StructureTypeInterface::IsSameObject(
    const TypeInterface& type_interface) const {
  // this == &type_interface���Ż��ֶ�
  // ����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  if (this == &type_interface) [[likely]] {
    // ���߹���һ���ڵ�
    return true;
  } else if (TypeInterface::IsSameObject(type_interface)) [[likely]] {
    const StructureTypeInterface& structure_type =
        static_cast<const StructType&>(type_interface);
    const std::string& structure_name_this = GetStructureName();
    const std::string& structure_name_another =
        structure_type.GetStructureName();
    if (!structure_name_this.empty()) [[likely]] {
      if (!structure_name_another.empty()) [[likely]] {
        // ���߶��Ǿ����ṹ
        // C�����о����ṹ������������
        return structure_name_this == structure_name_another;
      }
    }
    // ���߶��������ṹ
    // ��������ṹ��ͬ�����ʹ����ͬ������ָ�룬�ڵ�һ��if�ͷ���
  }
  return false;
}

}  // namespace c_parser_frontend::type_system
