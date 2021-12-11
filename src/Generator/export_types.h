/// @file export_types.h
/// @brief �洢���Ⱪ¶��һЩ��������
#ifndef COMMON_ENUM_AND_TYPES_H_
#define COMMON_ENUM_AND_TYPES_H_

#include <boost/serialization/array.hpp>
#include <boost/serialization/utility.hpp>
#include <string>
#include <vector>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/object_manager.h"
#include "Common/unordered_struct_manager.h"

namespace frontend::generator::syntax_generator {
/// @brief ID��װ����������ͬID��ö��
enum class WrapperLabel {
  kPriorityLevel,
  kNextWordToShiftIndex,
  kSyntaxAnalysisTableEntryId,
  kProductionBodyId,
};
/// @brief ����������
using OperatorAssociatityType = frontend::common::OperatorAssociatityType;
/// @brief ����ʽ�ڵ�����
/// @details
/// Ϊ��֧��ClassfiyProductionNodes�������Զ������ֵ
/// ����Զ������ֵ���������ֵ������С��sizeof(ProductionNodeType)
enum class ProductionNodeType {
  kTerminalNode,     ///< �ս����ʽ�ڵ�
  kOperatorNode,     ///< ������ڵ�
  kNonTerminalNode,  ///< ���ս����ʽ�ڵ�
  kEndNode           ///< �ļ�β�ڵ�
};
/// @brief �﷨������������
enum class ActionType {
  kShift,        ///< ����
  kReduct,       ///< ��Լ
  kShiftReduct,  ///< ������Լ
  kError,        ///< ����
  kAccept        ///< ����
};

// ǰ�������࣬������ȡ�������Ľṹ�����ı�ʶID
class BaseProductionNode;
class ProductionItemSet;
class ProcessFunctionInterface;
class SyntaxAnalysisTableEntry;

using frontend::common::ExplicitIdWrapper;
using frontend::common::ObjectManager;
using frontend::common::UnorderedStructManager;

/// @brief ��������ȼ�
/// @details
/// 1.����Խ�����ȼ�Խ��
/// 2.�����Լ��ͻ�Ҵ�����ڵ�Ϊ�����ʱ������������ȼ��������뻹�ǹ鲢
/// 3.��TailNodePriority���岻ͬ�������ȼ�Ӱ���﷨��������
using OperatorPriority =
    ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kPriorityLevel>;
/// @brief ���ս�ڵ����ʽ����
using ProductionBodyId =
    ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kProductionBodyId>;
/// @brief ������һ������ĵ��ʵ�λ��
using NextWordToShiftIndex =
    ExplicitIdWrapper<size_t, WrapperLabel,
                      WrapperLabel::kNextWordToShiftIndex>;
/// @brief �﷨��������ĿID
using SyntaxAnalysisTableEntryId =
    ExplicitIdWrapper<size_t, WrapperLabel,
                      WrapperLabel::kSyntaxAnalysisTableEntryId>;
/// @brief ����ID
using SymbolId =
    UnorderedStructManager<std::string, std::hash<std::string>>::ObjectId;
/// @brief ����ʽ�ڵ�ID
using ProductionNodeId = ObjectManager<BaseProductionNode>::ObjectId;
/// @brief �ID
using ProductionItemSetId = ObjectManager<ProductionItemSet>::ObjectId;
/// @brief �����װ��Լ���������ʵ�������������
using ProcessFunctionClassManagerType = ObjectManager<ProcessFunctionInterface>;
/// @brief ��װ��Լ���������ʵ��������ID
using ProcessFunctionClassId = ProcessFunctionClassManagerType::ObjectId;
/// @brief �﷨����������
using SyntaxAnalysisTableType = std::vector<SyntaxAnalysisTableEntry>;
}  // namespace frontend::generator::syntax_generator

namespace frontend::generator::dfa_generator {
namespace nfa_generator {
/// @brief �Զ������͵ķַ���ǩ
enum class WrapperLabel { kTailNodePriority, kTailNodeId };
/// @brief �������ȼ�������������ȼ���ͬ
using WordPriority =
    frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                        WrapperLabel::kTailNodePriority>;
/// @class WordAttachedData export_types.h
/// @brief ���ʸ������ݣ��ڼ�⵽��Ӧ����ʱ���أ�
struct WordAttachedData {
  using ProductionNodeId =
      frontend::generator::syntax_generator::ProductionNodeId;
  using ProductionNodeType =
      frontend::generator::syntax_generator::ProductionNodeType;
  using OperatorAssociatityType =
      frontend::generator::syntax_generator::OperatorAssociatityType;
  using OperatorPriority =
      frontend::generator::syntax_generator::OperatorPriority;

  /// @brief �������л������
  friend class boost::serialization::access;

  /// @brief ���л����õĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
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

  /// @brief ��ȡ����Ժ���������ȼ�
  /// @return ǰ�벿��Ϊ����ԣ���벿��Ϊ��������ȼ�
  /// @details ������һ�������Ƿ�Ϊ��Լȷ��ʹ����൥Ŀ����˫Ŀ���������
  std::pair<OperatorAssociatityType, OperatorPriority>
  GetAssociatityTypeAndPriority(bool is_last_operate_reduct) const;

  /// @brief ����ʽ�ڵ�ID
  /// @attention ID��Ψһ�ģ���һ��ID��Ӧ����������Ψһ
  ProductionNodeId production_node_id = ProductionNodeId::InvalidId();
  /// @brief �ڵ�����
  ProductionNodeType node_type;
  /// @brief ˫Ŀ����������
  /// @note �����������Ч�����������ʹ��Ĭ��ֵ�Ա���==��!=����
  OperatorAssociatityType binary_operator_associate_type =
      OperatorAssociatityType::kLeftToRight;
  /// @brief ˫Ŀ��������ȼ�
  /// @note �����������Ч�����������ʹ��Ĭ��ֵ�Ա���==��!=����
  OperatorPriority binary_operator_priority = OperatorPriority::InvalidId();
  /// @brief ��൥Ŀ����������
  /// @note �����������Ч�����������ʹ��Ĭ��ֵ�Ա���==��!=����
  OperatorAssociatityType unary_operator_associate_type =
      OperatorAssociatityType::kLeftToRight;
  /// @brief ��൥Ŀ��������ȼ�
  /// @note �����������Ч�����������ʹ��Ĭ��ֵ�Ա���==��!=����
  OperatorPriority unary_operator_priority = OperatorPriority::InvalidId();
};
}  // namespace nfa_generator

/// @class TransformArrayManager export_types.h
/// @brief ����ת�Ʊ�
/// @tparam BasicObjectType ���ڲ���������
/// @details
/// 1.������DfaGenerator��Ϊ�˱���ʹ��char���±�ʱʹ�ø��±�����Խ��
/// 2.�洢frontend::common::kCharNum��BasicObjectType���͵Ķ���
/// @note ����ֱ��ʹ��CHAR_MIN~CHAR_MAX����ֵ����
template <class BasicObjectType>
class TransformArrayManager {
 public:
  const BasicObjectType& operator[](char c) const {
    return transform_array_[(c + frontend::common::kCharNum) %
                            frontend::common::kCharNum];
  }
  BasicObjectType& operator[](char c) {
    return const_cast<BasicObjectType&>(
        static_cast<const TransformArrayManager&>(*this).operator[](c));
  }
  /// @brief ���ת�Ʊ�
  /// @param[in] fill_object ���������Ķ���
  void fill(const BasicObjectType& fill_object) {
    transform_array_.fill(fill_object);
  }

 private:
  /// @brief �������л������
  friend class boost::serialization::access;

  /// @brief ���л����õĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& transform_array_;
  }

  /// @brief ת�Ʊ�
  std::array<BasicObjectType, frontend::common::kCharNum> transform_array_;
};
/// @brief ���ʸ������ݣ��ڼ�⵽��Ӧ����ʱ���أ�
using WordAttachedData = nfa_generator::WordAttachedData;
/// @brief ID��װ���������ֲ�ͬID��ö��
enum class WrapperLabel { kTransformArrayId };
/// @brief ת�Ʊ�ID
using TransformArrayId =
    frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                        WrapperLabel::kTransformArrayId>;
/// @brief ת�Ʊ���Ŀ
using TransformArray = TransformArrayManager<TransformArrayId>;
/// @brief DFA��������
using DfaConfigType = std::vector<std::pair<TransformArray, WordAttachedData>>;
}  // namespace frontend::generator::dfa_generator
#endif  /// !COMMON_ENUM_AND_TYPES_H_