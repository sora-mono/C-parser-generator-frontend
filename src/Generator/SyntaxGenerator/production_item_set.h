#ifndef GENERATOR_SYNTAXGENERATOR_PRODUCTION_ITEM_SET_H_
#define GENERATOR_SYNTAXGENERATOR_PRODUCTION_ITEM_SET_H_

#include <unordered_set>

#include "Generator/export_types.h"
namespace frontend::generator::syntax_generator {
// �����ǰ������
class ProductionItemSet {
 public:
  // ��ڵ�������ݴ���������Ϊ
  // ����ʽ�ڵ�ID������ʽ��ID����һ������ĵ��ʵ�λ��
  using ProductionItem =
      std::tuple<ProductionNodeId, ProductionBodyId, NextWordToShiftIndex>;
  // ��ǰ����������
  using ForwardNodesContainer = std::unordered_set<ProductionNodeId>;
  // ������ϣProductionItem����
  // ��SyntaxGenerator�����������ø����ػ�std::hash<ProductionItem>
  // ������ProductionItem������Ϊstd::unordered_map��ֵ
  struct ProductionItemHasher {
    size_t operator()(const ProductionItem& production_item) const {
      auto& [production_node_id, production_body_id, point_index] =
          production_item;
      return production_node_id * production_body_id * point_index;
    }
  };
  // �洢�����ǰ���ڵ������
  using ProductionItemAndForwardNodesContainer =
      std::unordered_map<ProductionItem, ForwardNodesContainer,
                         ProductionItemHasher>;

  ProductionItemSet() {}
  ProductionItemSet(SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id)
      : syntax_analysis_table_entry_id_(syntax_analysis_table_entry_id) {}
  template <class ItemAndForwardNodes>
  ProductionItemSet(ItemAndForwardNodes&& item_and_forward_node_ids,
                    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id)
      : production_item_set_closure_available_(false),
        syntax_analysis_table_entry_id_(syntax_analysis_table_entry_id),
        item_and_forward_node_ids_(
            std::forward<ItemAndForwardNodes>(item_and_forward_node_ids)) {}
  ProductionItemSet(const ProductionItemSet&) = delete;
  ProductionItemSet& operator=(const ProductionItemSet&) = delete;
  ProductionItemSet(ProductionItemSet&& production_item_set)
      : production_item_set_closure_available_(std::move(
            production_item_set.production_item_set_closure_available_)),
        production_item_set_id_(
            std::move(production_item_set.production_item_set_id_)),
        syntax_analysis_table_entry_id_(
            std::move(production_item_set.syntax_analysis_table_entry_id_)),
        item_and_forward_node_ids_(
            std::move(production_item_set.item_and_forward_node_ids_)) {}
  ProductionItemSet& operator=(ProductionItemSet&& production_item_set);

  // ���ظ���Item������iterator���Ƿ�ɹ�����bool���
  // ���Item�Ѵ�����������ǰ������
  // bool�ڲ����ڸ���item�Ҳ���ɹ�ʱΪtrue
  // ����ʹ�õ���δ��װID
  // ���������������ǰ�����������ñհ���Ч
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddItemAndForwardNodeIds(const ProductionItem& item,
                           ForwardNodeIdContainer&& forward_node_ids);
  // ��AddItemAndForwardNodeIds������������ӵ���Ϊ������
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddMainItemAndForwardNodeIds(const ProductionItem& item,
                               ForwardNodeIdContainer&& forward_node_ids) {
    auto result = AddItemAndForwardNodeIds(
        item, std::forward<ForwardNodeIdContainer>(forward_node_ids));
    SetMainItem(result.first);
    return result;
  }

  // �жϸ���item�Ƿ��ڸ���ڣ����򷵻�true
  bool IsItemIn(const ProductionItem& item) const {
    return item_and_forward_node_ids_.find(item) !=
           item_and_forward_node_ids_.end();
  }
  // �жϸ����ıհ��Ƿ���Ч
  bool IsClosureAvailable() const {
    return production_item_set_closure_available_;
  }
  // ����production_item_set_id
  void SetProductionItemSetId(ProductionItemSetId production_item_set_id) {
    production_item_set_id_ = production_item_set_id;
  }
  // ��ȡproduction_item_set_id
  ProductionItemSetId GetProductionItemSetId() const {
    return production_item_set_id_;
  }

  // ���ø����ıհ���Ч����Ӧ�ɱհ���������
  void SetClosureAvailable() { production_item_set_closure_available_ = true; }
  // ��ȡȫ��������
  const std::list<ProductionItemAndForwardNodesContainer::const_iterator>&
  GetMainItems() const {
    return main_items_;
  }
  // ����һ��Ϊ������
  // Ҫ��ú�����δ��ӹ�
  // ���ñհ���Ч
  void SetMainItem(
      ProductionItemAndForwardNodesContainer ::const_iterator& item_iter);
  // ��ȡȫ����Ͷ�Ӧ����ǰ���ڵ�
  const ProductionItemAndForwardNodesContainer& GetItemsAndForwardNodeIds()
      const {
    return item_and_forward_node_ids_;
  }
  // ��ȡ���Ӧ���﷨��������ĿID
  SyntaxAnalysisTableEntryId GetSyntaxAnalysisTableEntryId() const {
    return syntax_analysis_table_entry_id_;
  }

  // ��������������ǰ�����ţ�ͬʱ֧�����뵥�����źͷ�������
  // �����Ƿ����
  // Ҫ�����Ѿ����ڣ����������AddItemAndForwardNodeIds
  // ���������µ���ǰ�����������ñհ���Ч
  template <class ForwardNodeIdContainer>
  bool AddForwardNodes(const ProductionItem& item,
                       ForwardNodeIdContainer&& forward_node_id_container);
  size_t Size() const { return item_and_forward_node_ids_.size(); }

 private:
  // ��������������ǰ������
  // �����Ƿ����
  // ���������µ���ǰ�����������ñհ���Ч
  template <class ForwardNodeIdContainer>
  bool AddForwardNodes(
      const ProductionItemAndForwardNodesContainer::iterator& iter,
      ForwardNodeIdContainer&& forward_node_id_container);
  // ���ñհ���Ч
  // Ӧ��ÿ���޸�����/�����ǰ�����ŵĺ�������
  void SetClosureNotAvailable() {
    production_item_set_closure_available_ = false;
  }

  // �洢ָ�������ĵ�����
  std::list<ProductionItemAndForwardNodesContainer::const_iterator> main_items_;
  // �����ıհ��Ƿ���Ч������հ���Ϊtrue��
  bool production_item_set_closure_available_ = false;
  // �ID
  ProductionItemSetId production_item_set_id_ =
      ProductionItemSetId::InvalidId();
  // ���Ӧ���﷨��������ĿID
  SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id_ =
      SyntaxAnalysisTableEntryId::InvalidId();
  // ��Ͷ�Ӧ����ǰ������
  ProductionItemAndForwardNodesContainer item_and_forward_node_ids_;
};

template <class ForwardNodeIdContainer>
std::pair<ProductionItemSet::ProductionItemAndForwardNodesContainer::iterator,
          bool>
ProductionItemSet::AddItemAndForwardNodeIds(
    const ProductionItem& item, ForwardNodeIdContainer&& forward_node_ids) {
  // �Ѿ�����հ���������������
  assert(!IsClosureAvailable());
  auto iter = item_and_forward_node_ids_.find(item);
  if (iter == item_and_forward_node_ids_.end()) {
    auto result = item_and_forward_node_ids_.emplace(
        item, std::unordered_set<ProductionNodeId>(
                  std::forward<ForwardNodeIdContainer>(forward_node_ids)));
    if (result.second) {
      SetClosureNotAvailable();
    }
    return result;
  } else {
    bool new_forward_node_inserted = AddForwardNodes(
        iter, std::forward<ForwardNodeIdContainer>(forward_node_ids));
    if (new_forward_node_inserted) {
      SetClosureNotAvailable();
    }
    return std::make_pair(iter, false);
  }
}

template <class ForwardNodeIdContainer>
inline bool ProductionItemSet::AddForwardNodes(
    const ProductionItem& item,
    ForwardNodeIdContainer&& forward_node_id_container) {
  auto iter = item_and_forward_node_ids_.find(item);
  assert(iter != item_and_forward_node_ids_.end());
  return AddForwardNodes(
      iter, std::forward<ForwardNodeIdContainer>(forward_node_id_container));
}

template <class ForwardNodeIdContainer>
inline bool ProductionItemSet::AddForwardNodes(
    const ProductionItemAndForwardNodesContainer::iterator& iter,
    ForwardNodeIdContainer&& forward_node_id_container) {
  assert(iter != item_and_forward_node_ids_.end());
  bool result;
  if constexpr (std::is_same_v<std::decay_t<ForwardNodeIdContainer>,
                               ProductionNodeId>) {
    // �Ե�����ǰ�������ػ�
    result = iter->second
                 .emplace(std::forward<ForwardNodeIdContainer>(
                     forward_node_id_container))
                 .second;
  } else {
    // �������ػ�
    result = false;
    for (auto& forward_node_id : forward_node_id_container) {
      result |= iter->second.insert(forward_node_id).second;
    }
  }
  // ���������µ���ǰ���ڵ������ñհ���Ч
  if (result) {
    SetClosureNotAvailable();
  }
  return result;
}

}  // namespace frontend::generator::syntax_generator

#endif  // !GENERATOR_SYNTAXGENERATOR_PRODUCTION_ITEM_SET_H_