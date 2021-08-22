#include "operator_node.h"

namespace c_parser_frontend::operator_node {
inline bool VarietyOperatorNode::SetVarietyType(
    std::shared_ptr<const TypeInterface>&& variety_type) {
  if (CheckVarietyTypeValid(*variety_type)) [[likely]] {
    // �������ϱ��������͵ı�׼ʱ�ſ�������
    variety_type_ = std::move(variety_type);
    return true;
  } else {
    return false;
  }
}
inline bool VarietyOperatorNode::CheckVarietyTypeValid(
    const TypeInterface& variety_type) {
  switch (variety_type.GetType()) {
    case StructOrBasicType::kEnd:
      // �����ʹ��������ڱ��ڵ�
    case StructOrBasicType::kNotSpecified:
      // ��ѡ���Ӧ�����ڸ�����������ѯ�����ú�����
      assert(false);
      [[fallthrough]];
    case StructOrBasicType::kFunction:
    // ������������Ϊ����
    case StructOrBasicType::kInitializeList:
      // ��ʼ���б�������Ϊ����
      return false;
      break;
    default:
      return true;
      break;
  }
}

inline bool BasicTypeInitializeOperatorNode::SetInitDataType(
    std::shared_ptr<const TypeInterface>&& variety_type) {
  if (CheckBasicTypeInitializeValid(*variety_type)) [[likely]] {
    return SetInitValueType(std::move(variety_type));
  } else {
    return false;
  }
}

inline bool BasicTypeInitializeOperatorNode::CheckBasicTypeInitializeValid(
    const TypeInterface& variety_type) {
  switch (variety_type.GetType()) {
    case StructOrBasicType::kPointer:
      // const char*
      return variety_type ==
             *CommonlyUsedTypeGenerator::GetConstExprStringType();
      break;
    case StructOrBasicType::kBasic:
      return true;
      break;
    default:
      return false;
      break;
  }
}

// �����Ƿ�Ϊ��Ч�ĳ�ʼ���������ͣ������Ч�����
inline bool InitializeOperatorNode::SetInitValueType(
    std::shared_ptr<const TypeInterface>&& init_value_type) {
  if (CheckInitValueTypeValid(*init_value_type)) [[likely]] {
    initialize_value_type_ = std::move(init_value_type);
    return true;
  } else {
    return false;
  }
}

inline bool InitializeOperatorNode::CheckInitValueTypeValid(
    const TypeInterface& init_value_type) {
  switch (init_value_type.GetType()) {
    case StructOrBasicType::kBasic:
      // ����
    case StructOrBasicType::kInitializeList:
      // ��ʼ���б�
      return true;
      break;
    case StructOrBasicType::kPointer:
      // ����Ƿ�Ϊ�ַ�������const char*
      return init_value_type ==
             *CommonlyUsedTypeGenerator::GetConstExprStringType();
      break;
    case StructOrBasicType::kEnd:
    case StructOrBasicType::kNotSpecified:
      // �����������Ͳ�Ӧ���������
      assert(false);
      [[fallthrough]];
    case StructOrBasicType::kEnum:
    case StructOrBasicType::kStruct:
    case StructOrBasicType::kUnion:
    case StructOrBasicType::kFunction:
      // c�������⼸�����Ͳ�������Ϊ��ʼ���ñ���
      return false;
    default:
      assert(false);
      // ��ֹ����
      return false;
      break;
  }
}

// ����ֵ�Ƿ�Ϊ��Ч�ĳ�ʼ���б�������ͣ����������Ч��ִ�в������

inline bool ListInitializeOperatorNode::AddListValue(
    std::shared_ptr<const InitializeOperatorNode>&& list_value) {
  if (CheckInitListValueTypeValid(*list_value->GetResultTypePointer()))
      [[likely]] {
    list_values_.emplace_back(std::move(list_value));
    return true;
  } else {
    return false;
  }
}

inline bool ListInitializeOperatorNode::SetInitListType(
    std::shared_ptr<const TypeInterface>&& list_type) {
  if (CheckInitListTypeValid(*list_type)) [[likely]] {
    return SetInitValueType(std::move(list_type));
  } else {
    return false;
  }
}

inline bool MathematicalOperatorNode::SetLeftOperatorNode(
    std::shared_ptr<const OperatorNodeInterface>&& left_operator_node) {
  switch (GetMathematicalOperation()) {
    case MathematicalOperation::kNot: {
      // ȡ��ΪһԪ����������������������������ͬ
      auto compute_result_node = std::make_shared<VarietyOperatorNode>(
          nullptr, ConstTag::kNonConst, LeftRightValueTag::kRightValue);
      bool result = compute_result_node->SetVarietyType(
          left_operator_node->GetResultTypePointer());
      if (result) [[likely]] {
        // left_operator_node�ǺϷ��Ĳ�������Ľڵ����������
        SetComputeResultNode(std::move(compute_result_node));
      } else {
        break;
      }
    }
      [[fallthrough]];
    default:
      left_operator_node_ = std::move(left_operator_node);
      break;
  }
  return true;
}

MathematicalOperatorNode::DeclineMathematicalComputeTypeResult
MathematicalOperatorNode::SetRightOperatorNode(
    std::shared_ptr<const OperatorNodeInterface>&& right_operator_node) {
  switch (GetMathematicalOperation()) {
    case MathematicalOperation::kNot:
      // һԪ�������Ӧ���ô˺���
      assert(false);
      break;
    default:
      break;
  }
  auto [compute_result_node, decline_result] = DeclineComputeResult(
      GetMathematicalOperation(), GetLeftOperatorNodePointer(),
      std::shared_ptr<const OperatorNodeInterface>(right_operator_node));
  if (compute_result_node != nullptr) [[likely]] {
    // �������㣬���������ҽڵ���������ڵ�
    right_operator_node_ = std::move(right_operator_node);
    compute_result_node_ = std::move(compute_result_node);
  }
  return decline_result;
}

std::pair<std::shared_ptr<const VarietyOperatorNode>,
          MathematicalOperatorNode::DeclineMathematicalComputeTypeResult>
MathematicalOperatorNode::DeclineComputeResult(
    MathematicalOperation mathematical_operation,
    std::shared_ptr<const OperatorNodeInterface>&& left_operator_node,
    std::shared_ptr<const OperatorNodeInterface>&& right_operator_node) {
  std::shared_ptr<const TypeInterface> left_operator_node_type =
      left_operator_node->GetResultTypePointer();
  std::shared_ptr<const TypeInterface> right_operator_node_type =
      left_operator_node->GetResultTypePointer();
  auto [compute_result_type, compute_result] =
      TypeInterface::DeclineMathematicalComputeResult(
          std::shared_ptr<const TypeInterface>(left_operator_node_type),
          std::shared_ptr<const TypeInterface>(right_operator_node_type));
  switch (compute_result) {
    case DeclineMathematicalComputeTypeResult::kComputable:
    case DeclineMathematicalComputeTypeResult::kLeftPointerRightOffset:
    case DeclineMathematicalComputeTypeResult::kLeftOffsetRightPointer:
    case DeclineMathematicalComputeTypeResult::kConvertToLeft:
    case DeclineMathematicalComputeTypeResult::kConvertToRight:
      // ��������ļ������
      {
        // �����������ڵ�
        auto compute_result_node = std::make_shared<VarietyOperatorNode>(
            nullptr, ConstTag::kNonConst, LeftRightValueTag::kRightValue);
        compute_result_node->SetVarietyType(std::move(compute_result_type));
        return std::make_pair(std::move(compute_result_node), compute_result);
      }
      break;
    default:
      return std::make_pair(std::shared_ptr<const VarietyOperatorNode>(nullptr),
                            compute_result);
      break;
  }
}

bool FunctionCallOperatorNode::SetFunctionType(
    std::shared_ptr<const TypeInterface>&& type_pointer) {
  // �ж������Ƿ�Ϸ�
  if (CheckFunctionTypeValid(*type_pointer)) [[likely]] {
    // ��ȡ�ȷ������
    const c_parser_frontend::type_system::FunctionType& function_type =
        static_cast<const c_parser_frontend::type_system::FunctionType&>(
            *type_pointer);
    if (CheckFunctionTypeValid(*type_pointer)) [[likely]] {
      // �ɹ����ú������ú󷵻ص�����
      // ���ԭ�����ɵĺ�����������ָ��
      function_arguments_for_call_.clear();
      function_arguments_offerred_.clear();
      // ���ú�������
      function_type_ =
          std::move(std::static_pointer_cast<
                    const c_parser_frontend::type_system::FunctionType>(
              type_pointer));
      // ���ݺ������ʹ����������Ͷ���
      // ������������
      auto return_object = std::make_shared<VarietyOperatorNode>(
          nullptr, function_type.GetReturnTypeConstTag(),
          LeftRightValueTag::kRightValue);
      return_object->SetVarietyType(function_type.GetReturnTypePointer());
      return_object_ = return_object;
      // ���ݺ����������������ṩ�������Ĳ����Ķ���
      for (auto& argument : function_type.GetArgumentTypes()) {
        auto variety_node = std::make_shared<VarietyOperatorNode>(
            argument.argument_name, argument.argument_const_tag,
            LeftRightValueTag::kRightValue);
        bool result = variety_node->SetVarietyType(
            std::shared_ptr<const TypeInterface>(argument.argument_type));
        assert(result == true);
        function_arguments_for_call_.emplace_back(std::move(variety_node));
      }
      return true;
    }
  }
  return false;
}

// ���һ���������������˳�������
// ���ش���ӵĲ����Ƿ�ͨ�����飬δͨ�������򲻻����

AssignableCheckResult FunctionCallOperatorNode::AddArgument(
    std::shared_ptr<const OperatorNodeInterface>&& argument_node) {
  // ��Ҫ���ýڵ��index
  size_t new_argument_index = function_arguments_offerred_.size();
  if (new_argument_index >= GetFunctionArgumentsForCall().size()) [[unlikely]] {
    // ��������������������Ӹ������
    // �ݲ�֧�ֿɱ����
    return AssignableCheckResult::kArgumentsFull;
  }
  // ���������������̵�ͬ����������ֵ
  AssignableCheckResult check_result = AssignOperatorNode::CheckAssignable(
      *GetFunctionArgumentsForCall()[new_argument_index], *argument_node, true);
  switch (check_result) {
    case AssignableCheckResult::kNonConvert:
    case AssignableCheckResult::kUpperConvert:
    case AssignableCheckResult::kConvertToVoidPointer:
    case AssignableCheckResult::kZeroConvertToPointer:
    case AssignableCheckResult::kUnsignedToSigned:
    case AssignableCheckResult::kSignedToUnsigned:
      // �������
      function_arguments_for_call_.emplace_back(std::move(argument_node));
      break;
    default:
      // ���������
      break;
  }
  return check_result;
}

inline bool SetTargetVariety(
    std::shared_ptr<const OperatorNodeInterface>&& target_variety) {
  switch (target_variety->GetGeneralOperatorType()) {
    case GeneralOperationType::kVariety:
      // ������ռ�Ľڵ�ֻ���Ǳ���
      target_variety = std::move(target_variety);
      return true;
      break;
    default:
      assert(false);
      // ��ֹ����
      return false;
      break;
  }
}

inline bool AllocateOperatorNode::SetTargetVariety(
    std::shared_ptr<const OperatorNodeInterface>&& target_variety) {
  if (CheckAllocatable(*target_variety)) [[likely]] {
    target_variety = std::move(target_variety);
    return true;
  } else {
    return false;
  }
}

bool AllocateOperatorNode::AddNumToAllocate(size_t num) {
  std::shared_ptr<const TypeInterface> type_now =
      target_variety_->GetResultTypePointer();
  // ����Ƿ����㹻��ά�������������
  for (size_t i = 0; i < num_to_allocate_.size(); i++) {
    if (type_now->GetType() == StructOrBasicType::kPointer) [[likely]] {
      type_now = type_now->GetNextNodePointer();
    } else {
      return false;
    }
  }
  // ����Ƿ��������������һά
  if (type_now->GetType() == StructOrBasicType::kPointer) [[likely]] {
    num_to_allocate_.push_back(num);
    return true;
  } else {
    return false;
  }
}

inline bool MemberAccessOperatorNode::CheckNodeToAccessValid(
    const OperatorNodeInterface& node_to_access) {
  // ֻ�ܶԽṹ���ͱ������ʳ�Ա
  switch (node_to_access.GetResultTypePointer()->GetType()) {
    case StructOrBasicType::kEnum:
    case StructOrBasicType::kStruct:
    case StructOrBasicType::kUnion:
      return true;
      break;
    default:
      return false;
      break;
  }
}

bool MemberAccessOperatorNode::SetAccessedNodeAndMemberName(
    std::string&& member_name_to_set) {
  // Ҫ�����ʳ�Ա�Ľڵ������
  auto accessed_node_type = GetNodeToAccessReference().GetResultTypePointer();
  switch (accessed_node_type->GetType()) {
    case StructOrBasicType::kStruct: {
      auto [struct_member_info_iter, struct_member_exist] =
          static_cast<const c_parser_frontend::type_system::StructType&>(
              *accessed_node_type)
              .GetStructMemberInfo(member_name_to_set);
      if (!struct_member_exist) [[unlikely]] {
        // �����ڸ����ĳ�Ա��
        return false;
      }
      // ���ɷ��ʳ�Ա��õ��Ľڵ�
      auto struct_node = std::make_shared<VarietyOperatorNode>(
          nullptr, struct_member_info_iter->second.first,
          LeftRightValueTag::kLeftValue);
      struct_node->SetVarietyType(std::shared_ptr<const TypeInterface>(
          struct_member_info_iter->second.second));
      SetAccessedNode(struct_node);
      member_name_ = std::move(member_name_to_set);
      return true;
    } break;
    case StructOrBasicType::kUnion: {
      auto [union_member_info_iter, union_member_exist] =
          static_cast<const c_parser_frontend::type_system::UnionType&>(
              *accessed_node_type)
              .GetUnionMemberInfo(member_name_to_set);
      if (!union_member_exist) [[unlikely]] {
        // �����ڸ����ĳ�Ա��
        return false;
      }
      // ���ɷ��ʳ�Ա��õ��Ľڵ�
      auto union_node = std::make_shared<VarietyOperatorNode>(
          nullptr, union_member_info_iter->second.first,
          LeftRightValueTag::kLeftValue);
      union_node->SetVarietyType(std::shared_ptr<const TypeInterface>(
          union_member_info_iter->second.second));
      SetAccessedNode(union_node);
      member_name_ = std::move(member_name_to_set);
      return true;
    } break;
    case StructOrBasicType::kEnum: {
      const auto& enum_type =
          static_cast<const c_parser_frontend::type_system::EnumType&>(
              *accessed_node_type);
      auto [enum_member_info_iter, enum_member_exist] =
          enum_type.GetEnumMemberInfo(member_name_to_set);
      if (!enum_member_exist) [[unlikely]] {
        // �����ڸ����ĳ�Ա��
        return false;
      }
      // ���ɷ��ʳ�Ա��õ��Ľڵ�
      auto enum_node = std::make_shared<BasicTypeInitializeOperatorNode>(
          InitializeType::kBasic,
          std::to_string(enum_member_info_iter->second));
      enum_node->SetInitDataType(enum_type.GetContainerTypePointer());
      SetAccessedNode(enum_node);
      member_name_ = std::move(member_name_to_set);
      return true;
    } break;
    default:
      assert(false);
      // ��ֹ����
      return false;
      break;
  }
}

// ������ǿ��Ը�ֵ������򲻻�����
// ����ָ��Ҫ���õ�������ֵ�Ľڵ��ָ����Ƿ�Ϊ����ʱ��ֵ
// ����ʱ��ֵ����Ա���ֵ�Ľڵ������const����
AssignableCheckResult AssignOperatorNode::SetNodeForAssign(
    std::shared_ptr<const OperatorNodeInterface>&& node_for_assign,
    bool is_announce) {
  AssignableCheckResult assignable_check_result = CheckAssignable(
      GetNodeToBeAssignedReference(), *node_for_assign, is_announce);
  switch (assignable_check_result) {
    case AssignableCheckResult::kNonConvert:
    case AssignableCheckResult::kUpperConvert:
    case AssignableCheckResult::kConvertToVoidPointer:
    case AssignableCheckResult::kZeroConvertToPointer:
    case AssignableCheckResult::kUnsignedToSigned:
    case AssignableCheckResult::kSignedToUnsigned:
      node_for_assign_ = std::move(node_for_assign);
      break;
    default:
      // �����Ը�ֵ�������ֱ�ӷ���
      break;
  }
  return assignable_check_result;
}

// �������ڵ�������Ƿ���Ը�ֵ
// ������ģ�������ʾ�Ƿ�Ϊ����ʱ��ֵ
// ��is_announce == trueʱ����node_to_be_assigned��const���
AssignableCheckResult AssignOperatorNode::CheckAssignable(
    const OperatorNodeInterface& node_to_be_assigned,
    const OperatorNodeInterface& node_for_assign, bool is_announce) {
  if (!is_announce) {
    // ��������ʱ��ֵ��Ҫ��鱻��ֵ�Ľڵ��Ƿ�Ϊconst
    if (node_to_be_assigned.GetResultConstTag() == ConstTag::kConst)
        [[unlikely]] {
      // ����ֵ�Ľڵ���const�Ҳ������������
      return AssignableCheckResult::kAssignedNodeIsConst;
    }
  }
  AssignableCheckResult check_result;
  switch (node_to_be_assigned.GetGeneralOperatorType()) {
    case GeneralOperationType::kLogicalOperation:
    case GeneralOperationType::kMathematicalOperation:
      // ��ֵ�������߼���������Ϊ��ֵ�����ܱ���ֵ
    case GeneralOperationType::kInitValue:
      // ��ʼ��ֵΪ��ֵ�����ܱ���ֵ
    case GeneralOperationType::kFunctionCall:
      // C�����к�������ֵΪ��ֵ�����ܱ���ֵ
      check_result = AssignableCheckResult::kAssignToRightValue;
      break;
    case GeneralOperationType::kVariety:
      // ������������ֵҲ��������ֵ���м��������Ҫ����
      if (static_cast<const VarietyOperatorNode&>(node_to_be_assigned)
              .GetLeftRightValueTag() == LeftRightValueTag::kRightValue)
          [[unlikely]] {
        // ��ֵ���ܱ���ֵ
        check_result = AssignableCheckResult::kAssignToRightValue;
        break;
      }
      [[fallthrough]];
    case GeneralOperationType::kAssign:
    case GeneralOperationType::kMemberAccess:
    case GeneralOperationType::kDeReference: {
      check_result =
          node_to_be_assigned.GetResultTypePointer()->CanBeAssignedBy(
              node_for_assign.GetResultTypePointer());
      // ��������ǽ�0��ֵ��ָ������Ҫ������
      // ֻ��ʹ�ñ����ڳ�����ʼ���ſ��ܳ���ʹ��0��ֵ��ָ������
      if (check_result == AssignableCheckResult::kMayBeZeroToPointer &&
          node_for_assign.GetGeneralOperatorType() ==
              GeneralOperationType::kInitValue) [[unlikely]] {
        const BasicTypeInitializeOperatorNode& initialize_node =
            static_cast<const BasicTypeInitializeOperatorNode&>(
                node_for_assign);
        // ȷ�ϳ�ʼ��ֵ�Ƿ�Ϊ0
        if (initialize_node.GetInitializeType() == InitializeType::kBasic &&
            initialize_node.GetValue() == "0") [[likely]] {
          check_result = AssignableCheckResult::kZeroConvertToPointer;
        } else {
          // ���ܽ�����0�����ֵ��ֵ��ָ��
          check_result = AssignableCheckResult::kCanNotConvert;
        }
      }
    } break;
    default:
      assert(false);
      break;
  }
  return check_result;
}

// �����Ƿ�������ã�������������򲻻�����

bool ObtainAddress::SetNodeToObtainAddress(
    std::shared_ptr<const OperatorNodeInterface>&& node_to_obtain_address) {
  bool check_result = CheckNodeToObtainAddress(*node_to_obtain_address);
  if (check_result) [[likely]] {
    // ����ȡ��ַ
    // ���ý�Ҫ��ȡ��ַ��ָ��
    node_to_obtain_address_ = node_to_obtain_address;
    // ����ȡ��ַ��õ��ı���������
    auto obtained_type = TypeInterface::ObtainAddress(
        node_to_obtain_address->GetResultTypePointer());
    // ����ȡ��ַ��õ��Ľڵ㣨����������ֵ�м������
    auto node_obtained_address = std::make_shared<VarietyOperatorNode>(
        nullptr, ConstTag::kNonConst, LeftRightValueTag::kRightValue);
    // ����ȡ��ַ��ı����ڵ�����
    bool result =
        node_obtained_address->SetVarietyType(std::move(obtained_type));
    assert(result == true);
    // ����ȡ��ַ��õ��Ľڵ�
    SetNodeObtainedAddress(std::move(node_obtained_address));
  }
  return check_result;
}

inline bool ObtainAddress::CheckNodeToObtainAddress(
    const OperatorNodeInterface& node_interface) {
  switch (node_interface.GetGeneralOperatorType()) {
    case GeneralOperationType::kDeReference:
    case GeneralOperationType::kMemberAccess:
    case GeneralOperationType::kAssign:
    case GeneralOperationType::kObtainAddress:
    case GeneralOperationType::kVariety:
      return true;
      break;
    default:
      return false;
      break;
  }
}

bool DereferenceOperatorNode::SetNodeToDereference(
    std::shared_ptr<const OperatorNodeInterface>&& node_to_dereference) {
  if (CheckNodeDereferenceAble(*node_to_dereference)) [[likely]] {
    // ���ô������õĽڵ�
    node_to_dereference_ = std::move(node_to_dereference);
    // ���������ú�õ��Ľڵ�
    auto node_to_dereference_type = std::static_pointer_cast<
        const c_parser_frontend::type_system::PointerType>(
        node_to_dereference->GetResultTypePointer());
    // ��ȡ�����ú�����ͺͽ����õĽ��
    auto [dereferenced_node_type, dereferenced_node_const_tag] =
        node_to_dereference_type->DeReference();
    auto dereferenced_node = std::make_shared<VarietyOperatorNode>(
        nullptr, dereferenced_node_const_tag, LeftRightValueTag::kRightValue);
    bool result =
        dereferenced_node->SetVarietyType(std::move(dereferenced_node_type));
    assert(result == true);
    SetDereferencedNode(std::move(dereferenced_node));
    return true;
  } else {
    return false;
  }
}

bool LogicalOperationOperatorNode::SetLeftOperatorNode(
    std::shared_ptr<const OperatorNodeInterface>&& left_operator_node) {
  std::shared_ptr<const TypeInterface> left_operator_node_type =
      left_operator_node->GetResultTypePointer();
  if (CheckLogicalTypeValid(*left_operator_node_type)) [[likely]] {
    left_operator_node_ = std::move(left_operator_node);
    return true;
  } else {
    return false;
  }
}

bool LogicalOperationOperatorNode::SetRightOperatorNode(
    std::shared_ptr<const OperatorNodeInterface>&& right_operator_node) {
  std::shared_ptr<const TypeInterface> right_operator_node_type =
      right_operator_node->GetResultTypePointer();
  if (CheckLogicalTypeValid(*right_operator_node_type)) [[likely]] {
    right_operator_node_ = std::move(right_operator_node);
    CreateAndSetResultNode();
    return true;
  } else {
    return false;
  }
}

// ����Ƿ���Բ����߼�����

inline bool LogicalOperationOperatorNode::CheckLogicalTypeValid(
    const TypeInterface& type_interface) {
  switch (type_interface.GetType()) {
    case StructOrBasicType::kBasic:
    case StructOrBasicType::kPointer:
      return true;
      break;
    default:
      return false;
      break;
  }
}

}  // namespace c_parser_frontend::operator_node