#include <array>
#include <cassert>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "Common/id_wrapper.h"
#include "Common/object_manager.h"
#include "Common/unordered_struct_manager.h"

#ifndef GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_
#define GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_

namespace frontend::generator::lexicalgenerator {
using frontend::common::ObjectManager;
using frontend::common::UnorderedStructManager;

struct StringHasher {
  frontend::common::StringHashType DoHash(const std::string& str) {
    return frontend::common::HashString(str);
  }
};
// TODO ���ɾ��δʹ�ò���ʽ�Ĺ���
// TODO ���ProductionNodeType::kEndNode
class LexicalGenerator {
  // Ϊ���涨��������Ͳ��ò�ʹ��ǰ������
  class BaseProductionNode;
  class ParsingTableEntry;
  class Core;

  enum class WrapperLabel {
    kCoreId,
    kPriorityLevel,
    kPointIndex,
    kParsingTableEntryId,
    kProductionBodyId
  };  // ������ID
  using CoreId = frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                                     WrapperLabel::kCoreId>;
  // ��������ȼ�
  using PriorityLevel =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPriorityLevel>;
  // ���λ��
  using PointIndex =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPointIndex>;
  // �﷨��������ĿID
  using ParsingTableEntryId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kParsingTableEntryId>;
  // ���ս�ڵ��ڲ���ʽ���
  using ProductionBodyId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kProductionBodyId>;
  // ����ʽ�ڵ�ID
  using ProductionNodeId = ObjectManager<BaseProductionNode>::ObjectId;
  // ��������ʽ��
  using ProductionBodyType = std::vector<ProductionNodeId>;
  // ����ʽ����
  using BodyContainerType = std::vector<ProductionBodyType>;
  // ����ID
  using SymbolId = UnorderedStructManager<std::string, StringHasher>::ObjectId;
  // ��ǰ���������������ڲ���������ProductionBodyId��PointIndex��ͬ������
  using InsideForwardNodesContainerType = std::unordered_set<ProductionNodeId>;
  // ��ǰ�������������м����������ProductionBodyId������
  using MiddleForwardNodesContainerType =
      std::vector<InsideForwardNodesContainerType>;
  // ��ǰ����������������������������ʽһһ��Ӧ��
  using OutSideForwardNodesContainerType =
      std::vector<MiddleForwardNodesContainerType>;
  // ��ͬ���Ӧ�ID��CoreId��
  using CoreIdContainerType = std::vector<std::vector<CoreId>>;
  // �ں��ڵ��������������������Ϊ
  // ����ʽ�ڵ�ID������ʽ��ID������ʽ���е��λ��
  using CoreItem = std::tuple<ProductionNodeId, ProductionBodyId, PointIndex>;
  // �ڵ����ͣ��ս���ţ�����������ս���ţ��ļ�β�ڵ�
  // Ϊ��֧��ClassfiyProductionNodes�������Զ������ֵ
  // ����Զ������ֵ���������ֵ������С��sizeof(ProductionNodeType)
  enum class ProductionNodeType {
    kTerminalNode,
    kOperatorNode,
    kNonTerminalNode,
    kEndNode
  };
  // �����������ͣ����ϣ��ҽ��
  enum class AssociatityType { kLeftAssociate, kRightAssociate };
  // �����������ͣ���Լ�����룬��������
  enum class ActionType { kReduction, kShift, kError, kAccept };

  // ��������ʽ�������ұ�������һ�������Ĳ���ʽ
  ProductionNodeId AnalysisProduction(const std::string& str);
  // ��ӷ��ţ����ط��ŵ�ID���Ƿ�ִ���˲��������ִ���˲�������򷵻�true
  std::pair<SymbolId, bool> AddSymbol(const std::string& str) {
    return manager_symbol_.AddObject(str);
  }
  // ��ȡ���Ŷ�ӦID
  SymbolId GetSymbolId(const std::string& str) {
    return manager_symbol_.GetObjectId(str);
  }
  // ͨ������ID��ѯ��Ӧ�ַ���
  const std::string& GetSymbolString(SymbolId symbol_id) {
    return manager_symbol_.GetObject(symbol_id);
  }
  // ���÷���ID���ڵ�ID��ӳ��
  void SetSymbolIdToProductionNodeIdMapping(SymbolId symbol_id,
                                            ProductionNodeId node_id);
  // �½��ս�ڵ㣬���ؽڵ�ID
  // �ڵ��Ѵ����Ҹ���symbol_id��ͬ������ID�򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId AddTerminalNode(SymbolId symbol_id);
  // �½�������ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId AddOperatorNode(SymbolId symbol_id,
                                   AssociatityType associatity_type,
                                   PriorityLevel priority_level);
  // �½����ս�ڵ㣬���ؽڵ�ID
  // �ڶ�������Ϊ����ʽ�壬�����ṹΪstd::initializer_list<std::vector<Object>>
  // ��std::vector<std::vector<Object>>
  template <class IdType>
  ProductionNodeId AddNonTerminalNode(SymbolId symbol_id, IdType&& bodys);
  // �½��ļ�β�ڵ㣬���ؽڵ�ID
  ProductionNodeId AddEndNode(SymbolId symbol_id = SymbolId::InvalidId());
  // ��ȡ�ڵ�
  BaseProductionNode& GetProductionNode(ProductionNodeId id) {
    return manager_nodes_[id];
  }
  // �����ս�ڵ���Ӳ���ʽ��
  template <class IdType>
  void AddNonTerminalNodeBody(ProductionNodeId node_id, IdType&& body) {
    static_cast<NonTerminalProductionNode*>(GetProductionNode(node_id))
        ->AddBody(std::forward<IdType>(body));
  }
  // ͨ������ID��ѯ��Ӧ���нڵ��ID���������򷵻ؿ�vector
  std::vector<ProductionNodeId> GetSymbolToProductionNodeIds(
      SymbolId symbol_id);
  // ��ȡcore_id��Ӧ����ʽ�
  Core& GetCore(CoreId core_id) {
    assert(core_id < cores_.size());
    return cores_[core_id];
  }
  // ����µĺ���
  CoreId AddNewCore() {
    CoreId core_id = CoreId(cores_.size());
    cores_.emplace_back();
    cores_.back().SetCoreId(core_id);
    return core_id;
  }
  // ItemsҪ��Ϊstd::vector<CoreItem>��std::initialize_list<CoreItem>
  // ForwardNodesҪ��Ϊstd::vector<Object>��std::initialize_list<Object>
  template <class Items, class ForwardNodes>
  CoreId AddNewCore(Items&& items, ForwardNodes&& forward_nodes);
  // ������Ӧ���ӳ�䣬�Ḳ��ԭ�м�¼
  void SetItemCoreId(const CoreItem& core_item, CoreId core_id);
  // �����������Ҫʹ��core.AddItem()���ᵼ���޷�����ӳ���¼
  // ��������ӦBaseProductionNode�и��CoreId�ļ�¼
  std::pair<std::set<CoreItem>::iterator, bool> AddItemToCore(
      const CoreItem& core_item, CoreId core_id) {
    assert(core_id != CoreId::InvalidId());
    auto iterator_and_state = GetCore(core_id).AddItem(core_item);
    if (iterator_and_state.second == true) {
      // ��ǰ�����������ӳ���¼
      SetItemCoreId(core_item, core_id);
    }
    return iterator_and_state;
  }
  // ��ȡ�������Ӧ���ID��ʹ�õĲ�������Ϊ��Ч����
  // δָ��������ʹ��SetItemCoreId���ã��򷵻�CoreId::InvalidId()
  CoreId GetCoreId(ProductionNodeId production_node_id,
                   ProductionBodyId production_body_id,
                   PointIndex point_index) {
    return GetProductionNode(production_node_id)
        .GetCoreId(production_body_id, point_index);
  }
  CoreId GetItemCoreId(const CoreItem& core_item);
  // ��ȡ�������Ӧ���ID����������ڼ�¼����뵽����ﲢ������ӦID
  // ���ص�bool�����Ƿ�core_item�����ڲ��ɹ���������ӵ��µ��
  // insert_core_id�ǲ����ڼ�¼ʱӦ�����Core��ID
  // ��ΪCoreId::InvalidId()�����½�Core
  std::pair<CoreId, bool> GetItemCoreIdOrInsert(
      const CoreItem& core_item, CoreId insert_core_id = CoreId::InvalidId());
  // ��ȡ��ǰ�����ż�
  const InsideForwardNodesContainerType& GetFowardNodes(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index) {
    return GetProductionNode(production_node_id)
        .GetForwardNodeIds(production_body_id, point_index);
  }
  // �����ǰ������
  void AddForwardNode(ProductionNodeId production_node_id,
                      ProductionBodyId production_body_id,
                      PointIndex point_index,
                      ProductionNodeId forward_node_id) {
    GetProductionNode(production_node_id)
        .AddForwardNode(production_body_id, point_index, forward_node_id);
  }
  // ���һ�������ڵ�������ǰ������
  template <class Container>
  void AddForwardNodeContainer(ProductionNodeId production_node_id,
                               ProductionBodyId production_body_id,
                               PointIndex point_index,
                               Container&& forward_nodes_container) {
    GetProductionNode(production_node_id)
        .AddForwardNodeContainer(
            production_body_id, point_index,
            std::forward<Container>(forward_nodes_container));
  }
  // �հ������е�first���������first_node_id��Ч�򷵻ؽ����Լ��ļ���
  // ��Ч�򷵻�next_node_ids
  std::unordered_set<ProductionNodeId> First(
      ProductionNodeId first_node_id,
      const std::unordered_set<ProductionNodeId>& next_node_ids);
  // ��ȡ���ĵ�ȫ����
  const std::set<CoreItem>& GetCoreItems(CoreId core_id) {
    return GetCore(core_id).GetItems();
  }
  // ��ȡ�����﷨��������Ŀ
  ParsingTableEntry& GetParsingTableEntry(ParsingTableEntryId id) {
    assert(id < lexical_config_parsing_table_.size());
    return lexical_config_parsing_table_[id];
  }
  // ���ø�����ʽID
  void SetRootProductionId(ProductionNodeId root_production) {
    root_production_ = root_production;
  }
  // ������հ���Ч������ÿ��Goto����հ���
  void SetCoreClosureAvailable(CoreId core_id) {
    GetCore(core_id).SetClosureAvailable();
  }
  bool IsClosureAvailable(CoreId core_id) {
    return GetCore(core_id).IsClosureAvailable();
  }
  // �Ը�����������հ����������ԭλ��
  void CoreClosure(CoreId core_id);
  // ɾ��Goto�����и���core_id�����л��棬����ִ�к�֤���治����
  void RemoveGotoCacheEntry(CoreId core_id) {
    assert(core_id != CoreId::InvalidId());
    auto [iter_begin, iter_end] = core_id_goto_core_id_.equal_range(core_id);
    for (; iter_begin != iter_end; ++iter_begin) {
      core_id_goto_core_id_.erase(iter_begin);
    }
  }
  // ����һ��Goto����
  void SetGotoCacheEntry(CoreId core_id_src,
                         ProductionNodeId transform_production_node_id,
                         CoreId core_id_dst) {
    assert(core_id_src != CoreId::InvalidId() &&
           core_id_dst != CoreId::InvalidId());
    auto [iter_begin, iter_end] =
        core_id_goto_core_id_.equal_range(core_id_src);
    for (; iter_begin != iter_end; ++iter_end) {
      // ������л��������޸�
      if (iter_begin->second.first == transform_production_node_id) {
        iter_begin->second.second = core_id_dst;
        return;
      }
    }
    // �����ڸ������������
    core_id_goto_core_id_.insert(std::make_pair(
        core_id_src,
        std::make_pair(transform_production_node_id, core_id_dst)));
  }
  // ��ȡһ��Goto���棬�������򷵻�CoreId::InvalidId()
  CoreId GetGotoCacheEntry(CoreId core_id_src,
                           ProductionNodeId transform_production_node_id) {
    assert(core_id_src != CoreId::InvalidId());
    CoreId core_id_dst = CoreId::InvalidId();
    auto [iter_begin, iter_end] =
        core_id_goto_core_id_.equal_range(core_id_src);
    for (; iter_begin != iter_end; ++iter_begin) {
      if (iter_begin->second.first == transform_production_node_id) {
        core_id_dst = iter_begin->second.second;
        break;
      }
    }
    return core_id_dst;
  }
  // Goto�����Ƿ���Ч����Ч����true
  bool IsGotoCacheAvailable(CoreId core_id) {
    return GetCore(core_id).IsClosureAvailable();
  }
  // ��ȡ������һ������ʽ�ڵ�ID������������򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId GetProductionBodyNextNodeId(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index);
  // ��ȡ�������ǰ���ڵ�ID������������򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId GetProductionBodyNextNextNodeId(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index);
  // Goto���ӹ��̣�����ItemGoto���ID���Ƿ����
  // ������������еļ�¼����뵽insert_core_id��
  // ��Ҫ����ʱinsert_core_idΪCoreId::InvalidId()�򴴽��µ�Core������
  // ����Goto���CoreId���Ƿ�ִ������core�в������
  // ����ڸ����������޷�ִ�з���CoreId::InvalidId()��false
  std::pair<CoreId, bool> ItemGoto(
      const CoreItem& item, ProductionNodeId transform_production_node_id,
      CoreId insert_core_id = CoreId::InvalidId());
  // ��ȡ�Ӹ��������ʼ������node_id��Ӧ�Ľڵ�󵽴�ĺ�����
  // �Զ����������ڵĺ�������Goto�Ľ��Ϊ���򷵻�CoreId::InvalidId()
  // ���صĵڶ������������ص�Core�Ƿ����½��ģ��½���Ϊtrue
  std::pair<CoreId, bool> Goto(CoreId core_id_src,
                               ProductionNodeId transform_production_node_id);
  // ������ǰ������
  void SpreadLookForwardSymbol(CoreId core_id);
  // �����в���ʽ�ڵ㰴��ProductionNodeType����
  // ����array�ڵ�vector�����Ͷ�Ӧ���±�ΪProductionNodeType�����͵�ֵ
  std::array<std::vector<ProductionNodeId>, sizeof(ProductionNodeType)>
  ClassifyProductionNodes();
  // ������﷨��������ĿID��ӳ��
  void SetItemToParsingEntryIdMapping(const CoreItem& item,
                                      ParsingTableEntryId entry_id) {
    item_to_parsing_table_entry_id_[item] = entry_id;
  }
  // ��ȡ�Ѵ洢����﷨��������ĿID��ӳ��
  // �������򷵻�ParsingTableEntryId::InvalidId()
  ParsingTableEntryId GetItemToParsingEntryId(const CoreItem& item);
  // ParsingTableMergeOptimize���ӹ��̣����������ͬ�ս�ڵ�����﷨��������Ŀ
  // ��equivalent_idsд����ͬ�ս�ڵ�ת�Ʊ�Ľڵ�ID���飬����ִ��ʵ�ʺϲ�����
  // ����д��ֻ��һ�������
  // entry_ids����ӦΪstd::vector<ParsingTableEntryId>
  template <class ParsingTableEntryIdContainer>
  void ParsingTableTerminalNodeClassify(
      const std::vector<ProductionNodeId>& terminal_node_ids, size_t index,
      ParsingTableEntryIdContainer&& entry_ids,
      std::vector<std::vector<ParsingTableEntryId>>* equivalent_ids);
  // ParsingTableMergeOptimize���ӹ��̣����������ͬ���ս�ڵ�����﷨��������Ŀ
  // ��equivalent_idsд����ͬ���ս�ڵ�ת�Ʊ�Ľڵ�ID���飬����ִ��ʵ�ʺϲ�����
  // ����д��ֻ��һ�������
  // entry_ids����ӦΪstd::vector<ParsingTableEntryId>
  template <class ParsingTableEntryIdContainer>
  void ParsingTableNonTerminalNodeClassify(
      const std::vector<ProductionNodeId>& nonterminal_node_ids, size_t index,
      ParsingTableEntryIdContainer&& entry_ids,
      std::vector<std::vector<ParsingTableEntryId>>* equivalent_ids);
  // ParsingTableMergeOptimize���ӹ��̣����������ͬ����﷨��������Ŀ
  //��һ������Ϊ�﷨�������������ս�ڵ�ID
  //�ڶ�������Ϊ�﷨�����������з��ս�ڵ�ID
  //���ؿ��Ժϲ����﷨��������Ŀ�飬�������������������Ŀ
  std::vector<std::vector<ParsingTableEntryId>> ParsingTableEntryClassify(
      const std::vector<ProductionNodeId>& terminal_node_ids,
      const std::vector<ProductionNodeId>& nonterminal_node_ids);
  // ���ݸ����ı�����ӳ���﷨��������ID
  void RemapParsingTableEntryId(
      const std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>&
          moved_entry_to_new_entry_id);
  // �ϲ��﷨����������ͬ���ͬʱ�����﷨�������С
  // �Ὣ�﷨�������ڵ����޸�Ϊ�µ���
  void ParsingTableMergeOptimize();
  // ����LALR(1)����
  void ParsingTableConstruct();

 private:
  // ������ʽID
  ProductionNodeId root_production_;
  // �����ս���š����ս���ŵȵĽڵ�
  ObjectManager<BaseProductionNode> manager_nodes_;
  // �������
  UnorderedStructManager<std::string, StringHasher> manager_symbol_;
  // ����ID��manager_symbol_)���ص�ID����Ӧ�ڵ��ӳ�䣬֧��һ�����Ŷ�Ӧ����ڵ�
  std::unordered_multimap<SymbolId, ProductionNodeId> symbol_id_to_node_id_;

  // ��﷨��������ĿID��ӳ��
  std::map<CoreItem, ParsingTableEntryId> item_to_parsing_table_entry_id_;
  // �ں�+���ں��
  std::vector<Core> cores_;
  // Goto���棬�洢���е�CoreI�ڲ�ͬ������Goto�Ľڵ�
  std::multimap<CoreId, std::pair<ProductionNodeId, CoreId>>
      core_id_goto_core_id_;
  // �﷨������
  std::vector<ParsingTableEntry> lexical_config_parsing_table_;

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
          base_symbol_id_(base_production_node.base_symbol_id_),
          base_forward_nodes_(
              std::move(base_production_node.base_forward_nodes_)),
          base_core_ids_(std::move(base_production_node.base_id_)) {}
    BaseProductionNode& operator=(BaseProductionNode&& base_production_node);

    void SetType(ProductionNodeType type) { base_type_ = type; }
    ProductionNodeType Type() const { return base_type_; }
    void SetThisNodeId(ProductionNodeId id) { base_id_ = id; }
    ProductionNodeId Id() const { return base_id_; }
    void SetSymbolId(SymbolId id) { base_symbol_id_ = id; }
    SymbolId GetSymbolId() const { return base_symbol_id_; }

    // ��ȡ��ǰ���ڵ�ID����
    const InsideForwardNodesContainerType& GetForwardNodeIds(
        ProductionBodyId production_body_id, PointIndex point_index) const {
      assert(point_index < base_forward_nodes_[production_body_id].size());
      return base_forward_nodes_[production_body_id][point_index];
    }
    // �����ǰ���ڵ�ID
    void AddForwardNode(ProductionBodyId production_body_id,
                        PointIndex point_index,
                        ProductionNodeId forward_node_id) {
      assert(point_index < base_forward_nodes_[production_body_id].size() &&
             forward_node_id.IsValid());
      base_forward_nodes_[production_body_id][point_index].insert(
          forward_node_id);
    }
    // �����ǰ���ڵ�ID����
    template <class Container>
    void AddForwardNodeContainer(ProductionBodyId production_body_id,
                                 PointIndex point_index,
                                 Container&& forward_nodes_container) {
      assert(point_index < base_forward_nodes_[production_body_id].size());
      base_forward_nodes_[production_body_id][point_index].merge(
          std::forward<Container>(forward_nodes_container));
    }
    // ���ø������Ӧ���ID
    void SetCoreId(ProductionBodyId production_body_id, PointIndex point_index,
                   CoreId core_id) {
      assert(production_body_id < base_core_ids_.size() &&
             point_index < base_core_ids_[production_body_id].size());
      base_core_ids_[production_body_id][point_index] = core_id;
    }
    // ��ȡ�������Ӧ���ID
    CoreId GetCoreId(ProductionBodyId production_body_id,
                     PointIndex point_index) {
      assert(production_body_id < base_core_ids_.size() &&
             point_index < base_core_ids_[production_body_id].size());
      return base_core_ids_[production_body_id][point_index];
    }
    // ���ò���ʽ���С
    void ResizeProductionBody(size_t new_size) {
      base_forward_nodes_.resize(new_size);
      base_core_ids_.resize(new_size);
    }
    // ���ò���ʽ���ڽڵ�������production_body.size()����Ҫ+1��
    void ResizeProductionBodyInsideNodes(ProductionBodyId production_node_id,
                                         size_t new_size) {
      assert(production_node_id < base_forward_nodes_.size());
      base_forward_nodes_[production_node_id].resize(new_size + 1);
      base_core_ids_[production_node_id].resize(new_size + 1,
                                                CoreId::InvalidId());
    }
    // ��ȡ�������Ӧ�Ĳ���ʽID������point_index�����ID
    // point_indexԽ��ʱ����ProducNodeId::InvalidId()
    // Ϊ��֧����ǰ������ڵ�����Խ��
    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index);

   private:
    // �ڵ�����
    ProductionNodeType base_type_;
    // �ڵ�ID
    ProductionNodeId base_id_;
    // �ڵ����ID
    SymbolId base_symbol_id_;
    // ��ǰ���ڵ㣬��������ɣ���������һ��
    // �м���ӦProductionBodyId�����ڲ���ʽ����Ŀ
    // ���ڲ��ӦPointIndex����С�ȶ�Ӧ����ʽ���ڽڵ���Ŀ��1
    OutSideForwardNodesContainerType base_forward_nodes_;
    // �洢���Ӧ���ID��CoreId��
    CoreIdContainerType base_core_ids_;
  };

  class TerminalProductionNode : public BaseProductionNode {
   public:
    TerminalProductionNode(SymbolId symbol_id)
        : BaseProductionNode(ProductionNodeType::kTerminalNode, symbol_id) {
      ResizeProductionBody(1);
      ResizeProductionBodyInsideNodes(ProductionBodyId(0), 1);
    }
    TerminalProductionNode(const TerminalProductionNode&) = delete;
    TerminalProductionNode& operator=(const TerminalProductionNode&) = delete;
    TerminalProductionNode(TerminalProductionNode&& terminal_production_node)
        : BaseProductionNode(std::move(terminal_production_node)) {}
    TerminalProductionNode& operator=(
        TerminalProductionNode&& terminal_production_node) {
      BaseProductionNode::operator=(std::move(terminal_production_node));
      return *this;
    }

    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) override;
  };

  class OperatorProductionNode : public TerminalProductionNode {
   public:
    OperatorProductionNode(SymbolId symbol_id, AssociatityType associatity_type,
                           PriorityLevel priority_level)
        : TerminalProductionNode(symbol_id),
          operator_associatity_type_(associatity_type),
          operator_priority_level_(priority_level) {}
    OperatorProductionNode(const OperatorProductionNode&) = delete;
    OperatorProductionNode& operator=(const OperatorProductionNode&) = delete;
    OperatorProductionNode(OperatorProductionNode&& operator_production_node)
        : TerminalProductionNode(std::move(operator_production_node)),
          operator_associatity_type_(
              operator_production_node.operator_associatity_type_),
          operator_priority_level_(
              operator_production_node.operator_priority_level_) {}
    OperatorProductionNode& operator=(
        OperatorProductionNode&& operator_production_node);

    void SetAssociatityType(AssociatityType type) {
      operator_associatity_type_ = type;
    }
    AssociatityType GetAssociatityType() const {
      return operator_associatity_type_;
    }
    void SetPriorityLevel(PriorityLevel level) {
      operator_priority_level_ = level;
    }
    PriorityLevel GetPriorityLevel() const { return operator_priority_level_; }

   private:
    // ����������
    AssociatityType operator_associatity_type_;
    // ��������ȼ�
    PriorityLevel operator_priority_level_;
  };

  class NonTerminalProductionNode : public BaseProductionNode {
   public:
    NonTerminalProductionNode(SymbolId symbol_id)
        : BaseProductionNode(ProductionNodeType::kNonTerminalNode, symbol_id) {}
    template <class IdType>
    NonTerminalProductionNode(SymbolId symbol_id, IdType&& body)
        : BaseProductionNode(ProductionNodeType::kNonTerminalNode, symbol_id),
          nonterminal_bodys_(std::forward<IdType>(body)) {}
    NonTerminalProductionNode(const NonTerminalProductionNode&) = delete;
    NonTerminalProductionNode& operator=(const NonTerminalProductionNode&) =
        delete;
    NonTerminalProductionNode(
        NonTerminalProductionNode&& nonterminal_production_node)
        : BaseProductionNode(std::move(nonterminal_production_node)),
          nonterminal_bodys_(nonterminal_production_node.nonterminal_bodys_) {}
    NonTerminalProductionNode& operator=(
        NonTerminalProductionNode&& nonterminal_production_node);

    template <class IdType>
    ProductionBodyId AddBody(IdType&& body);
    const ProductionBodyType& GetBody(ProductionBodyId body_id) const {
      return nonterminal_bodys_[body_id];
    }
    const BodyContainerType& GetAllBody() const { return nonterminal_bodys_; }

    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) override;

   private:
    // ���vector��÷��ս�����¸�������ʽ����
    // �ڲ�vector��ò���ʽ�����ﺬ�еĸ������Ŷ�Ӧ�Ľڵ�ID
    BodyContainerType nonterminal_bodys_;
  };

  // �ļ�β�ڵ�
  class EndNode : public BaseProductionNode {
   public:
    EndNode(SymbolId symbol_id)
        : BaseProductionNode(ProductionNodeType::kEndNode, symbol_id) {}
    EndNode(const EndNode&) = delete;
    EndNode& operator=(const EndNode&) = delete;
    EndNode(EndNode&& end_node) : BaseProductionNode(std::move(end_node)) {}
    EndNode& operator=(EndNode&& end_node) {
      BaseProductionNode::operator=(std::move(end_node));
      return *this;
    }
  };
  // �����ǰ������
  class Core {
   public:
    Core() {}
    template <class Items, class ForwardNodes>
    Core(Items&& items, ForwardNodes&& forward_nodes)
        : core_closure_available_(false),
          core_id_(CoreId::InvalidId()),
          core_items_(std::forward<Items>(items)) {}
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;
    Core(Core&& core)
        : core_closure_available_(std::move(core.core_closure_available_)),
          core_id_(std::move(core.core_id_)),
          core_items_(std::move(core.core_items_)) {}
    Core& operator=(Core&& core);
    // ��Ӧֱ��ʹ�ã��ᵼ���޷�����item��CoreId��ӳ��
    // ���ظ���Item������iterator��bool
    // bool�ڲ����ڸ���item�Ҳ���ɹ�ʱΪtrue
    std::pair<std::set<CoreItem>::iterator, bool> AddItem(
        const CoreItem& item) {
      SetClosureNotAvailable();
      return core_items_.insert(item);
    }
    // �жϸ���item�Ƿ��ڸ���ڣ����򷵻�true
    bool IsItemIn(const CoreItem& item) const {
      return core_items_.find(item) != core_items_.end();
    }
    bool IsClosureAvailable() const { return core_closure_available_; }
    // ����core_id
    void SetCoreId(CoreId core_id) { core_id_ = core_id; }
    CoreId GetCoreId() const { return core_id_; }
    // ���ø����ıհ���Ч����Ӧ�ɱհ���������
    void SetClosureAvailable() { core_closure_available_ = true; }
    // ���ø������ıհ���Ч��Ӧ��ÿ���޸���core_items_�ĺ�������
    void SetClosureNotAvailable() { core_closure_available_ = false; }
    const std::set<CoreItem>& GetItems() const { return core_items_; }
    size_t Size() { return core_items_.size(); }

   private:
    // �����ıհ��Ƿ���Ч������հ���֮��û�����κθ�����Ϊtrue��
    bool core_closure_available_ = false;
    // �ID
    CoreId core_id_;
    // �
    std::set<CoreItem> core_items_;
  };

  // �﷨��������Ŀ
  class ParsingTableEntry {
   public:
    using TerminalNodeActionAndTargetContainerType = std::unordered_map<
        ProductionNodeId,
        std::pair<ActionType,
                  std::variant<ParsingTableEntryId,
                               std::pair<ProductionNodeId, ProductionBodyId>>>>;

    ParsingTableEntry() {}
    ParsingTableEntry(const ParsingTableEntry&) = delete;
    ParsingTableEntry& operator=(const ParsingTableEntry&) = delete;
    ParsingTableEntry(ParsingTableEntry&& parsing_table_entry)
        : action_and_target_(std::move(parsing_table_entry.action_and_target_)),
          nonterminal_node_transform_table_(std::move(
              parsing_table_entry.nonterminal_node_transform_table_)) {}
    ParsingTableEntry& operator=(ParsingTableEntry&& parsing_table_entry);

    // ���øò���ʽ��ת�������µĶ�����Ŀ��ڵ�
    void SetTerminalNodeActionAndTarget(
        ProductionNodeId node_id, ActionType action_type,
        std::variant<ParsingTableEntryId,
                     std::pair<ProductionNodeId, ProductionBodyId>>
            target_id) {
      action_and_target_[node_id] = std::make_pair(action_type, target_id);
    }
    // ���ø���Ŀ������ս�ڵ��ת�Ƶ��Ľڵ�
    void SetNonTerminalNodeTransformId(ProductionNodeId node_id,
                                       ParsingTableEntryId id) {
      nonterminal_node_transform_table_[node_id] = id;
    }
    // ���ʸ���Ŀ�¸���ID�ս�ڵ����Ϊ��Ŀ��ID
    const TerminalNodeActionAndTargetContainerType::mapped_type& AtTerminalNode(
        ProductionNodeId node_id) {
      auto iter = action_and_target_.find(node_id);
      assert(iter != action_and_target_.end());
      return iter->second;
    }
    // ���ʸ���Ŀ�¸���ID���ս�ڵ�����ʱת�Ƶ�����ĿID
    ParsingTableEntryId ShiftNonTerminalNode(ProductionNodeId node_id) {
      auto iter = nonterminal_node_transform_table_.find(node_id);
      assert(iter != nonterminal_node_transform_table_.end());
      return iter->second;
    }
    // ��ȡȫ���ս�ڵ�Ĳ���
    const TerminalNodeActionAndTargetContainerType&
    GetAllTerminalNodeActionAndTarget() {
      return action_and_target_;
    }
    // ��ȡȫ�����ս�ڵ�ת�Ƶ��ı���
    const std::unordered_map<ProductionNodeId, ParsingTableEntryId>&
    GetAllNonTerminalNodeTransformTarget() {
      return nonterminal_node_transform_table_;
    }

   private:
    // ��ǰ������ID�µĲ�����Ŀ��ڵ�
    // ����Ϊ����ʱvariant�������ת�Ƶ���ID��ProductionBodyId��
    // ����Ϊ��Լʱvariant��ʹ�õĲ���ʽID�Ͳ���ʽ��ID��ProductionNodeId��
    // ����Ϊ���ܺͱ���ʱvariantδ����
    TerminalNodeActionAndTargetContainerType action_and_target_;
    // ������ս�ڵ��ת�Ƶ��Ĳ���ʽ�����
    std::unordered_map<ProductionNodeId, ParsingTableEntryId>
        nonterminal_node_transform_table_;
  };
};

template <class IdType>
inline LexicalGenerator::ProductionBodyId
LexicalGenerator::NonTerminalProductionNode::AddBody(IdType&& body) {
  ProductionBodyId body_id = nonterminal_bodys_.size();
  // ��������뵽����ʽ�������У���ɾ����ͬ����ʽ����
  nonterminal_bodys_.emplace_back(std::forward<IdType>(body));
  // Ϊ����ʽ������в�ͬλ�õĵ��Ӧ����ǰ�����������ռ�
  ResizeProductionBody(nonterminal_bodys_.size());
  // Ϊ��ͬ���λ�������ռ�
  ResizeProductionBodyInsideNodes(body.size());
  return body_id;
}

template <class IdType>
inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddNonTerminalNode(
    SymbolId symbol_id, IdType&& bodys) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<NonTerminalProductionNode>(
          ProductionNodeType::kNonTerminalNode, symbol_id,
          std::forward<IdType>(bodys));
  manager_nodes_.GetObject(node_id).SetThisNodeId(node_id);
  return node_id;
}

template <class Items, class ForwardNodes>
inline LexicalGenerator::CoreId LexicalGenerator::AddNewCore(
    Items&& items, ForwardNodes&& forward_nodes) {
  CoreId core_id = cores_.size();
  cores_.emplace_back(std::forward<Items>(items),
                      std::forward<ForwardNodes>(forward_nodes));
  return core_id;
}

template <class ParsingTableEntryIdContainer>
inline void LexicalGenerator::ParsingTableTerminalNodeClassify(
    const std::vector<ProductionNodeId>& terminal_node_ids, size_t index,
    ParsingTableEntryIdContainer&& entry_ids,
    std::vector<std::vector<ParsingTableEntryId>>* equivalent_ids) {
  if (index >= terminal_node_ids.size()) {
    // ������ɣ�ִ�кϲ�����
    assert(entry_ids.size() > 1);
    equivalent_ids->emplace_back(
        std::forward<ParsingTableEntryIdContainer>(entry_ids));
  } else {
    assert(GetProductionNode(terminal_node_ids[index]).Type() ==
               ProductionNodeType::kTerminalNode ||
           GetProductionNode(terminal_node_ids[index]).Type() ==
               ProductionNodeType::kOperatorNode);
    // ���������ת�������µ�ת�ƽ������
    std::map<const ParsingTableEntry::TerminalNodeActionAndTargetContainerType::
                 mapped_type,
             std::vector<ParsingTableEntryId>>
        classify_table;
    ProductionNodeId transform_id = terminal_node_ids[index];
    for (auto id : entry_ids) {
      // ����map���з���
      classify_table[GetParsingTableEntry(id).AtTerminalNode(transform_id)]
          .push_back(id);
    }
    size_t next_index = index + 1;
    for (auto vec : classify_table) {
      if (vec.second.size() > 1) {
        // ���ຬ�ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
        ParsingTableTerminalNodeClassify(terminal_node_ids, next_index,
                                         std::move(vec.second), equivalent_ids);
      }
    }
  }
}

template <class ParsingTableEntryIdContainer>
inline void LexicalGenerator::ParsingTableNonTerminalNodeClassify(
    const std::vector<ProductionNodeId>& nonterminal_node_ids, size_t index,
    ParsingTableEntryIdContainer&& entry_ids,
    std::vector<std::vector<ParsingTableEntryId>>* equivalent_ids) {
  if (index >= nonterminal_node_ids.size()) {
    // ������ɣ�ִ�кϲ�����
    assert(entry_ids.size() > 1);
    equivalent_ids->emplace_back(
        std::forward<ParsingTableEntryIdContainer>(entry_ids));
  } else {
    assert(GetProductionNode(nonterminal_node_ids[index]).Type() ==
           ProductionNodeType::kNonTerminalNode);
    // ���������ת�������µ�ת�ƽ������
    std::map<const ParsingTableEntryId, std::vector<ParsingTableEntryId>>
        classify_table;
    ProductionNodeId transform_id = nonterminal_node_ids[index];
    for (auto id : entry_ids) {
      // ����map���з���
      classify_table[GetParsingTableEntry(id).ShiftNonTerminalNode(
                         transform_id)]
          .push_back(id);
    }
    size_t next_index = index + 1;
    for (auto vec : classify_table) {
      if (vec.second.size() > 1) {
        // ���ຬ�ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
        ParsingTableTerminalNodeClassify(nonterminal_node_ids, next_index,
                                         std::move(vec.second), equivalent_ids);
      }
    }
  }
}

}  // namespace frontend::generator::lexicalgenerator
#endif  // !GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_