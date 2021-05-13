#include "Generator/LexicalGenerator/lexical_generator.h"

#include <queue>
// TODO ������ֱ����InvalidId�ȽϵĴ����Ϊʹ��IsValid()����
namespace frontend::generator::lexicalgenerator {

inline void LexicalGenerator::SetSymbolIdToProductionNodeIdMapping(
    SymbolId symbol_id, ProductionNodeId node_id) {
  symbol_id_to_node_id_.insert(std::make_pair(symbol_id, node_id));
}

inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddTerminalNode(
    SymbolId symbol_id) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<TerminalProductionNode>(symbol_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  return node_id;
}

inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddOperatorNode(
    SymbolId symbol_id, AssociatityType associatity_type,
    PriorityLevel priority_level) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<OperatorProductionNode>(
          symbol_id, associatity_type, priority_level);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  return node_id;
}

LexicalGenerator::ProductionNodeId LexicalGenerator::AddEndNode(
    SymbolId symbol_id) {
  ProductionNodeId node_id = manager_nodes_.EmplaceObject<EndNode>(symbol_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  return node_id;
}

std::vector<LexicalGenerator::ProductionNodeId>
LexicalGenerator::GetSymbolToProductionNodeIds(SymbolId symbol_id) {
  std::vector<LexicalGenerator::ProductionNodeId> node_ids;
  auto [iter_begin, iter_end] = symbol_id_to_node_id_.equal_range(symbol_id);
  assert(iter_begin != symbol_id_to_node_id_.end());
  for (; iter_begin != iter_end; ++iter_begin) {
    node_ids.push_back(iter_begin->second);
  }
  return node_ids;
}

void LexicalGenerator::SetItemCoreId(const CoreItem& core_item,
                                     CoreId core_id) {
  assert(core_id.IsValid());
  auto [production_node_id, production_body_id, point_index] = core_item;
  GetProductionNode(production_node_id)
      .SetCoreId(production_body_id, point_index, core_id);
}

LexicalGenerator::CoreId LexicalGenerator::GetItemCoreId(
    const CoreItem& core_item) {
  auto [production_node_id, production_body_id, point_index] = core_item;
  return GetCoreId(production_node_id, production_body_id, point_index);
}

std::pair<LexicalGenerator::CoreId, bool>
LexicalGenerator::GetItemCoreIdOrInsert(const CoreItem& core_item,
                                        CoreId insert_core_id) {
  CoreId core_id = GetItemCoreId(core_item);
  if (core_id == CoreId::InvalidId()) {
    if (insert_core_id == CoreId::InvalidId()) {
      core_id = AddNewCore();
    } else {
      core_id = insert_core_id;
    }
#ifdef _DEBUG
    bool result = AddItemToCore(core_item, core_id).second;
    // �������
    assert(result == true);
#else
    AddItemToCore(core_item, core_id);
#endif  // _DEBUG
    return std::make_pair(core_id, true);
  } else {
    return std::make_pair(core_id, false);
  }
}

std::unordered_set<LexicalGenerator::ProductionNodeId> LexicalGenerator::First(
    ProductionNodeId first_node_id,
    const std::unordered_set<ProductionNodeId>& next_node_ids) {
  if (first_node_id.IsValid()) {
    return std::unordered_set<ProductionNodeId>(first_node_id);
  } else {
    return next_node_ids;
  }
}

void LexicalGenerator::CoreClosure(CoreId core_id) {
  if (GetCore(core_id).IsClosureAvailable()) {
    // �հ���Ч����������
    return;
  }
  std::queue<const CoreItem*> items_waiting_process;
  for (auto& item : GetCoreItems(core_id)) {
    items_waiting_process.push(&item);
  }
  while (!items_waiting_process.empty()) {
    const CoreItem* item_now = items_waiting_process.front();
    items_waiting_process.pop();
    auto [production_node_id, production_body_id, point_index] = *item_now;
    ProductionNodeId next_production_node_id = GetProductionBodyNextNodeId(
        production_node_id, production_body_id, point_index);
    if (!next_production_node_id.IsValid()) {
      // ���Ѿ�������ˣ�û����һ���ڵ�
      continue;
    }
    NonTerminalProductionNode& next_production_node =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(next_production_node_id));
    if (next_production_node.Type() != ProductionNodeType::kNonTerminalNode) {
      // ���Ƿ��ս�ڵ㣬�޷�չ��
      continue;
    }
    ProductionNodeId next_next_production_node_id =
        GetProductionBodyNextNextNodeId(production_node_id, production_body_id,
                                        point_index);
    const std::unordered_set<ProductionNodeId>& forward_nodes_second_part =
        GetFowardNodes(production_node_id, production_body_id, point_index);
    // ��ȡ�����ɽڵ��������ǰ���ڵ�
    InsideForwardNodesContainerType forward_nodes =
        First(next_next_production_node_id, forward_nodes_second_part);
    const BodyContainerType& bodys = next_production_node.GetAllBody();
    for (size_t i = 0; i < bodys.size(); i++) {
      ProductionBodyId body_id(i);
      PointIndex point_index(0);
      auto [iter_item, inserted] = AddItemToCore(
          CoreItem(next_production_node_id, body_id, point_index), core_id);
      AddForwardNodeContainer(next_production_node_id, body_id, point_index,
                              forward_nodes);
      if (inserted) {
        // �²�����item��������еȴ�����
        items_waiting_process.push(&*iter_item);
      }
    }
  }
  SetCoreClosureAvailable(core_id);
}

LexicalGenerator::ProductionNodeId inline LexicalGenerator::
    GetProductionBodyNextNodeId(ProductionNodeId production_node_id,
                                ProductionBodyId production_body_id,
                                PointIndex point_index) {
  BaseProductionNode& production_node = GetProductionNode(production_node_id);
  return production_node.GetProductionNodeInBody(production_body_id,
                                                 PointIndex(point_index + 1));
}

LexicalGenerator::ProductionNodeId inline LexicalGenerator::
    GetProductionBodyNextNextNodeId(ProductionNodeId production_node_id,
                                    ProductionBodyId production_body_id,
                                    PointIndex point_index) {
  BaseProductionNode& production_node = GetProductionNode(production_node_id);
  return production_node.GetProductionNodeInBody(production_body_id,
                                                 PointIndex(point_index + 2));
}

std::pair<LexicalGenerator::CoreId, bool> LexicalGenerator::ItemGoto(
    const CoreItem& item, ProductionNodeId transform_production_node_id,
    CoreId insert_core_id) {
  assert(transform_production_node_id.IsValid());
  auto [item_production_node_id, item_production_body_id, item_point_index] =
      item;
  ProductionNodeId next_production_node_id = GetProductionBodyNextNodeId(
      item_production_node_id, item_production_body_id, item_point_index);
  if (manager_nodes_.IsSame(next_production_node_id,
                            transform_production_node_id)) {
    return GetItemCoreIdOrInsert(
        CoreItem(item_production_node_id, item_production_body_id,
                 ++item_point_index),
        insert_core_id);
  } else {
    return std::make_pair(CoreId::InvalidId(), false);
  }
}

std::pair<LexicalGenerator::CoreId, bool> LexicalGenerator::Goto(
    CoreId core_id_src, ProductionNodeId transform_production_node_id) {
  // Goto�����Ƿ���Ч
  bool map_cache_available = IsGotoCacheAvailable(core_id_src);
  // ������Ч
  if (map_cache_available) {
    return std::make_pair(
        GetGotoCacheEntry(core_id_src, transform_production_node_id), false);
  }
  // ������Ч�������core_id��Ӧ���еĻ���
  RemoveGotoCacheEntry(core_id_src);
  // �Ƿ��½�core��־
  bool core_constructed = false;
  CoreId core_id_dst = CoreId::InvalidId();
  CoreClosure(core_id_src);
  const auto& items = GetCoreItems(core_id_src);
  auto iter = items.begin();
  // �ҵ���һ���ɹ�Goto�����¼CoreId���Ƿ��½���Core
  while (iter != items.end()) {
    std::pair(core_id_dst, core_constructed) =
        ItemGoto(*iter, transform_production_node_id);
    if (core_id_dst.IsValid()) {
      break;
    }
    ++iter;
  }
  if (core_constructed) {
    // �������µ������Ҫ��ʣ�µ����ItemGoto�������Բ��������뵽�µ��
    assert(core_id_dst.IsValid());
    while (iter != items.end()) {
#ifdef _DEBUG
      // �����Ƿ�ÿһ����Goto���������ͬ��
      auto [core_id, constructed] =
          ItemGoto(*iter, transform_production_node_id, core_id_dst);
      assert(!core_id.IsValid() ||
             core_id == core_id_dst && constructed == true);
#else
      ItemGoto(*iter, transform_production_node_id, core_id_dst);
#endif  // _DEBUG
      ++iter;
    }
  }
#ifdef _DEBUG
  // �����Ƿ�ÿһ����Goto���������ͬ��
  else {
    while (iter != items.end()) {
      auto [core_id, constructed] =
          ItemGoto(*iter, transform_production_node_id, core_id_dst);
      assert(core_id == core_id_dst && constructed == false);
      ++iter;
    }
  }
#endif  // _DEBUG
  // ���û���
  SetGotoCacheEntry(core_id_src, transform_production_node_id, core_id_dst);
  return std::make_pair(core_id_dst, core_constructed);
}

void LexicalGenerator::SpreadLookForwardSymbol(CoreId core_id) {
  const auto& items = GetCoreItems(core_id);
  for (auto& item : items) {
    auto [production_node_id, production_body_id, point_index] = item;
    // ���ÿһ�����ܵ���һ�����Ŷ�ִ��һ��Goto��ʹ���ڴ�����ǰ������ǰ
    // ���п����õ��ĵ����ĳ����Ӧ���
    ProductionNodeId next_production_node_id = GetProductionBodyNextNodeId(
        production_node_id, production_body_id, point_index);
    if (next_production_node_id.IsValid()) {
      Goto(core_id, next_production_node_id);
    }
  }
  for (auto& item : items) {
    auto [production_node_id, production_body_id, point_index] = item;
    ProductionNodeId next_production_node_id = GetProductionBodyNextNodeId(
        production_node_id, production_body_id, point_index);
    if (next_production_node_id.IsValid()) {
      std::unordered_set<ProductionNodeId> forward_nodes =
          GetFowardNodes(production_node_id, production_body_id, point_index);
      // ������ǰ������
      AddForwardNodeContainer(production_node_id, production_body_id,
                              PointIndex(point_index + 1), forward_nodes);
    }
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

LexicalGenerator::ParsingTableEntryId LexicalGenerator::GetItemToParsingEntryId(
    const CoreItem& item) {
  auto iter = item_to_parsing_table_entry_id_.find(item);
  if (iter != item_to_parsing_table_entry_id_.end()) {
    return iter->second;
  } else {
    return ParsingTableEntryId::InvalidId();
  }
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

void LexicalGenerator::RemapParsingTableEntryId(
    const std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>&
        moved_entry_to_new_entry_id) {
  for (auto& entry : lexical_config_parsing_table_) {
    //�����ս�ڵ�Ķ���
    for (auto& action_and_target : entry.GetAllTerminalNodeActionAndTarget()) {
      ParsingTableEntryId old_entry_id, new_entry_id;
      switch (action_and_target.second.first) {
        case ActionType::kAccept:
        case ActionType::kError:
        case ActionType::kReduction:
          break;
        case ActionType::kShift:
          old_entry_id = *std::get_if<ParsingTableEntryId>(
              &action_and_target.second.second);
          new_entry_id = moved_entry_to_new_entry_id.find(old_entry_id)->second;
          // ����Ϊ�µ���ĿID
          entry.SetTerminalNodeActionAndTarget(action_and_target.first,
                                               action_and_target.second.first,
                                               new_entry_id);
          break;
        default:
          assert(false);
          break;
      }
    }
    //������ս�ڵ��ת��
    for (auto& target : entry.GetAllNonTerminalNodeTransformTarget()) {
      ParsingTableEntryId old_entry_id = target.second;
      ParsingTableEntryId new_entry_id =
          moved_entry_to_new_entry_id.find(old_entry_id)->second;
      entry.SetNonTerminalNodeTransformId(target.first, new_entry_id);
    }
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
  for (auto id : classified_production_node_ids[operator_index]) {
    // �������������ڵ���ӵ��ս�ڵ��������������ս�ڵ��
    classified_production_node_ids[terminal_index].push_back(id);
  }
  std::vector<std::vector<ParsingTableEntryId>> classified_ids =
      ParsingTableEntryClassify(
          classified_production_node_ids[terminal_index],
          classified_production_node_ids[nonterminal_index]);
  // �洢��ɾ���ľ���Ŀ����ͬ��Ŀ��ӳ��
  std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>
      old_entry_id_to_new_entry_id;
  for (auto& entry_ids : classified_ids) {
    // ��ӱ�ɾ���ľ���Ŀ����ͬ��Ŀ��ӳ��
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
  ParsingTableEntryId index_next_process_entry(0);
  // ��һ������λ��
  ParsingTableEntryId index_next_insert(0);
  // ���ƿ��ŷ�����㷨
  // Ѱ�ұ��ϲ�����Ŀ
  while (index_next_process_entry < lexical_config_parsing_table_.size()) {
    if (old_entry_id_to_new_entry_id.find(index_next_process_entry) ==
        old_entry_id_to_new_entry_id.end()) {
      // ����Ŀ����
      if (index_next_insert != index_next_process_entry) {
        // ��Ҫ�ƶ�
        lexical_config_parsing_table_[index_next_insert] =
            std::move(lexical_config_parsing_table_[index_next_process_entry]);
      }
      // ��ӳ�䱣����Ŀ����λ��
      moved_entry_to_new_entry_id[index_next_process_entry] = index_next_insert;
      ++index_next_insert;
    }
    ++index_next_process_entry;
  }
  for (auto& item : old_entry_id_to_new_entry_id) {
    // ��ӳ����ɾ����Ŀ������Ŀ
    assert(moved_entry_to_new_entry_id.find(item.second) !=
           moved_entry_to_new_entry_id.end());
    moved_entry_to_new_entry_id[item.first] =
        moved_entry_to_new_entry_id[item.second];
  }
  // ����ÿһ����Ŀ����������Ŀ��ӳ��
  // �ͷŶ���ռ�
  old_entry_id_to_new_entry_id.clear();
  lexical_config_parsing_table_.resize(index_next_insert + 1);
  // �����о�ID����Ϊ��ID
  RemapParsingTableEntryId(moved_entry_to_new_entry_id);
}

void LexicalGenerator::ParsingTableConstruct() {
  // Ϊÿ����Ч����﷨��������Ŀ
  // ʹ�ö��д洢��Ӷ�����Ŀ��ڵ������ȫ��������������
  // ��ֹ�������ת�Ƶ��Ľڵ���Ӧ��������Ŀ��û������
  std::queue<std::tuple<ParsingTableEntry*, ProductionNodeId, CoreItem>>
      node_action_waiting_set;
  for (auto& core : cores_) {
    for (auto& item : core.GetItems()) {
      ParsingTableEntryId entry_id(lexical_config_parsing_table_.size());
      lexical_config_parsing_table_.emplace_back();
      ParsingTableEntry& entry_now = lexical_config_parsing_table_.back();
      SetItemToParsingEntryIdMapping(item, entry_id);
      auto [production_node_id, production_body_id, point_index] = item;
      ProductionNodeId node_id = GetProductionBodyNextNodeId(
          production_node_id, production_body_id, point_index);
      if (node_id.IsValid()) {
        // ��������һ���ڵ㣬ִ���������
        assert(GetProductionNode(node_id).Type() !=
               ProductionNodeType::kEndNode);
        node_action_waiting_set.push(
            std::make_tuple(&entry_now, node_id,
                            CoreItem(production_node_id, production_body_id,
                                     PointIndex(point_index + 1))));
      } else {
        // �޿�����ڵ㣬��������ǰ���ڵ�ִ�й�Լ
        for (auto forward_node_id : GetFowardNodes(
                 production_node_id, production_body_id, point_index)) {
          entry_now.SetTerminalNodeActionAndTarget(
              forward_node_id, ActionType::kReduction,
              std::make_pair(production_node_id, production_body_id));
        }
      }
    }
  }
  // ��������е���������
  while (!node_action_waiting_set.empty()) {
    auto [entry_now, node_id, item] = node_action_waiting_set.front();
    node_action_waiting_set.pop();
    ParsingTableEntryId target_entry_id = GetItemToParsingEntryId(item);
    assert(target_entry_id.IsValid());
    switch (GetProductionNode(node_id).Type()) {
      case ProductionNodeType::kNonTerminalNode:
        entry_now->SetNonTerminalNodeTransformId(node_id, target_entry_id);
        break;
      case ProductionNodeType::kTerminalNode:
      case ProductionNodeType::kOperatorNode:
        entry_now->SetTerminalNodeActionAndTarget(node_id, ActionType::kShift,
                                                  target_entry_id);
        break;
      default:
        assert(false);
        break;
    }
  }
  ParsingTableMergeOptimize();
}

LexicalGenerator::ProductionNodeId
LexicalGenerator::TerminalProductionNode::GetProductionNodeInBody(
    ProductionBodyId production_body_id, PointIndex point_index) {
  assert(production_body_id == 0);
  if (point_index == 0) {
    return Id();
  } else {
    return ProductionNodeId::InvalidId();
  }
}

LexicalGenerator::NonTerminalProductionNode&
LexicalGenerator::NonTerminalProductionNode::operator=(
    NonTerminalProductionNode&& nonterminal_production_node) {
  BaseProductionNode::operator=(std::move(nonterminal_production_node));
  nonterminal_bodys_ =
      std::move(nonterminal_production_node.nonterminal_bodys_);
  return *this;
}

LexicalGenerator::ProductionNodeId
LexicalGenerator::NonTerminalProductionNode::GetProductionNodeInBody(
    ProductionBodyId production_body_id, PointIndex point_index) {
  assert(production_body_id < nonterminal_bodys_.size());
  if (point_index < nonterminal_bodys_[production_body_id].size()) {
    return nonterminal_bodys_[production_body_id][point_index];
  } else {
    return ProductionNodeId::InvalidId();
  }
}

LexicalGenerator::ParsingTableEntry&
LexicalGenerator::ParsingTableEntry::operator=(
    ParsingTableEntry&& parsing_table_entry) {
  action_and_target_ = std::move(parsing_table_entry.action_and_target_);
  nonterminal_node_transform_table_ =
      std::move(parsing_table_entry.nonterminal_node_transform_table_);
  return *this;
}

LexicalGenerator::BaseProductionNode&
LexicalGenerator::BaseProductionNode::operator=(
    BaseProductionNode&& base_production_node) {
  base_type_ = std::move(base_production_node.base_type_);
  base_id_ = std::move(base_production_node.base_id_);
  base_symbol_id_ = std::move(base_production_node.base_symbol_id_);
  base_forward_nodes_ = std::move(base_production_node.base_forward_nodes_);
  base_core_ids_ = std::move(base_production_node.base_core_ids_);
  return *this;
}

LexicalGenerator::OperatorProductionNode&
LexicalGenerator::OperatorProductionNode::operator=(
    OperatorProductionNode&& operator_production_node) {
  TerminalProductionNode::operator=(std::move(operator_production_node));
  operator_associatity_type_ =
      std::move(operator_production_node.operator_associatity_type_);
  operator_priority_level_ =
      std::move(operator_production_node.operator_priority_level_);
  return *this;
}

LexicalGenerator::Core& LexicalGenerator::Core::operator=(Core&& core) {
  core_closure_available_ = std::move(core.core_closure_available_);
  core_id_ = std::move(core.core_id_);
  core_items_ = std::move(core.core_items_);
  return *this;
}

}  // namespace frontend::generator::lexicalgenerator