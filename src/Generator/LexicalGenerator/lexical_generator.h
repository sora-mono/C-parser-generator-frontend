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
class LexicalGenerator {
  //Ϊ���涨��������Ͳ��ò�ʹ��ǰ������
  class BaseProductionNode;
  class ParsingTableEntry;
  class Core;

  enum class WrapperLabel {
    kCoreId,
    kPriorityLevel,
    kPointIndex,
    kParsingTableEntryId,
    kProductionBodyId
  };
  //����ʽ��ID
  using ProductionNodeId = ObjectManager<BaseProductionNode>::ObjectId;
  //���ŵ�ID
  using SymbolId = UnorderedStructManager<std::string, StringHasher>::ObjectId;
  //������ID
  using CoreId = frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                                     WrapperLabel::kCoreId>;
  //��������ȼ�
  using PriorityLevel =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPriorityLevel>;
  //���λ��
  using PointIndex =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPointIndex>;
  //�﷨��������ĿID
  using ParsingTableEntryId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kParsingTableEntryId>;
  //���ս�ڵ��ڲ���ʽ���
  using ProductionBodyId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kProductionBodyId>;
  //�ں��ڵ����
  using CoreItem = std::tuple<ProductionNodeId, ProductionBodyId, PointIndex>;
  //�ڵ����ͣ��ս���ţ�����������ս����
  enum class NodeType { kTerminalNode, kOperatorNode, kNonTerminalNode };
  //�����������ͣ����ϣ��ҽ��
  enum class AssociatityType { kLeftAssociate, kRightAssociate };
  //�����������ͣ���Լ�����룬��������
  enum class ActionType { kReduction, kShift, kError, kAccept };

  //��������ʽ�������ұ�������һ�������Ĳ���ʽ
  ProductionNodeId AnalysisProduction(const std::string& str);
  //��ӷ��ţ����ط��ŵ�ID������SymbolId::InvalidId()��������Ѵ���
  SymbolId AddSymbol(const std::string& str) {
    return manager_symbol_.AddObject(str).second;
  }
  //��ȡ���Ŷ�ӦID
  SymbolId GetSymbolId(const std::string& str) {
    return manager_symbol_.GetObjectId(str);
  }
  //ͨ������ID��ѯ��Ӧ�ַ���
  const std::string& GetSymbolString(SymbolId symbol_id) {
    return manager_symbol_.GetObject(symbol_id);
  }
  //���÷���ID���ڵ�ID��ӳ��
  void AddSymbolIdToProductionNodeIdMapping(SymbolId symbol_id,
                                            ProductionNodeId node_id);
  //�½��ս�ڵ㣬���ؽڵ�ID
  //�ڵ��Ѵ����Ҹ���symbol_id��ͬ������ID�򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId AddTerminalNode(SymbolId symbol_id);
  //�½�������ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId AddOperatorNode(SymbolId symbol_id,
                                   AssociatityType associatity_type,
                                   PriorityLevel priority_level);
  //�½����ս�ڵ㣬���ؽڵ�ID
  //�ڶ�������Ϊ����ʽ�壬�����ṹΪstd::initializer_list<std::vector<Object>>
  //��std::vector<std::vector<Object>>
  template <class IdType>
  ProductionNodeId AddNonTerminalNode(SymbolId symbol_id, IdType&& bodys);
  //��ȡ�ڵ�
  BaseProductionNode& GetProductionNode(ProductionNodeId id) {
    return manager_nodes_[id];
  }
  //�����ս�ڵ���Ӳ���ʽ��
  template <class IdType>
  void AddNonTerminalNodeBody(ProductionNodeId node_id, IdType&& body) {
    static_cast<NonTerminalProductionNode*>(GetProductionNode(node_id))
        ->AddBody(std::forward<IdType>(body));
  }
  //ͨ������ID��ѯ��Ӧ���нڵ��ID���������򷵻ؿ�vector
  std::vector<ProductionNodeId> GetSymbolToProductionNodeIds(
      SymbolId symbol_id);
  //����µĺ���
  CoreId AddNewCore() {
    CoreId core_id = CoreId(cores_.size());
    cores_.emplace_back();
    return core_id;
  }
  //��ȡcore_id��Ӧ����ʽ�
  Core& GetCore(CoreId core_id) {
    assert(core_id < cores_.size());
    return cores_[core_id];
  }
  // ItemsҪ��Ϊstd::vector<CoreItem>��std::initialize_list<CoreItem>
  // ForwardNodesҪ��Ϊstd::vector<Object>��std::initialize_list<Object>
  template <class Items, class ForwardNodes>
  CoreId AddNewCore(Items&& items, ForwardNodes&& forward_nodes);
  //����ں����Ӧ�ں˵�IDӳ�䣬�Ḳ��ԭ�м�¼
  void SetItemToCoreIdMapping(const CoreItem& core_item, CoreId core_id) {
    item_to_core_id_[core_item] = core_id;
  }
  //����������
  void AddItemToCore(const CoreItem& core_item, CoreId core_id) {
    GetCore(core_id).AddItem(core_item);
  }
  //��ȡ���Ӧ�ں˵�ID���������������뵽����ﲢ������ӦID
  CoreId GetItemCoreIdOrInsert(const CoreItem& core_item);
  //��ȡ���ĵ���ǰ�����ż�
  const std::unordered_set<ProductionNodeId>& GetCoreFowardNodes(
      CoreId core_id) {
    return GetCore(core_id).GetForwardNodes();
  }
  //��ȡ���ĵ�ȫ������Ĳ������򷵻�nullptr
  const std::set<CoreItem>& GetCoreItems(CoreId core_id) {
    return GetCore(core_id).GetItems();
  }
  //������������ǰ������
  void AddCoreLookForwardNode(CoreId core_id,
                              ProductionNodeId forward_node_id) {
    GetCore(core_id).AddForwardNode(forward_node_id);
  }

  //��ȡ�����﷨��������Ŀ
  ParsingTableEntry& GetParsingTableEntry(ParsingTableEntryId id) {
    assert(id < lexical_config_parsing_table_.size());
    return lexical_config_parsing_table_[id];
  }
  //���ø�����ʽID
  void SetRootProductionId(ProductionNodeId root_production) {
    root_production_ = root_production;
  }
  //�Ը�����������հ����������ԭλ�ã��Զ����ս���Ŵ�������Ӧ�
  //Ҳ���Զ�����Է��������ս����
  void CoreClosure(CoreId core_id);
  //��ȡ�Ӹ��������ʼ������node_id��Ӧ�Ľڵ�󵽴�ĺ�����
  //�Զ����������ڵĺ�������Goto�Ľ��Ϊ���򷵻�CoreId::InvalidId()
  CoreId Goto(CoreId core_id, ProductionNodeId node_id);
  //����LALR(1)����
  void LalrConstruct();
  //�Ը����ڵ�����λ����հ���������ǰ�����Ŵ�����ȥ
  std::set<CoreItem> ItemClosure(CoreItem item);

 private:
  //������ʽID
  ProductionNodeId root_production_;
  //�ս���š����ս���ŵȵĽڵ�
  ObjectManager<BaseProductionNode> manager_nodes_;
  //�������
  UnorderedStructManager<std::string, StringHasher> manager_symbol_;
  //����ID��manager_symbol_)���ص�ID����Ӧ�ڵ��ӳ�䣬֧��һ�����Ŷ�Ӧ����ڵ�
  std::unordered_multimap<SymbolId, ProductionNodeId> symbol_id_to_node_id_;

  //�ں�+���ں��
  std::vector<Core> cores_;
  //���Cores_ ID��ӳ��
  std::map<CoreItem, CoreId> item_to_core_id_;
  std::vector<ParsingTableEntry> lexical_config_parsing_table_;

  class BaseProductionNode {
   public:
    BaseProductionNode(NodeType type, SymbolId symbol_id)
        : base_type_(type),
          base_symbol_id_(symbol_id),
          base_id_(ProductionNodeId::InvalidId()) {}
    BaseProductionNode(const BaseProductionNode&) = delete;
    BaseProductionNode& operator=(const BaseProductionNode&) = delete;

    void SetType(NodeType type) { base_type_ = type; }
    NodeType GetType() { return base_type_; }
    void SetId(ProductionNodeId id) { base_id_ = id; }
    ProductionNodeId GetId() { return base_id_; }
    void SetSymbolId(SymbolId id) { base_symbol_id_ = id; }
    SymbolId GetSymbolId() { return base_symbol_id_; }

   private:
    //�ڵ�����
    NodeType base_type_;
    //�ڵ�ID
    ProductionNodeId base_id_;
    //�ڵ����ID
    SymbolId base_symbol_id_;
  };

  class TerminalProductionNode : public BaseProductionNode {
   public:
    TerminalProductionNode(NodeType type, SymbolId symbol_id)
        : BaseProductionNode(type, symbol_id) {}
    TerminalProductionNode(const TerminalProductionNode&) = delete;
    TerminalProductionNode& operator=(const TerminalProductionNode&) = delete;
  };

  class OperatorProductionNode : public TerminalProductionNode {
   public:
    OperatorProductionNode(NodeType type, SymbolId symbol_id,
                           AssociatityType associatity_type,
                           PriorityLevel priority_level)
        : TerminalProductionNode(type, symbol_id),
          operator_associatity_type_(associatity_type),
          operator_priority_level_(priority_level) {}
    OperatorProductionNode(const OperatorProductionNode&) = delete;
    OperatorProductionNode& operator=(const OperatorProductionNode&) = delete;

    void SetAssociatityType(AssociatityType type) {
      operator_associatity_type_ = type;
    }
    AssociatityType GetAssociatityType() { return operator_associatity_type_; }
    void SetPriorityLevel(PriorityLevel level) {
      operator_priority_level_ = level;
    }
    PriorityLevel GetPriorityLevel() { return operator_priority_level_; }

   private:
    // template <class IdType>
    // friend class ObjectManager;
    //����������
    AssociatityType operator_associatity_type_;
    //��������ȼ�
    PriorityLevel operator_priority_level_;
  };

  class NonTerminalProductionNode : public BaseProductionNode {
   public:
    using BodyContainerType = std::vector<std::vector<ProductionNodeId>>;
    NonTerminalProductionNode(NodeType type, SymbolId symbol_id)
        : BaseProductionNode(type, symbol_id) {}
    template <class IdType>
    NonTerminalProductionNode(NodeType type, SymbolId symbol_id, IdType&& body)
        : BaseProductionNode(type, symbol_id),
          nonterminal_bodys_(std::forward<IdType>(body)) {}
    NonTerminalProductionNode(const NonTerminalProductionNode&) = delete;
    NonTerminalProductionNode& operator=(const NonTerminalProductionNode&) =
        delete;

    template <class IdType>
    ProductionBodyId AddBody(IdType&& body);
    const std::vector<ProductionNodeId>& GetBody(ProductionBodyId body_id) {
      return nonterminal_bodys_[body_id];
    }
    void SetItemCoreId(ProductionBodyId body_id, PointIndex point_index,
                       CoreId core_id) {
      nonterminal_item_to_coreid_[body_id][point_index] = core_id;
    }
    CoreId GetItemCoreId(ProductionBodyId body_id, PointIndex point_index,
                         CoreId core_id) {
      return nonterminal_item_to_coreid_[body_id][point_index];
    }

   private:
    //���vector��÷��ս�����¸�������ʽ����
    //�ڲ�vector��ò���ʽ�����ﺬ�еĸ������Ŷ�Ӧ�Ľڵ�ID
    BodyContainerType nonterminal_bodys_;
    //��Ų�ͬ����ʽ����ͬpointλ�ö�Ӧ��CoreId
    std::vector<std::vector<CoreId>> nonterminal_item_to_coreid_;
  };

  //�����ǰ������
  class Core {
   public:
    Core() {}
    template <class Items, class ForwardNodes>
    Core(Items&& items, ForwardNodes&& forward_nodes)
        : items_(std::forward<Items>(items)),
          forward_nodes(std::forward<ForwardNodes>(forward_nodes)) {}
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;
    Core(Core&& core)
        : items_(std::move(core.items_)),
          forward_nodes(std::move(core.forward_nodes)) {}

    void AddItem(const CoreItem& item) { items_.insert(item); }
    const std::set<CoreItem>& GetItems() { return items_; }
    void AddForwardNode(ProductionNodeId node_id) {
      forward_nodes.insert(node_id);
    }
    const std::unordered_set<ProductionNodeId>& GetForwardNodes() {
      return forward_nodes;
    }

   private:
    //�
    std::set<CoreItem> items_;
    //��ǰ������
    std::unordered_set<ProductionNodeId> forward_nodes;
  };

  //�﷨��������Ŀ
  class ParsingTableEntry {
   public:
    ParsingTableEntry() {}
    ParsingTableEntry(const ParsingTableEntry&) = delete;
    ParsingTableEntry& operator=(const ParsingTableEntry&) = delete;

    //�����ս�ڵ�洢��С
    void ResizeTerminalStore(size_t new_size) {
      action_and_target_production_body.resize(new_size);
    }
    //���÷��ս�ڵ�洢��С
    void ResizeNonTerminalStore(size_t new_size) {
      nonterminal_node_transform_table.resize(new_size);
    }
    //���øò���ʽ��ת�������µĶ�����Ŀ��ڵ�
    void SetTerminalNodeActionAndTarget(
        size_t index, ActionType action_type,
        std::variant<ParsingTableEntryId, ProductionNodeId> target_id) {
      action_and_target_production_body[index] =
          std::make_pair(action_type, target_id);
    }
    void SetNonTerminalNodeTransformId(size_t index, ParsingTableEntryId id) {
      nonterminal_node_transform_table[index] = id;
    }
    //���ʸ���Ŀ�¸���ID�ս�ڵ����Ϊ��Ŀ��ID
    std::pair<ActionType, std::variant<ParsingTableEntryId, ProductionNodeId>>
    AtTerminalNode(size_t index) {
      return action_and_target_production_body[index];
    }
    //���ʸ���Ŀ�¸���ID���ս�ڵ�����ʱת�Ƶ�����ĿID
    ParsingTableEntryId AtNonTerminalNode(size_t index) {
      return nonterminal_node_transform_table[index];
    }

   private:
    //��ǰ������ID�µĲ�����Ŀ��ڵ�
    //����Ϊ����ʱvariant�������ת�Ƶ���ID��ProductionBodyId��
    //����Ϊ��Լʱvariant��ʹ�õĲ���ʽ���ID��ProductionNodeId��
    //����Ϊ���ܺͱ���ʱvariantδ����
    std::vector<std::pair<ActionType,
                          std::variant<ParsingTableEntryId, ProductionNodeId>>>
        action_and_target_production_body;
    //������ս�ڵ��ת�Ƶ��Ĳ���ʽ�����
    std::vector<ParsingTableEntryId> nonterminal_node_transform_table;
  };
};

template <class IdType>
inline LexicalGenerator::ProductionBodyId
LexicalGenerator::NonTerminalProductionNode::AddBody(IdType&& body) {
  size_t body_id = nonterminal_bodys_.size();
  //��������뵽����ʽ�������У���ɾ����ͬ����ʽ����
  nonterminal_bodys_.emplace_back(std::forward<IdType>(body));
  size_t point_indexs = body.size() + 1;
  //�����洢����ʽ��ͬ��λ�ö�Ӧ����ID�Ŀռ�
  nonterminal_item_to_coreid_.emplace_back();
  nonterminal_item_to_coreid_.back().resize(point_indexs);
  return body_id;
}

template <class IdType>
inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddNonTerminalNode(
    SymbolId symbol_id, IdType&& bodys) {
  ProductionNodeId node_id =
      manager_nodes_.EmplaceObject<NonTerminalProductionNode>(
          NodeType::kNonTerminalNode, symbol_id, std::forward<IdType>(bodys));
  manager_nodes_.GetObject(node_id).SetId(node_id);
  return ProductionNodeId(node_id);
}

template <class Items, class ForwardNodes>
inline LexicalGenerator::CoreId LexicalGenerator::AddNewCore(
    Items&& items, ForwardNodes&& forward_nodes) {
  CoreId core_id = cores_.size();
  cores_.emplace_back(std::forward<Items>(items),
                      std::forward<ForwardNodes>(forward_nodes));
  return core_id;
}

}  // namespace frontend::generator::lexicalgenerator
#endif  // !GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_