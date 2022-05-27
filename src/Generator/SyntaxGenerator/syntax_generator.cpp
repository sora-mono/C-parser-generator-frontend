#include "syntax_generator.h"

#include <queue>

namespace frontend::generator::syntax_generator {
ProductionNodeId SyntaxGenerator::AddTerminalProduction(
    std::string&& node_symbol, std::string&& body_symbol,
    WordPriority node_priority, bool is_key_word) {
  auto [node_symbol_id, node_symbol_inserted] = AddNodeSymbol(node_symbol);
  auto [body_symbol_id, body_symbol_inserted] = AddBodySymbol(body_symbol);
  ProductionNodeId production_node_id;
  if (body_symbol_inserted) {
    ProductionNodeId old_node_symbol_id =
        GetProductionNodeIdFromNodeSymbolId(node_symbol_id);
    if (old_node_symbol_id.IsValid()) {
      // ���ս�ڵ����ѱ�ʹ��
      OutPutError(std::format("�ս�ڵ����Ѷ��壺{:}", node_symbol));
      return ProductionNodeId::InvalidId();
    }
    // ��Ҫ���һ���µ��ս�ڵ�
    production_node_id = SubAddTerminalNode(node_symbol_id, body_symbol_id);
    frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
        word_attached_data;
    word_attached_data.production_node_id = production_node_id;
    word_attached_data.node_type = ProductionNodeType::kTerminalNode;
    // ��DFA������ע��ؼ���
    bool result;
    if (!is_key_word) [[likely]] {
      assert(node_priority == 0 || node_priority == 1);
      result = dfa_generator_.AddRegexpression(
          body_symbol, std::move(word_attached_data), node_priority);
    } else {
      assert(node_priority == 2);
      result = dfa_generator_.AddWord(
          body_symbol, std::move(word_attached_data), node_priority);
    }
    if (!result) [[unlikely]] {
      OutPutError(
          std::format("�ڲ������޷�����ս�ڵ�������ʽ {:}", body_symbol));
      exit(-1);
    }
  } else {
    // ���ս�ڵ�������Ѵ��ڣ���Ӧ�ظ����
    OutPutError(std::format("�������ͬһ����{:}", body_symbol));
    return ProductionNodeId::InvalidId();
  }
  // �����������
  OutPutInfo(
      std::format("�ɹ�����ս����ʽ�� {:} -> {:}", node_symbol, body_symbol));
  OutPutInfo(std::format(
      "����ʽ��ID��{:} ����ʽ��ID��{:} �ս����ʽID��{:} ����ʽ���ȼ���{:}",
      node_symbol_id.GetRawValue(), body_symbol_id.GetRawValue(),
      production_node_id.GetRawValue(), node_priority.GetRawValue()));
  // �ж�����ӵ��ս�ڵ��Ƿ�Ϊĳ��δ�������ʽ
  CheckNonTerminalNodeCanContinue(node_symbol);
  return production_node_id;
}

inline ProductionNodeId SyntaxGenerator::SubAddTerminalNode(
    SymbolId node_symbol_id, SymbolId body_symbol_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<TerminalProductionNode>(node_symbol_id,
                                                           body_symbol_id);
  manager_nodes_.GetObject(node_id).SetNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(body_symbol_id, node_id);
  return node_id;
}

ProductionNodeId SyntaxGenerator::AddBinaryOperator(
    std::string&& operator_symbol,
    OperatorAssociatityType binary_operator_associatity_type,
    OperatorPriority binary_operator_priority) {
  // ���������ʽ�����������ͬ
  auto [operator_node_symbol_id, operator_node_symbol_inserted] =
      AddNodeSymbol(operator_symbol);
  if (!operator_node_symbol_inserted) [[unlikely]] {
    OutPutError(std::format("�������{:} �Ѷ���", operator_symbol));
    return ProductionNodeId::InvalidId();
  }
  auto [operator_body_symbol_id, operator_body_symbol_inserted] =
      AddBodySymbol(operator_symbol);
  if (!operator_body_symbol_inserted) {
    OutPutError(std::format("�������{:} ����DFA����ӣ��޷�����Ϊ�����",
                            operator_symbol));
    return ProductionNodeId::InvalidId();
  }
  ProductionNodeId operator_node_id = SubAddBinaryOperatorNode(
      operator_node_symbol_id, binary_operator_associatity_type,
      binary_operator_priority);
  assert(operator_node_id.IsValid());
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      word_attached_data;
  word_attached_data.production_node_id = operator_node_id;
  word_attached_data.node_type = ProductionNodeType::kOperatorNode;
  word_attached_data.binary_operator_associate_type =
      binary_operator_associatity_type;
  word_attached_data.binary_operator_priority = binary_operator_priority;
  // ��DFA������ע��ؼ���
  bool result = dfa_generator_.AddWord(
      operator_symbol, std::move(word_attached_data),
      frontend::generator::dfa_generator::DfaGenerator::WordPriority(1));
  if (!result) {
    OutPutError(std::format("�ڲ������޷����˫Ŀ�����������ʽ {:}",
                            operator_symbol));
    exit(-1);
  }
  // �����������
  OutPutInfo(std::format("�ɹ����˫Ŀ����� {:}",
                         GetNodeSymbolStringFromId(operator_node_symbol_id)));
  OutPutInfo(std::format(
      "�������ID��{:} �������ID��{:} �����ID��{:} ˫Ŀ��������ȼ���{:}",
      operator_node_symbol_id.GetRawValue(),
      operator_body_symbol_id.GetRawValue(), operator_node_id.GetRawValue(),
      binary_operator_priority.GetRawValue()));
  return operator_node_id;
}

ProductionNodeId SyntaxGenerator::AddLeftUnaryOperator(
    std::string&& operator_symbol,
    OperatorAssociatityType unary_operator_associatity_type,
    OperatorPriority unary_operator_priority) {
  // ���������ʽ�����������ͬ
  auto [operator_node_symbol_id, operator_node_symbol_inserted] =
      AddNodeSymbol(operator_symbol);
  if (!operator_node_symbol_inserted) [[unlikely]] {
    OutPutError(std::format("�������{:} �Ѷ���\n", operator_symbol));
    return ProductionNodeId::InvalidId();
  }
  auto [operator_body_symbol_id, operator_body_symbol_inserted] =
      AddBodySymbol(operator_symbol);
  if (!operator_body_symbol_inserted) {
    OutPutError(std::format("�������{:} ����DFA����ӣ��޷�����Ϊ�����\n",
                            operator_symbol));
    return ProductionNodeId::InvalidId();
  }
  ProductionNodeId operator_node_id = SubAddUnaryOperatorNode(
      operator_node_symbol_id, unary_operator_associatity_type,
      unary_operator_priority);
  assert(operator_node_id.IsValid());
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      word_attached_data;
  word_attached_data.production_node_id = operator_node_id;
  word_attached_data.node_type = ProductionNodeType::kOperatorNode;
  word_attached_data.unary_operator_associate_type =
      unary_operator_associatity_type;
  word_attached_data.unary_operator_priority = unary_operator_priority;
  // ��DFA������ע��ؼ���
  bool result = dfa_generator_.AddWord(
      operator_symbol, std::move(word_attached_data),
      frontend::generator::dfa_generator::DfaGenerator::WordPriority(1));
  if (!result) [[unlikely]] {
    OutPutError(std::format("�ڲ������޷������൥Ŀ�����������ʽ {:}",
                            operator_symbol));
    exit(-1);
  }
  // �����������
  OutPutInfo(std::format("�ɹ������൥Ŀ����� {:}",
                         GetNodeSymbolStringFromId(operator_node_symbol_id)));
  OutPutInfo(std::format(
      "�������ID��{:} �������ID��{:} �����ID��{:} ��൥Ŀ��������ȼ���{:}",
      operator_node_symbol_id.GetRawValue(),
      operator_body_symbol_id.GetRawValue(), operator_node_id.GetRawValue(),
      unary_operator_priority.GetRawValue()));
  return operator_node_id;
}

ProductionNodeId SyntaxGenerator::AddBinaryLeftUnaryOperator(
    std::string&& operator_symbol,
    OperatorAssociatityType binary_operator_associatity_type,
    OperatorPriority binary_operator_priority,
    OperatorAssociatityType unary_operator_associatity_type,
    OperatorPriority unary_operator_priority) {
  // ���������ʽ�����������ͬ
  auto [operator_node_symbol_id, operator_node_symbol_inserted] =
      AddNodeSymbol(operator_symbol);
  if (!operator_node_symbol_inserted) [[unlikely]] {
    OutPutError(std::format("�������{:} �Ѷ���\n", operator_symbol));
    return ProductionNodeId::InvalidId();
  }
  auto [operator_body_symbol_id, operator_body_symbol_inserted] =
      AddBodySymbol(operator_symbol);
  if (!operator_body_symbol_inserted) {
    OutPutError(std::format("�������{:} ����DFA����ӣ��޷�����Ϊ�����\n",
                            operator_symbol));
    return ProductionNodeId::InvalidId();
  }
  ProductionNodeId operator_node_id = SubAddBinaryUnaryOperatorNode(
      operator_node_symbol_id, binary_operator_associatity_type,
      binary_operator_priority, unary_operator_associatity_type,
      unary_operator_priority);
  assert(operator_node_id.IsValid());
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      word_attached_data;
  word_attached_data.production_node_id = operator_node_id;
  word_attached_data.node_type = ProductionNodeType::kOperatorNode;
  word_attached_data.binary_operator_associate_type =
      binary_operator_associatity_type;
  word_attached_data.binary_operator_priority = binary_operator_priority;
  word_attached_data.unary_operator_associate_type =
      unary_operator_associatity_type;
  word_attached_data.unary_operator_priority = unary_operator_priority;
  // ��DFA������ע��ؼ���
  bool result = dfa_generator_.AddWord(
      operator_symbol, std::move(word_attached_data),
      frontend::generator::dfa_generator::DfaGenerator::WordPriority(1));
  if (!result) [[unlikely]] {
    OutPutError(
        std::format("�ڲ������޷����˫Ŀ����൥Ŀ�����������ʽ {:}",
                    operator_symbol));
    exit(-1);
  }
  // �����������
  OutPutInfo(std::format("�ɹ����˫Ŀ����൥Ŀ����� {:}",
                         GetNodeSymbolStringFromId(operator_node_symbol_id)));
  OutPutInfo(std::format(
      "�������ID��{:} �������ID��{:} �����ID��{:} ˫Ŀ��������ȼ���{:} "
      "��൥Ŀ��������ȼ���{:}",
      operator_node_symbol_id.GetRawValue(),
      operator_body_symbol_id.GetRawValue(), operator_node_id.GetRawValue(),
      binary_operator_priority.GetRawValue(),
      unary_operator_priority.GetRawValue()));
  return operator_node_id;
}

ProductionNodeId SyntaxGenerator::SubAddBinaryOperatorNode(
    SymbolId node_symbol_id, OperatorAssociatityType binary_associatity_type,
    OperatorPriority binary_priority) {
  ProductionNodeId node_id = manager_nodes_.EmplaceObject<BinaryOperatorNode>(
      node_symbol_id, binary_associatity_type, binary_priority);
  manager_nodes_.GetObject(node_id).SetNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}

ProductionNodeId SyntaxGenerator::SubAddUnaryOperatorNode(
    SymbolId node_symbol_id, OperatorAssociatityType unary_associatity_type,
    OperatorPriority unary_priority) {
  ProductionNodeId node_id = manager_nodes_.EmplaceObject<UnaryOperatorNode>(
      node_symbol_id, unary_associatity_type, unary_priority);
  manager_nodes_.GetObject(node_id).SetNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}

ProductionNodeId SyntaxGenerator::SubAddBinaryUnaryOperatorNode(
    SymbolId node_symbol_id, OperatorAssociatityType binary_associatity_type,
    OperatorPriority binary_priority,
    OperatorAssociatityType unary_associatity_type,
    OperatorPriority unary_priority) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<BinaryUnaryOperatorNode>(
          node_symbol_id, binary_associatity_type, binary_priority,
          unary_associatity_type, unary_priority);
  manager_nodes_.GetObject(node_id).SetNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}

ProductionNodeId SyntaxGenerator::AddNonTerminalProduction(
    std::string&& node_symbol, std::vector<std::string>&& subnode_symbols,
    ProcessFunctionClassId class_id) {
  assert(!node_symbol.empty() && !subnode_symbols.empty() &&
         class_id.IsValid());
  std::vector<ProductionNodeId> node_ids;
  // ���ս�ڵ���ڵ���һһ��Ӧ
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromNodeSymbol(node_symbol);
  if (!production_node_id.IsValid()) {
    // �÷��ս�ڵ���δע��
    SymbolId symbol_id = AddNodeSymbol(node_symbol).first;
    assert(symbol_id.IsValid());
    production_node_id = SubAddNonTerminalNode(symbol_id);
    // �����ӵĽڵ��Ƿ�ǰ������
    CheckNonTerminalNodeCanContinue(node_symbol);
  }
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  for (auto& subnode_symbol : subnode_symbols) {
    // ������ʽ������в���ʽ��ת��Ϊ�ڵ�ID
    ProductionNodeId subproduction_node_id =
        GetProductionNodeIdFromNodeSymbol(subnode_symbol);
    // ����ʽ��
    if (!subproduction_node_id.IsValid()) {
      // ����ʽ�ڵ�δ����
      // ��Ӵ������¼
      AddUnableContinueNonTerminalNode(subnode_symbol, std::move(node_symbol),
                                       std::move(subnode_symbols), class_id);
      return ProductionNodeId::InvalidId();
    }
    assert(subproduction_node_id.IsValid());
    node_ids.push_back(subproduction_node_id);
  }
  ProductionBodyId body_id =
      production_node.AddBody(std::move(node_ids), class_id);
  OutPutInfo(std::format("�ɹ���ӷ��ս�ڵ� ") +
             FormatSingleProductionBody(production_node_id, body_id));
  OutPutInfo(std::format("���ս�ڵ�ID��{:} ��ǰ����ʽ��ID��{:}",
                         production_node_id.GetRawValue(),
                         body_id.GetRawValue()));
  return production_node_id;
}

void SyntaxGenerator::SetNonTerminalNodeCouldEmptyReduct(
    const std::string& nonterminal_node_symbol) {
  ProductionNodeId nonterminal_node_id =
      GetProductionNodeIdFromNodeSymbol(nonterminal_node_symbol);
  assert(nonterminal_node_id.IsValid());
  NonTerminalProductionNode& nonterminal_production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(nonterminal_node_id));
  if (nonterminal_production_node.GetType() !=
      ProductionNodeType::kNonTerminalNode) [[unlikely]] {
    OutPutError(std::format("�޷����÷��ս����ʽ����Ĳ���ʽ��{:} ����չ�Լ",
                            nonterminal_node_symbol));
    exit(-1);
  }
  nonterminal_production_node.SetProductionCouldBeEmptyRedut();
}

inline ProductionNodeId SyntaxGenerator::SubAddNonTerminalNode(
    SymbolId node_symbol_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<NonTerminalProductionNode>(node_symbol_id);
  manager_nodes_.GetObject(node_id).SetNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}

inline ProductionNodeId SyntaxGenerator::AddEndNode() {
  ProductionNodeId node_id = manager_nodes_.EmplaceObject<EndNode>();
  GetProductionNode(node_id).SetSymbolId(AddNodeSymbol("$").first);
  GetProductionNode(node_id).SetNodeId(node_id);
  return node_id;
}

void SyntaxGenerator::SetRootProduction(
    const std::string& production_node_symbol) {
  if (GetRootProductionNodeId().IsValid()) [[unlikely]] {
    OutPutError(std::format("���ұ�������һ��������ʽ"));
    exit(-1);
  }
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromNodeSymbol(production_node_symbol);
  if (!production_node_id.IsValid()) [[unlikely]] {
    OutPutError(std::format("�����ڷ��ս����ʽ {:}", production_node_symbol));
    exit(-1);
  }
  SetRootProductionNodeId(production_node_id);
}

inline BaseProductionNode& SyntaxGenerator::GetProductionNode(
    ProductionNodeId production_node_id) {
  return const_cast<BaseProductionNode&>(
      static_cast<const SyntaxGenerator&>(*this).GetProductionNode(
          production_node_id));
}

inline const BaseProductionNode& SyntaxGenerator::GetProductionNode(
    ProductionNodeId production_node_id) const {
  return manager_nodes_[production_node_id];
}

inline BaseProductionNode& SyntaxGenerator::GetProductionNodeFromNodeSymbolId(
    SymbolId symbol_id) {
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromNodeSymbolId(symbol_id);
  assert(symbol_id.IsValid());
  return GetProductionNode(production_node_id);
}

inline BaseProductionNode& SyntaxGenerator::GetProductionNodeFromBodySymbolId(
    SymbolId symbol_id) {
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromBodySymbolId(symbol_id);
  assert(symbol_id.IsValid());
  return GetProductionNode(production_node_id);
}

inline const std::vector<ProductionNodeId>& SyntaxGenerator::GetProductionBody(
    ProductionNodeId production_node_id, ProductionBodyId production_body_id) {
  NonTerminalProductionNode& nonterminal_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  // ֻ�з��ս�ڵ���ж������ʽ�壬�������ڵ���øú���������
  assert(nonterminal_node.GetType() == ProductionNodeType::kNonTerminalNode);
  return nonterminal_node.GetBody(production_body_id).production_body;
}

inline ProductionItemSetId
SyntaxGenerator::GetProductionItemSetIdFromSyntaxAnalysisTableEntryId(
    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id) {
  auto iter = syntax_analysis_table_entry_id_to_production_item_set_id_.find(
      syntax_analysis_table_entry_id);
  assert(iter !=
         syntax_analysis_table_entry_id_to_production_item_set_id_.end());
  return iter->second;
}

inline ProductionItemSetId SyntaxGenerator::EmplaceProductionItemSet() {
  SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id =
      AddSyntaxAnalysisTableEntry();
  ProductionItemSetId production_item_set_id =
      production_item_sets_.EmplaceObject(syntax_analysis_table_entry_id);
  production_item_sets_[production_item_set_id].SetProductionItemSetId(
      production_item_set_id);
  SetSyntaxAnalysisTableEntryIdToProductionItemSetIdMapping(
      syntax_analysis_table_entry_id, production_item_set_id);
  return production_item_set_id;
}

inline void SyntaxGenerator::AddProductionItemBelongToProductionItemSetId(
    const ProductionItem& production_item,
    ProductionItemSetId production_item_set_id) {
  auto& [production_node_id, production_body_id, next_word_to_shift_index] =
      production_item;
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  assert(production_node.GetType() == ProductionNodeType::kNonTerminalNode);
  production_node.AddProductionItemBelongToProductionItemSetId(
      production_body_id, next_word_to_shift_index, production_item_set_id);
}

inline const std::list<ProductionItemSetId>&
SyntaxGenerator::GetProductionItemSetIdFromProductionItem(
    ProductionNodeId production_node_id, ProductionBodyId body_id,
    NextWordToShiftIndex next_word_to_shift_index) {
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  assert(production_node.GetType() == ProductionNodeType::kNonTerminalNode);
  return production_node.GetProductionItemSetIdFromProductionItem(
      body_id, next_word_to_shift_index);
}

void SyntaxGenerator::GetNonTerminalNodeFirstNodeIds(
    ProductionNodeId production_node_id, ForwardNodesContainer* result,
    std::unordered_set<ProductionNodeId>&& processed_nodes) {
  bool inserted = processed_nodes.insert(production_node_id).second;
  assert(inserted);
  assert(production_node_id.IsValid());
  assert(result);
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  assert(production_node.GetType() == ProductionNodeType::kNonTerminalNode);
  for (auto& body : production_node.GetAllBody()) {
    NextWordToShiftIndex node_id_index(0);
    // ���ڴ���Ĳ���ʽ�ڵ��ID
    ProductionNodeId node_id;
    while (node_id_index < body.production_body.size()) {
      node_id = body.production_body[node_id_index];
      if (GetProductionNode(node_id).GetType() !=
          ProductionNodeType::kNonTerminalNode) {
        result->insert(node_id);
        break;
      }
      assert(node_id.IsValid());
      // ���ýڵ��Ƿ��Ѿ������
      if (processed_nodes.find(node_id) == processed_nodes.end()) {
        // ����ýڵ�δ�������ȡ�ýڵ��first�ڵ�ID����ӵ���first�ڵ�ID������
        GetNonTerminalNodeFirstNodeIds(node_id, result,
                                       std::move(processed_nodes));
      }
      if (static_cast<NonTerminalProductionNode&>(GetProductionNode(node_id))
              .CouldBeEmptyReduct()) {
        // �÷��ս�ڵ���Կչ�Լ����Ҫ���Ǻ���Ľڵ�
        ++node_id_index;
      } else {
        // �Ѵ������нڵ㣬����ѭ��
        break;
      }
    }
  }
}

SyntaxGenerator::ForwardNodesContainer SyntaxGenerator::First(
    ProductionNodeId production_node_id, ProductionBodyId production_body_id,
    NextWordToShiftIndex next_word_to_shift_index,
    const ForwardNodesContainer& next_node_ids) {
  ProductionNodeId node_id = GetProductionNodeIdInBody(
      production_node_id, production_body_id, next_word_to_shift_index);
  if (node_id.IsValid()) {
    ForwardNodesContainer forward_nodes;
    switch (GetProductionNode(node_id).GetType()) {
      case ProductionNodeType::kTerminalNode:
      case ProductionNodeType::kOperatorNode:
        forward_nodes.insert(node_id);
        break;
      case ProductionNodeType::kNonTerminalNode:
        // �ϲ���ǰ���Ҳ���ս�ڵ���ܵ����е���ID
        GetNonTerminalNodeFirstNodeIds(node_id, &forward_nodes);
        if (static_cast<NonTerminalProductionNode&>(GetProductionNode(node_id))
                .CouldBeEmptyReduct()) {
          // ��ǰ���Ҳ�ķ��ս�ڵ���Կչ�Լ����Ҫ��������һ���ڵ�����
          forward_nodes.merge(
              First(production_node_id, production_body_id,
                    NextWordToShiftIndex(next_word_to_shift_index + 1),
                    next_node_ids));
        }
        break;
      default:
        assert(false);
        break;
    }
    return forward_nodes;
  } else {
    return next_node_ids;
  }
}

bool SyntaxGenerator::ProductionItemSetClosure(
    ProductionItemSetId production_item_set_id) {
  OutPutInfo(std::format("��ID = {:}���ִ�бհ�����",
                         production_item_set_id.GetRawValue()));
  OutPutInfo(std::format("������е������ǰ���������£�\n") +
             FormatProductionItems(production_item_set_id));
  ProductionItemSet& production_item_set =
      GetProductionItemSet(production_item_set_id);
  if (production_item_set.IsClosureAvailable()) {
    // �հ���Ч����������
    return false;
  }
  SyntaxAnalysisTableEntry& syntax_analysis_table_entry =
      GetSyntaxAnalysisTableEntry(
          production_item_set.GetSyntaxAnalysisTableEntryId());
  // ����﷨������
  syntax_analysis_table_entry.Clear();
  // ��շǺ�����
  production_item_set.ClearNotMainItem();
  // �洢��ǰ��չ������
  // �洢ָ���չ����ĵ�����
  std::queue<ProductionItemAndForwardNodesContainer::const_iterator>
      items_waiting_process;
  const auto& production_items_and_forward_nodes =
      GetProductionItemsAndForwardNodes(production_item_set_id);
  for (auto& iter : GetProductionItemSetMainItems(production_item_set_id)) {
    // ��������к�����ѹ����������
    items_waiting_process.push(iter);
  }
  while (!items_waiting_process.empty()) {
    auto item_now = items_waiting_process.front();
    items_waiting_process.pop();
    const auto [production_node_id, production_body_id,
                next_word_to_shift_index] = item_now->first;
    NonTerminalProductionNode& production_node_now =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id));
    assert(production_node_now.GetType() ==
           ProductionNodeType::kNonTerminalNode);
    ProductionNodeId next_production_node_id = GetProductionNodeIdInBody(
        production_node_id, production_body_id, next_word_to_shift_index);
    if (!next_production_node_id.IsValid()) {
      // �޺�̽ڵ㣬���ù�Լ��Ŀ

      // ��װ��Լʹ�õ�����
      SyntaxAnalysisTableEntry::ReductAttachedData reduct_attached_data(
          production_node_id,
          GetProcessFunctionClass(production_node_id, production_body_id),
          GetProductionBody(production_node_id, production_body_id));
      OutPutInfo(std::format("�") + FormatProductionItem(item_now->first));
      OutPutInfo(std::format("����ǰ�����ţ�") +
                 FormatLookForwardSymbols(item_now->second) + " ��ִ�й�Լ");
      const auto& production_body =
          production_node_now.GetBody(production_body_id).production_body;
      for (auto node_id : item_now->second) {
        // ��ÿ����ǰ���������ù�Լ����
        syntax_analysis_table_entry.SetTerminalNodeActionAndAttachedData(
            node_id, reduct_attached_data);
      }
      continue;
    }
    NonTerminalProductionNode& next_production_node =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(next_production_node_id));
    if (next_production_node.GetType() !=
        ProductionNodeType::kNonTerminalNode) {
      OutPutInfo(std::format("�") + FormatProductionItem(item_now->first) +
                 std::format(" ������һ������ķ���Ϊ�ս�ڵ����ֹչ��"));
      continue;
    }
    // չ�����ս�ڵ㣬��Ϊ�䴴����ǰ�����ż�
    ForwardNodesContainer&& forward_node_ids = First(
        production_node_id, production_body_id,
        NextWordToShiftIndex(next_word_to_shift_index + 1), item_now->second);
    OutPutInfo(std::format("�") + FormatProductionItem(item_now->first) +
               std::format(" ����չ�����ս�ڵ� {:}",
                           GetNextNodeToShiftSymbolString(
                               production_node_id, production_body_id,
                               next_word_to_shift_index)));
    OutPutInfo(std::format("�÷��ս�ڵ���е�ȫ����ǰ�����ţ�") +
               FormatLookForwardSymbols(forward_node_ids));
    for (auto body_id : next_production_node.GetAllBodyIds()) {
      // ���÷��ս�ڵ��е�ÿ������ʽ����뵽production_item_set�У����������
      auto [iter, inserted] = AddItemAndForwardNodeIdsToProductionItem(
          production_item_set_id,
          ProductionItem(next_production_node_id, body_id,
                         NextWordToShiftIndex(0)),
          forward_node_ids);
      if (inserted) {
        // ��������µ�������ӵ������еȴ���������¼�������ڵ����ID
        items_waiting_process.push(iter);
        OutPutInfo(std::format("����в������") +
                   FormatProductionItem(iter->first) +
                   std::format(" ��ǰ�����ţ�") +
                   FormatLookForwardSymbols(forward_node_ids));
      }
    }
    if (next_production_node.CouldBeEmptyReduct()) {
      // ���Ҳ�ķ��ս�ڵ���Կչ�Լ
      // �������ӿչ�Լ��õ������ǰ�����Ÿ���ԭ���������ǰ�����ż�
      auto [iter, inserted] = AddItemAndForwardNodeIdsToProductionItem(
          production_item_set_id,
          ProductionItem(production_node_id, production_body_id,
                         NextWordToShiftIndex(next_word_to_shift_index + 1)),
          item_now->second);
      if (inserted) {
        // ��������µ�������ӵ������еȴ�����
        items_waiting_process.push(iter);
        OutPutInfo(
            std::format("���ڷ��ս����ʽ {:} ���Կչ�Լ������в������",
                        GetNextNodeToShiftSymbolString(
                            production_node_id, production_body_id,
                            next_word_to_shift_index)) +
            FormatProductionItemAndLookForwardSymbols(iter->first,
                                                      item_now->second));
      }
    }
    OutPutInfo(std::format("�") +
               FormatProductionItemAndLookForwardSymbols(item_now->first,
                                                         item_now->second) +
               std::format(" ��չ��"));
  }
  SetProductionItemSetClosureAvailable(production_item_set_id);
  OutPutInfo(std::format("��ɶ�ProductionItemID = {:}����ıհ�����",
                         production_item_set_id.GetRawValue()));
  OutPutInfo(std::format("������е������ǰ���������£�\n") +
             FormatProductionItems(production_item_set_id));
  return true;
}

ProductionItemSetId SyntaxGenerator::GetProductionItemSetIdFromProductionItems(
    const std::list<SyntaxGenerator::ProductionItemAndForwardNodesContainer::
                        const_iterator>& items) {
#ifdef _DEBUG
  // �������в������ظ���
  // ���еĲ�������������ͬ�Ľڵ�ǰ���������ͬ�ڵ�������ͬ�ĳ�ֱ�Ҫ����Ϊ
  // ������ͬ�ڵ�ǰ������ͬ
  // ����ֻ����������ͬ�ڵ�ǰ�����Ƿ���ͬ
  for (auto iter = items.cbegin(); iter != items.cend(); ++iter) {
    auto compare_iter = iter;
    while (++compare_iter != items.cend()) {
      assert((*iter)->first != (*compare_iter)->first);
    }
  }
#endif  // _DEBUG

  // �洢�ID��������������ĸ�����ֻ�а���ȫ��������ſ��ܳ�Ϊ��Щ�����ڵ��
  std::unordered_map<ProductionItemSetId, size_t>
      production_item_set_includes_item_size;
  // ͳ�������������ĸ���
  for (const auto& item : items) {
    // �������ڵ�ǰ��������
    auto [production_node_id, production_body_id, next_word_to_shift_index] =
        item->first;
    // ��ȡ����ڵ���������
    ++next_word_to_shift_index;
    const auto& production_item_set_ids_item_belong_to =
        GetProductionItemSetIdFromProductionItem(
            production_node_id, production_body_id, next_word_to_shift_index);
    // �Ż��ֶΣ��������������κ���򲻴��ڰ������и�������
    if (production_item_set_ids_item_belong_to.empty()) [[likely]] {
      return ProductionItemSetId::InvalidId();
    }
    for (auto production_item_set_id : production_item_set_ids_item_belong_to) {
      auto [iter, inserted] = production_item_set_includes_item_size.emplace(
          production_item_set_id, 1);
      if (!inserted) {
        // ���ID�Ѿ���¼�����Ӹ��ID���ֵĴ���
        ++iter->second;
      }
    }
  }
  // ������г��ֹ����ID���ҳ��������и�������ID
  auto iter = production_item_set_includes_item_size.begin();
  for (; iter != production_item_set_includes_item_size.end(); ++iter) {
    if (iter->second != items.size()) [[likely]] {
      // ���û����������
      continue;
    }
    // ����Ƿ�������Ǹ���ĺ�����
    bool all_item_is_main_item = true;
    ProductionItemSet& production_item_set_may_belong_to =
        GetProductionItemSet(iter->first);
    for (const auto& item : items) {
      // ��������ڵ��õ�����
      auto item_after_shift = item->first;
      ++std::get<NextWordToShiftIndex>(item_after_shift);
      if (!production_item_set_may_belong_to.IsMainItem(item_after_shift)) {
        all_item_is_main_item = false;
        break;
      }
    }
    if (all_item_is_main_item) {
      return iter->first;
    }
  }
  return ProductionItemSetId::InvalidId();
}

bool SyntaxGenerator::
    SpreadLookForwardSymbolAndConstructSyntaxAnalysisTableEntry(
        ProductionItemSetId production_item_set_id) {
  // ���δִ�бհ�����������ִ�д�������
  if (!ProductionItemSetClosure(production_item_set_id)) [[unlikely]] {
    OutPutInfo(std::format(
        "ID = {:}�������հ���û���κθ��ģ��������´�����ǰ������",
        production_item_set_id.GetRawValue()));
    return false;
  }
  OutPutInfo(std::format("��ID = {:}���������ǰ������",
                         production_item_set_id.GetRawValue()));
  // ִ�бհ���������Կչ�Լ�ﵽ��ÿһ���production_item_set��
  // ���Դ���ʱ���迼��ĳ��չ�Լ���ܴﵽ�����Ϊ��Щ�������production_item_set��

  // ��Ҫ������ǰ�����ŵ�production_item_set��ID�������ID�����ظ�
  std::list<ProductionItemSetId> production_item_set_waiting_spread_ids;
  SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id =
      GetProductionItemSet(production_item_set_id)
          .GetSyntaxAnalysisTableEntryId();
  // ����ת������������Ա�֮��Ѱ��goto����������
  // ����ת��������ֵ�������ֵǰ����
  std::unordered_map<
      ProductionNodeId,
      std::list<ProductionItemAndForwardNodesContainer::const_iterator>>
      goto_table;
  // �ռ�����ת������
  const auto& items_and_forward_nodes =
      GetProductionItemsAndForwardNodes(production_item_set_id);
  for (auto iter = items_and_forward_nodes.cbegin();
       iter != items_and_forward_nodes.cend(); iter++) {
    auto [production_node_id, production_body_id, next_word_to_shift_index] =
        iter->first;
    NonTerminalProductionNode& production_node_now =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id));
    ProductionNodeId next_production_node_id =
        production_node_now.GetProductionNodeInBody(production_body_id,
                                                    next_word_to_shift_index);
    if (next_production_node_id.IsValid()) [[unlikely]] {
      // ������Լ����������
      goto_table[next_production_node_id].emplace_back(iter);
    }
  }
  // �ҵ�ÿ��ת�������µ��������Ƿ���ڹ�ͬ�����
  // �������������ת�Ƶ������������ǰ�����ţ���������������������
  for (const auto& transform_condition_and_transformed_data : goto_table) {
    // ��ȡת�ƺ�����ڵ����ID
    ProductionItemSetId production_item_set_after_transform_id =
        GetProductionItemSetIdFromProductionItems(
            transform_condition_and_transformed_data.second);
    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id_after_transform;
    if (production_item_set_after_transform_id.IsValid()) [[unlikely]] {
      // ת�Ƶ�����Ѵ���
      // ת�ƺ󵽴���﷨��������Ŀ
      syntax_analysis_table_entry_id_after_transform =
          GetProductionItemSet(production_item_set_after_transform_id)
              .GetSyntaxAnalysisTableEntryId();
      bool new_forward_node_inserted = false;
      // ���ת��������ת�Ƶ�����Щ�����ǰ������
      for (auto& item_and_forward_nodes_iter :
           transform_condition_and_transformed_data.second) {
        // ����������ź����
        ProductionItem shifted_item = item_and_forward_nodes_iter->first;
        // ��ȡ������ź��������
        ++std::get<NextWordToShiftIndex>(shifted_item);
        new_forward_node_inserted |=
            AddForwardNodes(production_item_set_after_transform_id,
                            shifted_item, item_and_forward_nodes_iter->second);
        OutPutInfo(std::format(
            "�����{:} ����ǰ������ {:} ��ִ���������",
            FormatProductionItem(item_and_forward_nodes_iter->first),
            GetNodeSymbolStringFromProductionNodeId(
                transform_condition_and_transformed_data.first)));
      }
      // ����������������������µ���ǰ�����������´�������
      if (new_forward_node_inserted) {
        production_item_set_waiting_spread_ids.push_back(
            production_item_set_after_transform_id);
        OutPutInfo(std::format(
            "ProductionItemSet ID:{:} "
            "����������������/����ǰ�����ţ����´����������ǰ������",
            production_item_set_after_transform_id.GetRawValue()));
      }
    } else {
      // ��������Щ��ɵ������Ҫ�½�
      production_item_set_after_transform_id = EmplaceProductionItemSet();
      syntax_analysis_table_entry_id_after_transform =
          GetProductionItemSet(production_item_set_after_transform_id)
              .GetSyntaxAnalysisTableEntryId();
      // �������к�����
      for (auto& item_and_forward_nodes_iter :
           transform_condition_and_transformed_data.second) {
        // ����������ź����
        ProductionItem shifted_item = item_and_forward_nodes_iter->first;
        // ��ȡ������ź��������
        ++std::get<NextWordToShiftIndex>(shifted_item);
        auto result = AddMainItemAndForwardNodeIdsToProductionItem(
            production_item_set_after_transform_id, shifted_item,
            item_and_forward_nodes_iter->second);
        assert(result.second);
      }
      production_item_set_waiting_spread_ids.push_back(
          production_item_set_after_transform_id);
    }
    SyntaxAnalysisTableEntry& syntax_analysis_table_entry =
        GetSyntaxAnalysisTableEntry(syntax_analysis_table_entry_id);
    // ����ת��������ת�Ƶ����е��
    switch (GetProductionNode(transform_condition_and_transformed_data.first)
                .GetType()) {
      case ProductionNodeType::kTerminalNode:
      case ProductionNodeType::kOperatorNode:
        syntax_analysis_table_entry.SetTerminalNodeActionAndAttachedData(
            transform_condition_and_transformed_data.first,
            SyntaxAnalysisTableEntry::ShiftAttachedData(
                syntax_analysis_table_entry_id_after_transform));
        OutPutInfo(std::format(
            "ID = {:}����������ս���� {:} ��ת�Ƶ�ID = {:}���",
            production_item_set_id.GetRawValue(),
            GetNodeSymbolStringFromProductionNodeId(
                transform_condition_and_transformed_data.first),
            production_item_set_after_transform_id.GetRawValue()));
        break;
      case ProductionNodeType::kNonTerminalNode:
        syntax_analysis_table_entry.SetNonTerminalNodeTransformId(
            transform_condition_and_transformed_data.first,
            syntax_analysis_table_entry_id_after_transform);
        OutPutInfo(std::format(
            "ID = {:}�����������ս���� {:} ��ת�Ƶ�ID = {:}���",
            production_item_set_id.GetRawValue(),
            GetNodeSymbolStringFromProductionNodeId(
                transform_condition_and_transformed_data.first),
            production_item_set_after_transform_id.GetRawValue()));
        break;
      default:
        assert(false);
        break;
    }
  }
  OutPutInfo(std::format("ID = {:}���������ǰ�����Ų��������",
                         production_item_set_id.GetRawValue()));
  for (auto production_item_set_waiting_spread_id :
       production_item_set_waiting_spread_ids) {
    // �������ɵ�ÿ�����������ǰ������
    SpreadLookForwardSymbolAndConstructSyntaxAnalysisTableEntry(
        production_item_set_waiting_spread_id);
  }
  return true;
}

std::array<std::vector<ProductionNodeId>, 4>
SyntaxGenerator::ClassifyProductionNodes() const {
  std::array<std::vector<ProductionNodeId>, 4> production_nodes;
  ObjectManager<BaseProductionNode>::ConstIterator iter =
      manager_nodes_.ConstBegin();
  while (iter != manager_nodes_.ConstEnd()) {
    production_nodes[static_cast<size_t>(iter->GetType())].push_back(
        iter->GetNodeId());
    ++iter;
  }
  return production_nodes;
}

void SyntaxGenerator::SyntaxAnalysisTableTerminalNodeClassify(
    const std::vector<ProductionNodeId>& terminal_node_ids, size_t index,
    std::list<SyntaxAnalysisTableEntryId>&& syntax_analysis_table_entry_ids,
    std::vector<std::list<SyntaxAnalysisTableEntryId>>* equivalent_ids) {
  if (index >= terminal_node_ids.size()) [[unlikely]] {
    // ������ɣ������Ժϲ�������ӵ����
    assert(syntax_analysis_table_entry_ids.size() > 1);
    equivalent_ids->emplace_back(std::move(syntax_analysis_table_entry_ids));
    return;
  }
  assert(GetProductionNode(terminal_node_ids[index]).GetType() ==
             ProductionNodeType::kTerminalNode ||
         GetProductionNode(terminal_node_ids[index]).GetType() ==
             ProductionNodeType::kOperatorNode);
  // �����������������ת�������µ�ת�ƽ������
  // ʹ��ActionAndAttachedDataPointerEqualTo���ж�����ת�ƽ���Ƿ���ͬ
  // ��ֵΪShiftAttachedData������ź�ת�Ƶ����﷨��������ĿID
  std::unordered_map<SyntaxAnalysisTableEntryId,
                     std::list<SyntaxAnalysisTableEntryId>>
      shift_classify_table;
  // ��ֵΪReductAttachedData��Լʱʹ�õİ�װReduct��������Ķ����ID
  // ���ID��Ψһ�Ĺ�Լ���ݣ����Կ���������ʶ��Լ����
  std::unordered_map<ProcessFunctionClassId,
                     std::list<SyntaxAnalysisTableEntryId>>
      reduct_classify_table;
  // ��ֵǰ�벿��ΪShiftReductAttachedData������ź�ת�Ƶ����﷨��������ĿID
  // ��ֵ��벿��ΪReductAttachedData��Լʱʹ�õİ�װReduct��������Ķ���ID
  std::unordered_map<
      std::pair<SyntaxAnalysisTableEntryId, ProcessFunctionClassId>,
      std::list<SyntaxAnalysisTableEntryId>,
      SyntaxAnalysisTableEntryIdAndProcessFunctionClassIdHasher>
      shift_reduct_classify_table;
  // �ڸ������²��ܹ�ԼҲ����������﷨��������ĿID
  std::list<SyntaxAnalysisTableEntryId> nothing_to_do_entry_ids;
  ProductionNodeId transform_id = terminal_node_ids[index];
  for (auto syntax_analysis_table_entry_id : syntax_analysis_table_entry_ids) {
    // ����unordered_map���з���
    auto action_and_attahced_data =
        GetSyntaxAnalysisTableEntry(syntax_analysis_table_entry_id)
            .AtTerminalNode(transform_id);
    if (action_and_attahced_data) [[unlikely]] {
      // �ڸ���ǰ���ڵ㴦����
      switch (action_and_attahced_data->GetActionType()) {
        case ActionType::kShift:
          shift_classify_table[action_and_attahced_data->GetShiftAttachedData()
                                   .GetNextSyntaxAnalysisTableEntryId()]
              .push_back(syntax_analysis_table_entry_id);
          break;
        case ActionType::kReduct:
          reduct_classify_table[action_and_attahced_data
                                    ->GetReductAttachedData()
                                    .GetProcessFunctionClassId()]
              .push_back(syntax_analysis_table_entry_id);
          break;
        case ActionType::kShiftReduct:
          shift_reduct_classify_table
              [std::make_pair(action_and_attahced_data->GetShiftAttachedData()
                                  .GetNextSyntaxAnalysisTableEntryId(),
                              action_and_attahced_data->GetReductAttachedData()
                                  .GetProcessFunctionClassId())]
                  .push_back(syntax_analysis_table_entry_id);
          break;
        default:
          assert(false);
          break;
      }
    } else {
      nothing_to_do_entry_ids.push_back(syntax_analysis_table_entry_id);
    }
  }

  size_t next_index = index + 1;
  // �����ڵ�ǰ����������Ľڵ�
  for (auto& syntax_analysis_table_entry_ids : shift_classify_table) {
    if (syntax_analysis_table_entry_ids.second.size() > 1) {
      // ���ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
      SyntaxAnalysisTableTerminalNodeClassify(
          terminal_node_ids, next_index,
          std::move(syntax_analysis_table_entry_ids.second), equivalent_ids);
    }
  }
  // �����ڵ�ǰ�����¹�Լ�Ľڵ�
  for (auto& syntax_analysis_table_entry_ids : reduct_classify_table) {
    if (syntax_analysis_table_entry_ids.second.size() > 1) {
      // ���ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
      SyntaxAnalysisTableTerminalNodeClassify(
          terminal_node_ids, next_index,
          std::move(syntax_analysis_table_entry_ids.second), equivalent_ids);
    }
  }
  // �����ڵ�ǰ�����¼ȿ�������Ҳ���Թ�Լ�Ľڵ�
  for (auto& syntax_analysis_table_entry_ids : shift_reduct_classify_table) {
    if (syntax_analysis_table_entry_ids.second.size() > 1) {
      // ���ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
      SyntaxAnalysisTableTerminalNodeClassify(
          terminal_node_ids, next_index,
          std::move(syntax_analysis_table_entry_ids.second), equivalent_ids);
    }
  }
  // �����ڵ�ǰ�����¼ȿ�������Ҳ���Թ�Լ�Ľڵ�
  if (nothing_to_do_entry_ids.size() > 1) [[likely]] {
    // ���ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
    SyntaxAnalysisTableTerminalNodeClassify(terminal_node_ids, next_index,
                                            std::move(nothing_to_do_entry_ids),
                                            equivalent_ids);
  }
}

void SyntaxGenerator::SyntaxAnalysisTableNonTerminalNodeClassify(
    const std::vector<ProductionNodeId>& nonterminal_node_ids, size_t index,
    std::list<SyntaxAnalysisTableEntryId>&& syntax_analysis_table_entry_ids,
    std::vector<std::list<SyntaxAnalysisTableEntryId>>* equivalent_ids) {
  if (index >= nonterminal_node_ids.size()) {
    // ������ɣ������Ժϲ�����Ŀ��ӵ����
    assert(syntax_analysis_table_entry_ids.size() > 1);
    equivalent_ids->emplace_back(std::move(syntax_analysis_table_entry_ids));
  } else {
    assert(GetProductionNode(nonterminal_node_ids[index]).GetType() ==
           ProductionNodeType::kNonTerminalNode);
    // ���������ת�������µ�ת�ƽ������
    std::unordered_map<SyntaxAnalysisTableEntryId,
                       std::list<SyntaxAnalysisTableEntryId>>
        classify_table;
    ProductionNodeId transform_id = nonterminal_node_ids[index];
    for (auto production_node_id : syntax_analysis_table_entry_ids) {
      // ����unordered_map���з���
      classify_table[GetSyntaxAnalysisTableEntry(production_node_id)
                         .AtNonTerminalNode(transform_id)]
          .push_back(production_node_id);
    }
    size_t next_index = index + 1;
    for (auto syntax_analysis_table_entry_ids : classify_table) {
      if (syntax_analysis_table_entry_ids.second.size() > 1) {
        // ���ຬ�ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
        SyntaxAnalysisTableNonTerminalNodeClassify(
            nonterminal_node_ids, next_index,
            std::move(syntax_analysis_table_entry_ids.second), equivalent_ids);
      }
    }
  }
}

std::vector<std::list<SyntaxAnalysisTableEntryId>>
SyntaxGenerator::SyntaxAnalysisTableEntryClassify(
    std::vector<ProductionNodeId>&& operator_node_ids,
    std::vector<ProductionNodeId>&& terminal_node_ids,
    std::vector<ProductionNodeId>&& nonterminal_node_ids) {
  // �洢��ͬת��������ת�Ʊ���ĿID
  std::vector<std::list<SyntaxAnalysisTableEntryId>> operator_classify_result,
      terminal_classify_result, final_classify_result;
  std::list<SyntaxAnalysisTableEntryId> entry_ids;
  for (size_t i = 0; i < syntax_analysis_table_.size(); i++) {
    // ������д�������﷨��������ĿID
    entry_ids.push_back(SyntaxAnalysisTableEntryId(i));
  }
  SyntaxAnalysisTableTerminalNodeClassify(
      operator_node_ids, 0, std::move(entry_ids), &operator_classify_result);
  for (auto& syntax_analysis_table_entry_ids : operator_classify_result) {
    if (syntax_analysis_table_entry_ids.size() > 1) {
      SyntaxAnalysisTableTerminalNodeClassify(
          terminal_node_ids, 0, std::move(syntax_analysis_table_entry_ids),
          &terminal_classify_result);
    }
  }
  for (auto& syntax_analysis_table_entry_ids : terminal_classify_result) {
    if (syntax_analysis_table_entry_ids.size() > 1) {
      SyntaxAnalysisTableNonTerminalNodeClassify(
          nonterminal_node_ids, 0, std::move(syntax_analysis_table_entry_ids),
          &final_classify_result);
    }
  }
  return final_classify_result;
}

inline void SyntaxGenerator::RemapSyntaxAnalysisTableEntryId(
    const std::unordered_map<SyntaxAnalysisTableEntryId,
                             SyntaxAnalysisTableEntryId>& old_id_to_new_id) {
  for (auto& entry : syntax_analysis_table_) {
    entry.ResetEntryId(old_id_to_new_id);
  }
}

void SyntaxGenerator::SyntaxAnalysisTableMergeOptimize() {
  std::array<std::vector<ProductionNodeId>, 4>&&
      classified_production_node_ids = ClassifyProductionNodes();
  constexpr size_t terminal_index =
      static_cast<size_t>(ProductionNodeType::kTerminalNode);
  constexpr size_t operator_index =
      static_cast<size_t>(ProductionNodeType::kOperatorNode);
  constexpr size_t nonterminal_index =
      static_cast<size_t>(ProductionNodeType::kNonTerminalNode);
  std::vector<std::list<SyntaxAnalysisTableEntryId>> classified_ids =
      SyntaxAnalysisTableEntryClassify(
          std::move(classified_production_node_ids[operator_index]),
          std::move(classified_production_node_ids[terminal_index]),
          std::move(classified_production_node_ids[nonterminal_index]));
  // �洢��Ҫ��ӳ�����ĿID������ĿID
  std::unordered_map<SyntaxAnalysisTableEntryId, SyntaxAnalysisTableEntryId>
      remapped_entry_id_to_new_entry_id;
  for (auto& entry_ids : classified_ids) {
    // ��ӱ��ϲ��ľ���Ŀ����ͬ��Ŀ��ӳ��
    // �ظ�����Ŀֻ�������������еĵ�һ���������ȫ��ӳ�䵽����
    auto iter = entry_ids.begin();
    SyntaxAnalysisTableEntryId new_id = *iter;
    ++iter;
    while (iter != entry_ids.end()) {
      assert(remapped_entry_id_to_new_entry_id.find(*iter) ==
             remapped_entry_id_to_new_entry_id.end());
      remapped_entry_id_to_new_entry_id[*iter] = new_id;
      ++iter;
    }
  }
  // �洢�ƶ������Ŀ������ĿID
  std::unordered_map<SyntaxAnalysisTableEntryId, SyntaxAnalysisTableEntryId>
      moved_entry_to_new_entry_id;
  // ��ʼ�ϲ�
  assert(syntax_analysis_table_.size() > 0);
  // ���ƿ��ŷ�����㷨
  // Ѱ��û�б��ϲ�����Ŀ������������vector���±�һ��

  // ��һ��Ҫ�������Ŀ
  SyntaxAnalysisTableEntryId next_process_entry_index(0);
  // ����ǰ��ά��ԭλ���﷨��������Ŀ
  while (next_process_entry_index < syntax_analysis_table_.size() &&
         remapped_entry_id_to_new_entry_id.find(next_process_entry_index) ==
             remapped_entry_id_to_new_entry_id.end())
    [[likely]] {
      // ��������Ŀ�Դ���ԭλ
      ++next_process_entry_index;
    }
  // ��һ������λ��
  SyntaxAnalysisTableEntryId next_insert_position_index(
      next_process_entry_index);
  ++next_process_entry_index;
  while (next_process_entry_index < syntax_analysis_table_.size()) {
    if (remapped_entry_id_to_new_entry_id.find(next_process_entry_index) ==
        remapped_entry_id_to_new_entry_id.end()) {
      // ����Ŀ����
      assert(next_insert_position_index != next_process_entry_index);
      // ��Ҫ�ƶ�����λ�ñ���vector����
      syntax_analysis_table_[next_insert_position_index] =
          std::move(syntax_analysis_table_[next_process_entry_index]);
      // ��ӳ�䱣����Ŀ����λ��
      moved_entry_to_new_entry_id[next_process_entry_index] =
          next_insert_position_index;
      ++next_insert_position_index;
    }
    ++next_process_entry_index;
  }
  // �ͷŶ���ռ�
  syntax_analysis_table_.resize(next_insert_position_index);
  // �����о�ID����Ϊ��ID
  RemapSyntaxAnalysisTableEntryId(moved_entry_to_new_entry_id);
}

inline void SyntaxGenerator::SaveConfig() const {
  dfa_generator_.SaveConfig();
  std::ofstream config_file(frontend::common::kSyntaxConfigFileName,
                            std::ios_base::binary | std::ios_base::out);
  // oarchiveҪ��config_file����ǰ�����������ļ��������ڷ����л�ʱ�����쳣
  {
    boost::archive::binary_oarchive oarchive(config_file);
    oarchive << *this;
  }
}

std::string SyntaxGenerator::FormatSingleProductionBody(
    ProductionNodeId nonterminal_node_id,
    ProductionBodyId production_body_id) const {
  std::string format_result;
  const NonTerminalProductionNode& production_node_now =
      static_cast<const NonTerminalProductionNode&>(
          GetProductionNode(nonterminal_node_id));
  assert(production_node_now.GetType() == ProductionNodeType::kNonTerminalNode);
  format_result += std::format(
      "{:} ->",
      GetNodeSymbolStringFromId(production_node_now.GetNodeSymbolId()));
  for (auto id :
       production_node_now.GetBody(production_body_id).production_body) {
    format_result +=
        std::format(" {:}", GetNodeSymbolStringFromProductionNodeId(id));
  }
  return format_result;
}

std::string SyntaxGenerator::FormatProductionBodys(
    ProductionNodeId nonterminal_node_id) {
  std::string format_result;
  const NonTerminalProductionNode& production_node_now =
      static_cast<const NonTerminalProductionNode&>(
          GetProductionNode(nonterminal_node_id));
  assert(production_node_now.GetType() == ProductionNodeType::kNonTerminalNode);
  for (auto& body_id : production_node_now.GetAllBodyIds()) {
    format_result += FormatSingleProductionBody(nonterminal_node_id, body_id);
    format_result += '\n';
  }
  return format_result;
}

std::string SyntaxGenerator::FormatProductionItem(
    const ProductionItem& production_item) const {
  std::string format_result;
  auto& [production_node_id, production_body_id, next_word_to_shift_index] =
      production_item;
  const NonTerminalProductionNode& production_node_now =
      static_cast<const NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  format_result += std::format(
      "{:} ->",
      GetNodeSymbolStringFromId(production_node_now.GetNodeSymbolId()));
  const auto& production_node_waiting_spread_body =
      production_node_now.GetBody(production_body_id).production_body;
  for (size_t i = 0; i < next_word_to_shift_index; ++i) {
    format_result +=
        std::format(" {:}", GetNodeSymbolStringFromProductionNodeId(
                                production_node_waiting_spread_body[i]));
  }
  format_result += std::format(" ��");
  for (size_t i = next_word_to_shift_index;
       i < production_node_waiting_spread_body.size(); i++) {
    format_result +=
        std::format(" {:}", GetNodeSymbolStringFromProductionNodeId(
                                production_node_waiting_spread_body[i]));
  }
  return format_result;
}

std::string SyntaxGenerator::FormatProductionItemAndLookForwardSymbols(
    const ProductionItem& production_item,
    const ForwardNodesContainer& look_forward_node_ids) const {
  return FormatProductionItem(production_item) + std::format(" ��ǰ�����ţ�") +
         FormatLookForwardSymbols(look_forward_node_ids);
}

std::string SyntaxGenerator::FormatLookForwardSymbols(
    const ForwardNodesContainer& look_forward_node_ids) const {
  if (look_forward_node_ids.empty()) [[unlikely]] {
    return std::string();
  }
  std::string format_result;
  for (const auto& node_id : look_forward_node_ids) {
    format_result += GetNodeSymbolStringFromProductionNodeId(node_id);
    format_result += ' ';
  }
  // ����β���ո�
  format_result.pop_back();
  return format_result;
}

std::string SyntaxGenerator::FormatProductionItems(
    ProductionItemSetId production_item_set_id) const {
  std::string format_result;
  const ProductionItemSet& production_item_set =
      GetProductionItemSet(production_item_set_id);
  for (const auto& item_and_forward_nodes :
       production_item_set.GetItemsAndForwardNodeIds()) {
    format_result += FormatProductionItem(item_and_forward_nodes.first);
    format_result += std::format(" ��ǰ�����ţ�");
    format_result += FormatLookForwardSymbols(item_and_forward_nodes.second);
    format_result += '\n';
  }
  if (!format_result.empty()) [[likely]] {
    // �������Ļ��з�
    format_result.pop_back();
  }
  return format_result;
}

void SyntaxGenerator::SyntaxAnalysisTableConstruct() {
  // �������뵽�ļ�βʱ���صĽڵ�
  ProductionNodeId end_production_node_id = AddEndNode();
  // �������������ļ�β��ʱ���ص�����
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      end_of_file_saved_data;
  end_of_file_saved_data.production_node_id = end_production_node_id;
  end_of_file_saved_data.node_type = ProductionNodeType::kEndNode;
  dfa_generator_.SetEndOfFileSavedData(std::move(end_of_file_saved_data));
  // ���ɳ�ʼ���������������ʽ����
  ProductionItemSetId root_production_item_set_id = EmplaceProductionItemSet();
  ProductionItemSet& root_production_item_set =
      GetProductionItemSet(root_production_item_set_id);
  // �����ڲ�������ʽ����ֹ�û�����ĸ�����ʽ���ж������ʽ�嵼��һϵ������
  // ������ʽ��ʹ�����޷�ͨ���궨����ַ�@�Է�ֹ���û�����Ľڵ�����ͻ
  ProductionNodeId inside_root_production_node_id =
      AddNonTerminalProduction<RootReductClass>(
          "@RootNode",
          {GetNodeSymbolStringFromProductionNodeId(GetRootProductionNodeId())});
  root_production_item_set.AddMainItemAndForwardNodeIds(
      ProductionItem(inside_root_production_node_id, ProductionBodyId(0),
                     NextWordToShiftIndex(0)),
      std::initializer_list<ProductionNodeId>{end_production_node_id});
  SetRootSyntaxAnalysisTableEntryId(
      root_production_item_set.GetSyntaxAnalysisTableEntryId());
  // ������ǰ������ͬʱ�����﷨������
  SpreadLookForwardSymbolAndConstructSyntaxAnalysisTableEntry(
      root_production_item_set_id);
  // �����ڲ�������ʽ�����ļ�βʱ���Խ��ܣ�����Ӧ�Կ���������
  SyntaxAnalysisTableEntryId inside_root_syntax_analysis_entry_id =
      root_production_item_set.GetSyntaxAnalysisTableEntryId();
  GetSyntaxAnalysisTableEntry(inside_root_syntax_analysis_entry_id)
      .SetAcceptInEofForwardNode(end_production_node_id);
  // ���������û����õ�root�ڵ���������ļ�β���صĽڵ�ʱʹ��Accept����
  SyntaxAnalysisTableEntryId entry_after_shift_user_defined_root =
      GetSyntaxAnalysisTableEntry(
          root_production_item_set.GetSyntaxAnalysisTableEntryId())
          .AtNonTerminalNode(GetRootProductionNodeId());
  GetSyntaxAnalysisTableEntry(entry_after_shift_user_defined_root)
      .SetAcceptInEofForwardNode(end_production_node_id);
  // �ϲ���Ч�ѹ���﷨������
  SyntaxAnalysisTableMergeOptimize();
}

void SyntaxGenerator::ConstructSyntaxConfig() {
  SyntaxGeneratorInit();
  ConfigConstruct();
  CheckUndefinedProductionRemained();
  dfa_generator_.DfaConstruct();
  SyntaxAnalysisTableConstruct();
  // ��������
  SaveConfig();
}

void SyntaxGenerator::SyntaxGeneratorInit() {
  undefined_productions_.clear();
  manager_nodes_.ObjectManagerInit();
  manager_node_symbol_.StructManagerInit();
  manager_terminal_body_symbol_.StructManagerInit();
  node_symbol_id_to_node_id_.clear();
  production_body_symbol_id_to_node_id_.clear();
  production_item_sets_.ObjectManagerInit();
  syntax_analysis_table_entry_id_to_production_item_set_id_.clear();
  root_production_node_id_ = ProductionNodeId::InvalidId();
  root_syntax_analysis_table_entry_id_ =
      SyntaxAnalysisTableEntryId::InvalidId();
  dfa_generator_.DfaInit();
  syntax_analysis_table_.clear();
  manager_process_function_class_.ObjectManagerInit();
}

void SyntaxGenerator::AddUnableContinueNonTerminalNode(
    const std::string& undefined_symbol, std::string&& node_symbol,
    std::vector<std::string>&& subnode_symbols,
    ProcessFunctionClassId class_id) {
  assert(!undefined_symbol.empty() && !node_symbol.empty() &&
         !subnode_symbols.empty() && class_id.IsValid());
  std::string temp_output(std::format("���ս����ʽ {:} ->", node_symbol));
  for (const auto& subnode_symbol : subnode_symbols) {
    temp_output += std::format(" {:}", subnode_symbol);
  }
  OutPutInfo(temp_output);
  OutPutInfo(
      std::format("������δ�������ʽ{:}�����Ƴ����", undefined_symbol));
  undefined_productions_.emplace(
      undefined_symbol, std::make_tuple(std::move(node_symbol),
                                        std::move(subnode_symbols), class_id));
}

void SyntaxGenerator::CheckNonTerminalNodeCanContinue(
    const std::string& added_node_symbol) {
  auto [iter_begin, iter_end] =
      undefined_productions_.equal_range(added_node_symbol);
  while (iter_begin != iter_end) {
    auto& [node_could_continue_to_add_symbol, node_body,
           process_function_class_id_] = iter_begin->second;
    std::string temp_output(
        std::format("���ս����ʽ {:} ->", node_could_continue_to_add_symbol));
    for (const auto& subnode_symbol : node_body) {
      temp_output += std::format(" {:}", subnode_symbol);
    }
    temp_output += std::format("�ָ����");
    OutPutInfo(temp_output);
    // ����޷������������ٴε���AddUnableContinueNonTerminalNode�������
    AddNonTerminalProduction(std::move(node_could_continue_to_add_symbol),
                             std::move(node_body), process_function_class_id_);
    undefined_productions_.erase(iter_begin++);
  }
}

void SyntaxGenerator::CheckUndefinedProductionRemained() {
  if (!undefined_productions_.empty()) {
    // �Դ���δ�������ʽ
    for (auto& item : undefined_productions_) {
      auto& [node_symbol, node_bodys, class_id] = item.second;
      std::string temp_output(
          std::format("���ս����ʽ�� {:} ->", node_symbol));
      for (auto& body : node_bodys) {
        temp_output += std::format(" {:}", body);
      }
      temp_output += "��";
      OutPutError(temp_output);
      OutPutError(std::format("����ʽ�� {:} δ����", item.first));
    }
    exit(-1);
  }
}

void SyntaxGenerator::AddKeyWord(std::string&& key_word) {
  // �ؼ������ȼ�Ĭ��Ϊ2
  // �Զ�����ͬ���ս�ڵ�
  // �ؼ��ֶ���ʱ��key_word�������ʶ�����������ʽ
  AddTerminalProduction(
      std::string(key_word), std::move(key_word),
      frontend::generator::dfa_generator::DfaGenerator::WordPriority(2), true);
}
ProductionNodeId SyntaxGenerator::GetProductionNodeIdFromNodeSymbolId(
    SymbolId node_symbol_id) {
  auto iter = node_symbol_id_to_node_id_.find(node_symbol_id);
  if (iter == node_symbol_id_to_node_id_.end()) {
    return ProductionNodeId::InvalidId();
  } else {
    return iter->second;
  }
}
ProductionNodeId SyntaxGenerator::GetProductionNodeIdFromBodySymbolId(
    SymbolId body_symbol_id) {
  auto iter = production_body_symbol_id_to_node_id_.find(body_symbol_id);
  if (iter != production_body_symbol_id_to_node_id_.end()) {
    return iter->second;
  } else {
    return ProductionNodeId::InvalidId();
  }
}

inline ProductionNodeId SyntaxGenerator::GetProductionNodeIdFromNodeSymbol(
    const std::string& node_symbol) {
  SymbolId node_symbol_id = GetNodeSymbolId(node_symbol);
  if (node_symbol_id.IsValid()) {
    return GetProductionNodeIdFromNodeSymbolId(node_symbol_id);
  } else {
    return ProductionNodeId::InvalidId();
  }
}

ProductionNodeId SyntaxGenerator::GetProductionNodeIdFromBodySymbol(
    const std::string& body_symbol) {
  SymbolId body_symbol_id = GetBodySymbolId(body_symbol);
  if (body_symbol_id.IsValid()) {
    return GetProductionNodeIdFromBodySymbolId(body_symbol_id);
  } else {
    return ProductionNodeId::InvalidId();
  }
}

}  // namespace frontend::generator::syntax_generator