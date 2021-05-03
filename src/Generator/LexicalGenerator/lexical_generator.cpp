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
      manager_nodes_.EmplaceObject<TerminalProductionNode>(
          ProductionNodeType::kTerminalNode, symbol_id);
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  return node_id;
}

inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddOperatorNode(
    SymbolId symbol_id, AssociatityType associatity_type,
    PriorityLevel priority_level) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<OperatorProductionNode>(
          ProductionNodeType::kOperatorNode, symbol_id, associatity_type,
          priority_level);
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
    //�������
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
    //�հ���Ч����������
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
      //���Ѿ�������ˣ�û����һ���ڵ�
      continue;
    }
    NonTerminalProductionNode& next_production_node =
        static_cast<NonTerminalProductionNode&>(
        GetProductionNode(next_production_node_id));
    if (next_production_node.Type() != ProductionNodeType::kNonTerminalNode) {
      //���Ƿ��ս�ڵ㣬�޷�չ��
      continue;
    }
    ProductionNodeId next_next_production_node_id =
        GetProductionBodyNextNextNodeId(production_node_id, production_body_id,
                                        point_index);
    const std::unordered_set<ProductionNodeId>& forward_nodes_second_part =
        GetFowardNodes(production_node_id, production_body_id, point_index);
    //��ȡ�����ɽڵ��������ǰ���ڵ�
     InsideForwardNodesContainerType forward_nodes =
        First(next_next_production_node_id, forward_nodes_second_part);
    const BodyContainerType& bodys = next_production_node.GetAllBody();
    for (size_t i = 0; i <bodys.size(); i++) {
      ProductionBodyId body_id(i);
      PointIndex point_index(0);
      auto[iter_item,inserted] = AddItemToCore(
          CoreItem(next_production_node_id, body_id,point_index),
          core_id);
      AddForwardNodeContainer(next_production_node_id, body_id, point_index,
                              forward_nodes);
      if (inserted) {
        //�²�����item��������еȴ�����
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
  //������Ч
  if (map_cache_available) {
    return std::make_pair(
        GetGotoCacheEntry(core_id_src, transform_production_node_id), false);
  }
  //������Ч�������core_id��Ӧ���еĻ���
  RemoveGotoCacheEntry(core_id_src);
  //�Ƿ��½�core��־
  bool core_constructed = false;
  CoreId core_id_dst = CoreId::InvalidId();
  CoreClosure(core_id_src);
  const auto& items = GetCoreItems(core_id_src);
  auto iter = items.begin();
  //�ҵ���һ���ɹ�Goto�����¼CoreId���Ƿ��½���Core
  while (iter != items.end()) {
    std::pair(core_id_dst, core_constructed) =
        ItemGoto(*iter, transform_production_node_id);
    if (core_id_dst.IsValid()) {
      break;
    }
    ++iter;
  }
  if (core_constructed == true) {
    //�������µ������Ҫ��ʣ�µ����ItemGoto�������Բ��������뵽�µ��
    assert(core_id_dst.IsValid());
    while (iter != items.end()) {
#ifdef _DEBUG
      //�����Ƿ�ÿһ����Goto���������ͬ��
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
  //�����Ƿ�ÿһ����Goto���������ͬ��
  else {
    while (iter!=items.end()) {
      auto [core_id, constructed] =
          ItemGoto(*iter, transform_production_node_id, core_id_dst);
      assert(core_id == core_id_dst && constructed == false);
      ++iter;
    }
  }
#endif  // _DEBUG
  //���û���
  SetGotoCacheEntry(core_id_src, transform_production_node_id, core_id_dst);
  return std::make_pair(core_id_dst, core_constructed);
}

void LexicalGenerator::SpreadLookForwardSymbol(
    CoreId core_id, ProductionNodeId transform_production_node_id) {
  const auto& items = GetCoreItems(core_id);
  for (auto& item : items) {
    auto [production_node_id, production_body_id, point_index] = item;
    //���ÿһ�����ܵ���һ�����Ŷ�ִ��һ��Goto��ʹ���ڴ�����ǰ������ǰ
    //���п����õ��ĵ����ĳ����Ӧ���
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
      //������ǰ������
      AddForwardNodeContainer(production_node_id, production_body_id,
                              PointIndex(point_index + 1),
                              std::move(forward_nodes));
    }
  }
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

}  // namespace frontend::generator::lexicalgenerator