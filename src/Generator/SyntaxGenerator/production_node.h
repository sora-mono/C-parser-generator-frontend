// ��������ʽ����
#ifndef GENERATOR_SYNTAXGENERATOR_PRODUCTION_NODE_H_
#define GENERATOR_SYNTAXGENERATOR_PRODUCTION_NODE_H_
#include "Generator/export_types.h"

namespace frontend::generator::syntax_generator {
// ���в���ʽ�ڵ��඼Ӧ�̳��Ը���
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

  struct NodeData {
    ProductionNodeType node_type;
    std::string node_symbol;
  };

  void SetType(ProductionNodeType type) { base_type_ = type; }
  ProductionNodeType Type() const { return base_type_; }
  void SetThisNodeId(ProductionNodeId production_node_id) {
    base_id_ = production_node_id;
  }
  ProductionNodeId Id() const { return base_id_; }
  void SetSymbolId(SymbolId production_node_id) {
    base_symbol_id_ = production_node_id;
  }
  SymbolId GetNodeSymbolId() const { return base_symbol_id_; }

  // ��ȡ�������Ӧ�Ĳ���ʽID������point_index�����ID
  // point_indexԽ��ʱ����ProducNodeId::InvalidId()
  // Ϊ��֧����ǰ������ڵ�����Խ��
  // ���ص��ұߵĲ���ʽID���������򷵻�ProductionNodeId::InvalidId()
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex point_index) const = 0;

 private:
  // �ڵ�����
  ProductionNodeType base_type_;
  // �ڵ�ID
  ProductionNodeId base_id_;
  // �ڵ����ID
  SymbolId base_symbol_id_;
};

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
  using NodeData = BaseProductionNode::NodeData;

  // ��ȡ/���ò���ʽ����
  SymbolId GetBodySymbolId() { return body_symbol_id_; }
  void SetBodySymbolId(SymbolId body_symbol_id) {
    body_symbol_id_ = body_symbol_id;
  }
  // ��Խ��ʱҲ�з���ֵΪ��֧�ֻ�ȡ��һ��/���¸����ڽڵ�Ĳ���
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex point_index) const override;

 private:
  // ����ʽ����
  SymbolId body_symbol_id_;
};

class OperatorProductionNode : public BaseProductionNode {
 public:
  OperatorProductionNode(SymbolId node_symbol_id,
                         OperatorAssociatityType associatity_type,
                         OperatorPriority priority_level)
      : BaseProductionNode(ProductionNodeType::kOperatorNode, node_symbol_id),
        operator_associatity_type_(associatity_type),
        operator_priority_level_(priority_level) {}
  OperatorProductionNode(const OperatorProductionNode&) = delete;
  OperatorProductionNode& operator=(const OperatorProductionNode&) = delete;
  OperatorProductionNode(OperatorProductionNode&& operator_production_node)
      : BaseProductionNode(std::move(operator_production_node)),
        operator_associatity_type_(
            std::move(operator_production_node.operator_associatity_type_)),
        operator_priority_level_(
            std::move(operator_production_node.operator_priority_level_)) {}
  OperatorProductionNode& operator=(
      OperatorProductionNode&& operator_production_node);

  struct NodeData : public TerminalProductionNode::NodeData {
    std::string symbol_;
  };
  void SetAssociatityType(OperatorAssociatityType type) {
    operator_associatity_type_ = type;
  }
  OperatorAssociatityType GetAssociatityType() const {
    return operator_associatity_type_;
  }
  void SetPriorityLevel(OperatorPriority level) {
    operator_priority_level_ = level;
  }
  OperatorPriority GetPriorityLevel() const { return operator_priority_level_; }
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex point_index) const {
    assert(false);
    // ��ֹ����
    return ProductionNodeId();
  }

 private:
  // ����������
  OperatorAssociatityType operator_associatity_type_;
  // ��������ȼ�
  OperatorPriority operator_priority_level_;
};

class NonTerminalProductionNode : public BaseProductionNode {
 public:
  struct ProductionBodyType {
    template <class BodyContainer>
    ProductionBodyType(BodyContainer&& production_body_,
                       ProcessFunctionClassId class_for_reduct_id_)
        : production_body(std::forward<BodyContainer>(production_body_)),
          class_for_reduct_id(class_for_reduct_id_) {
      cores_items_in_.resize(production_body.size() + 1);
    }

    // ����ʽ��
    std::vector<ProductionNodeId> production_body;
    // ÿ������ʽ���Ӧ�������������ڵ��
    // ��СΪproduction_body.size() + 1
    std::vector<std::list<ProductionItemSetId>> cores_items_in_;
    // ��Լ����ʽʹ�õ����ID
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

  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex point_index) const override;

  // ���һ������ʽ�壬Ҫ��IdTypeΪһ��vector������洢����ʽ�ڵ�ID
  template <class IdType>
  ProductionBodyId AddBody(IdType&& body,
                           ProcessFunctionClassId class_for_reduct_id_);
  // ��ȡ����ʽ��һ����
  const ProductionBodyType& GetProductionBody(
      ProductionBodyId production_body_id) {
    assert(production_body_id < nonterminal_bodys_.size());
    return nonterminal_bodys_[production_body_id];
  }
  // ���ø�������ʽ��ID��Ӧ��ProcessFunctionClass��ID
  void SetBodyProcessFunctionClassId(
      ProductionBodyId body_id, ProcessFunctionClassId class_for_reduct_id) {
    assert(body_id < nonterminal_bodys_.size());
    nonterminal_bodys_[body_id].class_for_reduct_id = class_for_reduct_id;
  }
  const ProductionBodyType& GetBody(ProductionBodyId body_id) const {
    return nonterminal_bodys_[body_id];
  }
  const std::vector<ProductionBodyType>& GetAllBody() const {
    return nonterminal_bodys_;
  }
  // ��ȡȫ������ʽ��ID
  std::vector<ProductionBodyId> GetAllBodyIds() const;
  // ���øò���ʽ�����Կչ�Լ
  void SetProductionShouldNotEmptyReduct() { could_empty_reduct_ = false; }
  void SetProductionCouldBeEmptyRedut() { could_empty_reduct_ = true; }
  // ��ѯ�ò���ʽ�Ƿ���Կչ�Լ
  bool CouldBeEmptyReduct() const { return could_empty_reduct_; }
  // ������������ID
  // Ҫ�������е��ID�ظ�
  void AddProductionItemBelongToProductionItemSetId(
      ProductionBodyId body_id, NextWordToShiftIndex point_index,
      ProductionItemSetId core_id);
  // ��ȡ�����������ȫ���ID
  const std::list<ProductionItemSetId>&
  GetProductionItemSetIdFromProductionItem(ProductionBodyId body_id,
                                           NextWordToShiftIndex point_index) {
    return nonterminal_bodys_[body_id].cores_items_in_[point_index];
  }
  // ���ظ�������ʽ��ID��Ӧ��ProcessFunctionClass��ID
  ProcessFunctionClassId GetBodyProcessFunctionClassId(
      ProductionBodyId body_id) const {
    assert(body_id < nonterminal_bodys_.size());
    return nonterminal_bodys_[body_id].class_for_reduct_id;
  }

 private:
  // �洢����ʽ��
  std::vector<ProductionBodyType> nonterminal_bodys_;
  // ��־�ò���ʽ�Ƿ����Ϊ��
  bool could_empty_reduct_ = false;
};

// �ļ�β�ڵ�
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
  virtual ProductionNodeId GetProductionNodeInBody(
      ProductionBodyId production_body_id,
      NextWordToShiftIndex point_index) const override {
    assert(false);
    return ProductionNodeId::InvalidId();
  }
  // ��ȡ��װ�û��Զ��庯�����ݵ���Ķ���ID
  virtual ProcessFunctionClassId GetBodyProcessFunctionClassId(
      ProductionBodyId production_body_id) const {
    assert(false);
    return ProcessFunctionClassId::InvalidId();
  }
};
}  // namespace frontend::generator::syntax_generator

#endif  // !GENERATOR_SYNTAXGENERATOR_PRODUCTION_NODE_H_