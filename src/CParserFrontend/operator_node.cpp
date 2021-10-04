#include "operator_node.h"

#include <format>
#include <iostream>
namespace c_parser_frontend::operator_node {
inline bool VarietyOperatorNode::SetVarietyType(
    const std::shared_ptr<const TypeInterface>& variety_type) {
  if (CheckVarietyTypeValid(*variety_type)) [[likely]] {
    // �������ϱ��������͵ı�׼ʱ�ſ�������
    variety_type_ = variety_type;
    return true;
  } else {
    return false;
  }
}

bool VarietyOperatorNode::SetVarietyTypeNoCheckFunctionType(
    const std::shared_ptr<const TypeInterface>& variety_type) {
  if (variety_type->GetType() == StructOrBasicType::kFunction ||
      CheckVarietyTypeValid(*variety_type)) {
    variety_type_ = variety_type;
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
    const std::shared_ptr<const TypeInterface>& variety_type) {
  if (CheckBasicTypeInitializeValid(*variety_type)) [[likely]] {
    return SetInitValueType(variety_type);
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
inline bool InitializeOperatorNodeInterface::SetInitValueType(
    const std::shared_ptr<const TypeInterface>& init_value_type) {
  if (CheckInitValueTypeValid(*init_value_type)) [[likely]] {
    initialize_value_type_ = init_value_type;
    return true;
  } else {
    return false;
  }
}

inline bool InitializeOperatorNodeInterface::CheckInitValueTypeValid(
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

bool ListInitializeOperatorNode::AddListValue(
    const std::shared_ptr<const InitializeOperatorNodeInterface>& list_value) {
  if (CheckInitListValueTypeValid(*list_value->GetResultTypePointer()))
      [[likely]] {
    list_values_.emplace_back(list_value);
    return true;
  } else {
    return false;
  }
}

bool ListInitializeOperatorNode::SetInitListType(
    const std::shared_ptr<const TypeInterface>& list_type) {
  if (CheckInitListTypeValid(*list_type)) [[likely]] {
    return SetInitValueType(list_type);
  } else {
    return false;
  }
}

bool MathematicalOperatorNode::SetLeftOperatorNode(
    const std::shared_ptr<const OperatorNodeInterface>& left_operator_node) {
  switch (GetMathematicalOperation()) {
    case MathematicalOperation::kNot:
    case MathematicalOperation::kLogicalNegative: {
      // ��λȡ�����߼���ΪһԪ����������������������������ͬ
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
      left_operator_node_ = left_operator_node;
      break;
  }
  return true;
}

MathematicalOperatorNode::DeclineMathematicalComputeTypeResult
MathematicalOperatorNode::SetRightOperatorNode(
    const std::shared_ptr<const OperatorNodeInterface>& right_operator_node) {
  switch (GetMathematicalOperation()) {
    case MathematicalOperation::kNot:
    case MathematicalOperation::kLogicalNegative:
      // һԪ�������Ӧ���ô˺���
      assert(false);
      break;
    default:
      break;
  }
  auto [compute_result_node, decline_result] =
      DeclineComputeResult(GetMathematicalOperation(),
                           GetLeftOperatorNodePointer(), right_operator_node);
  if (compute_result_node != nullptr) [[likely]] {
    // �������㣬���������ҽڵ���������ڵ�
    right_operator_node_ = right_operator_node;
    compute_result_node_ = std::move(compute_result_node);
  }
  return decline_result;
}

std::pair<std::shared_ptr<const VarietyOperatorNode>,
          MathematicalOperatorNode::DeclineMathematicalComputeTypeResult>
MathematicalOperatorNode::DeclineComputeResult(
    MathematicalOperation mathematical_operation,
    const std::shared_ptr<const OperatorNodeInterface>& left_operator_node,
    const std::shared_ptr<const OperatorNodeInterface>& right_operator_node) {
  std::shared_ptr<const TypeInterface> left_operator_node_type =
      left_operator_node->GetResultTypePointer();
  std::shared_ptr<const TypeInterface> right_operator_node_type =
      left_operator_node->GetResultTypePointer();
  auto [compute_result_type, compute_result] =
      TypeInterface::DeclineMathematicalComputeResult(left_operator_node_type,
                                                      right_operator_node_type);
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
      return std::make_pair(nullptr, compute_result);
      break;
  }
}

bool FunctionCallOperatorNode::SetFunctionObjectToCall(
    const std::shared_ptr<const OperatorNodeInterface>&
        function_object_to_call) {
  // �ж������Ƿ�Ϸ�
  auto function_type = function_object_to_call->GetResultTypePointer();
  if (CheckFunctionTypeValid(*function_type)) [[likely]] {
    // ��ȡ�ȷ������
    auto exact_function_type = std::static_pointer_cast<
        const c_parser_frontend::type_system::FunctionType>(function_type);
    // �ɹ����ú������ú󷵻ص�����
    // ���ԭ�����ɵĺ�����������ָ��
    function_arguments_offerred_.GetFunctionCallArguments().clear();
    // ���ú�������
    function_type_ = exact_function_type;
    // ���ݺ������ʹ����������Ͷ���
    // ������������
    auto return_object = std::make_shared<VarietyOperatorNode>(
        nullptr, exact_function_type->GetReturnTypeConstTag(),
        LeftRightValueTag::kRightValue);
    return_object->SetVarietyType(exact_function_type->GetReturnTypePointer());
    return_object_ = return_object;
    return true;
  } else {
    return false;
  }
}

// ���һ���������������˳�������
// ���ش���ӵĲ����Ƿ�ͨ�����飬δͨ�������򲻻����

AssignableCheckResult FunctionCallOperatorNode::AddFunctionCallArgument(
    const std::shared_ptr<const OperatorNodeInterface>& argument_node,
    const std::shared_ptr<std::list<
        std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
        sentences_to_get_argument) {
  // ��Ҫ���ýڵ��index
  size_t new_argument_index =
      function_arguments_offerred_.GetFunctionCallArguments().size();
  if (new_argument_index >= GetFunctionTypePointer()->GetArguments().size())
      [[unlikely]] {
    // ��������������������Ӹ������
    // �ݲ�֧�ֿɱ����
    return AssignableCheckResult::kArgumentsFull;
  }
  // ���������������̵�ͬ����������ֵ
  AssignableCheckResult check_result = AssignOperatorNode::CheckAssignable(
      *GetFunctionTypePointer()
           ->GetArguments()[new_argument_index]
           .variety_operator_node,
      *argument_node, true);
  switch (check_result) {
    case AssignableCheckResult::kNonConvert:
    case AssignableCheckResult::kUpperConvert:
    case AssignableCheckResult::kConvertToVoidPointer:
    case AssignableCheckResult::kZeroConvertToPointer:
    case AssignableCheckResult::kUnsignedToSigned:
    case AssignableCheckResult::kSignedToUnsigned:
      // �������
      function_arguments_offerred_.AddFunctionCallArgument(
          argument_node, sentences_to_get_argument);
      break;
    default:
      // ���������
      break;
  }
  return check_result;
}

bool AllocateOperatorNode::SetTargetVariety(
    const std::shared_ptr<const OperatorNodeInterface>& target_variety) {
  if (CheckAllocatable(*target_variety)) [[likely]] {
    target_variety_ = target_variety;
    return true;
  } else {
    return false;
  }
}

bool MemberAccessOperatorNode::CheckNodeToAccessValid(
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
      auto& struct_type =
          static_cast<const c_parser_frontend::type_system::StructType&>(
              *accessed_node_type);
      MemberIndex struct_member_index =
          struct_type.GetStructMemberIndex(member_name_to_set);
      if (!struct_member_index.IsValid()) [[unlikely]] {
        // �����ڸ����ĳ�Ա��
        return false;
      }
      const auto& struct_member_info =
          struct_type.GetStructMemberInfo(struct_member_index);
      // ���ɷ��ʳ�Ա��õ��Ľڵ�
      auto struct_node = std::make_shared<VarietyOperatorNode>(
          nullptr, struct_member_info.second, LeftRightValueTag::kLeftValue);
      bool result = struct_node->SetVarietyType(struct_member_info.first);
      assert(result);
      SetAccessedNode(struct_node);
      member_index_ = struct_member_index;
      return true;
    } break;
    case StructOrBasicType::kUnion: {
      auto& union_type =
          static_cast<const c_parser_frontend::type_system::UnionType&>(
              *accessed_node_type);
      MemberIndex union_member_index =
          union_type.GetUnionMemberIndex(member_name_to_set);
      if (!union_member_index.IsValid()) [[unlikely]] {
        // �����ڸ����ĳ�Ա��
        return false;
      }
      auto& union_member_info =
          union_type.GetUnionMemberInfo(union_member_index);
      // ���ɷ��ʳ�Ա��õ��Ľڵ�
      auto union_node = std::make_shared<VarietyOperatorNode>(
          nullptr, union_member_info.second, LeftRightValueTag::kLeftValue);
      bool result = union_node->SetVarietyType(union_member_info.first);
      assert(result);
      SetAccessedNode(union_node);
      member_index_ = union_member_index;
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
// ����ʱ��ֵ����Ա���ֵ�Ľڵ������const���Բ�������ʹ�ó�ʼ���б�
// ��Ҫ�����ñ���ֵ�Ľڵ㣨SetNodeToBeAssigned��
AssignableCheckResult AssignOperatorNode::SetNodeForAssign(
    const std::shared_ptr<const OperatorNodeInterface>& node_for_assign,
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
      node_for_assign_ = node_for_assign;
      break;
    default:
      // �����Ը�ֵ�������ֱ�ӷ���
      break;
  }
  return assignable_check_result;
}

// �������ڵ�������Ƿ���Ը�ֵ
// ������ģ�������ʾ�Ƿ�Ϊ����ʱ��ֵ
// ��is_announce == trueʱ
// ����node_to_be_assigned��const���������ʹ�ó�ʼ���б�
AssignableCheckResult AssignOperatorNode::CheckAssignable(
    const OperatorNodeInterface& node_to_be_assigned,
    const OperatorNodeInterface& node_for_assign, bool is_announce) {
  if (!is_announce) [[unlikely]] {
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
    case GeneralOperationType::kVariety: {
      auto variety_node =
          static_cast<const VarietyOperatorNode&>(node_to_be_assigned);
      // ������������ֵҲ��������ֵ���м��������Ҫ����
      if (variety_node.GetLeftRightValueTag() == LeftRightValueTag::kRightValue)
          [[unlikely]] {
        // ��ֵ���ܱ���ֵ
        check_result = AssignableCheckResult::kAssignToRightValue;
        break;
      }
    }
      [[fallthrough]];
    case GeneralOperationType::kAssign:
    case GeneralOperationType::kMemberAccess:
    case GeneralOperationType::kDeReference: {
      check_result =
          node_to_be_assigned.GetResultTypePointer()->CanBeAssignedBy(
              *node_for_assign.GetResultTypePointer());
      switch (check_result) {
        case AssignableCheckResult::kNonConvert: {
          // �������ֵ�Ľڵ���������ֵ�Ľڵ�������ȫ��ͬ��operator==()��
          // ��Ϊ�������������ñ���ֵ�Ľڵ��������ָ��ָ��������ֵ��������
          // �Խ�ʡ�ڴ棨Υ��constԭ��
          auto node_to_be_assigned_type_pointer =
              node_to_be_assigned.GetResultTypePointer();
          auto node_for_assign_type_pointer =
              node_for_assign.GetResultTypePointer();
          if (node_to_be_assigned.GetGeneralOperatorType() ==
                  GeneralOperationType::kVariety &&
              *node_to_be_assigned_type_pointer ==
                  *node_for_assign_type_pointer) [[likely]] {
            const_cast<VarietyOperatorNode&>(
                static_cast<const VarietyOperatorNode&>(node_to_be_assigned))
                .SetVarietyType(node_for_assign_type_pointer);
          }
        }
          [[fallthrough]];
        case AssignableCheckResult::kUpperConvert:
        case AssignableCheckResult::kConvertToVoidPointer:
        case AssignableCheckResult::kZeroConvertToPointer:
        case AssignableCheckResult::kUnsignedToSigned:
        case AssignableCheckResult::kSignedToUnsigned:
          break;
        case AssignableCheckResult::kMayBeZeroToPointer:
          // ��������ǽ�0��ֵ��ָ������Ҫ������
          // ���������ֵ���Ƿ�Ϊ�����ڳ���
          if (node_for_assign.GetGeneralOperatorType() ==
              GeneralOperationType::kInitValue) [[likely]] {
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
          } else {
            // �޷�ֱ�Ӹ�ֵ����Ҫǿ������ת��
            check_result = AssignableCheckResult::kCanNotConvert;
          }
          break;
        case AssignableCheckResult::kInitializeList:
          // ������ֵ���ǳ�ʼ���б�������Ӧ�ĺ�������
          return VarietyAssignableByInitializeList(
              static_cast<const VarietyOperatorNode&>(node_to_be_assigned),
              static_cast<const ListInitializeOperatorNode&>(node_for_assign));
          break;
        case AssignableCheckResult::kLowerConvert:
        case AssignableCheckResult::kCanNotConvert:
        case AssignableCheckResult::kAssignedNodeIsConst:
        case AssignableCheckResult::kAssignToRightValue:
        case AssignableCheckResult::kArgumentsFull:
          break;
        default:
          assert(false);
          break;
      }
    } break;
    default:
      assert(false);
      break;
  }
  return check_result;
}

AssignableCheckResult AssignOperatorNode::VarietyAssignableByInitializeList(
    const VarietyOperatorNode& variety_node,
    const ListInitializeOperatorNode& list_initialize_operator_node) {
  switch (variety_node.GetVarietyTypeReference().GetType()) {
    case StructOrBasicType::kBasic:
    case StructOrBasicType::kEnum: {
      // �Ի�������ʹ�ó�ʼ���б�ʱ��ʼ���б���ֻ�ܴ���һ��ֵ
      // ��ֻʹ��һ���ʼ���б�
      auto initialize_members = list_initialize_operator_node.GetListValues();
      // ����ʼ���б����Ƿ�ֻ����һ��ֵ
      if (initialize_members.size() == 1) [[likely]] {
        auto& initialize_value = initialize_members.front();
        // ����Ƿ�ֻʹ��һ���ʼ���б�
        if (initialize_value->GetInitializeType() !=
            InitializeType::kInitializeList) [[unlikely]] {
          return CheckAssignable(variety_node, *initialize_value, true);
        }
      }
      // �����������������޷���ֵ
      return AssignableCheckResult::kCanNotConvert;
    } break;
    case StructOrBasicType::kPointer: {
      const auto& pointer_type =
          static_cast<const c_parser_frontend::type_system::PointerType&>(
              variety_node.GetVarietyTypeReference());
      const auto& list_values = list_initialize_operator_node.GetListValues();
      size_t list_size = list_values.size();
      size_t pointer_array_size = pointer_type.GetArraySize();
      // ����Ƿ�Ϊ��ָ��ʹ�ó�ʼ���б��ʼ��
      if (pointer_array_size == 0) {
        // ��ָ�룬ʹ�ñ����Ա�����ֵ����
        // ���������ֵ�ĳ�ʼ���б��Ƿ�ֻ��һ��ֵ�����ֵ���ǳ�ʼ���б�
        auto& value_for_assign =
            static_cast<const BasicTypeInitializeOperatorNode&>(
                *list_values.front());
        if (list_values.size() == 1 && value_for_assign.GetInitializeType() !=
                                           InitializeType::kInitializeList)
            [[likely]] {
          return CheckAssignable(variety_node, value_for_assign, true);
        } else {
          return AssignableCheckResult::kCanNotConvert;
        }
      }
      // ����������һ�ֱȽϵ���ʱ�ڵ�
      auto [dereferenced_type, const_tag] = pointer_type.DeReference();
      auto sub_variety_node = std::make_unique<VarietyOperatorNode>(
          nullptr, const_tag, LeftRightValueTag::kLeftValue);
      auto set_type_result =
          sub_variety_node->SetVarietyTypeNoCheckFunctionType(
              dereferenced_type);
      AssignableCheckResult check_result =
          AssignableCheckResult::kCanNotConvert;
      // ����Ƿ��ʼ���б���ÿһ��ֵ�����Ը�ֵ
      for (const auto& value : list_values) {
        check_result = CheckAssignable(*sub_variety_node, *value, true);
        switch (check_result) {
          case AssignableCheckResult::kNonConvert:
          case AssignableCheckResult::kUpperConvert:
          case AssignableCheckResult::kConvertToVoidPointer:
          case AssignableCheckResult::kZeroConvertToPointer:
          case AssignableCheckResult::kUnsignedToSigned:
          case AssignableCheckResult::kSignedToUnsigned:
            break;
          case AssignableCheckResult::kLowerConvert:
          case AssignableCheckResult::kCanNotConvert:
          case AssignableCheckResult::kAssignedNodeIsConst:
          case AssignableCheckResult::kInitializeListTooLarge:
            // �޷���ֵ�����
            // ֱ�ӷ��أ������ʣ�ಿ��
            return check_result;
            break;
          case AssignableCheckResult::kInitializeList:
            // �ڵ��øú���ǰ�Ѿ���CheckAssignable���ص�
          case AssignableCheckResult::kAssignToRightValue:
          case AssignableCheckResult::kArgumentsFull:
          default:
            assert(false);
            break;
        }
      }
      // ���нڵ��ͨ�����ԣ����ָ��ָ��������С
      if (pointer_array_size == -1) {
        // ��Ҫ�Զ����������С
        const_cast<c_parser_frontend::type_system::PointerType&>(pointer_type)
            .SetArraySize(list_size);
      } else if (list_size > pointer_array_size) [[unlikely]] {
        // ��ʼ���б������������Ŀ����ָ������ʱ�������С
        check_result = AssignableCheckResult::kInitializeListTooLarge;
      }
      return check_result;
    } break;
    case StructOrBasicType::kUnion: {
      const auto& list_values = list_initialize_operator_node.GetListValues();
      const auto& union_size =
          static_cast<
              const c_parser_frontend::type_system::StructureTypeInterface&>(
              variety_node.GetVarietyTypeReference())
              .TypeSizeOf();
      // ����ʼ���б����Ƿ�ֻ�洢��һ����Ա�Ҹó�Ա���ǳ�ʼ���б�
      if (list_values.size() == 1 && list_values.front()->GetInitializeType() !=
                                         InitializeType::kInitializeList)
          [[likely]] {
        // ���������ֵ�Ķ����Ƿ�С�ڵ��ڹ����������Ķ���
        if (union_size >=
            list_values.front()->GetResultTypePointer()->TypeSizeOf()) {
          return AssignableCheckResult::kNonConvert;
        }
      }
      return AssignableCheckResult::kCanNotConvert;
    }
    case StructOrBasicType::kStruct: {
      const auto& list_values = list_initialize_operator_node.GetListValues();
      const auto& structure_members =
          static_cast<
              const c_parser_frontend::type_system::StructureTypeInterface&>(
              variety_node.GetVarietyTypeReference())
              .GetStructureMembers();
      // �����Ƚ�����ʱ�ڵ�
      auto sub_variety_node = std::make_unique<VarietyOperatorNode>(
          nullptr, ConstTag::kNonConst, LeftRightValueTag::kLeftValue);
      if (list_values.size() > structure_members.size()) [[unlikely]] {
        // �����ĳ�ʼ����Ա�б��С���ڽṹ���ڳ�Ա��Ŀ
        return AssignableCheckResult::kInitializeListTooLarge;
      }
      // �����ʼ���б��е�ֵ�Ƿ���Ը��ṹ���Ա��˳��ֵ
      AssignableCheckResult check_result =
          AssignableCheckResult::kCanNotConvert;
      auto structure_member_iter = structure_members.begin();
      auto list_iter = list_values.begin();
      for (; list_iter != list_values.end();
           ++list_iter, ++structure_member_iter) {
        // ���ýṹ���Ա����
        sub_variety_node->SetVarietyType(structure_member_iter->first);
        sub_variety_node->SetConstTag(structure_member_iter->second);
        check_result = CheckAssignable(*sub_variety_node, **list_iter, true);
        switch (check_result) {
          case AssignableCheckResult::kNonConvert:
          case AssignableCheckResult::kUpperConvert:
          case AssignableCheckResult::kConvertToVoidPointer:
          case AssignableCheckResult::kZeroConvertToPointer:
          case AssignableCheckResult::kUnsignedToSigned:
          case AssignableCheckResult::kSignedToUnsigned:
            break;
          case AssignableCheckResult::kLowerConvert:
          case AssignableCheckResult::kCanNotConvert:
          case AssignableCheckResult::kAssignedNodeIsConst:
          case AssignableCheckResult::kInitializeListTooLarge:
            // �޷���ֵ�������ж�ʣ�����ֱ�ӷ���
            return check_result;
            break;
          case AssignableCheckResult::kInitializeList:
          case AssignableCheckResult::kAssignToRightValue:
          case AssignableCheckResult::kArgumentsFull:
          case AssignableCheckResult::kMayBeZeroToPointer:
          default:
            assert(false);
            break;
        }
      }
      // ��ʼ���б������е�ֵ�����Ը��ṹ���Ա��ֵ
      return check_result;
    } break;
    case StructOrBasicType::kInitializeList:
    case StructOrBasicType::kFunction:
    default:
      assert(false);
      // ��ֹ����
      return AssignableCheckResult();
      break;
  }
}

// �����Ƿ�������ã�������������򲻻�����

bool ObtainAddressOperatorNode::SetNodeToObtainAddress(
    const std::shared_ptr<const VarietyOperatorNode>& node_to_obtain_address) {
  bool check_result = CheckNodeToObtainAddress(*node_to_obtain_address);
  if (check_result) [[likely]] {
    // ����ȡ��ַ
    // ���ý�Ҫ��ȡ��ַ��ָ��
    node_to_obtain_address_ = node_to_obtain_address;
    // ����ȡ��ַ��õ��ı���������
    auto obtained_type = TypeInterface::ObtainAddressOperatorNode(
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

inline bool ObtainAddressOperatorNode::CheckNodeToObtainAddress(
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
    const std::shared_ptr<const OperatorNodeInterface>& node_to_dereference) {
  if (CheckNodeDereferenceAble(*node_to_dereference)) [[likely]] {
    // ���ô������õĽڵ�
    node_to_dereference_ = node_to_dereference;
    // ���������ú�õ��Ľڵ�
    auto node_to_dereference_type = std::static_pointer_cast<
        const c_parser_frontend::type_system::PointerType>(
        node_to_dereference->GetResultTypePointer());
    // ��ȡ�����ú�����ͺͽ����õĽ��
    auto [dereferenced_node_type, dereferenced_node_const_tag] =
        node_to_dereference_type->DeReference();
    auto dereferenced_node = std::make_shared<VarietyOperatorNode>(
        nullptr, dereferenced_node_const_tag, LeftRightValueTag::kRightValue);
    bool result = dereferenced_node->SetVarietyTypeNoCheckFunctionType(
        std::move(dereferenced_node_type));
    assert(result == true);
    SetDereferencedNode(std::move(dereferenced_node));
    return true;
  } else {
    return false;
  }
}

inline bool DereferenceOperatorNode::CheckNodeDereferenceAble(
    const OperatorNodeInterface& node_to_dereference) {
  // ���ҽ���Ϊָ��ʱ���Խ�����
  // ��Ҫ������õõ��Ĳ���void����
  auto result_type = node_to_dereference.GetResultTypePointer();
  bool result = result_type->GetType() == StructOrBasicType::kPointer;
  // ��ȡָ��ڵ����һ���ڵ㣬�ж��Ƿ�Ϊvoid����
  auto& next_type_node = result_type->GetNextNodeReference();
  result &= !(next_type_node.GetType() == StructOrBasicType::kBasic &&
              static_cast<const c_parser_frontend::type_system::BasicType&>(
                  next_type_node)
                      .GetBuiltInType() == BuiltInType::kVoid);
  return result;
}

bool LogicalOperationOperatorNode::SetLeftOperatorNode(
    const std::shared_ptr<const OperatorNodeInterface>& left_operator_node) {
  std::shared_ptr<const TypeInterface> left_operator_node_type =
      left_operator_node->GetResultTypePointer();
  if (CheckLogicalTypeValid(*left_operator_node_type)) [[likely]] {
    left_operator_node_ = left_operator_node;
    return true;
  } else {
    return false;
  }
}

bool LogicalOperationOperatorNode::SetRightOperatorNode(
    const std::shared_ptr<const OperatorNodeInterface>& right_operator_node) {
  std::shared_ptr<const TypeInterface> right_operator_node_type =
      right_operator_node->GetResultTypePointer();
  if (CheckLogicalTypeValid(*right_operator_node_type)) [[likely]] {
    right_operator_node_ = right_operator_node;
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

bool TypeConvert::GenerateDestinationNode(
    const std::shared_ptr<const TypeInterface>& new_type,
    ConstTag new_const_tag) {
  // ��ȡ���ս���ڵ���Ϊ����ת���Ľڵ�
  auto node_for_convert = GetSourceNodeReference().GetResultOperatorNode();
  // �������nullptr�����������ǽ���ڵ㣬��VarietyOperatorNode
  if (node_for_convert == nullptr) [[likely]] {
    node_for_convert = GetSourceNodePointer();
  }
  // ����һ�ݶ�����Ϊת����õ��Ķ����޸�����Ϊת����������
  auto node_converted = node_for_convert->SelfCopy(new_type);
  // �ж��Ƿ񲻿ɸ���
  if (node_converted == nullptr) [[unlikely]] {
    return false;
  }
  destination_node_ = std::move(node_converted);
  return true;
}

bool TemaryOperatorNode::SetBranchCondition(
    const std::shared_ptr<const OperatorNodeInterface>& branch_condition,
    const std::shared_ptr<const std::list<
        std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
        flow_control_node_container) {
  if (CheckBranchConditionValid(*branch_condition)) [[likely]] {
    branch_condition_ = branch_condition;
    condition_flow_control_node_container_ = flow_control_node_container;
    return true;
  } else {
    return false;
  }
}

bool TemaryOperatorNode::SetTrueBranch(
    const std::shared_ptr<const OperatorNodeInterface>& true_branch,
    const std::shared_ptr<const std::list<
        std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
        flow_control_node_container) {
  assert(GetBranchConditionPointer() != nullptr);
  if (GetBranchConditionReference().GetGeneralOperatorType() !=
      GeneralOperationType::kInitValue) [[likely]] {
    // ��֧������Ϊ�����ڳ���
    if (CheckBranchValid(*true_branch)) [[likely]] {
      true_branch_ = true_branch;
      true_branch_flow_control_node_container_ = flow_control_node_container;
      return true;
    } else {
      return false;
    }
  } else {
    // ��֧����Ϊ�����ڳ���
    const auto& branch_condition =
        static_cast<const BasicTypeInitializeOperatorNode&>(
            GetBranchConditionReference());
    assert(branch_condition.GetInitializeType() !=
           InitializeType::kInitializeList);
    const std::string& branch_value = branch_condition.GetValue();
    assert(branch_value == "0" || branch_value == "1");
    if (branch_value == "1") {
      // ѡ�����֧����Ҫ���
      if (CheckBranchValid(*true_branch)) [[likely]] {
        true_branch_ = true_branch;
        true_branch_flow_control_node_container_ = flow_control_node_container;
        // ͬʱ���ý����֧�ڵ�
        result_ = true_branch;
        return true;
      } else {
        return false;
      }
    } else {
      // ѡ��ٷ�֧�����֧��������
      true_branch_ = true_branch;
      true_branch_flow_control_node_container_ = flow_control_node_container;
      return true;
    }
  }
  ConstructResultNode();
}

bool TemaryOperatorNode::SetFalseBranch(
    const std::shared_ptr<const OperatorNodeInterface>& false_branch,
    const std::shared_ptr<const std::list<
        std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>>&
        flow_control_node_container) {
  assert(GetBranchConditionPointer() != nullptr);
  if (GetBranchConditionReference().GetGeneralOperatorType() !=
      GeneralOperationType::kInitValue) [[likely]] {
    // ��֧������Ϊ�����ڳ���
    if (CheckBranchValid(*false_branch)) [[likely]] {
      false_branch_ = false_branch;
      false_branch_flow_control_node_container_ = flow_control_node_container;
      return true;
    } else {
      return false;
    }
  } else {
    // ��֧����Ϊ�����ڳ���
    const auto& branch_condition =
        static_cast<const BasicTypeInitializeOperatorNode&>(
            GetBranchConditionReference());
    assert(branch_condition.GetInitializeType() !=
           InitializeType::kInitializeList);
    const std::string& branch_value = branch_condition.GetValue();
    assert(branch_value == "0" || branch_value == "1");
    if (branch_value == "0") {
      // ѡ��ٷ�֧����Ҫ���
      if (CheckBranchValid(*false_branch)) [[likely]] {
        false_branch_ = false_branch;
        false_branch_flow_control_node_container_ = flow_control_node_container;
        // ͬʱ���ý����֧�ڵ�
        result_ = false_branch;
        return true;
      } else {
        return false;
      }
    } else {
      // ѡ�����֧���ٷ�֧��������
      false_branch_ = false_branch;
      false_branch_flow_control_node_container_ = flow_control_node_container;
      return true;
    }
  }
  ConstructResultNode();
}

bool TemaryOperatorNode::CheckBranchConditionValid(
    const OperatorNodeInterface& branch_condition) {
  auto result_type = branch_condition.GetResultTypePointer();
  switch (result_type->GetType()) {
    case StructOrBasicType::kBasic:
      // ��һ���ж��Ƿ�Ϊvoid
      return static_cast<const c_parser_frontend::type_system::BasicType&>(
                 *result_type)
                 .GetBuiltInType() != BuiltInType::kVoid;
      break;
    case StructOrBasicType::kPointer:
      return true;
      break;
    default:
      return false;
      break;
  }
}

bool TemaryOperatorNode::CheckBranchValid(const OperatorNodeInterface& branch) {
  return CheckBranchConditionValid(branch);
}

bool TemaryOperatorNode::ConstructResultNode() {
  // ����֧�����Ƿ�Ϊ�����ڳ���
  // ���Ϊ���������ڵ�������
  // ����Ƿ�����������֧
  if (GetBranchConditionReference().GetGeneralOperatorType() !=
          GeneralOperationType::kInitValue &&
      GetTrueBranchPointer() != nullptr && GetFalseBranchPointer() != nullptr)
      [[likely]] {
    // �����֧δ����
    // ��ȡ������֧������ת����������
    std::shared_ptr<const TypeInterface> common_type;
    auto assignable_check_result = AssignOperatorNode::CheckAssignable(
        GetTrueBranchReference(), GetFalseBranchReference(), false);
    switch (assignable_check_result) {
      case AssignableCheckResult::kNonConvert:
      case AssignableCheckResult::kUpperConvert:
      case AssignableCheckResult::kConvertToVoidPointer:
      case AssignableCheckResult::kZeroConvertToPointer:
      case AssignableCheckResult::kUnsignedToSigned:
      case AssignableCheckResult::kSignedToUnsigned:
        common_type = GetTrueBranchReference().GetResultTypePointer();
        break;
      case AssignableCheckResult::kLowerConvert:
        // ���Է���ת��
        common_type = GetFalseBranchReference().GetResultTypePointer();
        break;
      case AssignableCheckResult::kInitializeList:
        // ��֧���ڷǱ����ڳ���������ʹ�ó�ʼ���б�
        return false;
        break;
      case AssignableCheckResult::kCanNotConvert:
      case AssignableCheckResult::kAssignedNodeIsConst:
      case AssignableCheckResult::kAssignToRightValue:
        // �޷���true��֧ת����false��֧�����Է���ת��
        assignable_check_result = AssignOperatorNode::CheckAssignable(
            GetFalseBranchReference(), GetTrueBranchReference(), false);
        switch (assignable_check_result) {
          case AssignableCheckResult::kCanNotConvert:
          case AssignableCheckResult::kAssignedNodeIsConst:
          case AssignableCheckResult::kAssignToRightValue:
            return false;
            break;
          case AssignableCheckResult::kZeroConvertToPointer:
          case AssignableCheckResult::kConvertToVoidPointer:
            common_type = GetFalseBranchReference().GetResultTypePointer();
            break;
            // �������������Ѵ�����ϻ�Ӧ����
          default:
            assert(false);
            break;
        }
        break;
      case AssignableCheckResult::kArgumentsFull:
      case AssignableCheckResult::kInitializeListTooLarge:
      case AssignableCheckResult::kMayBeZeroToPointer:
      default:
        assert(false);
        break;
    }
    auto result = std::make_shared<VarietyOperatorNode>(
        nullptr, ConstTag::kNonConst, LeftRightValueTag::kRightValue);
    bool set_type_result = result->SetVarietyType(common_type);
    assert(set_type_result);
    result_ = result;
  }
  return true;
}

MathematicalOperation
MathematicalAndAssignOperationToMathematicalOperation(
    MathematicalAndAssignOperation mathematical_and_assign_operation) {
  switch (mathematical_and_assign_operation) {
    case MathematicalAndAssignOperation::kOrAssign:
      return MathematicalOperation::kOr;
      break;
    case MathematicalAndAssignOperation::kXorAssign:
      return MathematicalOperation::kXor;
      break;
    case MathematicalAndAssignOperation::kAndAssign:
      return MathematicalOperation::kAnd;
      break;
    case MathematicalAndAssignOperation::kLeftShiftAssign:
      return MathematicalOperation::kLeftShift;
      break;
    case MathematicalAndAssignOperation::kRightShiftAssign:
      return MathematicalOperation::kRightShift;
      break;
    case MathematicalAndAssignOperation::kPlusAssign:
      return MathematicalOperation::kPlus;
      break;
    case MathematicalAndAssignOperation::kMinusAssign:
      return MathematicalOperation::kMinus;
      break;
    case MathematicalAndAssignOperation::kMultipleAssign:
      return MathematicalOperation::kMultiple;
      break;
    case MathematicalAndAssignOperation::kDivideAssign:
      return MathematicalOperation::kDivide;
      break;
    case MathematicalAndAssignOperation::kModAssign:
      return MathematicalOperation::kMod;
      break;
    default:
      assert(false);
      // ��ֹ����
      return MathematicalOperation();
      break;
  }
}

}  // namespace c_parser_frontend::operator_node

// ʹ��ǰ�����������Ƶ�������ʹ������ָ��
namespace c_parser_frontend::type_system {
bool FunctionType::ArgumentInfo::operator==(
    const ArgumentInfo& argument_info) const {
  // ������������Ӱ���Ƿ�Ϊͬһ���������Ƚ����ͺ��Ƿ�Ϊconst
  return variety_operator_node->GetConstTag() ==
             argument_info.variety_operator_node->GetConstTag() &&
         variety_operator_node->GetVarietyTypeReference() ==
             argument_info.variety_operator_node->GetVarietyTypeReference();
}
void FunctionType::AddFunctionCallArgument(
    const std::shared_ptr<
        const c_parser_frontend::operator_node::VarietyOperatorNode>&
        argument) {
  argument_infos_.emplace_back(ArgumentInfo(argument));
}
}  // namespace c_parser_frontend::type_system