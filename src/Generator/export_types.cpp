#include "export_types.h"

namespace frontend::generator::dfa_generator::nfa_generator {

// ������һ�������Ƿ�Ϊ��Լ�ж�ʹ����൥Ŀ��������ȼ�����˫Ŀ��������ȼ�
// ���ػ�ȡ���Ľ�����ͺ����ȼ�

std::pair<frontend::generator::syntax_generator::OperatorAssociatityType,
          size_t>
WordAttachedData::GetAssociatityTypeAndPriority(
    bool is_last_operate_reduct) const {
  assert(
      node_type ==
      frontend::generator::syntax_generator::ProductionNodeType::kOperatorNode);
  if (binary_operator_priority != -1) {
    if (unary_operator_priority != -1) {
      // �������������
      if (is_last_operate_reduct) {
        // �ϴβ���Ϊ��Լ��Ӧʹ����൥Ŀ���������
        return std::make_pair(unary_operator_associate_type,
                              unary_operator_priority);
      } else {
        // �ϴβ���Ϊ���룬Ӧʹ��˫Ŀ���������
        return std::make_pair(binary_operator_associate_type,
                              binary_operator_priority);
      }
    } else {
      // ������˫Ŀ��������壬ֱ�ӷ���
      return std::make_pair(binary_operator_associate_type,
                            binary_operator_priority);
    }
  } else {
    // �����ڵ�Ŀ��������壬ֱ�ӷ���
    return std::make_pair(unary_operator_associate_type,
                          unary_operator_priority);
  }
}
bool nfa_generator::WordAttachedData::operator==(
    const WordAttachedData& saved_data) const {
  return (production_node_id == saved_data.production_node_id &&
          node_type == saved_data.node_type &&
          binary_operator_associate_type ==
              saved_data.binary_operator_associate_type &&
          binary_operator_priority == saved_data.binary_operator_priority &&
          unary_operator_associate_type ==
              saved_data.unary_operator_associate_type &&
          unary_operator_priority == saved_data.unary_operator_priority);
}
}  // namespace frontend::generator::dfa_generator::nfa_generator