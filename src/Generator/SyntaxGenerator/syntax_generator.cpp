#include "syntax_generator.h"

#include <queue>

namespace frontend::generator::syntax_generator {
SyntaxGenerator::ProductionNodeId SyntaxGenerator::AddTerminalNode(
    const std::string& node_symbol, const std::string& body_symbol,
    WordPriority node_priority) {
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
    bool result = dfa_generator_.AddRegexpression(
        body_symbol, std::move(word_attached_data), node_priority);
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

inline SyntaxGenerator::ProductionNodeId SyntaxGenerator::SubAddTerminalNode(
    SymbolId node_symbol_id, SymbolId body_symbol_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<TerminalProductionNode>(node_symbol_id,
                                                           body_symbol_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(body_symbol_id, node_id);
  return node_id;
}

SyntaxGenerator::ProductionNodeId SyntaxGenerator::AddBinaryOperatorNode(
    const std::string& operator_symbol,
    OperatorAssociatityType binary_operator_associatity_type,
    OperatorPriority binary_operator_priority_level) {
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
  ProductionNodeId operator_node_id = SubAddOperatorNode(
      operator_node_symbol_id, binary_operator_associatity_type,
      binary_operator_priority_level);
  assert(operator_node_id.IsValid());
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      word_attached_data;
  word_attached_data.production_node_id = operator_node_id;
  word_attached_data.node_type = ProductionNodeType::kOperatorNode;
  word_attached_data.binary_operator_associate_type =
      binary_operator_associatity_type;
  word_attached_data.binary_operator_priority = binary_operator_priority_level;
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
      binary_operator_priority_level.GetRawValue()));
  return operator_node_id;
}

SyntaxGenerator::ProductionNodeId SyntaxGenerator::AddLeftUnaryOperatorNode(
    const std::string& operator_symbol,
    OperatorAssociatityType unary_operator_associatity_type,
    OperatorPriority unary_operator_priority_level) {
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
  ProductionNodeId operator_node_id = SubAddOperatorNode(
      operator_node_symbol_id, unary_operator_associatity_type,
      unary_operator_priority_level);
  assert(operator_node_id.IsValid());
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      word_attached_data;
  word_attached_data.production_node_id = operator_node_id;
  word_attached_data.node_type = ProductionNodeType::kOperatorNode;
  word_attached_data.unary_operator_associate_type =
      unary_operator_associatity_type;
  word_attached_data.unary_operator_priority = unary_operator_priority_level;
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
      unary_operator_priority_level.GetRawValue()));
  return operator_node_id;
}

SyntaxGenerator::ProductionNodeId SyntaxGenerator::AddBinaryUnaryOperatorNode(
    const std::string& operator_symbol,
    OperatorAssociatityType binary_operator_associatity_type,
    OperatorPriority binary_operator_priority_level,
    OperatorAssociatityType unary_operator_associatity_type,
    OperatorPriority unary_operator_priority_level) {
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
  ProductionNodeId operator_node_id = SubAddOperatorNode(
      operator_node_symbol_id, unary_operator_associatity_type,
      unary_operator_priority_level);
  assert(operator_node_id.IsValid());
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      word_attached_data;
  word_attached_data.production_node_id = operator_node_id;
  word_attached_data.node_type = ProductionNodeType::kOperatorNode;
  word_attached_data.binary_operator_associate_type =
      binary_operator_associatity_type;
  word_attached_data.binary_operator_priority = binary_operator_priority_level;
  word_attached_data.unary_operator_associate_type =
      unary_operator_associatity_type;
  word_attached_data.unary_operator_priority = unary_operator_priority_level;
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
      binary_operator_priority_level.GetRawValue(),
      unary_operator_priority_level.GetRawValue()));
  return operator_node_id;
}

inline SyntaxGenerator::ProductionNodeId SyntaxGenerator::SubAddOperatorNode(
    SymbolId node_symbol_id, OperatorAssociatityType associatity_type,
    OperatorPriority priority_level) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<OperatorProductionNode>(
          node_symbol_id, associatity_type, priority_level);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}

SyntaxGenerator::ProductionNodeId SyntaxGenerator::AddNonTerminalNode(
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
  OutPutInfo(std::format("���ս�ڵ�ID:��{:} ��ǰ����ʽ��ID��{:}",
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
  nonterminal_production_node.SetProductionCouldBeEmptyRedut();
}

inline SyntaxGenerator::ProductionNodeId SyntaxGenerator::SubAddNonTerminalNode(
    SymbolId node_symbol_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<NonTerminalProductionNode>(node_symbol_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}

inline SyntaxGenerator::ProductionNodeId SyntaxGenerator::AddEndNode() {
  ProductionNodeId node_id = manager_nodes_.EmplaceObject<EndNode>();
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  return node_id;
}

void SyntaxGenerator::SetRootProduction(
    const std::string& production_node_symbol) {
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromNodeSymbol(production_node_symbol);
  if (GetRootProductionNodeId().IsValid()) [[unlikely]] {
    OutPutError(std::format("���ұ�������һ��������ʽ"));
    exit(-1);
  }
  assert(production_node_id.IsValid());
  // ���ɸ�����ʽ
  // ������ʽ��ʹ�����޷�ͨ���궨����ַ�@�Է�ֹ���û�����Ľڵ�����ͻ
  ProductionNodeId root_production_node_id =
      AddNonTerminalNode<RootReductClass>("@RootNode",
                                          {production_node_symbol});
  SetRootProductionNodeId(root_production_node_id);
}

// ��ȡ�ڵ�

inline SyntaxGenerator::BaseProductionNode& SyntaxGenerator::GetProductionNode(
    ProductionNodeId production_node_id) {
  return const_cast<BaseProductionNode&>(
      static_cast<const SyntaxGenerator&>(*this).GetProductionNode(
          production_node_id));
}

inline const SyntaxGenerator::BaseProductionNode&
SyntaxGenerator::GetProductionNode(ProductionNodeId production_node_id) const {
  return manager_nodes_[production_node_id];
}

inline SyntaxGenerator::BaseProductionNode&
SyntaxGenerator::GetProductionNodeFromNodeSymbolId(SymbolId symbol_id) {
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromNodeSymbolId(symbol_id);
  assert(symbol_id.IsValid());
  return GetProductionNode(production_node_id);
}

inline SyntaxGenerator::BaseProductionNode&
SyntaxGenerator::GetProductionNodeBodyFromSymbolId(SymbolId symbol_id) {
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromBodySymbolId(symbol_id);
  assert(symbol_id.IsValid());
  return GetProductionNode(production_node_id);
}

// ��ȡ���ս�ڵ��е�һ������ʽ��

inline const std::vector<SyntaxGenerator::ProductionNodeId>&
SyntaxGenerator::GetProductionBody(ProductionNodeId production_node_id,
                                   ProductionBodyId production_body_id) {
  NonTerminalProductionNode& nonterminal_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  // ֻ�з��ս�ڵ���ж������ʽ�壬�������ڵ���øú���������
  assert(nonterminal_node.Type() == ProductionNodeType::kNonTerminalNode);
  return nonterminal_node.GetBody(production_body_id).production_body;
}

// ��¼�����������ĺ���ID

inline void SyntaxGenerator::AddCoreItemBelongToCoreId(
    const CoreItem& core_item, CoreId core_id) {
  auto& [production_node_id, production_body_id, point_index] = core_item;
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  assert(production_node.Type() == ProductionNodeType::kNonTerminalNode);
  production_node.AddCoreItemBelongToCoreId(production_body_id, point_index,
                                            core_id);
}

inline const std::list<SyntaxGenerator::CoreId>&
SyntaxGenerator::GetCoreIdFromCoreItem(ProductionNodeId production_node_id,
                                       ProductionBodyId body_id,
                                       PointIndex point_index) {
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  assert(production_node.Type() == ProductionNodeType::kNonTerminalNode);
  return production_node.GetCoreIdFromCoreItem(body_id, point_index);
}

void SyntaxGenerator::GetNonTerminalNodeFirstNodeIds(
    ProductionNodeId production_node_id,
    std::unordered_set<ProductionNodeId>* result,
    std::unordered_set<ProductionNodeId>&& processed_nodes) {
  bool inserted = processed_nodes.insert(production_node_id).second;
  assert(inserted);
  if (!production_node_id.IsValid()) [[unlikely]] {
    return;
  }
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  assert(production_node.Type() == ProductionNodeType::kNonTerminalNode);
  for (auto& body : production_node.GetAllBody()) {
    ProductionBodyId node_id_index(0);
    // ���ڴ���Ĳ���ʽ�ڵ��ID
    ProductionNodeId node_id;
    while (node_id_index < body.production_body.size()) {
      node_id = body.production_body[node_id_index];
      if (GetProductionNode(node_id).Type() !=
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

std::unordered_set<SyntaxGenerator::ProductionNodeId> SyntaxGenerator::First(
    ProductionNodeId production_node_id, ProductionBodyId production_body_id,
    PointIndex point_index, const ForwardNodesContainerType& next_node_ids) {
  ProductionNodeId node_id = GetProductionNodeIdInBody(
      production_node_id, production_body_id, point_index);
  if (node_id.IsValid()) {
    std::unordered_set<ProductionNodeId> return_set;
    switch (GetProductionNode(node_id).Type()) {
      case ProductionNodeType::kTerminalNode:
      case ProductionNodeType::kOperatorNode:
        return_set.insert(node_id);
        break;
      case ProductionNodeType::kNonTerminalNode:
        // �ϲ���ǰ���Ҳ���ս�ڵ���ܵ����е���ID
        GetNonTerminalNodeFirstNodeIds(node_id, &return_set);
        if (static_cast<NonTerminalProductionNode&>(GetProductionNode(node_id))
                .CouldBeEmptyReduct()) {
          // ��ǰ���Ҳ�ķ��ս�ڵ���Կչ�Լ����Ҫ��������һ���ڵ�����
          return_set.merge(First(production_node_id, production_body_id,
                                 PointIndex(point_index + 1), next_node_ids));
        }
        break;
      default:
        assert(false);
        break;
    }
    return return_set;
  } else {
    return next_node_ids;
  }
}

bool SyntaxGenerator::CoreClosure(CoreId core_id) {
  OutPutInfo(
      std::format("�Ժ��ģ�ID = {:}��ִ�бհ�����", core_id.GetRawValue()));
  Core& core = GetCore(core_id);
  if (core.IsClosureAvailable()) {
    // �հ���Ч����������
    return false;
  }
  ParsingTableEntry& parsing_table_entry =
      GetParsingTableEntry(core.GetParsingTableEntryId());
  // �洢��ǰ��չ������
  // �洢ָ���չ����ĵ�����
  std::queue<std::map<CoreItem, ForwardNodesContainerType>::const_iterator>
      items_waiting_process;
  const auto& core_items_and_forward_nodes =
      GetCoreItemsAndForwardNodes(core_id);
  for (auto& iter : GetCoreMainItems(core_id)) {
    // ��������к�����ѹ����������
    items_waiting_process.push(iter);
  }
  while (!items_waiting_process.empty()) {
    auto item_now = items_waiting_process.front();
    items_waiting_process.pop();
    const auto [production_node_id, production_body_id, point_index] =
        item_now->first;
    NonTerminalProductionNode& production_node_now =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id));
    assert(production_node_now.Type() == ProductionNodeType::kNonTerminalNode);
    ProductionNodeId next_production_node_id = GetProductionNodeIdInBody(
        production_node_id, production_body_id, point_index);
    if (!next_production_node_id.IsValid()) {
      // �޺�̽ڵ㣬���ù�Լ��Ŀ

      // ��װ��Լʹ�õ�����
      ParsingTableEntry::ReductAttachedData reduct_attached_data(
          production_node_id,
          GetProcessFunctionClass(production_node_id, production_body_id),
          GetProductionBody(production_node_id, production_body_id));
      OutPutInfo(std::format("�") + FormatItem(item_now->first));
      OutPutInfo(std::format("����ǰ�����ţ�") +
                 FormatLookForwardSymbols(item_now->second) + " ��ִ�й�Լ");
      const auto& production_body =
          production_node_now.GetBody(production_body_id).production_body;
      for (auto node_id : item_now->second) {
        // ��ÿ����ǰ���������ù�Լ����
        parsing_table_entry.SetTerminalNodeActionAndAttachedData(
            node_id, reduct_attached_data);
      }
      continue;
    }
    NonTerminalProductionNode& next_production_node =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(next_production_node_id));
    if (next_production_node.Type() != ProductionNodeType::kNonTerminalNode) {
      continue;
    }
    // չ�����ս�ڵ㣬��Ϊ�䴴����ǰ�����ż�
    ForwardNodesContainerType forward_node_ids =
        First(production_node_id, production_body_id,
              PointIndex(point_index + 1), item_now->second);
    for (auto body_id : next_production_node.GetAllBodyIds()) {
      // ���÷��ս�ڵ��е�ÿ������ʽ����뵽core�У����������
      auto [iter, inserted] = AddItemAndForwardNodeIdsToCore(
          core_id, CoreItem(next_production_node_id, body_id, PointIndex(0)),
          forward_node_ids);
      if (inserted) {
        // ��������µ�������ӵ������еȴ���������¼�������ڵ��º���ID
        items_waiting_process.push(iter);
      }
    }
    if (next_production_node.CouldBeEmptyReduct()) {
      // ���Ҳ�ķ��ս�ڵ���Կչ�Լ
      // �������ӿչ�Լ��õ������ǰ�����Ÿ���ԭ���������ǰ�����ż�
      auto [iter, inserted] = AddItemAndForwardNodeIdsToCore(
          core_id,
          CoreItem(production_node_id, production_body_id,
                   PointIndex(point_index + 1)),
          item_now->second);
      if (inserted) {
        // ��������µ�������ӵ������еȴ�����
        items_waiting_process.push(iter);
      }
    }
    OutPutInfo(std::format("�") + FormatItem(item_now->first));
    const auto& production_node_waiting_spread_body =
        production_node_now.GetBody(production_body_id).production_body;
    OutPutInfo(std::format(" ��չ��"));
    OutPutInfo(std::format("�ýڵ㵱ǰӵ�е���ǰ�����ţ� ") +
               FormatLookForwardSymbols(item_now->second));
  }
  SetCoreClosureAvailable(core_id);
  return true;
}

SyntaxGenerator::CoreId SyntaxGenerator::GetCoreIdFromCoreItems(
    const std::list<SyntaxGenerator::CoreItem>& items) {
#ifdef _DEBUG
  // �������в������ظ���
  for (auto iter = items.cbegin(); iter != items.cend(); ++iter) {
    auto compare_iter = iter;
    while (++compare_iter != items.cend()) {
      assert(*iter != *compare_iter);
    }
  }
#endif  // _DEBUG

  // �洢����ID��������������ĸ�����ֻ�а���ȫ��������ſ��ܳ�Ϊ��Щ�����ڵĺ���
  std::unordered_map<CoreId, size_t> core_includes_item_size;
  // ͳ�ƺ��İ���������ĸ���
  for (const auto& item : items) {
    auto [production_node_id, production_body_id, point_index] = item;
    for (auto core_id : GetCoreIdFromCoreItem(
             production_node_id, production_body_id, point_index)) {
      auto [iter, inserted] = core_includes_item_size.emplace(core_id, 1);
      if (!inserted) {
        // �ú���ID�Ѿ���¼�����Ӹú���ID���ֵĴ���
        ++iter->second;
      }
    }
  }
  // ������г��ֹ��ĺ���ID���ҳ��������и�����ĺ���ID
  auto iter = core_includes_item_size.begin();
  while (iter != core_includes_item_size.end()) {
    if (iter->second != items.size()) [[likely]] {
      // �ú���û����������
      continue;
    }
    // ���ú��ĺ�������Ŀ�Ƿ���ڸ�������Ŀ
    // ��Ϊ���߶��������ظ������ֻҪ��Ŀ��ȶ��߾Ͱ�����ͬ��
    if (GetCoreMainItems(iter->first).size() == items.size()) {
      return iter->first;
    }
  }
  return CoreId::InvalidId();
}

bool SyntaxGenerator::SpreadLookForwardSymbolAndConstructParsingTableEntry(
    CoreId core_id) {
  // ���δִ�бհ�����������ִ�д�������
  if (!CoreClosure(core_id)) [[unlikely]] {
    return false;
  }
  // ִ�бհ���������Կչ�Լ�ﵽ��ÿһ���core��
  // ���Դ���ʱ���迼��ĳ��չ�Լ���ܴﵽ�����Ϊ��Щ�������core��

  // ��Ҫ������ǰ�����ŵ�core��ID�������ID�����ظ�
  std::list<CoreId> core_waiting_spread_ids;
  ParsingTableEntryId parsing_table_entry_id =
      GetCore(core_id).GetParsingTableEntryId();
  // ����ת������������Ա�֮��Ѱ��goto����������
  // ����ת��������ֵ�������ֵ��õ�����
  std::unordered_map<ProductionNodeId, std::list<CoreItem>> goto_table;
  // �ռ�����ת������
  for (const auto& item_and_forward_nodes :
       GetCoreItemsAndForwardNodes(core_id)) {
    auto [production_node_id, production_body_id, point_index] =
        item_and_forward_nodes.first;
    NonTerminalProductionNode& production_node_now =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id));
    ProductionNodeId next_production_node_id =
        production_node_now.GetProductionNodeInBody(production_body_id,
                                                    point_index);
    if (next_production_node_id.IsValid()) [[unlikely]] {
      // ������Լ����������
      goto_table[next_production_node_id].emplace_back(
          production_node_id, production_body_id, PointIndex(point_index + 1));
    }
  }
  // �ҵ�ÿ��ת�������µ��������Ƿ���ڹ�ͬ����������������ת�Ƶ����
  // ���������
  for (const auto& transform_condition_and_transformed_items : goto_table) {
    // ��ȡת�ƺ�����ڵĺ��ĵ�ID
    CoreId core_after_transform_id = GetCoreIdFromCoreItems(
        transform_condition_and_transformed_items.second);
    if (core_after_transform_id.IsValid()) [[unlikely]] {
      // ת�Ƶ�����Ѵ���
      // ת�ƺ󵽴���﷨��������Ŀ
      ParsingTableEntryId parsing_table_entry_id_after_transform =
          GetCore(core_after_transform_id).GetParsingTableEntryId();
      ParsingTableEntry& parsing_table_entry =
          GetParsingTableEntry(parsing_table_entry_id);
      // ����ת��������ת�Ƶ����е��
      switch (GetProductionNode(transform_condition_and_transformed_items.first)
                  .Type()) {
        case ProductionNodeType::kTerminalNode:
        case ProductionNodeType::kOperatorNode:
          parsing_table_entry.SetTerminalNodeActionAndAttachedData(
              transform_condition_and_transformed_items.first,
              ParsingTableEntry::ShiftAttachedData(
                  parsing_table_entry_id_after_transform));
          break;
        case ProductionNodeType::kNonTerminalNode:
          parsing_table_entry.SetNonTerminalNodeTransformId(
              transform_condition_and_transformed_items.first,
              parsing_table_entry_id_after_transform);
          break;
        default:
          assert(false);
          break;
      }
      bool new_forward_node_inserted = false;
      // ���ת��������ת�Ƶ�����Щ�����ǰ������
      for (auto& item : transform_condition_and_transformed_items.second) {
        new_forward_node_inserted |= AddForwardNodes(
            core_after_transform_id, item, GetForwardNodeIds(core_id, item));
      }
      // ����������������������µ���ǰ���ڵ������´�������
      if (new_forward_node_inserted) {
        core_waiting_spread_ids.push_back(core_after_transform_id);
        OutPutInfo(std::format(
            "Core ID:{:} "
            "����������������/����ǰ�����ţ����´����������ǰ������"));
      }
    } else {
      // ��������Щ��ɵĺ��ģ���Ҫ�½�
      CoreId new_core_id = EmplaceCore();
      ParsingTableEntryId parsing_table_entry_id_after_transform =
          GetCore(new_core_id).GetParsingTableEntryId();
      ParsingTableEntry& parsing_table_entry =
          GetParsingTableEntry(parsing_table_entry_id);
      // ����ת��������ת�Ƶ����е��
      switch (GetProductionNode(transform_condition_and_transformed_items.first)
                  .Type()) {
        case ProductionNodeType::kTerminalNode:
        case ProductionNodeType::kOperatorNode:
          parsing_table_entry.SetTerminalNodeActionAndAttachedData(
              transform_condition_and_transformed_items.first,
              ParsingTableEntry::ShiftAttachedData(
                  parsing_table_entry_id_after_transform));
          break;
        case ProductionNodeType::kNonTerminalNode:
          parsing_table_entry.SetNonTerminalNodeTransformId(
              transform_condition_and_transformed_items.first,
              parsing_table_entry_id_after_transform);
          break;
        default:
          assert(false);
          break;
      }
      // �������к�����
      for (auto& item : transform_condition_and_transformed_items.second) {
        auto result = AddMainItemAndForwardNodeIdsToCore(
            new_core_id, item, GetForwardNodeIds(core_id, item));
        assert(result.second);
      }
      core_waiting_spread_ids.push_back(new_core_id);
    }
  }
  for (auto core_waiting_spread_id : core_waiting_spread_ids) {
    // �������ɵ�ÿ�����������ǰ������
    SpreadLookForwardSymbolAndConstructParsingTableEntry(
        core_waiting_spread_id);
  }
  return true;
}

std::array<std::vector<SyntaxGenerator::ProductionNodeId>,
           sizeof(SyntaxGenerator::ProductionNodeType)>
SyntaxGenerator::ClassifyProductionNodes() {
  std::array<std::vector<ProductionNodeId>, sizeof(ProductionNodeType)>
      production_nodes;
  ObjectManager<BaseProductionNode>::Iterator iter = manager_nodes_.Begin();
  while (iter != manager_nodes_.End()) {
    production_nodes[static_cast<size_t>(iter->Type())].push_back(iter->Id());
    ++iter;
  }
  return production_nodes;
}

std::vector<std::vector<SyntaxGenerator::ParsingTableEntryId>>
SyntaxGenerator::ParsingTableEntryClassify(
    const std::vector<ProductionNodeId>& terminal_node_ids,
    const std::vector<ProductionNodeId>& nonterminal_node_ids) {
  // �洢��ͬ�ս�ڵ�ת�Ʊ����Ŀ
  std::vector<std::vector<ParsingTableEntryId>> terminal_classify_result,
      final_classify_result;
  std::vector<ParsingTableEntryId> entry_ids;
  entry_ids.reserve(syntax_parsing_table_.size());
  for (size_t i = 0; i < syntax_parsing_table_.size(); i++) {
    // ������д�������﷨��������ĿID
    entry_ids.push_back(ParsingTableEntryId(i));
  }
  ParsingTableTerminalNodeClassify(terminal_node_ids, 0, entry_ids,
                                   &terminal_classify_result);
  for (auto vec_entry_ids : terminal_classify_result) {
    ParsingTableNonTerminalNodeClassify(nonterminal_node_ids, 0, vec_entry_ids,
                                        &final_classify_result);
  }
  return final_classify_result;
}

inline void SyntaxGenerator::RemapParsingTableEntryId(
    const std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>&
        moved_entry_id_to_new_entry_id) {
  for (auto& entry : syntax_parsing_table_) {
    entry.ResetEntryId(moved_entry_id_to_new_entry_id);
  }
}

void SyntaxGenerator::ParsingTableMergeOptimize() {
  std::array<std::vector<ProductionNodeId>, sizeof(ProductionNodeType)>
      classified_production_node_ids;
  size_t terminal_index =
      static_cast<size_t>(ProductionNodeType::kTerminalNode);
  size_t operator_index =
      static_cast<size_t>(ProductionNodeType::kOperatorNode);
  size_t nonterminal_index =
      static_cast<size_t>(ProductionNodeType::kNonTerminalNode);
  for (auto production_node_id :
       classified_production_node_ids[operator_index]) {
    // �������������ڵ���ӵ��ս�ڵ��������������ս�ڵ��
    classified_production_node_ids[terminal_index].push_back(
        production_node_id);
  }
  std::vector<std::vector<ParsingTableEntryId>> classified_ids =
      ParsingTableEntryClassify(
          classified_production_node_ids[terminal_index],
          classified_production_node_ids[nonterminal_index]);
  // �洢���о���Ŀ������Ŀ��ӳ��
  std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>
      old_entry_id_to_new_entry_id;
  for (auto& entry_ids : classified_ids) {
    // ��ӱ��ϲ��ľ���Ŀ����ͬ��Ŀ��ӳ��
    // �ظ�����Ŀֻ�������������еĵ�һ���������ȫ��ӳ�䵽����
    ParsingTableEntryId new_id = entry_ids.front();
    for (auto old_id : entry_ids) {
      assert(old_entry_id_to_new_entry_id.find(old_id) ==
             old_entry_id_to_new_entry_id.end());
      old_entry_id_to_new_entry_id[old_id] = new_id;
    }
  }
  // �洢�ƶ������Ŀ������ĿID
  std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>
      moved_entry_to_new_entry_id;
  // ��ʼ�ϲ�
  assert(syntax_parsing_table_.size() > 0);
  // ��һ��Ҫ�������Ŀ
  ParsingTableEntryId next_process_entry_index(0);
  // ��һ������λ��
  ParsingTableEntryId next_insert_position_index(0);
  // ���ƿ��ŷ�����㷨
  // Ѱ��û�б��ϲ�����Ŀ������������vector���±�һ��
  while (next_process_entry_index < syntax_parsing_table_.size()) {
    if (old_entry_id_to_new_entry_id.find(next_process_entry_index) ==
        old_entry_id_to_new_entry_id.end()) {
      // ����Ŀ����
      if (next_insert_position_index != next_process_entry_index) {
        // ��Ҫ�ƶ�����λ�ñ���vector����
        syntax_parsing_table_[next_insert_position_index] =
            std::move(syntax_parsing_table_[next_process_entry_index]);
      }
      // ��ӳ�䱣����Ŀ����λ��
      moved_entry_to_new_entry_id[next_process_entry_index] =
          next_insert_position_index;
      ++next_insert_position_index;
    }
    ++next_process_entry_index;
  }
  for (auto& item : old_entry_id_to_new_entry_id) {
    // ��ӳ���ѱ��ϲ���Ŀ��������Ψһ��Ŀ
    // ����ӳ�䱣����Ŀ����λ�õĲ���һ�𹹳���������Ŀ��ӳ���
    // ���о���Ŀ��ӵ���Լ���Ӧ������Ŀ�����ϲ�����Ŀӳ�䵽
    assert(moved_entry_to_new_entry_id.find(item.second) !=
           moved_entry_to_new_entry_id.end());
    // item_and_forward_node_ids.first�Ǿ�entry��ID��item.second�Ǳ���������entry��ID
    moved_entry_to_new_entry_id[item.first] =
        moved_entry_to_new_entry_id[item.second];
  }
  // ����ÿһ����Ŀ����������Ŀ��ӳ��
  // �ͷŶ���ռ�
  old_entry_id_to_new_entry_id.clear();
  syntax_parsing_table_.resize(next_insert_position_index);
  // �����о�ID����Ϊ��ID
  RemapParsingTableEntryId(moved_entry_to_new_entry_id);
}

std::string SyntaxGenerator::FormatSingleProductionBody(
    ProductionNodeId nonterminal_node_id,
    ProductionBodyId production_body_id) const {
  std::string format_result;
  const NonTerminalProductionNode& production_node_now =
      static_cast<const NonTerminalProductionNode&>(
          GetProductionNode(nonterminal_node_id));
  assert(production_node_now.Type() == ProductionNodeType::kNonTerminalNode);
  format_result += std::format(
      "{:} ->",
      GetNodeSymbolStringFromId(production_node_now.GetNodeSymbolId()));
  for (auto id :
       production_node_now.GetBody(production_body_id).production_body) {
    std::cout << std::format(
        " {:}",
        GetNodeSymbolStringFromId(GetProductionNode(id).GetNodeSymbolId()));
  }
  return format_result;
}

std::string SyntaxGenerator::FormatProductionBodys(
    ProductionNodeId nonterminal_node_id) {
  std::string format_result;
  const NonTerminalProductionNode& production_node_now =
      static_cast<const NonTerminalProductionNode&>(
          GetProductionNode(nonterminal_node_id));
  assert(production_node_now.Type() == ProductionNodeType::kNonTerminalNode);
  for (auto& body_id : production_node_now.GetAllBodyIds()) {
    format_result += FormatSingleProductionBody(nonterminal_node_id, body_id);
    format_result += '\n';
  }
  return format_result;
}

std::string SyntaxGenerator::FormatItem(const CoreItem& core_item) const {
  std::string format_result;
  auto& [production_node_id, production_body_id, point_index] = core_item;
  const NonTerminalProductionNode& production_node_now =
      static_cast<const NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  format_result += std::format(
      "{:} ->",
      GetNodeSymbolStringFromId(production_node_now.GetNodeSymbolId()));
  const auto& production_node_waiting_spread_body =
      production_node_now.GetBody(production_body_id).production_body;
  for (size_t i = 0; i < point_index; ++i) {
    format_result += std::format(
        " {:}", GetNodeSymbolStringFromId(
                    GetProductionNode(production_node_waiting_spread_body[i])
                        .GetNodeSymbolId()));
  }
  format_result += std::format(" ��");
  for (size_t i = point_index; i < production_node_waiting_spread_body.size();
       i++) {
    format_result += std::format(
        " {:}", GetNodeSymbolStringFromId(
                    GetProductionNode(production_node_waiting_spread_body[i])
                        .GetNodeSymbolId()));
  }
  return format_result;
}

std::string SyntaxGenerator::FormatLookForwardSymbols(
    const std::unordered_set<ProductionNodeId>& look_forward_node_ids) const {
  if (look_forward_node_ids.empty()) [[unlikely]] {
    return std::string();
  }
  std::string format_result;
  for (const auto& node_id : look_forward_node_ids) {
    format_result +=
        GetNodeSymbolStringFromId(GetProductionNode(node_id).GetNodeSymbolId());
    format_result += ' ';
  }
  // ����β���ո�
  format_result.pop_back();
  return format_result;
}

std::string SyntaxGenerator::FormatCoreItems(CoreId core_id) const {
  std::string format_result;
  const Core& core = GetCore(core_id);
  for (const auto& item_and_forward_nodes : core.GetItemsAndForwardNodeIds()) {
    format_result += FormatItem(item_and_forward_nodes.first);
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

SyntaxGenerator::SyntaxGenerator() {
  SyntaxGeneratorInit();
  ConfigConstruct();
  CheckUndefinedProductionRemained();
  dfa_generator_.DfaReconstrcut();
  ParsingTableConstruct();
  // ��������
  SaveConfig();
}

void SyntaxGenerator::ParsingTableConstruct() {
  // �������뵽�ļ�βʱ���صĽڵ�
  ProductionNodeId end_production_node_id = AddEndNode();
  // �������������ļ�β��ʱ���ص�����
  frontend::generator::dfa_generator::DfaGenerator::WordAttachedData
      end_of_file_saved_data;
  end_of_file_saved_data.production_node_id = end_production_node_id;
  GetProductionNode(end_production_node_id)
      .SetSymbolId(AddNodeSymbol("$").first);
  dfa_generator_.SetEndOfFileSavedData(std::move(end_of_file_saved_data));
  // ���ɳ�ʼ���������������ʽ����
  CoreId root_core_id = EmplaceCore();
  Core& root_core = GetCore(root_core_id);
  root_core.AddMainItemAndForwardNodeIds(
      CoreItem(GetRootProductionNodeId(), ProductionBodyId(0), PointIndex(0)),
      std::initializer_list<ProductionNodeId>{end_production_node_id});
  SetRootParsingTableEntryId(root_core.GetParsingTableEntryId());
  // ������ǰ������ͬʱ�����﷨������
  SpreadLookForwardSymbolAndConstructParsingTableEntry(root_core_id);
  // �ϲ���Ч�ѹ���﷨������
  ParsingTableMergeOptimize();
}

void SyntaxGenerator::SyntaxGeneratorInit() {
  NodeNumInit();
  undefined_productions_.clear();
  manager_nodes_.ObjectManagerInit();
  manager_node_symbol_.StructManagerInit();
  manager_terminal_body_symbol_.StructManagerInit();
  node_symbol_id_to_node_id_.clear();
  production_body_symbol_id_to_node_id_.clear();
  cores_.ObjectManagerInit();
  parsing_table_entry_id_to_core_id_.clear();
  root_production_node_id_ = ProductionNodeId::InvalidId();
  root_parsing_table_entry_id_ = ParsingTableEntryId::InvalidId();
  dfa_generator_.DfaInit();
  syntax_parsing_table_.clear();
  manager_process_function_class_.ObjectManagerInit();
}

void SyntaxGenerator::AddUnableContinueNonTerminalNode(
    const std::string& undefined_symbol, std::string&& node_symbol,
    std::vector<std::string>&& subnode_symbols,
    ProcessFunctionClassId class_id) {
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
    ProductionNodeId node_id =
        AddNonTerminalNode(std::move(node_could_continue_to_add_symbol),
                           std::move(node_body), process_function_class_id_);
    if (node_id.IsValid()) {
      // �ɹ���ӷ��ս�ڵ㣬ɾ��������¼
      iter_begin = undefined_productions_.erase(iter_begin);
    } else {
      ++iter_begin;
    }
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

void SyntaxGenerator::AddKeyWord(const std::string& key_word) {
  // �ؼ������ȼ�Ĭ��Ϊ2
  // �Զ�����ͬ���ս�ڵ�
  AddTerminalNode(
      key_word, key_word,
      frontend::generator::dfa_generator::DfaGenerator::WordPriority(2));
}

SyntaxGenerator::ProductionNodeId
SyntaxGenerator::GetProductionNodeIdFromBodySymbolId(SymbolId body_symbol_id) {
  auto iter = production_body_symbol_id_to_node_id_.find(body_symbol_id);
  if (iter != production_body_symbol_id_to_node_id_.end()) {
    return iter->second;
  } else {
    return ProductionNodeId::InvalidId();
  }
}

SyntaxGenerator::ProductionNodeId
SyntaxGenerator::GetProductionNodeIdFromNodeSymbol(
    const std::string& body_symbol) {
  SymbolId node_symbol_id = GetNodeSymbolId(body_symbol);
  if (node_symbol_id.IsValid()) {
    return GetProductionNodeIdFromNodeSymbolId(node_symbol_id);
  } else {
    return ProductionNodeId::InvalidId();
  }
}

SyntaxGenerator::ProductionNodeId
SyntaxGenerator::GetProductionNodeIdFromBodySymbol(
    const std::string& body_symbol) {
  SymbolId body_symbol_id = GetBodySymbolId(body_symbol);
  if (body_symbol_id.IsValid()) {
    return GetProductionNodeIdFromBodySymbolId(body_symbol_id);
  } else {
    return ProductionNodeId::InvalidId();
  }
}

inline SyntaxGenerator::ProductionNodeId
SyntaxGenerator::TerminalProductionNode::GetProductionNodeInBody(
    ProductionBodyId production_body_id, PointIndex point_index) const {
  assert(production_body_id == 0);
  if (point_index == 0) {
    return Id();
  } else {
    return ProductionNodeId::InvalidId();
  }
}

std::vector<SyntaxGenerator::ProductionBodyId>
SyntaxGenerator::NonTerminalProductionNode::GetAllBodyIds() const {
  std::vector<ProductionBodyId> production_body_ids;
  for (size_t i = 0; i < nonterminal_bodys_.size(); i++) {
    production_body_ids.push_back(ProductionBodyId(i));
  }
  return production_body_ids;
}

// ����������ĺ���ID
// Ҫ�������еĺ���ID�ظ�

inline void
SyntaxGenerator::NonTerminalProductionNode::AddCoreItemBelongToCoreId(
    ProductionBodyId body_id, PointIndex point_index, CoreId core_id) {
#ifdef _DEBUG
  const auto& core_ids_already_in =
      nonterminal_bodys_[body_id].cores_items_in_[point_index];
  for (auto core_id_already_in : core_ids_already_in) {
    assert(core_id != core_id_already_in);
  }
#endif  // _DEBUG
  nonterminal_bodys_[body_id].cores_items_in_[point_index].push_back(core_id);
}

inline SyntaxGenerator::ProductionNodeId
SyntaxGenerator::NonTerminalProductionNode::GetProductionNodeInBody(
    ProductionBodyId production_body_id, PointIndex point_index) const {
  assert(production_body_id < nonterminal_bodys_.size());
  if (point_index <
      nonterminal_bodys_[production_body_id].production_body.size()) {
    return nonterminal_bodys_[production_body_id].production_body[point_index];
  } else {
    return ProductionNodeId::InvalidId();
  }
}

inline SyntaxGenerator::ParsingTableEntry&
SyntaxGenerator::ParsingTableEntry::operator=(
    ParsingTableEntry&& parsing_table_entry) {
  action_and_attached_data_ =
      std::move(parsing_table_entry.action_and_attached_data_);
  nonterminal_node_transform_table_ =
      std::move(parsing_table_entry.nonterminal_node_transform_table_);
  return *this;
}

void SyntaxGenerator::ParsingTableEntry::ResetEntryId(
    const std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>&
        old_entry_id_to_new_entry_id) {
  //�����ս�ڵ�Ķ���
  for (auto& action_and_attached_data : GetAllActionAndAttachedData()) {
    // ��ȡԭ��ĿID������
    ShiftAttachedData& shift_attached_data =
        action_and_attached_data.second->GetShiftAttachedData();
    // ����Ϊ�µ���ĿID
    shift_attached_data.SetNextParsingTableEntryId(
        old_entry_id_to_new_entry_id
            .find(shift_attached_data.GetNextParsingTableEntryId())
            ->second);
  }
  //������ս�ڵ��ת��
  for (auto& target : GetAllNonTerminalNodeTransformTarget()) {
    ParsingTableEntryId old_entry_id = target.second;
    ParsingTableEntryId new_entry_id =
        old_entry_id_to_new_entry_id.find(old_entry_id)->second;
    SetNonTerminalNodeTransformId(target.first, new_entry_id);
  }
}

SyntaxGenerator::BaseProductionNode&
SyntaxGenerator::BaseProductionNode::operator=(
    BaseProductionNode&& base_production_node) {
  base_type_ = std::move(base_production_node.base_type_);
  base_id_ = std::move(base_production_node.base_id_);
  base_symbol_id_ = std::move(base_production_node.base_symbol_id_);
  return *this;
}

SyntaxGenerator::OperatorProductionNode&
SyntaxGenerator::OperatorProductionNode::operator=(
    OperatorProductionNode&& operator_production_node) {
  BaseProductionNode::operator=(std::move(operator_production_node));
  operator_associatity_type_ =
      std::move(operator_production_node.operator_associatity_type_);
  operator_priority_level_ =
      std::move(operator_production_node.operator_priority_level_);
  return *this;
}

SyntaxGenerator::Core& SyntaxGenerator::Core::operator=(Core&& core) {
  core_closure_available_ = std::move(core.core_closure_available_);
  core_id_ = std::move(core.core_id_);
  parsing_table_entry_id_ = std::move(core.parsing_table_entry_id_);
  item_and_forward_node_ids_ = std::move(core.item_and_forward_node_ids_);
  return *this;
}

// ��Ӻ�����
// Ҫ��ú�����δ��ӹ�

inline void SyntaxGenerator::Core::SetMainItem(
    std::map<CoreItem, std::unordered_set<ProductionNodeId>>::const_iterator&
        item_iter) {
#ifdef _DEBUG
  // �������ظ��������еĺ�����Ϊ������
  for (const auto& main_item_already_in : GetMainItems()) {
    assert(item_iter->first != main_item_already_in->first);
  }
#endif  // _DEBUG
  main_items_.emplace_back(item_iter);
  SetClosureNotAvailable();
}

bool SyntaxGenerator::ParsingTableEntry::ShiftAttachedData::operator==(
    const ActionAndAttachedDataInterface& shift_attached_data) const {
  return ActionAndAttachedDataInterface::operator==(shift_attached_data) &&
         next_entry_id_ ==
             static_cast<const ShiftAttachedData&>(shift_attached_data)
                 .next_entry_id_;
}

bool SyntaxGenerator::ParsingTableEntry::ReductAttachedData::operator==(
    const ActionAndAttachedDataInterface& reduct_attached_data) const {
  if (ActionAndAttachedDataInterface::operator==(reduct_attached_data))
      [[likely]] {
    const ReductAttachedData& real_type_reduct_attached_data =
        static_cast<const ReductAttachedData&>(reduct_attached_data);
    return reducted_nonterminal_node_id_ ==
               real_type_reduct_attached_data.reducted_nonterminal_node_id_ &&
           process_function_class_id_ ==
               real_type_reduct_attached_data.process_function_class_id_ &&
           production_body_ == real_type_reduct_attached_data.production_body_;
  } else {
    return false;
  }
}

bool SyntaxGenerator::ParsingTableEntry::ShiftReductAttachedData::operator==(
    const ActionAndAttachedDataInterface& shift_reduct_attached_data) const {
  return ActionAndAttachedDataInterface::operator==(
             shift_reduct_attached_data) &&
         shift_attached_data_ == static_cast<const ShiftReductAttachedData&>(
                                     shift_reduct_attached_data)
                                     .shift_attached_data_ &&
         reduct_attached_data_ == static_cast<const ShiftReductAttachedData&>(
                                      shift_reduct_attached_data)
                                      .reduct_attached_data_;
}

inline const SyntaxGenerator::ParsingTableEntry::ShiftAttachedData&
SyntaxGenerator::ParsingTableEntry::ActionAndAttachedDataInterface::
    GetShiftAttachedData() const {
  assert(false);
  // ��ֹ����
  return reinterpret_cast<const ShiftAttachedData&>(*this);
}

inline const SyntaxGenerator::ParsingTableEntry::ReductAttachedData&
SyntaxGenerator::ParsingTableEntry::ActionAndAttachedDataInterface::
    GetReductAttachedData() const {
  assert(false);
  // ��ֹ����
  return reinterpret_cast<const ReductAttachedData&>(*this);
}

inline const SyntaxGenerator::ParsingTableEntry::ShiftReductAttachedData&
SyntaxGenerator::ParsingTableEntry::ActionAndAttachedDataInterface::
    GetShiftReductAttachedData() const {
  assert(false);
  // ��ֹ����
  return reinterpret_cast<const ShiftReductAttachedData&>(*this);
}

inline bool SyntaxGenerator::ActionAndAttachedDataPointerEqualTo::operator()(
    const ParsingTableEntry::ActionAndAttachedDataInterface* const& lhs,
    const ParsingTableEntry::ActionAndAttachedDataInterface* const& rhs) const {
  if (lhs == nullptr) {
    if (rhs == nullptr) {
      return true;
    } else {
      return false;
    }
  } else if (rhs == nullptr) {
    return false;
  } else {
    return lhs->operator==(*rhs);
  }
}

}  // namespace frontend::generator::syntax_generator