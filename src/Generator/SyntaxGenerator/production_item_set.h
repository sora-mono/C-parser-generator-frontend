/// @file production_item_set.h
/// @brief �洢�����ǰ�����ŵ���
/// @details
/// �����ڹ����﷨����������ʹ��
#ifndef GENERATOR_SYNTAXGENERATOR_PRODUCTION_ITEM_SET_H_
#define GENERATOR_SYNTAXGENERATOR_PRODUCTION_ITEM_SET_H_

#include <unordered_set>

#include "Generator/export_types.h"
namespace frontend::generator::syntax_generator {
/// @class ProductionItemSet production_item_set.h
/// @brief �洢�����ǰ�����ŵ���
class ProductionItemSet {
 public:
  /// @brief ��ڵ�������ݴ���������Ϊ
  /// ����ʽ�ڵ�ID������ʽ��ID����һ������ĵ��ʵ�λ��
  using ProductionItem =
      std::tuple<ProductionNodeId, ProductionBodyId, NextWordToShiftIndex>;
  /// @brief ��ǰ����������
  using ForwardNodesContainer = std::unordered_set<ProductionNodeId>;
  /// @brief ��ϣProductionItem����
  /// ͨ����������ProductionItem������Ϊstd::unordered_map��ֵ
  struct ProductionItemHasher {
    size_t operator()(const ProductionItem& production_item) const {
      return std::get<ProductionNodeId>(production_item) *
             std::get<ProductionBodyId>(production_item) *
             std::get<NextWordToShiftIndex>(production_item);
    }
  };
  /// @brief �洢�����ǰ���ڵ������
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

  /// @brief ����в�����Ͷ�Ӧ����ǰ������
  /// @param[in] item �����������
  /// @param[in] forward_node_ids ��������������ǰ������
  /// @return ����ָ�����Item��iterator���Ƿ����������
  /// @note
  /// ���Item�Ѵ�����������ǰ�������ҵڶ������ز���Ϊfalse
  /// ����ʹ�õ���δ��װID
  /// @attention ����ʱ�������û����հ�����Ϊ����հ����������֮��Ĺ���
  /// �����б��ٴ�����
  /// forward_node_ids���벻Ϊ�գ������һ������ǰ������
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddItemAndForwardNodeIds(const ProductionItem& item,
                           ForwardNodeIdContainer&& forward_node_ids);
  /// @brief ����в��������Ͷ�Ӧ����ǰ������
  /// @param[in] item �����������
  /// @param[in] forward_node_ids ��������������ǰ������
  /// @return ����ָ�����Item��iterator���Ƿ����������
  /// @note
  /// ���Item�Ѵ�����������ǰ�������ҵڶ������ز���Ϊfalse
  /// ����ʹ�õ���δ��װID
  /// ����հ�ʱ�Ӻ����ʼչ��
  /// @attention ����ʱ�������û����հ�����Ϊ����հ����������֮��Ĺ���
  /// �����б��ٴ�����
  /// forward_node_ids���벻Ϊ�գ������һ������ǰ������
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddMainItemAndForwardNodeIds(const ProductionItem& item,
                               ForwardNodeIdContainer&& forward_node_ids) {
    auto result = AddItemAndForwardNodeIds(
        item, std::forward<ForwardNodeIdContainer>(forward_node_ids));
    SetMainItem(result.first);
    return result;
  }

  /// @brief �жϸ������Ƿ��ڸ����
  /// @return ���ظ������Ƿ��ڸ����
  /// @retval true ���������ڸ����
  /// @retval false ��������ڸ����
  bool IsItemIn(const ProductionItem& item) const {
    return item_and_forward_node_ids_.find(item) !=
           item_and_forward_node_ids_.end();
  }
  /// @brief ��ȡ�����ıհ��Ƿ���Ч
  /// @return ���ظ����ıհ��Ƿ���Ч
  /// @retval true ������հ���Ч����������
  /// @retval false ������հ���Ч����Ҫ��հ�����ʹ��
  bool IsClosureAvailable() const {
    return production_item_set_closure_available_;
  }
  /// @brief �����ID
  /// @param[in] production_item_set_id ���ID
  void SetProductionItemSetId(ProductionItemSetId production_item_set_id) {
    production_item_set_id_ = production_item_set_id;
  }
  /// @brief ��ȡ�ID
  /// @return �����ID
  ProductionItemSetId GetProductionItemSetId() const {
    return production_item_set_id_;
  }

  /// @brief ���ø����ıհ���Ч
  /// @attention �ú�����Ӧ�ɱհ���������
  void SetClosureAvailable() { production_item_set_closure_available_ = true; }
  /// @brief ��ȡȫ��ָ�������ĵ�����
  /// @return ����ȫ��ָ�������ĵ�������const����
  const std::list<ProductionItemAndForwardNodesContainer::const_iterator>&
  GetMainItemIters() const {
    return main_items_;
  }
  /// @brief ��ȡȫ����Ͷ�Ӧ����ǰ���ڵ�
  /// @return ����ȫ����Ͷ�Ӧ����ǰ���ڵ��const����
  const ProductionItemAndForwardNodesContainer& GetItemsAndForwardNodeIds()
      const {
    return item_and_forward_node_ids_;
  }
  /// @brief ��ȡ�����Ӧ���﷨��������ĿID
  /// @return �����Ӧ���﷨��������ĿID
  SyntaxAnalysisTableEntryId GetSyntaxAnalysisTableEntryId() const {
    return syntax_analysis_table_entry_id_;
  }
  /// @brief ���ø����Ӧ���﷨��������ĿID
  /// @param[in] syntax_analysis_table_entry_id �������Ӧ���﷨��������ĿID
  void SetSyntaxAnalysisTableEntryId(
      SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id) {
    syntax_analysis_table_entry_id_ = syntax_analysis_table_entry_id;
  }
  /// @brief ��������������ǰ������
  /// @param[in] item ���������ǰ�����ŵ���
  /// @param[in] forward_node_id_container ������ӵ���ǰ������
  /// @return �����Ƿ��������ǰ������
  /// @retval true ���������ǰ������
  /// @retval false ��forward_node_id_container��ȫ����ǰ�����ž��Ѵ���
  /// @note
  /// 1.forward_node_id_container֧�����뵥����ǰ�����źʹ洢��ǰ�����ŵ�����
  /// 2.item����Ϊ��������������AddItemAndForwardNodeIds�����ƺ���
  /// 3.���������µ���ǰ�����������ñհ���Ч
  /// @attention item�����Ѿ���ӵ������
  template <class ForwardNodeIdContainer>
  bool AddForwardNodes(const ProductionItem& item,
                       ForwardNodeIdContainer&& forward_node_id_container);
  /// @brief ������зǺ�����
  void ClearNotMainItem();
  /// @brief �жϸ������Ƿ�Ϊ����ĺ�����
  /// @param[in] item �����жϵ���
  /// @return ���ظ������Ƿ�Ϊ����ĺ�������жϽ��
  /// @retval true �������Ǹ���ĺ�����
  /// @retval false ������Ǹ���ĺ�����򲻴����ڸ��
  /// @warning ʱ�临�Ӷ�O(n)��nΪ�����������
  bool IsMainItem(const ProductionItem& item);
  /// @brief ��ȡ�����ĸ���
  /// @return ������ĸ���
  size_t Size() const { return item_and_forward_node_ids_.size(); }

 private:
  /// @brief ����һ��Ϊ������
  /// @param[in] item_iter ��ָ��Ҫ������Ϊ���������ĵ�����
  /// @note
  /// Ҫ��ָ������δ�����óɺ������
  /// �Զ����ñհ���Ч
  void SetMainItem(
      ProductionItemAndForwardNodesContainer ::const_iterator& item_iter);
  /// @brief ��������������ǰ������
  /// @param[in] iter ���������ǰ�����ŵ���
  /// @param[in] forward_node_id_container ������ӵ���ǰ������
  /// @return �����Ƿ�������µ���ǰ������
  /// @retval true ���������ǰ������
  /// @retval false ��forward_node_id_container��ȫ����ǰ�����ž��Ѵ���
  /// @note
  /// 1.�����е���������ǰ���������ն�Ӧ�߸ýӿ�
  /// 2.forward_node_id_container֧�����뵥����ǰ�����źʹ洢��ǰ�����ŵ�����
  /// 3.���������µ���ǰ�����������ñհ���Ч
  template <class ForwardNodeIdContainer>
  bool AddForwardNodes(
      const ProductionItemAndForwardNodesContainer::iterator& iter,
      ForwardNodeIdContainer&& forward_node_id_container);
  /// @brief ���ñհ���Ч
  /// @note ÿ���޸�����/�����ǰ�����ŵĺ�����Ӧ���ñհ���Ч
  void SetClosureNotAvailable() {
    production_item_set_closure_available_ = false;
  }
  /// @brief ��ȡȫ��ָ�������ĵ�����
  /// @return ����ȫ��ָ�������ĵ�����������
  std::list<ProductionItemAndForwardNodesContainer::const_iterator>&
  GetMainItemIters() {
    return main_items_;
  }

  /// @brief �洢ָ�������ĵ�����
  std::list<ProductionItemAndForwardNodesContainer::const_iterator> main_items_;
  /// @brief �����ıհ��Ƿ���Ч������հ���δ���Ĺ���Ϊtrue��
  bool production_item_set_closure_available_ = false;
  /// @brief �ID
  ProductionItemSetId production_item_set_id_ =
      ProductionItemSetId::InvalidId();
  /// @brief ���Ӧ���﷨��������ĿID
  SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id_ =
      SyntaxAnalysisTableEntryId::InvalidId();
  /// @brief ��Ͷ�Ӧ����ǰ������
  ProductionItemAndForwardNodesContainer item_and_forward_node_ids_;
};

template <class ForwardNodeIdContainer>
std::pair<ProductionItemSet::ProductionItemAndForwardNodesContainer::iterator,
          bool>
ProductionItemSet::AddItemAndForwardNodeIds(
    const ProductionItem& item, ForwardNodeIdContainer&& forward_node_ids) {
  // �Ѿ�����հ���������������
  assert(!IsClosureAvailable());
  // �κ������Я����ǰ������
  assert(forward_node_ids.size() != 0);
  auto iter = item_and_forward_node_ids_.find(item);
  if (iter == item_and_forward_node_ids_.end()) {
    return item_and_forward_node_ids_.emplace(
        item, std::unordered_set<ProductionNodeId>(
                  std::forward<ForwardNodeIdContainer>(forward_node_ids)));
  } else {
    AddForwardNodes(iter,
                    std::forward<ForwardNodeIdContainer>(forward_node_ids));
    return std::make_pair(iter, false);
  }
}

template <class ForwardNodeIdContainer>
inline bool ProductionItemSet::AddForwardNodes(
    const ProductionItem& item,
    ForwardNodeIdContainer&& forward_node_id_container) {
#ifdef _DEBUG
  // ���������Ǻ�����
  bool is_main_item = false;
  for (const auto& main_item_iter : GetMainItemIters()) {
    if (main_item_iter->first == item) [[unlikely]] {
      is_main_item = true;
      break;
    }
  }
  assert(is_main_item);
#endif  // _DEBUG
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