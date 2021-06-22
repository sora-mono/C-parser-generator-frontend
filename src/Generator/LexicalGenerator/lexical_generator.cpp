#include "lexical_generator.h"

#include <fstream>
#include <queue>

#include "Generator/LexicalGenerator/lexical_generator.h"
namespace frontend::generator::lexicalgenerator {
inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddTerminalNode(
    const std::string& node_symbol, const std::string& body_symbol,
    frontend::generator::dfa_generator::DfaGenerator::TailNodePriority
        node_priority) {
  // �ս�ڵ�Ӧ�����ʽ��󶨣�����ʽ��ֻ��һ������
  auto [node_symbol_id, node_symbol_inserted] = AddNodeSymbol(node_symbol);
  auto [body_symbol_id, body_symbol_inserted] = AddBodySymbol(body_symbol);
  ProductionNodeId production_node_id;
  if (body_symbol_inserted) {
    ProductionNodeId old_node_symbol_id =
        GetProductionNodeIdFromNodeSymbolId(node_symbol_id);
    if (old_node_symbol_id.IsValid()) {
      // ����ʽ��󶨵Ľڵ��Ǽ�����ӵ�ȫ�½ڵ㣬����ʽ���󶨵Ľڵ�һ��������ͬ
      printf("�ض����ս����ʽ����%s\n", node_symbol.c_str());
      return ProductionNodeId::InvalidId();
    }
    // ��Ҫ���һ���µ��ս�ڵ�
    production_node_id = SubAddTerminalNode(node_symbol_id, body_symbol_id);
    frontend::generator::dfa_generator::DfaGenerator::SavedData saved_data_;
    saved_data_.production_node_id = production_node_id;
    saved_data_.node_type = ProductionNodeType::kTerminalNode;
    saved_data_.process_function_class_id_ =
        ProcessFunctionClassId::InvalidId();
    // ��DFA������ע��ؼ���
    dfa_generator_.AddRegexpression(body_symbol, saved_data_, node_priority);
  } else {
    // ���ս�ڵ�������Ѵ��ڣ���Ӧ�ظ����
    production_node_id = GetProductionNodeIdFromBodySymbolId(body_symbol_id);
    assert(production_node_id.IsValid());
    if (node_symbol_inserted) {
      // ����Ӳ���ʽ�����ڵ��ӳ��
      SetNodeSymbolIdToProductionNodeIdMapping(body_symbol_id,
                                               production_node_id);
    } else if (production_node_id != GetProductionNodeIdFromNodeSymbolId(
                                         node_symbol_id)) [[unlikely]] {
      // �ڵ����󶨵Ľڵ������ʽ��󶨵Ľڵ㲻ͬ�������ض������
      fprintf(stderr, "�ض����ս����ʽ����%s\n", node_symbol.c_str());
    }
  }
  // �ж�����ӵ��ս�ڵ��Ƿ�Ϊĳ��δ�������ʽ
  CheckNonTerminalNodeCanContinue(node_symbol);
  return production_node_id;
}

inline LexicalGenerator::ProductionNodeId LexicalGenerator::SubAddTerminalNode(
    SymbolId node_symbol_id, SymbolId body_symbol_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<TerminalProductionNode>(node_symbol_id,
                                                           body_symbol_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(body_symbol_id, node_id);
  return node_id;
}

#ifdef USE_AMBIGUOUS_GRAMMAR
inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddOperatorNode(
    const std::string& operator_symbol, AssociatityType associatity_type,
    OperatorPriority priority_level, ProcessFunctionClassId class_id) {
  auto [operator_symbol_id, inserted] = AddBodySymbol(operator_symbol);
  assert(operator_symbol_id.IsValid());
  if (!inserted) [[unlikely]] {
    fprintf(stderr, "�ض����������%s\n", operator_symbol.c_str());
    return ProductionNodeId::InvalidId();
  }
  // ���������ʽ�����������ͬ
  SymbolId operator_node_symbol_id;
  std::pair(operator_node_symbol_id, inserted) = AddNodeSymbol(operator_symbol);
  if (!inserted) [[unlikely]] {
    fprintf(stderr, "�������%s�Ĳ���ʽ���ѱ�ռ��\n", operator_symbol.c_str());
    return ProductionNodeId::InvalidId();
  }
  assert(class_id.IsValid());
  ProductionNodeId operator_node_id = SubAddOperatorNode(
      operator_symbol_id, associatity_type, priority_level, class_id);
  assert(operator_node_id.IsValid());
  frontend::generator::dfa_generator::DfaGenerator::SavedData saved_data_;
  saved_data_.production_node_id = operator_node_id;
  saved_data_.node_type = ProductionNodeType::kOperatorNode;
  saved_data_.process_function_class_id_ = class_id;
  saved_data_.associate_type = associatity_type;
  saved_data_.priority = priority_level;
  // ��DFA������ע��ؼ���
  dfa_generator_.AddKeyword(
      operator_symbol, saved_data_,
      frontend::generator::dfa_generator::DfaGenerator::TailNodePriority(
          priority_level));
  return operator_node_id;
}

inline LexicalGenerator::ProductionNodeId LexicalGenerator::SubAddOperatorNode(
    SymbolId node_symbol_id, AssociatityType associatity_type,
    OperatorPriority priority_level, ProcessFunctionClassId class_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<OperatorProductionNode>(
          node_symbol_id, node_symbol_id, associatity_type, priority_level,
          class_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  SetBodySymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}
#endif  // USE_AMBIGUOUS_GRAMMAR

inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddNonTerminalNode(
    const std::string& node_symbol,
    const std::vector<std::pair<std::string, bool>>& subnode_symbols,
    ProcessFunctionClassId class_id) {
  assert(!node_symbol.empty() && !subnode_symbols.empty() &&
         class_id.IsValid());
  std::vector<ProductionNodeId> node_ids;
  // ���ս�ڵ���ڵ���һһ��Ӧ
  ProductionNodeId production_node_id =
      GetProductionNodeIdFromNodeSymbol(node_symbol);
  if (!production_node_id.IsValid()) {
    // ���ս�ڵ���δע��
    SymbolId symbol_id = AddNodeSymbol(node_symbol).first;
    assert(symbol_id.IsValid());
    production_node_id = SubAddNonTerminalNode(symbol_id);
    // �����ӵĽڵ��Ƿ�ǰ������
    CheckNonTerminalNodeCanContinue(node_symbol);
  }
  for (auto& subnode_symbol : subnode_symbols) {
    // ������ʽ������в���ʽ��ת��Ϊ�ڵ�ID
    ProductionNodeId subproduction_node_id;
    if (subnode_symbol.second) [[unlikely]] {
      // �����ս�ڵ�
      SymbolId body_symbol_id = GetBodySymbolId(subnode_symbol.first);
      if (!body_symbol_id.IsValid()) {
        // ����ʽ�����δע�ᣬ�Զ����������ʽ�����ͬ���Ĳ���ʽ��
        // ʹ�����ȼ�0��Ϊ�ؼ��ֵ���������һ������
        subproduction_node_id =
            AddTerminalNode(subnode_symbol.first, subnode_symbol.first,
                            frontend::generator::dfa_generator ::nfa_generator::
                                NfaGenerator::TailNodePriority(0));
        assert(subproduction_node_id.IsValid());
      } else {
        // ����ʽ�������ע��
        subproduction_node_id =
            GetProductionNodeIdFromBodySymbolId(body_symbol_id);
        assert(subproduction_node_id.IsValid());
      }
    } else [[likely]] {
      if (subnode_symbol.first == "@") {
        // �սڵ��ǣ����ø��ս�ڵ������չ�Լ���
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id))
            .SetProductionCouldBeEmpty();
      } else {
        subproduction_node_id =
            GetProductionNodeIdFromNodeSymbol(subnode_symbol.first);
      }
      // ����ʽ��
      if (!subproduction_node_id.IsValid()) [[unlikely]] {
        // ����ʽ�ڵ�δ����
        // ��Ӵ������¼
        AddUnableContinueNonTerminalNode(subnode_symbol.first, node_symbol,
                                         subnode_symbols, class_id);
      }
    }
    node_ids.push_back(subproduction_node_id);
  }
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  ProductionBodyId body_id = production_node.AddBody(std::move(node_ids));
  production_node.SetBodyProcessFunctionClassId(body_id, class_id);
  return production_node_id;
}

inline LexicalGenerator::ProductionNodeId
LexicalGenerator::SubAddNonTerminalNode(SymbolId node_symbol_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<NonTerminalProductionNode>(node_symbol_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  SetNodeSymbolIdToProductionNodeIdMapping(node_symbol_id, node_id);
  return node_id;
}

// inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddEndNode(
//    SymbolId symbol_id) {
//  ProductionNodeId node_id = manager_nodes_.EmplaceObject<EndNode>(symbol_id);
//  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
//  return node_id;
//}

LexicalGenerator::ProductionNodeId
LexicalGenerator::GetProductionNodeIdFromNodeSymbol(SymbolId symbol_id) {
  assert(symbol_id.IsValid());
  auto iter = node_symbol_id_to_node_id_.find(symbol_id);
  assert(iter != node_symbol_id_to_node_id_.end());
  return iter->second;
}

template <class ForwardNodeIdContainer>
inline std::pair<
    std::map<LexicalGenerator::CoreItem,
             std::unordered_set<LexicalGenerator::ProductionNodeId>>::iterator,
    bool>
LexicalGenerator::AddItemAndForwardNodeIdsToCore(
    CoreId core_id, const CoreItem& core_item,
    ForwardNodeIdContainer&& forward_node_ids) {
  assert(core_id.IsValid());
  return GetCore(core_id).AddItemAndForwardNodeIds(
      core_item, std::forward<ForwardNodeIdContainer>(forward_node_ids));
}

std::unordered_set<LexicalGenerator::ProductionNodeId>
LexicalGenerator::GetNonTerminalNodeFirstNodes(
    ProductionNodeId production_node_id,
    std::unordered_set<ProductionNodeId>&& processed_nodes) {
  if (!production_node_id.IsValid()) {
    return std::unordered_set<ProductionNodeId>();
  }
  NonTerminalProductionNode& production_node =
      static_cast<NonTerminalProductionNode&>(
          GetProductionNode(production_node_id));
  assert(production_node.Type() == ProductionNodeType::kNonTerminalNode);
  std::unordered_set<ProductionNodeId> return_set;
  for (auto& body : production_node.GetAllBody()) {
    // ���ڴ���Ĳ���ʽ�ڵ��ID
    ProductionNodeId node_id = body.front();
    BaseProductionNode& node = GetProductionNode(node_id);
    if (node.Type() == ProductionNodeType::kNonTerminalNode) {
      return_set.merge(
          GetNonTerminalNodeFirstNodes(node_id, std::move(processed_nodes)));
    } else {
      return_set.insert(node_id);
    }
  }
  processed_nodes.insert(production_node_id);
  return return_set;
}

std::unordered_set<LexicalGenerator::ProductionNodeId> LexicalGenerator::First(
    ProductionNodeId production_node_id, ProductionBodyId production_body_id,
    PointIndex point_index,
    const InsideForwardNodesContainerType& next_node_ids) {
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
        node_id = GetProductionNodeIdInBody(production_node_id,
                                            production_body_id, point_index);
        // �ϲ����ս�ڵ��������ǰ������
        return_set.merge(GetNonTerminalNodeFirstNodes(node_id));
        if (static_cast<NonTerminalProductionNode&>(
                GetProductionNode(production_node_id))
                .CouldBeEmpty()) {
          // �÷��ս�ڵ���Կչ�Լ
          return_set.merge(First(production_node_id, production_body_id,
                                 ++point_index, next_node_ids));
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

void LexicalGenerator::CoreClosure(CoreId core_id) {
  Core& core = GetCore(core_id);
  if (core.IsClosureAvailable()) {
    // �հ���Ч����������
    return;
  }
  std::queue<
      const std::pair<const CoreItem, std::unordered_set<ProductionNodeId>>*>
      items_waiting_process;
  for (const auto& item_and_forward_node_ids :
       GetCoreItemsAndForwardNodes(core_id)) {
    // ����е�ǰ���е���ѹ����������
    items_waiting_process.push(&item_and_forward_node_ids);
  }
  while (!items_waiting_process.empty()) {
    auto item_now = items_waiting_process.front();
    items_waiting_process.pop();
    const auto [production_node_id, production_body_id, point_index] =
        item_now->first;
    ProductionNodeId next_production_node_id = GetProductionNodeIdInBody(
        production_node_id, production_body_id, point_index);
    if (!next_production_node_id.IsValid()) {
      // �޺�̽ڵ㣬���ù�Լ��Ŀ
      ParsingTableEntry& parsing_table_entry =
          GetParsingTableEntry(core.GetParsingTableEntryId());
      // ��װ��Լʹ�õ�����
      ParsingTableEntry::ReductAttachedData reduct_attached_data;
      reduct_attached_data.process_function_class_id_ =
          GetProcessFunctionClassId(production_node_id, production_body_id);
      reduct_attached_data.production_body_ =
          GetProductionBody(production_node_id, production_body_id);
      reduct_attached_data.reducted_nonterminal_node_id_ = production_node_id;
      for (auto node_id : item_now->second) {
        // ��ÿ����ǰ���������ù�Լ����
        parsing_table_entry.SetTerminalNodeActionAndAttachedData(
            node_id, ActionType::kReduct, reduct_attached_data);
      }
      continue;
    }
    NonTerminalProductionNode& production_node =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(next_production_node_id));
    if (production_node.Type() != ProductionNodeType::kNonTerminalNode) {
      // ���Ƿ��ս�ڵ㣬�޷�չ��
      continue;
    }
    // չ�����ս�ڵ㣬��Ϊ�䴴����ǰ�����ż�
    InsideForwardNodesContainerType forward_node_ids = First(
        production_node_id, production_body_id, PointIndex(point_index + 1));
    NonTerminalProductionNode& production_node_waiting_spread =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(next_production_node_id));
    assert(production_node_waiting_spread.Type() ==
           ProductionNodeType::kNonTerminalNode);
    for (auto body_id : production_node_waiting_spread.GetAllBodyIds()) {
      // ���÷��ս�ڵ��е�ÿ������ʽ����뵽core�У����������
      auto [iter, inserted] = AddItemAndForwardNodeIdsToCore(
          core_id, CoreItem(next_production_node_id, body_id, PointIndex(0)),
          forward_node_ids);
      if (inserted) {
        // ��������µ�������ӵ������еȴ�����
        items_waiting_process.push(&*iter);
      }
    }
    if (production_node.CouldBeEmpty()) {
      // �÷��ս�ڵ���Կչ�Լ
      // �������ӿչ�Լ������ǰ�����Ÿ���ԭ���������ǰ�����ż�
      auto [iter, inserted] = AddItemAndForwardNodeIdsToCore(
          core_id,
          CoreItem(production_node_id, production_body_id,
                   PointIndex(point_index + 1)),
          GetForwardNodeIds(core_id, item_now->first));
      if (inserted) {
        // ��������µ�������ӵ������еȴ�����
        items_waiting_process.push(&*iter);
      }
    }
  }
  SetCoreClosureAvailable(core_id);
}

void LexicalGenerator::SpreadLookForwardSymbol(CoreId core_id) {
  CoreClosure(core_id);
  // ִ�бհ���������Կչ�Լ�ﵽ��ÿһ���core��
  // ���Դ���ʱ���迼��ĳ��չ�Լ���ܴﵽ�����Ϊ��Щ�������core��

  // �´�����core��ID
  std::vector<CoreId> new_core_ids;
  for (auto& item : GetCoreItemsAndForwardNodes(core_id)) {
    auto [production_node_id, production_body_id, point_index] = item.first;
    ProductionNodeId next_production_node_id = GetProductionNodeIdInBody(
        production_node_id, production_body_id, point_index);
    if (!next_production_node_id.IsValid()) {
      // û����һ���ڵ�
      continue;
    }
    ParsingTableEntry& parsing_table_entry =
        GetParsingTableEntry(GetCore(core_id).GetParsingTableEntryId());
    BaseProductionNode& next_production_node =
        GetProductionNode(next_production_node_id);
    switch (next_production_node.Type()) {
      case ProductionNodeType::kTerminalNode:
      case ProductionNodeType::kOperatorNode: {
        const ParsingTableEntry::ActionAndAttachedData* action_and_target =
            parsing_table_entry.AtTerminalNode(next_production_node_id);
        // ����ڵ�󵽴�ĺ���ID
        CoreId next_core_id;
        if (action_and_target == nullptr) {
          // �����ڸ�ת�������µ���ķ�������Ŀ����Ҫ�½�core
          next_core_id = EmplaceCore();
          // ��¼����ӵĺ���ID
          new_core_ids.push_back(next_core_id);
        } else {
          // ͨ��ת�Ƶ����﷨��������ĿID�����Ӧ�ĺ���ID
          next_core_id = GetCoreIdFromParsingTableEntryId(
              action_and_target->GetShiftAttachedData().next_entry_id_);
        }
        Core& new_core = GetCore(next_core_id);
        // ������ڵ�������ӵ�������
        // ��ǰ�����ż�������ڵ�ǰ����ǰ�����ż���ͬ
        new_core.AddItemAndForwardNodeIds(
            CoreItem(production_node_id, production_body_id,
                     PointIndex(point_index + 1)),
            GetForwardNodeIds(core_id, item.first));
        // �����﷨�������ڸ��ս�ڵ��µ�ת������
        parsing_table_entry.SetTerminalNodeActionAndAttachedData(
            next_production_node_id, ActionType::kShift,
            ParsingTableEntry::ShiftAttachedData(
                new_core.GetParsingTableEntryId()));
      } break;
      case ProductionNodeType::kNonTerminalNode: {
        ParsingTableEntryId next_parsing_table_entry_id =
            parsing_table_entry.AtNonTerminalNode(next_production_node_id);
        CoreId next_core_id;
        if (!next_parsing_table_entry_id.IsValid()) {
          // �����ڸ�ת�������µ�����﷨��������Ŀ����Ҫ�½�core
          next_core_id = EmplaceCore();
          new_core_ids.push_back(next_core_id);
        } else {
          next_core_id =
              GetCoreIdFromParsingTableEntryId(next_parsing_table_entry_id);
        }
        Core& new_core = GetCore(next_core_id);
        // ��������ǰ�����ż�����ǰ�����ż�������ڵ�ǰ����ǰ�����ż���ͬ
        new_core.AddItemAndForwardNodeIds(
            CoreItem(production_node_id, production_body_id,
                     PointIndex(point_index + 1)),
            GetForwardNodeIds(core_id, item.first));
        // �����﷨�������ڸ÷��ս�ڵ��µ�ת������
        parsing_table_entry.SetNonTerminalNodeTransformId(
            next_production_node_id, new_core.GetParsingTableEntryId());
      } break;
      default:
        break;
    }
  }
  for (auto core_id : new_core_ids) {
    // �������ɵ�ÿ�����������ǰ������
    SpreadLookForwardSymbol(core_id);
  }
}

std::array<std::vector<LexicalGenerator::ProductionNodeId>,
           sizeof(LexicalGenerator::ProductionNodeType)>
LexicalGenerator::ClassifyProductionNodes() {
  std::array<std::vector<ProductionNodeId>, sizeof(ProductionNodeType)>
      production_nodes;
  ObjectManager<BaseProductionNode>::Iterator iter = manager_nodes_.Begin();
  while (iter != manager_nodes_.End()) {
    production_nodes[static_cast<size_t>(iter->Type())].push_back(iter->Id());
    ++iter;
  }
  return production_nodes;
}

std::vector<std::vector<LexicalGenerator::ParsingTableEntryId>>
LexicalGenerator::ParsingTableEntryClassify(
    const std::vector<ProductionNodeId>& terminal_node_ids,
    const std::vector<ProductionNodeId>& nonterminal_node_ids) {
  // �洢��ͬ�ս�ڵ�ת�Ʊ����Ŀ
  std::vector<std::vector<ParsingTableEntryId>> terminal_classify_result,
      final_classify_result;
  std::vector<ParsingTableEntryId> entry_ids;
  entry_ids.reserve(lexical_config_parsing_table_.size());
  for (size_t i = 0; i < lexical_config_parsing_table_.size(); i++) {
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

inline void LexicalGenerator::RemapParsingTableEntryId(
    const std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>&
        moved_entry_id_to_new_entry_id) {
  for (auto& entry : lexical_config_parsing_table_) {
    entry.ResetEntryId(moved_entry_id_to_new_entry_id);
  }
}

void LexicalGenerator::ParsingTableMergeOptimize() {
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
  assert(lexical_config_parsing_table_.size() > 0);
  // ��һ��Ҫ�������Ŀ
  ParsingTableEntryId next_process_entry_index(0);
  // ��һ������λ��
  ParsingTableEntryId next_insert_position_index(0);
  // ���ƿ��ŷ�����㷨
  // Ѱ��û�б��ϲ�����Ŀ������������vector���±�һ��
  while (next_process_entry_index < lexical_config_parsing_table_.size()) {
    if (old_entry_id_to_new_entry_id.find(next_process_entry_index) ==
        old_entry_id_to_new_entry_id.end()) {
      // ����Ŀ����
      if (next_insert_position_index != next_process_entry_index) {
        // ��Ҫ�ƶ�����λ�ñ���vector����
        lexical_config_parsing_table_[next_insert_position_index] =
            std::move(lexical_config_parsing_table_[next_process_entry_index]);
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
  lexical_config_parsing_table_.resize(next_insert_position_index);
  // �����о�ID����Ϊ��ID
  RemapParsingTableEntryId(moved_entry_to_new_entry_id);
}

void LexicalGenerator::AnalysisKeyWord(const std::string& str) {
  std::sregex_iterator iter(str.cbegin(), str.cend(), keyword_regex_);
  std::sregex_iterator iter_end;
  FILE* config_file = GetConfigConstructFilePointer();
  for (; iter != iter_end; ++iter) {
    std::string keyword = iter->str(1);
    fprintf(config_file, "// �ؼ��֣�%s\n", keyword.c_str());
    fprintf(config_file, "PrintKeyWordConstructData(\"%s\")", keyword.c_str());
  }
}

void LexicalGenerator::AnalysisProductionConfig(const std::string& file_name) {
  // ��֧�ֻ��й���
  std::ifstream file(file_name);
  if (!file.is_open()) {
    fprintf(stderr, "�򿪲���ʽ�����ļ���%s ʧ��\n", file_name.c_str());
    return;
  }
  std::string temp;
  // ��ǰ���������
  size_t part_num = 0;
  while (part_num < frontend::common::kFunctionPartSize) {
    std::getline(file, temp);
    if (!file) {
      break;
    }
    if (temp.empty()) {
      continue;
    }
    if (temp[0] == '@') [[unlikely]] {
      // �����ǣ�1��@����ע�ͣ�2��@����������ָ�
      if (temp.size() >= 2 && temp[1] == '@') {
        // �ù������������ս�ڵ㣩����
        ++part_num;
        break;
      } else {
        continue;
      }
    }
    switch (part_num) {
      case 0:
        // �ؼ��ֶ�����
        AnalysisKeyWord(temp);
        break;
      case 1:
        // �����ս�ڵ㶨����
        AnalysisTerminalProduction(temp);
        break;
      case 2:
        // �����������
        AnalysisOperatorProduction(temp);
        break;
      case 3:
        // ��ͨ����ʽ������
        AnalysisNonTerminalProduction(temp);
        break;
      default:
        assert(false);
        break;
    }
  }
}

void LexicalGenerator::AnalysisTerminalProduction(const std::string& str,
                                                  size_t priority) {
  std::smatch match_result;
  std::regex_match(str, match_result, terminal_node_regex_);
  if (match_result.size() < 3) [[unlikely]] {
    fprintf(stderr, "�ս����ʽ��%s �������﷨�淶\n", str.c_str());
    return;
  }
  // ����ʽ��
  std::string node_symbol = match_result[1].str();
  // ����ʽ��
  std::string body_symbol = match_result[2].str();
  if (node_symbol.empty()) [[unlikely]] {
    fprintf(stderr, "�ս����ʽ��%s ȱ�ٲ���ʽ��\n", str.c_str());
    return;
  }
  if (body_symbol.empty()) [[unlikely]] {
    fprintf(stderr, "�ս����ʽ��%s ȱ�ٲ���ʽ��\n", str.c_str());
    return;
  }
  PrintTerminalNodeConstructData(std::move(node_symbol), std::move(body_symbol),
                                 priority);
}

#ifdef USE_AMBIGUOUS_GRAMMAR
void LexicalGenerator::AnalysisOperatorProduction(const std::string& str) {
  std::smatch match_result;
  std::regex_match(str, match_result, operator_node_regex_);
  if (match_result.size() == 0) {
    fprintf(stderr, "��������������Ϲ淶��%s\n", str.c_str());
    return;
  }
  OperatorData operator_data;
  // �������
  operator_data.operator_symbol = match_result[1].str();
  if (operator_data.operator_symbol.empty()) {
    fprintf(stderr, "�������ȱʧ\n");
    return;
  }
  // ��������ȼ�
  operator_data.priority = match_result[2].str();
  if (operator_data.priority.empty()) {
    fprintf(stderr, "�������%s ���ȼ�ȱʧ\n", operator_data.priority.c_str());
    return;
  }
  // ����������
  operator_data.associatity_type = match_result[3].str();
  if (operator_data.associatity_type.empty()) {
    fprintf(stderr, "�������%s ����������ȱʧ\n",
            operator_data.operator_symbol.c_str());
    return;
  }

  operator_data.reduct_function = match_result[4].str();

#ifdef USE_USER_DEFINED_FILE
  operator_data.include_files = GetFilesName(match_result[5].str());
#endif  // USE_USER_DEFINED_FILE
  PrintOperatorNodeConstructData(std::move(operator_data));
}
#endif  // USE_AMBIGUOUS_GRAMMAR

void lexicalgenerator::LexicalGenerator::AnalysisNonTerminalProduction(
    const std::string& str) {
  std::smatch match_result;
  std::regex_match(str, match_result, nonterminal_node_regex_);
  if (match_result.size() == 0) {
    // δ����
    fprintf(stderr, "���ս�ڵ㣺%s �������淶\n", str.c_str());
    return;
  }
  NonTerminalNodeData node_data;
  node_data.node_symbol = match_result[1].str();
  std::string body_symbol_str = match_result[2].str();
  node_data.body_symbols = GetBodySymbol(body_symbol_str);
  if (body_symbol_str.find('|') != std::string::npos) {
    node_data.use_same_process_function_class = true;
  } else {
    node_data.use_same_process_function_class = false;
  }
  node_data.reduct_function = match_result[3].str();
#ifdef USE_USER_DEFINED_FILE
  node_data.include_files = GetFilesName(match_result[4].str());
#endif  // USE_USER_DEFINED_FILE
  PrintNonTerminalNodeConstructData(std::move(node_data));
}

void LexicalGenerator::ParsingTableConstruct() {}

void LexicalGenerator::OpenConfigFile() {
  FILE* function_class_file_ptr = GetProcessFunctionClassFilePointer();
  if (function_class_file_ptr == nullptr) [[likely]] {
    errno_t result = fopen_s(&GetProcessFunctionClassFilePointer(),
                             "process_functions_classes.h", "w+");
    // ���±��޸Ĺ���ָ��
    function_class_file_ptr = GetProcessFunctionClassFilePointer();
    if (result != 0) [[unlikely]] {
      fprintf(stderr, "��LexicalConfig/process_functions.hʧ�ܣ������룺%d\n",
              result);
      return;
    }
    fprintf(
        function_class_file_ptr,
        "#ifndef GENERATOR_LEXICALGENERATOR_LEXICALCONFIG_PROCESSFUNCTIONS_H_\n"
        "#define "
        "GENERATOR_LEXICALGENERATOR_LEXICALCONFIG_PROCESSFUNCTIONS_H_\n");
  }
  FILE* config_file_ptr = GetConfigConstructFilePointer();
  if (config_file_ptr == nullptr) [[likely]] {
    errno_t result =
        fopen_s(&GetConfigConstructFilePointer(), "config_construct.cpp", "w+");
    // ���±��޸Ĺ���ָ��
    config_file_ptr = GetConfigConstructFilePointer();
    if (result != 0) [[unlikely]] {
      fprintf(stderr,
              "��LexicalConfig/config_construct.cppʧ�ܣ������룺%d\n",
              result);
      return;
    }
    fprintf(config_file_ptr,
            "#include \"lexical_generator.h\"\n"
            "namespace frontend::generator::lexicalgenerator {\n"
            "void LexicalGenerator::ConfigConstruct() { \n");
  }
}

void LexicalGenerator::CloseConfigFile() {
  FILE* function_class_file_ptr = GetProcessFunctionClassFilePointer();
  if (function_class_file_ptr != nullptr) [[likely]] {
    fprintf(function_class_file_ptr,
            "#endif  // "
            "!GENERATOR_LEXICALGENERATOR_LEXICALCONFIG_PROCESSFUNCTIONS_H_");
    fclose(function_class_file_ptr);
  }
  FILE* config_file_ptr = GetConfigConstructFilePointer();
  if (config_file_ptr != nullptr) [[likely]] {
    // ����Ƿ���δ�������ʽ
    fprintf(config_file_ptr, "CheckUndefinedProductionRemained();\n");
    fprintf(config_file_ptr,
            "}\n"
            "}  // namespace frontend::generator::lexicalgenerator");
    fclose(config_file_ptr);
  }
}

std::vector<std::pair<std::string, bool>> LexicalGenerator::GetBodySymbol(
    const std::string& str) {
  std::sregex_iterator regex_iter(str.cbegin(), str.cend(), body_symbol_regex_);
  std::vector<std::pair<std::string, bool>> bodys;
  std::sregex_iterator regex_iter_end;
  for (; regex_iter != regex_iter_end; ++regex_iter) {
    bool is_terminal_symbol = *regex_iter->str(1).cbegin() == '"';
    bodys.emplace_back(std::make_pair(regex_iter->str(2), is_terminal_symbol));
  }
  return bodys;
}

std::vector<std::string> LexicalGenerator::GetFunctionsName(
    const std::string& str) {
  std::sregex_iterator regex_iter(str.cbegin(), str.cend(), function_regex_);
  std::vector<std::string> functions;
  std::sregex_iterator regex_iter_end;
  for (; regex_iter != regex_iter_end; ++regex_iter) {
    std::string function_name = regex_iter->str(1);
    if (function_name == "@") {
      // ʹ��ռλ����ʾ�պ���
      function_name.clear();
    }
    functions.emplace_back(std::move(function_name));
  }
  return functions;
}

std::vector<std::string> LexicalGenerator::GetFilesName(
    const std::string& str) {
  std::sregex_iterator regex_iter(str.cbegin(), str.cend(), file_regex_);
  std::vector<std::string> include_files;
  std::sregex_iterator regex_iter_end;
  for (; regex_iter != regex_iter_end; ++regex_iter) {
    std::string include_file_name = regex_iter->str(1);
    if (include_file_name == "@") {
      // ʹ��ռλ������û�а����ļ�
      break;
    }
    include_files.emplace_back(std::move(include_file_name));
  }
  return include_files;
}

void LexicalGenerator::AddUnableContinueNonTerminalNode(
    const std::string& undefined_symbol, const std::string& node_symbol,
    const std::vector<std::pair<std::string, bool>>& subnode_symbols,
    ProcessFunctionClassId class_id) {
  // ʹ��insert��Ϊ�����ж������ʽ����ͬһ��δ����Ĳ���ʽ
  undefined_productions_.insert(
      std::make_pair(undefined_symbol,
                     std::make_tuple(node_symbol, subnode_symbols, class_id)));
}

void LexicalGenerator::CheckNonTerminalNodeCanContinue(
    const std::string& node_symbol) {
  if (!undefined_productions_.empty()) {
    auto [iter_begin, iter_end] =
        undefined_productions_.equal_range(node_symbol);
    if (iter_begin != undefined_productions_.end()) {
      for (; iter_begin != iter_end; ++iter_begin) {
        auto& [node_symbol, node_body_ptr, process_function_class_id_] =
            iter_begin->second;
        AddNonTerminalNode(node_symbol, node_body_ptr,
                           process_function_class_id_);
      }
      // ɾ������ӵļ�¼
      undefined_productions_.erase(iter_begin, iter_end);
    }
  }
}

void LexicalGenerator::CheckUndefinedProductionRemained() {
  if (!undefined_productions_.empty()) {
    // �Դ���δ�������ʽ
    for (auto& item : undefined_productions_) {
      auto& [node_symbol, node_bodys, class_id] = item.second;
      fprintf(stderr, "����ʽ��%s\n����ʽ�壺", node_symbol.c_str());
      for (auto& body : node_bodys) {
        fprintf(stderr, "%s ", body.first.c_str());
      }
      fprintf(stderr, "��\n����ʽ��%s δ����\n", item.first.c_str());
    }
  }
}

inline void LexicalGenerator::AddKeyWord(const std::string& key_word) {
  // �ؼ������ȼ�Ĭ��Ϊ1
  // �Զ�����ͬ���ս�ڵ�
  AddTerminalNode(
      key_word, key_word,
      frontend::generator::dfa_generator::DfaGenerator::TailNodePriority(1));
}

void LexicalGenerator::PrintKeyWordConstructData(const std::string& keyword) {
  fprintf(GetConfigConstructFilePointer(), "// �ؼ��֣�%s\n", keyword.c_str());
  fprintf(GetConfigConstructFilePointer(), "AddKeyWord(%s);\n",
          keyword.c_str());
}

void LexicalGenerator::PrintTerminalNodeConstructData(std::string&& node_symbol,
                                                      std::string&& body_symbol,
                                                      size_t priority) {
  fprintf(GetConfigConstructFilePointer(),
          "// �ս�ڵ�����%s\n// �ս�ڵ��壺%s\n // ���ȼ���%llu",
          node_symbol.c_str(), body_symbol.c_str(),
          static_cast<unsigned long long>(priority));
  fprintf(GetConfigConstructFilePointer(),
          "AddTerminalNode(%s,%s,PriorityLevel(%s));\n", node_symbol.c_str(),
          body_symbol.c_str(), std::to_string(priority).c_str());
}

#ifdef USE_AMBIGUOUS_GRAMMAR
void LexicalGenerator::PrintOperatorNodeConstructData(OperatorData&& data) {
  // ����һ�����
  int operator_num = GetNodeNum();
  std::string class_name = class_name + std::to_string(operator_num) +
                           data.associatity_type + data.priority;
  // ���±�������Ľ�����ַ���
  if (data.associatity_type == "L") {
    data.associatity_type = "AssociatityType::kLeftAssociate";
  } else {
    assert(data.associatity_type == "R");
    data.associatity_type = "AssociatityType::kRightAssociate";
  }
  fprintf(GetConfigConstructFilePointer(),
          "// ������� %s\n// ����ԣ�%s\n// ���ȼ���%s\n",
          data.operator_symbol.c_str(), data.associatity_type.c_str(),
          data.priority.c_str());
  fprintf(
      GetConfigConstructFilePointer(),
      "AddOperatorNode<%s>(\"%s\",AssociatityType(%s),PriorityLevel(%s));\n",
      class_name.c_str(), data.operator_symbol.c_str(),
      data.associatity_type.c_str(), data.priority.c_str());
  FILE* function_file = GetProcessFunctionClassFilePointer();
  fprintf(function_file,
          "class %s : public ProcessFunctionInterface {\n public:\n",
          class_name.c_str());
  fprintf(function_file, "// ������� %s\n// ����ԣ�%s\n// ���ȼ���%s\n",
          data.operator_symbol.c_str(), data.associatity_type.c_str(),
          data.priority.c_str());
#ifdef USE_USER_DEFINED_FILE
  for (size_t i = 0; i < data.include_files.size(); i++) {
    fprintf(function_file, "  #include\"%s\"\n", data.include_files[i].c_str());
  }
#endif  // USE_USER_DEFINED_FILE
  PrintProcessFunction(function_file, data);
  fprintf(function_file, " private:\n");
  fprintf(function_file, "}\n");
}

#endif  // USE_AMBIGUOUS_GRAMMAR

void LexicalGenerator::PrintNonTerminalNodeConstructData(
    NonTerminalNodeData&& data) {
  int node_num = GetNodeNum();
  std::string class_name = data.node_symbol + '_' + std::to_string(node_num);
  FILE* function_file = GetProcessFunctionClassFilePointer();
  fprintf(function_file, "class %s :public ProcessFunctionInterface {\n",
          class_name.c_str());
  fprintf(function_file, "// ���ս�ڵ�����%s\n", data.node_symbol.c_str());
  PrintProcessFunction(function_file, data);
  fprintf(function_file, " private:\n");
#ifdef USE_USER_DEFINED_FILE
  for (size_t i = 0; i < data.include_files.size(); i++) {
    fprintf(function_file, "  #include\"%s\"\n", data.include_files[i].c_str());
  }
#endif  // USE_USER_DEFINED_FILE
  fprintf(function_file, "}\n");

  FILE* config_file = GetConfigConstructFilePointer();
  if (data.use_same_process_function_class) {
    // �÷��ս�ڵ�洢����ս�ڵ��ҹ��ô�����
    // ����һ����һ�޶��ı�����
    std::string variety_name =
        class_name + std::to_string('_') + std::to_string(node_num);
    fprintf(config_file,
            "ProcessFunctionClassId %s = "
            "CreateProcessFunctionClassObject<%s>();\n",
            variety_name.c_str(), class_name.c_str());
    for (size_t i = 0; i < data.body_symbols.size(); i++) {
      if (!data.body_symbols[i].second) {
        fprintf(stderr,
                "���ս�ڵ㣺%s �У���%llu������ʽ�岻���ս�ڵ����ʽ\n",
                data.node_symbol.c_str(), static_cast<unsigned long long>(i));
      }
      fprintf(config_file, "AddNonTerminalNode(\"%s\",{{\"%s\",true}},%s);\n",
              data.node_symbol.c_str(), data.body_symbols[i].first.c_str(),
              variety_name.c_str());
    }
  } else {
    // ����������ս�ڵ��һ������ʽ��
    if (data.body_symbols.empty()) [[unlikely]] {
      fprintf(stderr, "���ս�ڵ㣺%s û�в���ʽ��\n",
              data.node_symbol.c_str());
      return;
    }
    // ���ʹ�ó�ʼ���б��ʾ��ÿ������ʽ��
    std::string is_terminal_body =
        data.body_symbols.front().second ? "true" : "false";
    fprintf(config_file, "AddNonTerminalNode<%s>(\"%s\",{{\"%s\",%s}",
            class_name.c_str(), data.node_symbol.c_str(),
            data.body_symbols.front().first.c_str(), is_terminal_body.c_str());
    for (size_t i = 1; i < data.body_symbols.size(); i++) {
      std::string is_terminal_body =
          data.body_symbols[i].second ? "true" : "false";
      fprintf(config_file, ",{\"%s\",%s}", data.body_symbols[i].first.c_str(),
              is_terminal_body.c_str());
    }
    fprintf(config_file, "};\n");
  }
}

LexicalGenerator::ProductionNodeId
LexicalGenerator::GetProductionNodeIdFromBodySymbolId(SymbolId body_symbol_id) {
  auto iter = production_body_symbol_id_to_node_id_.find(body_symbol_id);
  if (iter != production_body_symbol_id_to_node_id_.end()) {
    return iter->second;
  } else {
    return ProductionNodeId::InvalidId();
  }
}

LexicalGenerator::ProductionNodeId
LexicalGenerator::GetProductionNodeIdFromNodeSymbol(
    const std::string& body_symbol) {
  SymbolId node_symbol_id = GetNodeSymbolId(body_symbol);
  if (node_symbol_id.IsValid()) {
    return GetProductionNodeIdFromNodeSymbolId(node_symbol_id);
  } else {
    return ProductionNodeId::InvalidId();
  }
}

LexicalGenerator::ProductionNodeId
LexicalGenerator::GetProductionNodeIdFromBodySymbol(
    const std::string& body_symbol) {
  SymbolId body_symbol_id = GetBodySymbolId(body_symbol);
  if (body_symbol_id.IsValid()) {
    return GetProductionNodeIdFromBodySymbolId(body_symbol_id);
  } else {
    return ProductionNodeId::InvalidId();
  }
}

inline LexicalGenerator::ProductionNodeId
LexicalGenerator::TerminalProductionNode::GetProductionNodeInBody(
    ProductionBodyId production_body_id, PointIndex point_index) {
  assert(production_body_id == 0);
  if (point_index == 0) {
    return Id();
  } else {
    return ProductionNodeId::InvalidId();
  }
}

inline void
LexicalGenerator::NonTerminalProductionNode::ResizeProductionBodyNum(
    size_t new_size) {
  nonterminal_bodys_.resize(new_size);
  process_function_class_ids_.resize(new_size);
}

void LexicalGenerator::NonTerminalProductionNode::ResizeProductionBodyNodeNum(
    ProductionBodyId production_body_id, size_t new_size) {
  assert(production_body_id < nonterminal_bodys_.size());
  nonterminal_bodys_[production_body_id].resize(new_size);
}

std::vector<LexicalGenerator::ProductionBodyId>
LexicalGenerator::NonTerminalProductionNode::GetAllBodyIds() const {
  std::vector<ProductionBodyId> production_body_ids;
  for (size_t i = 0; i < nonterminal_bodys_.size(); i++) {
    production_body_ids.push_back(ProductionBodyId(i));
  }
  return production_body_ids;
}

inline LexicalGenerator::ProductionNodeId
LexicalGenerator::NonTerminalProductionNode::GetProductionNodeInBody(
    ProductionBodyId production_body_id, PointIndex point_index) {
  assert(production_body_id < nonterminal_bodys_.size());
  if (point_index < nonterminal_bodys_[production_body_id].size()) {
    return nonterminal_bodys_[production_body_id][point_index];
  } else {
    return ProductionNodeId::InvalidId();
  }
}

inline LexicalGenerator::ParsingTableEntry&
LexicalGenerator::ParsingTableEntry::operator=(
    ParsingTableEntry&& parsing_table_entry) {
  action_and_attached_data_ =
      std::move(parsing_table_entry.action_and_attached_data_);
  nonterminal_node_transform_table_ =
      std::move(parsing_table_entry.nonterminal_node_transform_table_);
  return *this;
}

void LexicalGenerator::ParsingTableEntry::ResetEntryId(
    const std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>&
        old_entry_id_to_new_entry_id) {
  //�����ս�ڵ�Ķ���
  for (auto& action_and_attached_data : GetAllActionAndAttachedData()) {
    // ��ȡԭ��ĿID������
    ParsingTableEntryId& old_entry_id =
        action_and_attached_data.second.GetShiftAttachedData().next_entry_id_;
    // ����Ϊ�µ���ĿID
    old_entry_id = old_entry_id_to_new_entry_id.find(old_entry_id)->second;
  }
  //������ս�ڵ��ת��
  for (auto& target : GetAllNonTerminalNodeTransformTarget()) {
    ParsingTableEntryId old_entry_id = target.second;
    ParsingTableEntryId new_entry_id =
        old_entry_id_to_new_entry_id.find(old_entry_id)->second;
    SetNonTerminalNodeTransformId(target.first, new_entry_id);
  }
}

LexicalGenerator::BaseProductionNode&
LexicalGenerator::BaseProductionNode::operator=(
    BaseProductionNode&& base_production_node) {
  base_type_ = std::move(base_production_node.base_type_);
  base_id_ = std::move(base_production_node.base_id_);
  base_symbol_id_ = std::move(base_production_node.base_symbol_id_);
  return *this;
}

#ifdef USE_AMBIGUOUS_GRAMMAR
LexicalGenerator::OperatorProductionNode&
LexicalGenerator::OperatorProductionNode::operator=(
    OperatorProductionNode&& operator_production_node) {
  TerminalProductionNode::operator=(std::move(operator_production_node));
  operator_associatity_type_ =
      std::move(operator_production_node.operator_associatity_type_);
  operator_priority_level_ =
      std::move(operator_production_node.operator_priority_level_);
  process_function_class_id_ =
      std::move(operator_production_node.process_function_class_id_);
  return *this;
}
#endif  // USE_AMBIGUOUS_GRAMMAR

LexicalGenerator::Core& LexicalGenerator::Core::operator=(Core&& core) {
  core_closure_available_ = std::move(core.core_closure_available_);
  core_id_ = std::move(core.core_id_);
  parsing_table_entry_id_ = std::move(core.parsing_table_entry_id_);
  item_and_forward_node_ids_ = std::move(core.item_and_forward_node_ids_);
  return *this;
}

const std::regex LexicalGenerator::terminal_node_regex_(
    R"!(^\s*([a-zA-Z][a-zA-Z0-9_]*)\s*->\s*(\S+)\s*$)!");
const std::regex LexicalGenerator::operator_node_regex_(
    R"(^\s*(\S+)\s*@\s*([1-9][0-9]*)\s*@\s*([LR])\s*\{\s*([a-zA-Z_])"
    R"([a-zA-Z0-9_]*)\s*\}\s*(?:\{\s*([^\}]+)\s*\})?\s*$)");
const std::regex LexicalGenerator::nonterminal_node_regex_(
    R"(^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*->\s*([^=]+)\s*(?:=>\s*\{\s*)"
    R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*\})\s*(?:\{\s*([^\}]+)\s*\})?\s*$)");
const std::regex LexicalGenerator::keyword_regex_(R"!("(\S*)")!");
const std::regex LexicalGenerator::body_symbol_regex_(
    R"((")?([a-zA-Z_][a-zA-Z0-9_]*)(\1))");
const std::regex LexicalGenerator::function_regex_(
    R"(((?:[a-zA-Z_][a-zA-Z0-9_]*)|@))");
const std::regex LexicalGenerator::file_regex_(
    R"(((?:[a-zA-Z_][a-zA-Z0-9_.\- ]*)|@))");

}  // namespace frontend::generator::lexicalgenerator