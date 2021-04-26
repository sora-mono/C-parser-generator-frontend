#include <cassert>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "Common/node_manager.h"
#include "Common/unordered_struct_manager.h"

#ifndef GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_
#define GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_

namespace frontend::generator::lexicalgenerator {
using frontend::common::NodeManager;
using frontend::common::UnorderedStructManager;

struct StringHasher {
  frontend::common::StringHashType DoHash(const std::string& str) {
    return frontend::common::HashString(str);
  }
};
// TODO ���ɾ��δʹ�ò���ʽ�Ĺ���
class LexicalGenerator {
  //Ϊ���涨��������Ͳ��ò�ʹ��ǰ������
  class BaseNode;
  class ParsingTableEntry;
  class Core;

  //����ʽ��ID
  using NodeId = NodeManager<BaseNode>::NodeId;
  //���ŵ�ID
  using SymbolId = UnorderedStructManager<std::string, StringHasher>::NodeId;
  //������ID
  using CoreId = size_t;
  //��������ȼ�
  using PriorityLevel = size_t;
  //���λ��
  using PointIndex = size_t;
  //�﷨��������ĿID
  using ParsingTableEntryId = size_t;
  //�ں��ڵ����
  using CoreItem = std::pair<NodeId, PointIndex>;
  //�ڵ����ͣ��ս���ţ�����������ս����
  enum class NodeType { kTerminalNode, kOperatorNode, kNonTerminalNode };
  //�����������ͣ����ϣ��ҽ��
  enum class AssociatityType { kLeftAssociate, kRightAssociate };
  //�����������ͣ���Լ�����룬��������
  enum class ActionType { kReduction, kShift, kError, kAccept };

  //����ʽ�����ID��ʹ�ö���洢��NodeId��variant�е�������
  class ProductionBodyId {
   public:
    ProductionBodyId() : production_body_id_(-1) {}
    ProductionBodyId(size_t production_body_id)
        : production_body_id_(production_body_id) {}
    operator size_t() { return production_body_id_; }

   private:
    size_t production_body_id_;
  };
  //��������ʽ�������ұ�������һ�������Ĳ���ʽ
  NodeId AnalysisProduction(const std::string& str);
  //��ӷ��ţ����ط��ŵ�ID������-1��������Ѵ���
  SymbolId AddSymbol(const std::string& str) {
    return manager_symbol_.AddObject(str).second;
  }
  //��ȡ���Ŷ�ӦID���������򷵻�-1
  SymbolId GetSymbolId(const std::string& str) {
    return manager_symbol_.GetObjectId(str);
  }
  //ͨ������ID��ѯ��Ӧ�ַ������������򷵻�nullptr
  const std::string* GetSymbolString(SymbolId symbol_id) {
    return manager_symbol_.GetObjectPtr(symbol_id);
  }
  //���÷���ID���ڵ�ID��ӳ��
  void AddSymbolIdToNodeIdMapping(SymbolId symbol_id, NodeId node_id);
  //�½��ս�ڵ㣬���ؽڵ�ID���ڵ��Ѵ����Ҹ���symbol_id��ͬ������ID�򷵻�-1
  NodeId AddTerminalNode(SymbolId symbol_id);
  //�½�������ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�-1
  NodeId AddOperatorNode(SymbolId symbol_id, AssociatityType associatity_type,
                         PriorityLevel priority_level);
  //�½����ս�ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�-1
  //�ڶ�������Ϊ����ʽ�壬�����ṹΪstd::initializer_list<std::vector<NodeId>>
  //��std::vector<std::vector<NodeId>>
  template <class T>
  NodeId AddNonTerminalNode(SymbolId symbol_id, T&& bodys);
  //ͨ������ID��ѯ��Ӧ���нڵ��ID���������򷵻ؿ�vector
  std::vector<NodeId> GetNodeIds(SymbolId symbol_id);
  //����µĺ���
  CoreId CreateNewCore() {
    CoreId core_id = cores_.size();
    cores_.emplace_back();
    return core_id;
  }
  // ItemsҪ��Ϊstd::vector<CoreItem>��std::initialize_list<CoreItem>
  // ForwardNodesҪ��Ϊstd::vector<NodeId>��std::initialize_list<NodeId>
  template <class Items, class ForwardNodes>
  CoreId CreateNewCore(Items&& items, ForwardNodes&& forward_nodes);
  //��ȡ���Ӧ�ں˵�ID���������������뵽�����ﲢ������ӦID
  CoreId GetItemCoreIdOrInsert(const CoreItem& core_item);
  //����ں����Ӧ�ں˵�IDӳ�䣬����Ѵ����Ҷ�Ӧcore_id����������򷵻�false
  bool AddItemToCoreIdMapping(const CoreItem& core_item, CoreId core_id);
  //����������KPK
  CoreId AddItemToCore(const CoreItem& core_item, CoreId core_id);
  //��ȡ���ĵ���ǰ�����ż������Ĳ������򷵻�nullptr
  const std::set<NodeId>* GetCoreFowardNodes(CoreId core_id);
  //��ȡ���ĵ�ȫ������Ĳ������򷵻�nullptr
  const std::set<CoreItem>* GetCoreItems(CoreId core_id);
  //������������ǰ������
  void AddCoreLookForwardNode(CoreId core_id, NodeId forward_node_id);

  //��ȡ�����﷨��������Ŀ
  ParsingTableEntry& GetParsingTableEntry(ParsingTableEntryId id) {
    assert(id < lexical_config_parsing_table_.size());
    return lexical_config_parsing_table_[id];
  }
  //���ø�����ʽID
  void SetRootProductionId(NodeId root_production) {
    assert(manager_nodes_.GetNode(root_production) != nullptr);
    root_production_ = root_production;
  }
  //�Ը�����������հ����������ԭλ�ã��Զ����ս���Ŵ�������Ӧ�
  //Ҳ���Զ�����Է��������ս����
  void Closure(CoreId core_id);
  //��ȡ�Ӹ��������ʼ������node_id��Ӧ�Ľڵ�󵽴�ĺ�����
  //�Զ����������ڵĺ�������Goto�Ľ��Ϊ���򷵻�-1
  CoreId Goto(CoreId core_id, NodeId node_id);
  //����LALR(1)����
  void LalrConstruct();
  //�Ը����ڵ�����λ����հ���������ǰ�����Ŵ�����ȥ
  std::set<CoreItem> Closure(NodeId node_id, PointIndex point_index);

 private:
  //������ʽID
  NodeId root_production_;
  //�ս���š����ս���ŵȵĽڵ�
  NodeManager<BaseNode> manager_nodes_;
  //�������
  UnorderedStructManager<std::string, StringHasher> manager_symbol_;
  //����ID��manager_symbol_)���ص�ID����Ӧ�ڵ��ӳ�䣬֧��һ�����Ŷ�Ӧ����ڵ�
  std::unordered_multimap<SymbolId, NodeId> symbol_id_to_node_id_;

  //�ں�+���ں��
  std::vector<Core> cores_;
  //���Cores_ ID��ӳ��
  std::map<CoreItem, CoreId> item_to_core_id_;
  std::vector<ParsingTableEntry> lexical_config_parsing_table_;

  class BaseNode {
   public:
    BaseNode(NodeType type, SymbolId symbol_id)
        : base_type_(type), base_symbol_id_(symbol_id) {}
    BaseNode(const BaseNode&) = delete;
    BaseNode& operator=(const BaseNode&) = delete;

    void SetType(NodeType type) { base_type_ = type; }
    NodeType GetType() { return base_type_; }
    void SetId(NodeId id) { base_id_ = id; }
    NodeId GetId() { return base_id_; }
    void SetSymbolId(SymbolId id) { base_symbol_id_ = id; }
    SymbolId GetSymbolId() { return base_symbol_id_; }

   private:
    NodeType base_type_;
    NodeId base_id_;
    SymbolId base_symbol_id_;
  };

  class TerminalNode : public BaseNode {
   public:
    TerminalNode(NodeType type, SymbolId symbol_id)
        : BaseNode(type, symbol_id) {}
    TerminalNode(const TerminalNode&) = delete;
    TerminalNode&operator=(const TerminalNode&) = delete;
  };
  class OperatorNode : public TerminalNode {
    OperatorNode(NodeType type, SymbolId symbol_id,
                 AssociatityType associatity_type, PriorityLevel priority_level)
        : TerminalNode(type, symbol_id),
          operator_associatity_type_(associatity_type),
          operator_priority_level_(priority_level) {}
    OperatorNode(const OperatorNode&) = delete;
    OperatorNode& operator=(const OperatorNode&) = delete;

    void SetAssociatityType(AssociatityType type) {
      operator_associatity_type_ = type;
    }
    AssociatityType GetAssociatityType() { return operator_associatity_type_; }
    void SetPriorityLevel(PriorityLevel level) {
      operator_priority_level_ = level;
    }
    PriorityLevel GetPriorityLevel() { return operator_priority_level_; }

   private:
     template<class T>
    friend class NodeManager;
    AssociatityType operator_associatity_type_;
    PriorityLevel operator_priority_level_;
  };

  class NonTerminalNode : public BaseNode {
   public:
    using BodyId = size_t;
    NonTerminalNode(NodeType type, SymbolId symbol_id)
        : BaseNode(type, symbol_id) {}
    template <class T>
    NonTerminalNode(NodeType type, SymbolId symbol_id, T&& body)
        : BaseNode(type, symbol_id),
          nonterminal_bodys_(std::forward<T>(body)) {}
    NonTerminalNode(const NonTerminalNode&) = delete;
    NonTerminalNode& operator=(const NonTerminalNode&) = delete;

    template <class T>
    BodyId AddBody(T&& body);
    std::vector<NodeId> GetBody(BodyId body_id) {
      return nonterminal_bodys_[body_id];
    }
    void SetItemCoreId(BodyId body_id, PointIndex point_index, CoreId core_id) {
      nonterminal_item_to_coreid_[body_id][point_index] = core_id;
    }
    CoreId GetItemCoreId(BodyId body_id, PointIndex point_index,
                         CoreId core_id) {
      return nonterminal_item_to_coreid_[body_id][point_index];
    }

   private:
    //���vector��÷��ս�����¸�������ʽ����
    //�ڲ�vector��ò���ʽ�����ﺬ�еĸ������Ŷ�Ӧ�Ľڵ�ID
    std::vector<std::vector<NodeId>> nonterminal_bodys_;
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
          forward_nodes_(std::forward<ForwardNodes>(forward_nodes)) {}
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;
    Core(Core&& core)
        : items_(std::move(core.items_)),
          forward_nodes_(std::move(core.forward_nodes_)) {}

    void AddItem(const CoreItem& item) { items_.insert(item); }
    const std::set<CoreItem>& GetItems() { return items_; }
    void AddForwardNode(NodeId node_id) { forward_nodes_.insert(node_id); }
    const std::set<NodeId>& GetForwardNodes() { return forward_nodes_; }

   private:
    //�
    std::set<CoreItem> items_;
    //��ǰ������
    std::set<NodeId> forward_nodes_;
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
        std::variant<ProductionBodyId, NodeId> target_id) {
      action_and_target_production_body[index] =
          std::make_pair(action_type, target_id);
    }
    void SetNonTerminalNodeTransformId(size_t index, ProductionBodyId id) {
      nonterminal_node_transform_table[index] = id;
    }
    //���ʸ���Ŀ�¸���ID�ս�ڵ����Ϊ��Ŀ��ID
    std::pair<ActionType, std::variant<ProductionBodyId, NodeId>>
    AtTerminalNode(size_t index) {
      return action_and_target_production_body[index];
    }
    //���ʸ���Ŀ�¸���ID���ս�ڵ�����ʱת�Ƶ�����ĿID
    ProductionBodyId AtNonTerminalNode(size_t index) {
      return nonterminal_node_transform_table[index];
    }

   private:
    //��ǰ������ID�µĲ�����Ŀ��ڵ�
    //����Ϊ����ʱvariant�������ת�Ƶ���ID��ProductionBodyId��
    //����Ϊ��Լʱvariant��ʹ�õĲ���ʽ���ID��NodeId��
    //����Ϊ���ܺͱ���ʱvariantδ����
    std::vector<std::pair<ActionType, std::variant<ProductionBodyId, NodeId>>>
        action_and_target_production_body;
    //������ս�ڵ��ת�Ƶ��Ĳ���ʽ�����
    std::vector<ProductionBodyId> nonterminal_node_transform_table;
  };
};

template <class T>
inline LexicalGenerator::NonTerminalNode::BodyId
LexicalGenerator::NonTerminalNode::AddBody(T&& body) {
  size_t body_id = nonterminal_bodys_.size();
  //��������뵽����ʽ�������У���ɾ����ͬ����ʽ����
  nonterminal_bodys_.emplace_back(std::forward<T>(body));
  size_t point_indexs = body.size() + 1;
  //�����洢����ʽ��ͬ��λ�ö�Ӧ����ID�Ŀռ�
  nonterminal_item_to_coreid_.emplace_back();
  nonterminal_item_to_coreid_.back().resize(point_indexs);
  return body_id;
}

template <class T>
inline LexicalGenerator::NodeId LexicalGenerator::AddNonTerminalNode(SymbolId symbol_id,
                                                   T&& bodys) {
  NodeId node_id = manager_nodes_.EmplaceNode<NonTerminalNode>(
      NodeType::kNonTerminalNode, symbol_id, std::forward<T>(bodys));
  manager_nodes_.GetNode(node_id)->SetId(node_id);
  return NodeId(node_id);
}

template <class Items, class ForwardNodes>
inline LexicalGenerator::CoreId LexicalGenerator::CreateNewCore(Items&& items,
                                              ForwardNodes&& forward_nodes) {
  CoreId core_id= cores_.size();
  cores_.emplace_back(std::forward<Items>(items),
                      std::forward<ForwardNodes>(forward_nodes));
  return core_id;
}

}  // namespace frontend::generator::lexicalgenerator
#endif  // !GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_