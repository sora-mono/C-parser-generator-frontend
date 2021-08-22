#include "type_system.h"

namespace c_parser_frontend::type_system {
bool operator==(const std::shared_ptr<TypeInterface>& type_interface1,
                const std::shared_ptr<TypeInterface>& type_interface2) {
  return *type_interface1 == *type_interface2;
}

std::pair<std::shared_ptr<const TypeInterface>, TypeSystem::GetTypeResult>
TypeSystem::GetType(const std::string& type_name,
                    StructOrBasicType type_prefer) {
  auto iter = GetTypeNameToNode().find(type_name);
  if (iter == GetTypeNameToNode().end()) [[unlikely]] {
    return std::make_pair(std::shared_ptr<TypeInterface>(),
                          GetTypeResult::kTypeNameNotFound);
  }
  std::shared_ptr<const TypeInterface>* shared_pointer =
      std::get_if<std::shared_ptr<const TypeInterface>>(&iter->second);
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
    auto& vector_pointer = *std::get_if<
        std::unique_ptr<std::vector<std::shared_ptr<const TypeInterface>>>>(
        &iter->second);
    // �ж��Ƿ�ָ������ѡ������
    if (type_prefer == StructOrBasicType::kNotSpecified) [[likely]] {
      if (vector_pointer->front()->GetType() == StructOrBasicType::kBasic) {
        // ����ƥ��kBasic
        return std::make_pair(vector_pointer->front(),
                              GetTypeResult::kSuccess);
      } else {
        // vector��û��kBasic������һ���洢��������ָ�룬��Щָ��ͬ��
        return std::make_pair(std::shared_ptr<TypeInterface>(),
                              GetTypeResult::kSeveralSameLevelMatches);
      }
    } else {
      // ����vector�������������ѡ��������ͬ��ָ��
      for (auto& pointer : *vector_pointer) {
        if (pointer->GetType() == type_prefer) [[unlikely]] {
          return std::make_pair(pointer, GetTypeResult::kSuccess);
        }
      }
      // û��ƥ�������ѡ������
      return std::make_pair(std::shared_ptr<TypeInterface>(),
                            GetTypeResult::kNoMatchTypePrefer);
    }
  }
}

inline AssignableCheckResult BasicType::CanBeAssignedBy(
    std::shared_ptr<const TypeInterface>&& type_interface) const {
  // �������������ֵ������
  switch (type_interface->GetType()) {
    // ��������ֻ���û������͸�ֵ
    // c�����в���ʹ�ó�ʼ���б��ʼ���������ͣ����ɣ�
    case StructOrBasicType::kBasic:
      break;
    case StructOrBasicType::kNotSpecified:
      // �����ͽ��ɳ����ڸ�����������ȡ���Ͳ�����
      assert(false);
      break;
    default:
      return AssignableCheckResult::kCanNotConvert;
      break;
  }
  // ���ת������
  const BasicType& basic_type = static_cast<const BasicType&>(*type_interface);
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

inline bool BasicType::IsSameObject(const TypeInterface& type_interface) const {
  // this == &basic_type���Ż��ֶΣ�����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  const BasicType& basic_type = static_cast<const BasicType&>(type_interface);
  return this == &type_interface ||
         TypeInterface::IsSameObject(type_interface) &&
             GetBuiltInType() == basic_type.GetBuiltInType() &&
             GetSignTag() == basic_type.GetSignTag();
}

inline bool FunctionType::IsSameObject(
    const TypeInterface& type_interface) const {
  // this == &type_interface���Ż��ֶ�
  // ����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  if (this == &type_interface) [[likely]] {
    return true;
  }
  if (TypeInterface::IsSameObject(type_interface)) [[likely]] {
    if (GetFunctionName().empty()) {
      // ����ָ���һ���ڵ㣬�������ÿ�
      // ����ָ����Ҫ�Ƚϲ����ͷ���ֵ
      const FunctionType& function_type =
          static_cast<const FunctionType&>(type_interface);
      return function_type.GetFunctionName().empty() &&
             GetArgumentTypes() == function_type.GetArgumentTypes() &&
             GetReturnTypeReference() == function_type.GetReturnTypeReference();
    } else {
      // ��������/��������/���������һ���ڵ�
      // C���Բ�֧�ֺ������أ�����ֻ���жϺ������Ƿ���ͬ
      return GetFunctionName() ==
             static_cast<const FunctionType&>(type_interface).GetFunctionName();
    }
  }
}

inline AssignableCheckResult PointerType::CanBeAssignedBy(
    std::shared_ptr<const TypeInterface>&& type_interface) const {
  // �������������ֵ������
  switch (type_interface->GetType()) {
    case StructOrBasicType::kBasic:
      // ����ֵΪ0ʱ���Ը�ֵ��ָ��
      return AssignableCheckResult::kMayBeZeroToPointer;
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
        // ��������ֵ�ĺ���ת��Ϊ����ָ����ʽ
        std::shared_ptr<const TypeInterface> function_pointer =
            FunctionType::ConvertToFunctionPointer(std::move(type_interface));
        // ��ʹ��β�ݹ��Ż���ֹfunction_pointer����ǰ�ͷ�
        AssignableCheckResult check_result = CanBeAssignedBy(
            std::shared_ptr<const TypeInterface>(function_pointer));
        return check_result;
      }
      break;
    default:
      return AssignableCheckResult::kCanNotConvert;
      break;
  }
  // ���const���
  const PointerType& pointer_type =
      static_cast<const PointerType&>(*type_interface);
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
  switch (GetNextNodeReference().GetType()) {
    case StructOrBasicType::kPointer:
      // ��һ���ڵ���ָ�룬������Ƚ�
      return GetNextNodeReference().CanBeAssignedBy(
          type_interface->GetNextNodePointer());
    default:
      // ��һ���ڵ㲻��ָ����Ƚ��Ƿ�Ϊ��ͬ����
      if (GetNextNodeReference() == type_interface->GetNextNodeReference())
          [[likely]] {
        return AssignableCheckResult::kNonConvert;
      } else if (type_interface->GetNextNodeReference().GetType() !=
                     StructOrBasicType::kPointer &&
                 GetNextNodeReference() ==
                     *CommonlyUsedTypeGenerator::GetBasicType<
                         BuiltInType::kVoid, SignTag::kUnsigned>()) {
        // ����ֵ�Ķ���Ϊvoidָ�룬��ֵ�Ķ���Ϊͬ��ά��ָ��
        return AssignableCheckResult::kConvertToVoidPointer;
      }
      break;
  }
}

bool StructType::IsSameObject(const TypeInterface& type_interface) const {
  // this == &type_interface���Ż��ֶ�
  // ����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  if (this == &type_interface) [[likely]] {
    // ���߹���һ���ڵ�
    return true;
  } else if (TypeInterface::IsSameObject(type_interface)) [[likely]] {
    const StructType& struct_type =
        static_cast<const StructType&>(type_interface);
    const std::string* struct_name_this = GetStructName();
    const std::string* struct_name_argument = struct_type.GetStructName();
    if (struct_name_this != nullptr) [[likely]] {
      if (struct_name_argument != nullptr) [[likely]] {
        // ���߶��Ǿ����ṹ��
        // C�����о����ṹ�������������
        return *struct_name_this == *struct_name_argument;
      }
    } else if (struct_name_argument == nullptr) {
      // ���߶��������ṹ��
      return GetStructMembers() == struct_type.GetStructMembers();
    }
  }
  return false;
}

bool UnionType::IsSameObject(const TypeInterface& type_interface) const {
  // this == &type_interface���Ż��ֶ�
  // ����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  if (this == &type_interface) [[likely]] {
    // ���߹���һ���ڵ�
    return true;
  } else if (TypeInterface::IsSameObject(type_interface)) [[likely]] {
    const UnionType& union_type = static_cast<const UnionType&>(type_interface);
    const std::string* union_name_this = GetUnionName();
    const std::string* union_name_argument = union_type.GetUnionName();
    if (union_name_this != nullptr) [[likely]] {
      if (union_name_argument != nullptr) [[likely]] {
        // ���߶��Ǿ���������
        // C�����о��������������������
        return *union_name_this == *union_name_argument;
      }
    } else if (union_name_argument == nullptr) {
      // ���߶��������ṹ��
      return GetUnionMembers() == union_type.GetUnionMembers();
    }
  }
  return false;
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
    const std::string* enum_name_this = GetEnumName();
    const std::string* enum_name_argument = enum_type.GetEnumName();
    if (enum_name_this != nullptr) [[likely]] {
      if (enum_name_argument != nullptr) [[likely]] {
        // ���߶��Ǿ���ö��
        // C�����о���ö�ٸ�����������
        return *enum_name_this == *enum_name_argument;
      }
    } else if (enum_name_argument == nullptr) {
      // ���߶�������ö��
      return GetEnumMembers() == enum_type.GetEnumMembers();
    }
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

inline bool PointerType::IsSameObject(
    const TypeInterface& type_interface) const {
  // this == &basic_type���Ż��ֶΣ�����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
  // �������׳���ָ��ͬһ���ڵ�����
  return this == &type_interface ||
         TypeInterface::IsSameObject(type_interface) &&
             GetConstTag() ==
                 static_cast<const PointerType&>(type_interface).GetConstTag();
}

std::shared_ptr<TypeInterface>
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
    case BuiltInType::kBool:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kBool, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kBool, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kChar:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kChar, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kChar, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kShort:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kShort, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kShort, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kInt:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kInt, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kInt, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kLong:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kLong, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kLong, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kFloat:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kFloat, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kFloat, SignTag::kUnsigned>();
          break;
        default:
          assert(false);
          break;
      }
      break;
    case BuiltInType::kDouble:
      switch (sign_tag) {
        case c_parser_frontend::type_system::SignTag::kSigned:
          return GetBasicType<BuiltInType::kDouble, SignTag::kSigned>();
          break;
        case c_parser_frontend::type_system::SignTag::kUnsigned:
          return GetBasicType<BuiltInType::kDouble, SignTag::kUnsigned>();
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
    std::shared_ptr<const TypeInterface>&& left_compute_type,
    std::shared_ptr<const TypeInterface>&& right_compute_type) {
  switch (left_compute_type->GetType()) {
    case StructOrBasicType::kBasic: {
      switch (right_compute_type->GetType()) {
        case StructOrBasicType::kBasic:
          if (static_cast<long long>(left_compute_type->GetType()) >=
              static_cast<long long>(right_compute_type->GetType())) {
            // �Ҳ��������������
            return std::make_pair(
                std::move(left_compute_type),
                DeclineMathematicalComputeTypeResult::kConvertToLeft);
          } else {
            // ��������������Ҳ�
            return std::make_pair(
                std::move(right_compute_type),
                DeclineMathematicalComputeTypeResult::kConvertToRight);
          }
          break;
        case StructOrBasicType::kPointer:
          // ָ����ƫ�������
          if (static_cast<long long>(left_compute_type->GetType()) <
              static_cast<long long>(BuiltInType::kFloat)) [[likely]] {
            // ƫ���������ͣ�������Ϊָ���ƫ����
            return std::make_pair(
                std::move(right_compute_type),
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
              static_cast<long long>(BuiltInType::kFloat)) [[likely]] {
            // ƫ���������ͣ�������Ϊָ���ƫ����
            return std::make_pair(
                std::move(left_compute_type),
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

}  // namespace c_parser_frontend::type_system
