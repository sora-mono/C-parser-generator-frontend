/// @file production_node.h
/// @brief ��������ʽ����
/// @details
/// ���ļ��ж�������������ʽ����
/// TerminalProductionNode ���ս����ʽ
/// OperatorProductionNode �������
/// NonTerminalProductionNode �����ս����ʽ
/// EndNode ���ļ�β
/// ��Щ��������ʾ��ͬ����ʽ
#ifndef GENERATOR_SYNTAXGENERATOR_PRODUCTION_NODE_H_
#define GENERATOR_SYNTAXGENERATOR_PRODUCTION_NODE_H_
#include "Generator/export_types.h"

namespace frontend::generator::syntax_generator {
/// @class BaseProductionNode production_node.h
/// @brief ���������б�ʾ�������ʽ����������Ļ���
/// @details
/// �����а������ڵ����͡��ڵ�ID������ʽ��ID
/// @attention ���б�ʾ�������ʽ�������඼Ӧ�̳��Ը���
class BaseProductionNode {
 public:
  BaseProductionNode(ProductionNodeType type, SymbolId symbol_id)
      : base_type_(type),
        base_symbol_id_(symbol_id),
        base_id_(ProductionNodeId::InvalidId()) {}
  BaseProductionNode(const BaseProductionNode&) = delete;
  BaseProductionNode& operator=(const BaseProductionNode&) = delete;
  BaseProductionNode(BaseProductionNode&& base_production_node)
      : base_type_(base_production_node.base_type_),
        base_id_(base_production_node.base_id_),
        base_symbol_id_(base_production_node.base_symbol_id_) {}
  BaseProductionNode& operator=(BaseProductionNode&& base_production_node);
  virtual ~BaseProductionNode() {}

  /// @brief ���ò���ʽ����
  /// @param[in] type ������ʽ����
  void SetType(ProductionNodeType type) { base_type_ = type; }
  /// @brief ��ȡ����ʽ����
  /// @return ���ز���ʽ����
  ProductionNodeType GetType() const { return base_type_; }
  /// @brief ���øýڵ�ID
  /// @param[in] production_node_id �������õĽڵ�ID
  void SetNodeId(ProductionNodeId production_node_id) {
    base_id_ = production_node_id;
  }
  /// @brief ��ȡ�ýڵ�ID
  /// @return ���ظýڵ�ID
  ProductionNodeId GetNodeId() const { return base_id_; }
  /// @brief ���øýڵ��ʾ�Ĳ���ʽ��ID
  /// @param[in] production_node_id �������õĲ���ʽ��ID
  void SetSymbolId(SymbolId production_node_id) {
    base_symbol_id_ = production_node_id;
  }
  /// @brief ��ȡ�ýڵ��ʾ�Ĳ���ʽ��ID
  /// @return ���ظýڵ��ʾ�Ĳ���ʽ��ID
  SymbolId GetNodeSymbolId() const { return base_symbol_id_; }

  /// @brief ���ݸ�������ʽ�����һ������Ĳ���ʽλ�û�ȡ��һ������Ĳ���ʽID
  /// @param[in] production_body_id ���Ӳ���ʽ����
  /// @param[in] next_word_to_shift_index ������ʽ���в���ʽID��λ��
  /// @return ����production_body_id��next_word_to_shift_indexָ���Ĳ���ʽID
  /// @details
  /// point_indexԽ�磨���ڵ���production_body_idָ�����Ӳ���ʽ�в���ʽ��Ŀ��ʱ
  /// ����ProducNodeId::InvalidId()
  /// production_body_id���ó����ò���ʽӵ�еĲ���ʽ�����
  /// �ս����ʽ�����������ʽֻ����һ���Ӳ���ʽ���Ӳ���ʽ��ֻ����һ������ʽ��
  /// ����Ӧʹ��
  /// ProductionBodyId(0),NextWordToShiftIndex(0)�ſ��Ի�ȡ��Ч����ʽID
  /// @note
  /// ʹ�þ�����
  ///   ����ʽ����
  ///   IdOrEquivence -> ConstTag Id
  ///   IdOrEquivence -> IdOrEquivence "[" Num "]"
  ///   IdOrEquivence -> IdOrEquivence "[" "]"
  ///   IdOrEquivence -> ConstTag "*" IdOrEquivence
  /// ��ȡIdOrEquivence����ʽ�ڵڶ����Ӳ���ʽ�е���������ʽ��"Num"����ID
  /// @code{.cpp}
  ///   NonTerminalProductionNode example_node;
  ///   example_node.GetProductionNodeInBody(ProductionBodyId(1),
  ///                                        NextWordToShiftIndex(2));
  /// @endcode
  /// @attention �����������Ӧ��д�ú��������øı�ԭʼ����
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex next_word_to_shift_index) const = 0;

 private:
  /// @brief ����ʽ����
  ProductionNodeType base_type_;
  /// @brief �ڵ�ID
  ProductionNodeId base_id_;
  /// @brief ����ʽ��ID
  SymbolId base_symbol_id_;
};

/// @class TerminalProductionNode production_node.h
/// @brief ��ʾ�ս����ʽ
/// @note
/// �ս����ʽ��
/// Id    ->    [a-zA-Z_][a-zA-Z0-9_]*
/// ^����ʽ��   ^����ʽ����
class TerminalProductionNode : public BaseProductionNode {
 public:
  TerminalProductionNode(SymbolId node_symbol_id, SymbolId body_symbol_id)
      : BaseProductionNode(ProductionNodeType::kTerminalNode, node_symbol_id) {
    SetBodySymbolId(body_symbol_id);
  }
  TerminalProductionNode(const TerminalProductionNode&) = delete;
  TerminalProductionNode& operator=(const TerminalProductionNode&) = delete;
  TerminalProductionNode(TerminalProductionNode&& terminal_production_node)
      : BaseProductionNode(std::move(terminal_production_node)),
        body_symbol_id_(std::move(terminal_production_node.body_symbol_id_)) {}
  TerminalProductionNode& operator=(
      TerminalProductionNode&& terminal_production_node) {
    BaseProductionNode::operator=(std::move(terminal_production_node));
    body_symbol_id_ = std::move(terminal_production_node.body_symbol_id_);
    return *this;
  }

  /// @brief ��ȡ����ʽ����ID
  /// @return ���ز���ʽ����ID
  SymbolId GetBodySymbolId() const { return body_symbol_id_; }
  /// @brief ���ò���ʽ����ID
  /// @param[in] body_symbol_id �������õĲ���ʽ����ID
  void SetBodySymbolId(SymbolId body_symbol_id) {
    body_symbol_id_ = body_symbol_id;
  }
  /// @brief ��ȡ����ʽ���ڵĲ���ʽ
  /// @param[in] production_body_id ������ȡ�Ĳ���ʽ��ID
  /// @param[in] next_word_to_shift_index ������ȡ���Ӳ���ʽλ��
  /// @attention �ս�ڵ���øú���ʱproduction_body_id����Ϊ0
  /// next_word_to_shift_index����0ʱ�򷵻�NextWordToShiftIndex::InvalidId()
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex next_word_to_shift_index) const override;

 private:
  /// @brief ����ʽ����
  SymbolId body_symbol_id_;
};

/// @class OperatorNodeInterface production_node.h
/// @brief ��ʾ���������Ļ���
/// @details
/// �����֧������˫Ŀ���������͵�Ŀ���������
/// ˫Ŀ���������ʹ��ʱ��������ڵĲ���ʽ�������඼Ӧ���ڲ���ʽ
/// ����
/// Example-> Example1 "+" Example2
/// ����"+"Ϊ�����������ʹ��˫Ŀ���������
/// ��Ŀ����������֧����൥Ŀ��������壬��֧���Ҳ൥Ŀ���������
/// ����
/// Assignable -> "!" Assignable
/// ����"!"Ϊ�����������ʹ�õ�Ŀ���������
/// @attention ���б�ʾ��������඼Ӧ�Ӹ�������
class OperatorNodeInterface : public BaseProductionNode {
 public:
  enum class OperatorType {
    kBinary,     // ˫Ŀ�����
    kLeftUnary,  // ��൥Ŀ�����
    kBinaryLeftUnary  // ͬʱ����˫Ŀ������������൥Ŀ���������������
  };
  /// @brief ��ȡ����ʽ���ڵĲ���ʽ
  /// @param[in] production_body_id ������ȡ�Ĳ���ʽ��ID
  /// @param[in] next_word_to_shift_index ������ȡ���Ӳ���ʽλ��
  /// @attention �ս�ڵ���øú���ʱproduction_body_id����Ϊ0
  /// next_word_to_shift_index����0ʱ�򷵻�NextWordToShiftIndex::InvalidId()
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex next_word_to_shift_index) const final;

  /// @brief �������������
  /// @param[in] operator_type ���µ����������
  void SetOperatorType(OperatorType operator_type) {
    operator_type_ = operator_type;
  }
  /// @brief ��ȡ���������
  /// @return �������������
  /// @retval OperatorType::kBinary ��˫Ŀ�����
  /// @retval OperatorType::kLeftUnary ����൥Ŀ�����
  /// @retval OperatorType::kBinaryLeftUnary
  /// ��ͬʱ����˫Ŀ���������൥Ŀ���������
  OperatorType GetOperatorType() const { return operator_type_; }

 protected:
  OperatorNodeInterface(OperatorType operator_type, SymbolId operator_symbol_id)
      : BaseProductionNode(ProductionNodeType::kOperatorNode,
                           operator_symbol_id),
        operator_type_(operator_type) {}
  OperatorNodeInterface(const OperatorNodeInterface&) = delete;
  OperatorNodeInterface& operator=(const OperatorNodeInterface&) = delete;
  OperatorNodeInterface(OperatorNodeInterface&& operator_production_node)
      : BaseProductionNode(std::move(operator_production_node)),
        operator_type_(std::move(operator_production_node.operator_type_)) {}
  OperatorNodeInterface& operator=(
      OperatorNodeInterface&& operator_production_node);

 private:
  OperatorType operator_type_;
};

/// @class BinaryOperatorNode production_node.h
/// @brief ��ʾ˫Ŀ�����
class BinaryOperatorNode : public OperatorNodeInterface {
 public:
  BinaryOperatorNode(SymbolId operator_symbol_id,
                     OperatorAssociatityType binary_operator_associatity_type,
                     OperatorPriority binary_operator_priority)
      : OperatorNodeInterface(OperatorType::kBinary, operator_symbol_id),
        unary_operator_associatity_type_(binary_operator_associatity_type),
        unary_operator_priority_(binary_operator_priority) {}

  /// @brief ��������������
  /// @param[in] type �������õ�����������
  /// @note
  /// ���������Էִ����ҽ�Ϻʹ��ҵ�����
  void SetAssociatityType(OperatorAssociatityType type) {
    unary_operator_associatity_type_ = type;
  }
  /// @brief ��ȡ����������
  /// @return ��������������
  /// @retval OperatorAssociatityType::kLeftToRight �������ҽ��
  /// @retval OperatorAssociatityType::kRightToLeft �����ҵ�����
  OperatorAssociatityType GetAssociatityType() const {
    return unary_operator_associatity_type_;
  }
  /// @brief ������������ȼ��ȼ�
  /// @param[in] priority �������õ���������ȼ��ȼ�
  /// @attention ��������ȼ��뵥�����ȼ����岻ͬ
  /// ��������ȼ��������﷨������ֻ�������ӵ��
  /// �������ȼ������ڴʷ�������ÿ�����ʶ���
  void SetPriority(OperatorPriority priority) {
    unary_operator_priority_ = priority;
  }
  /// @brief ��ȡ��������ȼ�
  /// @return ������������ȼ�
  OperatorPriority GetPriority() const { return unary_operator_priority_; }

 private:
  /// @brief ����������
  OperatorAssociatityType unary_operator_associatity_type_;
  /// @brief ��������ȼ�
  OperatorPriority unary_operator_priority_;
};

/// @class UnaryOperatorNode production_node.h
/// @brief ��ʾ��Ŀ�����
/// @attention ��֧����൥Ŀ�����
class UnaryOperatorNode : public OperatorNodeInterface {
 public:
  UnaryOperatorNode(SymbolId operator_symbol_id,
                    OperatorAssociatityType unary_operator_associatity_type,
                    OperatorPriority unary_operator_priority)
      : OperatorNodeInterface(OperatorType::kLeftUnary, operator_symbol_id),
        unary_operator_associatity_type_(unary_operator_associatity_type),
        unary_operator_priority_(unary_operator_priority) {}

  /// @brief ��������������
  /// @param[in] type �������õ�����������
  /// @note
  /// ���������Էִ����ҽ�Ϻʹ��ҵ�����
  void SetAssociatityType(OperatorAssociatityType type) {
    unary_operator_associatity_type_ = type;
  }
  /// @brief ��ȡ����������
  /// @return ��������������
  /// @retval OperatorAssociatityType::kLeftToRight �������ҽ��
  /// @retval OperatorAssociatityType::kRightToLeft �����ҵ�����
  OperatorAssociatityType GetAssociatityType() const {
    return unary_operator_associatity_type_;
  }
  /// @brief ������������ȼ��ȼ�
  /// @param[in] priority �������õ���������ȼ��ȼ�
  /// @attention ��������ȼ��뵥�����ȼ����岻ͬ
  /// ��������ȼ��������﷨������ֻ�������ӵ��
  /// �������ȼ������ڴʷ�������ÿ�����ʶ���
  void SetPriority(OperatorPriority priority) {
    unary_operator_priority_ = priority;
  }
  /// @brief ��ȡ��������ȼ�
  /// @return ������������ȼ�
  OperatorPriority GetPriority() const { return unary_operator_priority_; }

 private:
  /// @brief ����������
  OperatorAssociatityType unary_operator_associatity_type_;
  /// @brief ��������ȼ�
  OperatorPriority unary_operator_priority_;
};

/// @class BinaryUnaryOperatorNode production_node.h
/// @brief ��ʾͬʱ����˫Ŀ������͵�Ŀ���������������
/// @attention ��֧����൥Ŀ�����
class BinaryUnaryOperatorNode : public OperatorNodeInterface {
 public:
  BinaryUnaryOperatorNode(
      SymbolId operator_symbol_id,
      OperatorAssociatityType binary_operator_associatity_type,
      OperatorPriority binary_operator_priority,
      OperatorAssociatityType unary_operator_associatity_type,
      OperatorPriority unary_operator_priority)
      : OperatorNodeInterface(OperatorType::kBinaryLeftUnary,
                              operator_symbol_id),
        binary_operator_associatity_type_(binary_operator_associatity_type),
        binary_operator_priority_(binary_operator_priority),
        unary_operator_associatity_type_(unary_operator_associatity_type),
        unary_operator_priority_(unary_operator_priority) {}

  /// @brief ����˫Ŀ����������
  /// @param[in] type �������õ�����������
  /// @note
  /// ���������Էִ����ҽ�Ϻʹ��ҵ�����
  void SetBinaryOperatorAssociatityType(OperatorAssociatityType type) {
    binary_operator_associatity_type_ = type;
  }
  /// @brief ���õ�Ŀ����������
  /// @param[in] type �������õĵ�Ŀ����������
  /// @note
  /// ���������Էִ����ҽ�Ϻʹ��ҵ�����
  void SetUnaryOperatorAssociatityType(OperatorAssociatityType type) {
    unary_operator_associatity_type_ = type;
  }
  /// @brief ��ȡ˫Ŀ����������
  /// @return ����˫Ŀ����������
  /// @retval OperatorAssociatityType::kLeftToRight �������ҽ��
  /// @retval OperatorAssociatityType::kRightToLeft �����ҵ�����
  OperatorAssociatityType GetBinaryOperatorAssociatityType() const {
    return binary_operator_associatity_type_;
  }
  /// @brief ��ȡ��Ŀ����������
  /// @return ���ص�Ŀ����������
  /// @retval OperatorAssociatityType::kLeftToRight �������ҽ��
  /// @retval OperatorAssociatityType::kRightToLeft �����ҵ�����
  OperatorAssociatityType GetUnaryOperatorAssociatityType() const {
    return unary_operator_associatity_type_;
  }
  /// @brief ����˫Ŀ��������ȼ��ȼ�
  /// @param[in] priority �������õ�˫Ŀ��������ȼ��ȼ�
  /// @attention ��������ȼ��뵥�����ȼ����岻ͬ
  /// ��������ȼ��������﷨������ֻ�������ӵ��
  /// �������ȼ������ڴʷ�������ÿ�����ʶ���
  void SetBinaryOperatorPriority(OperatorPriority priority) {
    binary_operator_priority_ = priority;
  }
  /// @brief ���õ�Ŀ��������ȼ��ȼ�
  /// @param[in] priority �������õĵ�Ŀ��������ȼ��ȼ�
  /// @attention ��������ȼ��뵥�����ȼ����岻ͬ
  /// ��������ȼ��������﷨������ֻ�������ӵ��
  /// �������ȼ������ڴʷ�������ÿ�����ʶ���
  void SetUnaryOperatorPriority(OperatorPriority priority) {
    unary_operator_priority_ = priority;
  }
  /// @brief ��ȡ˫Ŀ��������ȼ�
  /// @return ����˫Ŀ��������ȼ�
  OperatorPriority GetBinaryOperatorPriority() const {
    return binary_operator_priority_;
  }
  /// @brief ��ȡ��Ŀ��������ȼ�
  /// @return ���ص�Ŀ��������ȼ�
  OperatorPriority GetUnaryOperatorPriority() const {
    return unary_operator_priority_;
  }

 private:
  /// @brief ˫Ŀ����������µĽ����
  OperatorAssociatityType binary_operator_associatity_type_;
  /// @brief ˫Ŀ����������µ����ȼ�
  OperatorPriority binary_operator_priority_;
  /// @brief ��Ŀ����������µĽ����
  OperatorAssociatityType unary_operator_associatity_type_;
  /// @brief ��Ŀ����������µ����ȼ�
  OperatorPriority unary_operator_priority_;
};

/// @class NonTerminalProductionNode production_node.h
/// @brief ��ʾ���ս����ʽ
class NonTerminalProductionNode : public BaseProductionNode {
 public:
  /// @class NonTerminalProductionNode::ProductionBodyType production_node.h
  /// @brief ��ʾ�������ս����ʽ��
  struct ProductionBodyType {
    template <class BodyContainer>
    ProductionBodyType(BodyContainer&& production_body_,
                       ProcessFunctionClassId class_for_reduct_id_)
        : production_body(std::forward<BodyContainer>(production_body_)),
          class_for_reduct_id(class_for_reduct_id_) {
      cores_items_in_.resize(production_body.size() + 1);
    }

    /// @brief ����ʽ��
    std::vector<ProductionNodeId> production_body;
    /// @brief ÿ������ʽ���Ӧ�������������ڵ��
    /// @note
    /// ��СΪproduction_body.size() + 1
    /// ����ʱʹ�õ��±�����
    /// ʹ���±�0��
    /// IdOrEquivence -> �� IdOrEquivence "[" Num "]"
    /// ʹ���±�3��
    /// IdOrEquivence -> IdOrEquivence "[" Num �� "]"
    /// ʹ���±�4��
    /// IdOrEquivence -> IdOrEquivence "[" Num "]" ��
    std::vector<std::list<ProductionItemSetId>> cores_items_in_;
    /// @brief ��Լ����ʽʹ�õİ�װ��Լ���������ʵ��������ID
    ProcessFunctionClassId class_for_reduct_id;
  };

  NonTerminalProductionNode(SymbolId symbol_id)
      : BaseProductionNode(ProductionNodeType::kNonTerminalNode, symbol_id) {}
  template <class IdType>
  NonTerminalProductionNode(SymbolId symbol_id, IdType&& body)
      : BaseProductionNode(ProductionNodeType::kNonTerminalNode, symbol_id),
        nonterminal_bodys_(std::forward<IdType>(body)) {}
  NonTerminalProductionNode(const NonTerminalProductionNode&) = delete;
  NonTerminalProductionNode& operator=(const NonTerminalProductionNode&) =
      delete;
  NonTerminalProductionNode(NonTerminalProductionNode&& node)
      : BaseProductionNode(std::move(node)),
        nonterminal_bodys_(std::move(node.nonterminal_bodys_)),
        could_empty_reduct_(std::move(node.could_empty_reduct_)) {}
  NonTerminalProductionNode& operator=(NonTerminalProductionNode&& node) {
    BaseProductionNode::operator=(std::move(node));
    nonterminal_bodys_ = std::move(node.nonterminal_bodys_);
    could_empty_reduct_ = std::move(node.could_empty_reduct_);
    return *this;
  }

  /// @brief ��ȡ����ʽ���ڵĲ���ʽ
  /// @param[in] production_body_id ������ȡ�Ĳ���ʽ��ID
  /// @param[in] next_word_to_shift_index ������ȡ���Ӳ���ʽλ��
  /// @attention �ս�ڵ���øú���ʱproduction_body_id����Ϊ0
  /// next_word_to_shift_index����0ʱ�򷵻�NextWordToShiftIndex::InvalidId()
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex next_word_to_shift_index) const override;

  /// @brief ���һ������ʽ��
  /// @tparam IdType �洢����ʽ���и�����ʽID����������֧��vector
  /// @param[in] body ������ӵĲ���ʽ��
  /// @param[in] class_for_reduct_id
  /// ����װ��Լ�ò���ʽ��ĺ��������ʵ��������ID
  /// @return ���ظò���ʽ���ڷ��ս�ڵ��ڵ�ID
  /// @note body�ڵĸ�����ʽID����д˳������
  /// �ú����޼���ظ�����
  template <class IdType>
  ProductionBodyId AddBody(IdType&& body,
                           ProcessFunctionClassId class_for_reduct_id);
  /// @brief ��ȡ����ʽ��һ����
  /// @param[in] production_body_id ��Ҫ��ȡ�Ĳ���ʽ��ID
  /// @return ���ز���ʽ���const����
  /// @note production_body_id�����Ӧ���ڵĲ���ʽ��
  const ProductionBodyType& GetProductionBody(
      ProductionBodyId production_body_id) const {
    assert(production_body_id < nonterminal_bodys_.size());
    return nonterminal_bodys_[production_body_id];
  }
  /// @brief ���ø�������ʽ��ID��Լʹ�õĶ����ID
  /// @param[in] body_id �������ù�Լ�ö���ID�Ĳ���ʽ��ID
  /// @param[in] class_for_reduct_id �������õĹ�Լ�ö���ID
  void SetBodyProcessFunctionClassId(
      ProductionBodyId body_id, ProcessFunctionClassId class_for_reduct_id) {
    assert(body_id < nonterminal_bodys_.size());
    nonterminal_bodys_[body_id].class_for_reduct_id = class_for_reduct_id;
  }
  /// @brief ��ȡ����ʽ��
  /// @param[in] body_id ������ȡ�Ĳ���ʽ��ID
  /// @return ���ػ�ȡ���Ĳ���ʽ�����ݽṹconst����
  /// @note body_id�����Ӧ��Ч�Ĳ���ʽ��
  const ProductionBodyType& GetBody(ProductionBodyId body_id) const {
    return nonterminal_bodys_[body_id];
  }
  /// @brief ��ȡȫ������ʽ��
  /// @return ���ش洢����ʽ�������const����
  const std::vector<ProductionBodyType>& GetAllBody() const {
    return nonterminal_bodys_;
  }
  /// @brief ��ȡȫ����Ч�Ĳ���ʽ��ID
  /// @return ���ش洢ȫ����Ч�Ĳ���ʽ��ID��vector����
  std::vector<ProductionBodyId> GetAllBodyIds() const;
  /// @brief ���øò���ʽ�����Կչ�Լ
  void SetProductionShouldNotEmptyReduct() { could_empty_reduct_ = false; }
  /// @brief ���øò���ʽ���Կչ�Լ
  void SetProductionCouldBeEmptyRedut() { could_empty_reduct_ = true; }
  /// @brief ��ѯ�ò���ʽ�Ƿ���Կչ�Լ
  /// @return ���ظò���ʽ�Ƿ���Կչ�Լ
  /// @retval true ���ò���ʽ���Կչ�Լ
  /// @retval false ���ò���ʽ�����Կչ�Լ
  bool CouldBeEmptyReduct() const { return could_empty_reduct_; }
  /// @brief ��Ӳ���ʽ��ĳ���������ID
  /// @param[in] body_id ��������������Ĳ���ʽ��ID
  /// @param[in] next_word_to_shift_index ��ָ�������һ������ĵ��ʵ�λ��
  /// @param[in] production_item_set_id �������������ID
  /// @details body_id��next_word_to_shift_index��ָͬ��Ҫ��������ID����
  /// @note ��ȥ�ع��ܣ�Ҫ��������ӹ��ĵ��ID�ظ�
  void AddProductionItemBelongToProductionItemSetId(
      ProductionBodyId body_id, NextWordToShiftIndex next_word_to_shift_index,
      ProductionItemSetId production_item_set_id);
  /// @brief ��ȡָ����������ȫ�����ID
  /// @param[in] body_id ��ָ���������Ĳ���ʽ��ID
  /// @param[in] next_word_to_shift_index ��ָ�������һ�����뵥�ʵ�λ��
  /// @return ���ش洢����������ȫ���ID������
  /// @note body_id��next_word_to_shift_index��ָͬ��һ����
  const std::list<ProductionItemSetId>&
  GetProductionItemSetIdFromProductionItem(
      ProductionBodyId body_id, NextWordToShiftIndex next_word_to_shift_index) {
    return nonterminal_bodys_[body_id]
        .cores_items_in_[next_word_to_shift_index];
  }
  /// @brief ��ȡָ������ʽ���װ��Լ�ú��������ʵ��������ID
  /// @return ���ذ�װ��Լ�ú��������ʵ��������ID
  ProcessFunctionClassId GetBodyProcessFunctionClassId(
      ProductionBodyId body_id) const {
    assert(body_id < nonterminal_bodys_.size());
    return nonterminal_bodys_[body_id].class_for_reduct_id;
  }

 private:
  /// @brief �洢����ʽ��
  std::vector<ProductionBodyType> nonterminal_bodys_;
  /// @brief ��־�ò���ʽ�Ƿ����Ϊ��
  bool could_empty_reduct_ = false;
};

/// @class EndNode production_node.h
/// @brief ��ʾ�ļ�β
class EndNode : public BaseProductionNode {
 public:
  EndNode()
      : BaseProductionNode(ProductionNodeType::kEndNode,
                           SymbolId::InvalidId()) {}
  EndNode(const EndNode&) = delete;
  EndNode& operator=(const EndNode&) = delete;
  EndNode(EndNode&& end_node) : BaseProductionNode(std::move(end_node)) {}
  EndNode& operator=(EndNode&& end_node) {
    BaseProductionNode::operator=(std::move(end_node));
    return *this;
  }
  /// @attention �ú�����Ӧ�����ã�EndNode�����ڸ�����
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex point_index) const override {
    assert(false);
    return ProductionNodeId::InvalidId();
  }
};
}  // namespace frontend::generator::syntax_generator

#endif  /// !GENERATOR_SYNTAXGENERATOR_PRODUCTION_NODE_H_