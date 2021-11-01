// �洢���Ⱪ¶��һЩ��������
#ifndef COMMON_ENUM_AND_TYPES_H_
#define COMMON_ENUM_AND_TYPES_H_

#include <boost/serialization/array.hpp>
#include <string>
#include <vector>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/object_manager.h"
#include "Common/unordered_struct_manager.h"

namespace frontend::generator::syntax_generator {
// ID��װ����������ͬID��ö��
enum class WrapperLabel {
  kPriorityLevel,
  kNextWordToShiftIndex,
  kSyntaxAnalysisTableEntryId,
  kProductionBodyId,
};
using OperatorAssociatityType = frontend::common::OperatorAssociatityType;
// ����ʽ�ڵ����ͣ��ս���ţ�����������ս���ţ��ļ�β�ڵ�
// Ϊ��֧��ClassfiyProductionNodes�������Զ������ֵ
// ����Զ������ֵ���������ֵ������С��sizeof(ProductionNodeType)
enum class ProductionNodeType {
  kTerminalNode,
  kOperatorNode,
  kNonTerminalNode,
  kEndNode
};
// �����������ͣ���Լ�����룬����͹�Լ����������
enum class ActionType { kReduct, kShift, kShiftReduct, kError, kAccept };

// ǰ�������࣬������ȡ�������Ľṹ�����ı�ʶID
class BaseProductionNode;
class ProductionItemSet;
class ProcessFunctionInterface;
class SyntaxAnalysisTableEntry;

using frontend::common::ExplicitIdWrapper;
using frontend::common::ObjectManager;
using frontend::common::UnorderedStructManager;

// ��������ȼ�������Խ�����ȼ�Խ�ߣ�����������ڵ���Ч
// ��TailNodePriority���岻ͬ�������ȼ�Ӱ���﷨��������
// �����������������ʱ�������뻹�ǹ鲢
using OperatorPriority =
    ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kPriorityLevel>;
// ���ս�ڵ��ڲ���ʽ���
using ProductionBodyId =
    ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kProductionBodyId>;
// �﷨����ʱһ������һ������ĵ��ʵ�λ��
using NextWordToShiftIndex =
    ExplicitIdWrapper<size_t, WrapperLabel,
                      WrapperLabel::kNextWordToShiftIndex>;
// �﷨��������ĿID
using SyntaxAnalysisTableEntryId =
    ExplicitIdWrapper<size_t, WrapperLabel,
                      WrapperLabel::kSyntaxAnalysisTableEntryId>;
// ����ID
using SymbolId =
    UnorderedStructManager<std::string, std::hash<std::string>>::ObjectId;
// ����ʽ�ڵ�ID
using ProductionNodeId = ObjectManager<BaseProductionNode>::ObjectId;
// �ID
using ProductionItemSetId = ObjectManager<ProductionItemSet>::ObjectId;
// �����װ�û��Զ��庯�������ݵ�����ѷ�����������
using ProcessFunctionClassManagerType = ObjectManager<ProcessFunctionInterface>;
// ��װ�û��Զ��庯�������ݵ�����ѷ������ID
using ProcessFunctionClassId = ProcessFunctionClassManagerType::ObjectId;
// �﷨����������
using SyntaxAnalysisTableType = std::vector<SyntaxAnalysisTableEntry>;
}  // namespace frontend::generator::syntax_generator

namespace frontend::generator::dfa_generator {
namespace nfa_generator {
// �Զ������͵ķַ���ǩ
enum class WrapperLabel { kTailNodePriority, kTailNodeId };
// �������ȼ�������������ȼ���ͬ
using WordPriority =
    frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                        WrapperLabel::kTailNodePriority>;
// ���ʸ������ݣ��ڼ�⵽��Ӧ����ʱ���أ�
struct WordAttachedData {
  using ProductionNodeId =
      frontend::generator::syntax_generator::ProductionNodeId;
  using ProductionNodeType =
      frontend::generator::syntax_generator::ProductionNodeType;
  using OperatorAssociatityType =
      frontend::generator::syntax_generator::OperatorAssociatityType;
  using OperatorPriority =
      frontend::generator::syntax_generator::OperatorPriority;

  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& production_node_id;
    ar& node_type;
    ar& binary_operator_associate_type;
    ar& binary_operator_priority;
    ar& unary_operator_associate_type;
    ar& unary_operator_priority;
  }

  bool operator==(const WordAttachedData& saved_data) const;

  bool operator!=(const WordAttachedData& saved_data) const {
    return !operator==(saved_data);
  }

  // ������һ�������Ƿ�Ϊ��Լ�ж�ʹ����൥Ŀ��������ȼ�����˫Ŀ��������ȼ�
  // ���ػ�ȡ���Ľ�����ͺ����ȼ�
  std::pair<OperatorAssociatityType, OperatorPriority>
  GetAssociatityTypeAndPriority(bool is_last_operate_reduct) const;

  // ����ʽ�ڵ�ID
  // Ӧ��֤ID��Ψһ�ģ���һ��ID��Ӧ��������Ψһ
  ProductionNodeId production_node_id = ProductionNodeId::InvalidId();
  // �ڵ�����
  ProductionNodeType node_type;
  // ������������������Ч�����������ʹ��Ĭ��ֵ�Ա���==��!=����
  // ˫Ŀ����������
  OperatorAssociatityType binary_operator_associate_type =
      OperatorAssociatityType::kLeftToRight;
  // ˫Ŀ��������ȼ�
  OperatorPriority binary_operator_priority = OperatorPriority::InvalidId();
  // ��൥Ŀ����������
  OperatorAssociatityType unary_operator_associate_type =
      OperatorAssociatityType::kLeftToRight;
  // ��൥Ŀ��������ȼ�
  OperatorPriority unary_operator_priority = OperatorPriority::InvalidId();
};
}  // namespace nfa_generator

// ����ת�Ʊ��ã�������DfaGenerator��Ϊ�˱���ʹ��char���±�ʱʹ�ø��±�����Խ��
// ����ֱ��ʹ��CHAR_MIN~CHAR_MAX����ֵ����
template <class BasicObjectType>
class TransformArrayManager {
 public:
  BasicObjectType& operator[](char c) {
    return transform_array_[(c + frontend::common::kCharNum) %
                            frontend::common::kCharNum];
  }
  void fill(const BasicObjectType& fill_object) {
    transform_array_.fill(fill_object);
  }

 private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& transform_array_;
  }

  std::array<BasicObjectType, frontend::common::kCharNum> transform_array_;
};
// ���ʸ������ݣ��ڼ�⵽��Ӧ����ʱ���أ�
using WordAttachedData = nfa_generator::WordAttachedData;
// ID��װ���������ֲ�ͬID��ö��
enum class WrapperLabel { kTransformArrayId };
// ״̬ת�Ʊ�ID
using TransformArrayId =
    frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                        WrapperLabel::kTransformArrayId>;
// ״̬ת�Ʊ���Ŀ
using TransformArray = TransformArrayManager<TransformArrayId>;
// DFA��������
using DfaConfigType = std::vector<std::pair<TransformArray, WordAttachedData>>;
}  // namespace frontend::generator::dfa_generator
#endif  // !COMMON_ENUM_AND_TYPES_H_