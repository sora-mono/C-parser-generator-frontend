#include <cassert>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

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
  class BaseNode;
  class ParsingTableEntry;
  class Core;

  //����ʽ��ID
  using ObjectId = ObjectManager<BaseNode>::ObjectId;
  //���ŵ�ID
  using SymbolId = UnorderedStructManager<std::string, StringHasher>::ObjectId;
  //������ID
  using CoreId = size_t;
  //��������ȼ�
  using PriorityLevel = size_t;
  //���λ��
  using PointIndex = size_t;
  //�﷨��������ĿID
  using ParsingTableEntryId = size_t;
  //�ں��ڵ����
  using CoreItem = std::pair<ObjectId, PointIndex>;
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
  ObjectId AnalysisProduction(const std::string& str);
  //��ӷ��ţ����ط��ŵ�ID������-1��������Ѵ���
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
  void AddSymbolIdToNodeIdMapping(SymbolId symbol_id, ObjectId node_id);
  //�½��ս�ڵ㣬���ؽڵ�ID���ڵ��Ѵ����Ҹ���symbol_id��ͬ������ID�򷵻�-1
  ObjectId AddTerminalNode(SymbolId symbol_id);
  //�½�������ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�-1
  ObjectId AddOperatorNode(SymbolId symbol_id, AssociatityType associatity_type,
                         PriorityLevel priority_level);
  //�½����ս�ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�-1
  //�ڶ�������Ϊ����ʽ�壬�����ṹΪstd::initializer_list<std::vector<Object>>
  //��std::vector<std::vector<Object>>
  template <class T>
  ObjectId AddNonTerminalNode(SymbolId symbol_id, T&& bodys);
  //��ȡ�ڵ�
  BaseNode* GetNode(ObjectId id) {
    assert(id < manager_nodes_.Size());
    return manager_nodes_[id];
  }
  //�����ս�ڵ���Ӳ���ʽ��
  template <class T>
  void AddNonTerminalNodeBody(ObjectId node_id, T&& body) {
    static_cast<NonTerminalNode*>(GetNode(node_id))
        ->AddBody(std::forward<T>(body));
  }
  //ͨ������ID��ѯ��Ӧ���нڵ��ID���������򷵻ؿ�vector
  std::vector<ObjectId> GetSymbolToNodeIds(SymbolId symbol_id);
  //����µĺ���
  CoreId AddNewCore() {
    CoreId core_id = cores_.size();
    cores_.emplace_back();
    return core_id;
  }
  //��ȡcore_id��Ӧ����ʽ�
  Core* GetCore(CoreId core_id) {
    assert(core_id < cores_.size());
    return &cores_[core_id];
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
    Core* core = GetCore(core_id);
    core->AddItem(core_item);
  }
  //��ȡ���Ӧ�ں˵�ID���������������뵽����ﲢ������ӦID
  CoreId GetItemCoreIdOrInsert(const CoreItem& core_item);
  //��ȡ���ĵ���ǰ�����ż�
  const std::set<ObjectId>* GetCoreFowardNodes(CoreId core_id) {
    return GetCore(core_id)->GetForwardNodes();
  }
  //��ȡ���ĵ�ȫ������Ĳ������򷵻�nullptr
  const std::set<CoreItem>* GetCoreItems(CoreId core_id) {
    return GetCore(core_id)->GetItems();
  }
  //������������ǰ������
  void AddCoreLookForwardNode(CoreId core_id, ObjectId forward_node_id) {
    GetCore(core_id)->AddForwardNode(forward_node_id);
  }

  //��ȡ�����﷨��������Ŀ
  ParsingTableEntry& GetParsingTableEntry(ParsingTableEntryId id) {
    assert(id < lexical_config_parsing_table_.size());
    return lexical_config_parsing_table_[id];
  }
  //���ø�����ʽID
  void SetRootProductionId(ObjectId root_production) {
    root_production_ = root_production;
  }
  //�Ը�����������հ����������ԭλ�ã��Զ����ս���Ŵ�������Ӧ�
  //Ҳ���Զ�����Է��������ս����
  void CoreClosure(CoreId core_id);
  //��ȡ�Ӹ��������ʼ������node_id��Ӧ�Ľڵ�󵽴�ĺ�����
  //�Զ����������ڵĺ�������Goto�Ľ��Ϊ���򷵻�-1
  CoreId Goto(CoreId core_id, ObjectId node_id);
  //����LALR(1)����
  void LalrConstruct();
  //�Ը����ڵ�����λ����հ���������ǰ�����Ŵ�����ȥ
  std::set<CoreItem> ItemClosure(CoreItem item);

 private:
  //������ʽID
  ObjectId root_production_;
  //�ս���š����ս���ŵȵĽڵ�
  ObjectManager<BaseNode> manager_nodes_;
  //�������
  UnorderedStructManager<std::string, StringHasher> manager_symbol_;
  //����ID��manager_symbol_)���ص�ID����Ӧ�ڵ��ӳ�䣬֧��һ�����Ŷ�Ӧ����ڵ�
  std::unordered_multimap<SymbolId, ObjectId> symbol_id_to_node_id_;

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
    void SetId(ObjectId id) { base_id_ = id; }
    ObjectId GetId() { return base_id_; }
    void SetSymbolId(SymbolId id) { base_symbol_id_ = id; }
    SymbolId GetSymbolId() { return base_symbol_id_; }

   private:
    NodeType base_type_;
    ObjectId base_id_;
    SymbolId base_symbol_id_;
  };

  class TerminalNode : public BaseNode {
   public:
    TerminalNode(NodeType type, SymbolId symbol_id)
        : BaseNode(type, symbol_id) {}
    TerminalNode(const TerminalNode&) = delete;
    TerminalNode& operator=(const TerminalNode&) = delete;
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
    template <class T>
    friend class ObjectManager;
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
    std::vector<ObjectId> GetBody(BodyId body_id) {
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
    std::vector<std::vector<ObjectId>> nonterminal_bodys_;
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
    const std::set<CoreItem>* GetItems() { return &items_; }
    void AddForwardNode(ObjectId node_id) { forward_nodes.insert(node_id); }
    const std::set<ObjectId>* GetForwardNodes() { return &forward_nodes; }

   private:
    //�
    std::set<CoreItem> items_;
    //��ǰ������
    std::set<ObjectId> forward_nodes;
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
        std::variant<ProductionBodyId, ObjectId> target_id) {
      action_and_target_production_body[index] =
          std::make_pair(action_type, target_id);
    }
    void SetNonTerminalNodeTransformId(size_t index, ProductionBodyId id) {
      nonterminal_node_transform_table[index] = id;
    }
    //���ʸ���Ŀ�¸���ID�ս�ڵ����Ϊ��Ŀ��ID
    std::pair<ActionType, std::variant<ProductionBodyId, ObjectId>>
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
    std::vector<std::pair<ActionType, std::variant<ProductionBodyId, ObjectId>>>
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
inline LexicalGenerator::ObjectId LexicalGenerator::AddNonTerminalNode(
    SymbolId symbol_id, T&& bodys) {
  ObjectId node_id = manager_nodes_.EmplaceObject<NonTerminalNode>(
      NodeType::kNonTerminalNode, symbol_id, std::forward<T>(bodys));
  manager_nodes_.GetObject(node_id)->SetId(node_id);
  return ObjectId(node_id);
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