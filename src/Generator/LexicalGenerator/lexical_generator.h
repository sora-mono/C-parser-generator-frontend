#include <any>
#include <array>
#include <cassert>
#include <functional>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/object_manager.h"
#include "Common/unordered_struct_manager.h"
#include "Generator/DfaGenerator/dfa_generator.h"

#ifndef GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_
#define GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_

namespace frontend::parser::lexicalmachine {
class LexicalMachine;
}

// TODO ���ɾ��δʹ�ò���ʽ�Ĺ���
namespace frontend::generator::lexicalgenerator {
using frontend::common::ObjectManager;
using frontend::common::UnorderedStructManager;

struct StringHasher {
  frontend::common::StringHashType DoHash(const std::string& str) {
    return frontend::common::HashString(str);
  }
};

// �ӿ��࣬�����û����庯�����Ӹ�������
class ProcessFunctionInterface {
 public:
#ifdef USE_USER_DATA
  // �洢�û��Զ�������
  using UserData = std::any;
#endif  // USE_USER_DATA

#ifdef USE_INIT_FUNCTION
  virtual void Init() = 0;
#endif  // USE_INIT_FUNCTION

#ifdef USE_SHIFT_FUNCTION
#ifdef USE_USER_DATA
  virtual UserData Shift(size_t index) = 0;
#endif  // USE_USER_DATA
#else
  virtual void Shift(size_t index) = 0;
#endif  // USE_SHIFT_FUNCTION

#ifdef USE_REDUCT_FUNCTION
#ifdef USE_USER_DATA
  // �������Խ����ջʱ��Խ��
  virtual void Reduct(std::vector<UserData>&& user_data) = 0;
#else
  virtual void Reduct() = 0;
#endif  // USE_USER_DATA
#endif  // USE_REDUCT_FUNCTION
};

class LexicalGenerator {
  class BaseProductionNode;
  class ParsingTableEntry;
  class Core;
  // ID��װ����������ͬID��ö��
  enum class WrapperLabel {
    kCoreId,
    kPriorityLevel,
    kPointIndex,
    kParsingTableEntryId,
    kProductionBodyId,
    kProcessFunctionClassId
  };
  // ����ʽ�ڵ�����
  using ProductionNodeType = frontend::common::ProductionNodeType;
  // ������ID
  using CoreId = frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                                     WrapperLabel::kCoreId>;

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ��������ȼ�������Խ�����ȼ�Խ�ߣ�����������ڵ���Ч
  // ��TailNodePriority���岻ͬ�������ȼ�Ӱ���﷨��������
  // �����������������ʱ�������뻹�ǹ鲢
  using PriorityLevel =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPriorityLevel>;
#endif  // USE_AMBIGUOUS_GRAMMAR

  // ���λ��
  using PointIndex =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPointIndex>;
  // �﷨��������ĿID
  using ParsingTableEntryId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kParsingTableEntryId>;
  // �﷨����������
  using ParsingTableType = std::vector<ParsingTableEntry>;
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
  // �ں��ڵ��������������������Ϊ
  // ����ʽ�ڵ�ID������ʽ��ID������ʽ���е��λ��
  using CoreItem = std::tuple<ProductionNodeId, ProductionBodyId, PointIndex>;
  // ��װ�û��Զ��庯�������ݵ�����ѷ������ID
  using ProcessFunctionClassId = frontend::common::ProcessFunctionClassId;
  // �����װ�û��Զ��庯�������ݵ�����ѷ�����������
  using ProcessFunctionClassManagerType =
      ObjectManager<ProcessFunctionInterface>;
  // �����������ͣ����ϣ��ҽ��
  using AssociatityType = frontend::common::AssociatityType;
  // �����������ͣ���Լ�����룬��������
  enum class ActionType { kReduction, kShift, kError, kAccept };

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ���������
  struct OperatorData {
    std::string operator_symbol;
    std::string priority;
    std::string associatity_type;
#ifdef USE_INIT_FUNCTION
    std::string init_function;
#endif  // USE_INIT_FUNCTION
#ifdef USE_SHIFT_FUNCTION
    std::vector<std::string> shift_functions;
#endif  // USE_SHIFT_FUNCTION
#ifdef USE_REDUCT_FUNCTION
    std::string reduct_function;
#endif  // USE_REDUCT_FUNCTION
#ifdef USE_USER_DEFINED_FILE
    std::vector<std::string> include_files;
#endif  // USE_USER_DEFINED_FILE
  };
#endif  // USE_AMBIGUOUS_GRAMMAR

  struct NonTerminalNodeData {
    std::string node_symbol;
    // �����string����û��˫����
    // bool��ʾ�Ƿ�Ϊ�ս�ڵ���
    std::vector<std::pair<std::string, bool>> body_symbols;

#ifdef USE_INIT_FUNCTION
    std::string init_function;
#endif  // USE_INIT_FUNCTION
#ifdef USE_SHIFT_FUNCTION
    std::vector<std::string> shift_functions;
#endif  // USE_SHIFT_FUNCTION
#ifdef USE_REDUCT_FUNCTION
    std::string reduct_function;
#endif  // USE_REDUCT_FUNCTION
#ifdef USE_USER_DEFINED_FILE
    std::vector<std::string> include_files;
#endif  // USE_USER_DEFINED_FILE

    // �ò���ʽ�ڵ��Ƿ�ֻ��������ͬ���ս����ʽ
    // ����Example -> "int"|"char"|"double"
    // ��Щ�ս����ʽ�����ô�����
    bool use_same_process_function_class = false;
  };

 public:
  LexicalGenerator() {
    OpenConfigFile();
    ClearNodeNum();
  }
  ~LexicalGenerator() { CloseConfigFile(); }

  // �����ؼ���
  void AnalysisKeyWord(const std::string& str);
  // �����ս�ڵ����ʽ�ļ�
  void AnalysisProductionConfig(const std::string& file_name);
  // �����ս�ڵ����ʽ�������ұ�������һ�������Ĳ���ʽ
  // ��ͨ�ս�ڵ�Ĭ�����ȼ�Ϊ0����ͣ�
  void AnalysisTerminalProduction(const std::string& str, size_t priority = 0);
  // �������������ʽ�������ұ�������һ�������Ĳ���ʽ
  void AnalysisOperatorProduction(const std::string& str);
  // �������ս�ڵ����ʽ�������ұ�������һ�������Ĳ���ʽ
  void AnalysisNonTerminalProduction(const std::string& str);
  // ����LALR����ĸ�����Χ��Ϣ
  void ConfigConstruct();
  // ����LALR(1)����
  void ParsingTableConstruct();

 private:
  FILE*& GetConfigConstructFilePointer() { return config_construct_code_file_; }
  FILE*& GetProcessFunctionClassFilePointer() {
    return process_function_class_file_;
  }
  // �������ļ�
  void OpenConfigFile();
  // �ر������ļ�
  void CloseConfigFile();
  // ��ȡһ���ڵ���
  int GetNodeNum() { return node_num_++; }
  // ��λ�ڵ���
  void ClearNodeNum() { node_num_ = 0; }
  // ���ַ���������������ս�����壬����˳����ȡ����
  // ÿ���е�bool�����Ƿ�Ϊ�ս�ڵ��壬���������Ƿ���"���ж�
  // ���ص�string��û��˫����
  std::vector<std::pair<std::string, bool>> GetBodySymbol(
      const std::string& str);
  // ���ַ��������������������������˳����ȡ����
  std::vector<std::string> GetFunctionsName(const std::string& str);
  // ���ַ���������������ļ���������˳����ȡ����
  // ����@ʱ��ֹ��ȡ
  std::vector<std::string> GetFilesName(const std::string& str);
  // �������ļ���д�����ɹؼ��ֵĲ���
  void PrintKeyWordConstructData(const std::string& keyword);
  // �������ļ���д�������ս�ڵ�Ĳ���
  void PrintTerminalNodeConstructData(std::string&& node_symbol,
                                      std::string&& body_symbol,
                                      size_t priority);
  // �ӹ��̣��������ļ�д��init_function, shift_function��reduct_function
  template <class T>
  void PrintProcessFunction(FILE* function_file, const T& data);
#ifdef USE_AMBIGUOUS_GRAMMAR
  // �������ļ���д������������Ĳ���
  void PrintOperatorNodeConstructData(OperatorData&& data);
#endif  // USE_AMBIGUOUS_GRAMMAR

  // �������ļ���д�����ɷ��ս�ڵ�Ĳ���
  void PrintNonTerminalNodeConstructData(NonTerminalNodeData&& data);
  // ��Ӳ���ʽ�������ط��ŵ�ID���Ƿ�ִ���˲������
  // ִ���˲�������򷵻�true
  std::pair<SymbolId, bool> AddNodeSymbol(const std::string& node_symbol) {
    assert(node_symbol.size() != 0);
    return manager_node_symbol_.AddObject(node_symbol);
  }
  // ��Ӳ���ʽ����ţ����ط��ŵ�ID���Ƿ�ִ���˲������
  // ִ���˲�������򷵻�true
  std::pair<SymbolId, bool> AddBodySymbol(const std::string& body_symbol) {
    assert(body_symbol.size() != 0);
    return manager_terminal_body_symbol_.AddObject(body_symbol);
  }
  // ��ȡ����ʽ����ӦID���������򷵻�SymbolId::InvalidId()
  SymbolId GetNodeSymbolId(const std::string& node_symbol) {
    assert(node_symbol.size() != 0);
    return manager_node_symbol_.GetObjectId(node_symbol);
  }
  // ��ȡ����ʽ����Ŷ�Ӧ��ID���������򷵻�SymbolId::InvalidId()
  SymbolId GetBodySymbolId(const std::string& body_symbol) {
    assert(body_symbol.size() != 0);
    return manager_terminal_body_symbol_.GetObjectId(body_symbol);
  }
  // ͨ������ʽ��ID��ѯ��Ӧ�ַ���
  const std::string& GetNodeSymbolStringFromId(SymbolId node_symbol_id) {
    assert(node_symbol_id.IsValid());
    return manager_node_symbol_.GetObject(node_symbol_id);
  }
  // ͨ������ʽ�����ID��ѯ��Ӧ���ַ���
  const std::string& GetBodySymbolStringFromId(SymbolId body_symbol_id) {
    assert(body_symbol_id.IsValid());
    return manager_terminal_body_symbol_.GetObject(body_symbol_id);
  }
  // ���ò���ʽ��ID���ڵ�ID��ӳ��
  void SetNodeSymbolIdToProductionNodeIdMapping(SymbolId node_symbol_id,
                                                ProductionNodeId node_id) {
    assert(node_symbol_id.IsValid() && node_id.IsValid());
    node_symbol_id_to_node_id_[node_symbol_id] = node_id;
  }
  // ���ò���ʽ�����ID���ڵ�ID��ӳ��
  void SetBodySymbolIdToProductionNodeIdMapping(SymbolId body_symbol_id,
                                                ProductionNodeId node_id) {
    assert(body_symbol_id.IsValid() && node_id.IsValid());
    production_body_symbol_id_to_node_id_[body_symbol_id] = node_id;
  }
  // ������ʽ��IDת��Ϊ����ʽ�ڵ�ID
  ProductionNodeId GetProductionNodeIdFromNodeSymbolId(
      SymbolId node_symbol_id) {
    auto iter = node_symbol_id_to_node_id_.find(node_symbol_id);
    assert(iter != node_symbol_id_to_node_id_.end());
    return iter->second;
  }
  // ������ʽ�����IDת��Ϊ����ʽ�ڵ�ID
  // ������ʱ�򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId GetProductionNodeIdFromBodySymbolId(SymbolId body_symbol_id);
  // ������ʽ��ת��Ϊ����ʽ�ڵ�ID
  ProductionNodeId GetProductionNodeIdFromNodeSymbol(
      const std::string& body_symbol);
  // ������ʽ�����ת��Ϊ����ʽ�ڵ�ID
  ProductionNodeId GetProductionNodeIdFromBodySymbol(
      const std::string& body_symbol);
  // ��ȡ����ʽ��Ĳ���ʽ�ڵ�ID
  ProductionNodeId GetProductionNodeIdInBody(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index) {
    return GetProductionNode(production_node_id)
        .GetProductionNodeInBody(production_body_id, point_index);
  }
  // ��Ӱ�װ�������������
  template <class ProcessFunctionClass>
  ProcessFunctionClassId CreateProcessFunctionClassObject() {
    return manager_process_function_class_
        .EmplaceObject<ProcessFunctionClass>();
  }
  // ��ȡ��װ�����������ݵ���Ķ���ID
  ProcessFunctionClassId GetProcessFunctionClass(
      ProductionNodeId production_node_id,
      ProductionBodyId production_body_id) {
    return GetProductionNode(production_node_id)
        .GetBodyProcessFunctionClassId(production_body_id);
  }
  // �����Ϊ����ʽ��δ��������²��ܼ�����ӵķ��ս�ڵ�
  // ��һ������Ϊδ����Ĳ���ʽ��
  // ������������ͬAddNonTerminalNode
  // �����Ḵ��һ�ݸ��������豣��ԭ���Ĳ�������������
  void AddUnableContinueNonTerminalNode(
      const std::string& undefined_symbol, const std::string& node_symbol,
      const std::vector<std::pair<std::string, bool>>& subnode_symbols,
      ProcessFunctionClassId class_id);
  // �������Ľڵ����ɺ��Ƿ����������Ϊ���ֲ���ʽ��δ��������õ�
  // ���ս����ʽ��ӹ���
  void CheckNonTerminalNodeCanContinue(const std::string& node_symbol);
  // ����Ƿ���δ����Ĳ���ʽ��
  // ����������в���ʽ��Ӻ���
  // ��stderr���������Ϣ������У�
  void CheckUndefinedProductionRemained();
  // ��ӹؼ��֣��Զ�����ͬ���ս�ڵ�
  void AddKeyWord(const std::string& key_word);
  // �½��ս�ڵ㣬���ؽڵ�ID
  // �ڵ��Ѵ����Ҹ���symbol_id��ͬ������ID�򷵻�ProductionNodeId::InvalidId()
  // �����������ǽڵ����ȼ���0����Ϊ��ͨ�ʵ����ȼ���1����Ϊ�ؼ��ֵ����ȼ�
  // ��������ȼ����Թ�����������û�������;
  ProductionNodeId AddTerminalNode(
      const std::string& node_symbol, const std::string& body_symbol,
      frontend::generator::dfa_generator::DfaGenerator::TailNodePriority
          node_priority);
  // �ӹ��̣������ڴ����ڵ�
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddTerminalNode(SymbolId node_symbol_id,
                                      SymbolId body_symbol_id);

#ifdef USE_AMBIGUOUS_GRAMMAR
  // �½�������ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  // ���ģ�庯���ͷ�ģ�庯��Ϊ�˽��ʹ�������������ֹ��������
  // ����������������ֱ�ӵ��ã�class_id�ǰ�װ�û����庯�������ݵ���Ķ���ID
  // Ĭ����ӵ�������ʷ��������ȼ�������������ȼ�
  template <class ProcessFunctionClass>
  ProductionNodeId AddOperatorNode(const std::string& operator_symbol,
                                   AssociatityType associatity_type,
                                   PriorityLevel priority_level);
  ProductionNodeId AddOperatorNode(const std::string& operator_symbol,
                                   AssociatityType associatity_type,
                                   PriorityLevel priority_level,
                                   ProcessFunctionClassId class_id);
  // �ӹ��̣������ڴ����ڵ�
  // ������ڵ���ͬ�������
  // �Զ����½ڵ���ID���ڵ�ID��ӳ���
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddOperatorNode(SymbolId node_symbol_id,
                                      AssociatityType associatity_type,
                                      PriorityLevel priority_level,
                                      ProcessFunctionClassId class_id);
#endif  // USE_AMBIGUOUS_GRAMMAR

  // �½����ս�ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򲻻ᴴ���µĽڵ�
  // node_symbolΪ����ʽ����subnode_symbols�ǲ���ʽ��
  // class_id������ӵİ�װ�û��Զ��庯�������ݵ���Ķ���ID
  // ���ģ�庯���ͷ�ģ�庯��Ϊ�˽��ʹ�������������ֹ��������
  // ����������������ֱ�ӵ��ã�class_id�ǰ�װ�û����庯�������ݵ���Ķ���ID
  template <class ProcessFunctionClass>
  ProductionNodeId AddNonTerminalNode(
      const std::string& node_symbol,
      const std::vector<std::string>& subnode_symbols);
  ProductionNodeId AddNonTerminalNode(
      const std::string& node_symbol,
      const std::vector<std::pair<std::string, bool>>& subnode_symbols,
      ProcessFunctionClassId class_id);
  // �ӹ��̣������ڴ����ڵ�
  // �Զ����½ڵ���ID���ڵ�ID��ӳ���
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddNonTerminalNode(SymbolId symbol_id);
  // �½��ļ�β�ڵ㣬���ؽڵ�ID
  ProductionNodeId AddEndNode(SymbolId symbol_id = SymbolId::InvalidId());

  // ��ȡ�ڵ�
  BaseProductionNode& GetProductionNode(ProductionNodeId id) {
    return manager_nodes_[id];
  }
  BaseProductionNode& GetProductionNodeFromNodeSymbolId(SymbolId id) {
    ProductionNodeId production_node_id =
        GetProductionNodeIdFromNodeSymbolId(id);
    assert(production_node_id.IsValid());
    return GetProductionNode(production_node_id);
  }
  BaseProductionNode& GetProductionNodeBodySymbolId(SymbolId id) {
    ProductionNodeId production_node_id =
        GetProductionNodeIdFromBodySymbolId(id);
    assert(production_node_id.IsValid());
    return GetProductionNode(production_node_id);
  }
  // �����ս�ڵ���Ӳ���ʽ��
  template <class IdType>
  void AddNonTerminalNodeBody(ProductionNodeId node_id, IdType&& body) {
    static_cast<NonTerminalProductionNode*>(GetProductionNode(node_id))
        ->AddBody(std::forward<IdType>(body));
  }
  // ͨ������ID��ѯ��Ӧ�ڵ��ID����Ҫ��֤ID��Ч
  ProductionNodeId GetProductionNodeIdFromNodeSymbol(SymbolId symbol_id);
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
      const CoreItem& core_item, CoreId core_id);
  // ��ȡ�������Ӧ���ID��ʹ�õĲ�������Ϊ��Ч����
  // δָ��������ʹ��SetItemCoreId���ã��򷵻�CoreId::InvalidId()
  CoreId GetCoreId(ProductionNodeId production_node_id,
                   ProductionBodyId production_body_id, PointIndex point_index);
  CoreId GetItemCoreId(const CoreItem& core_item);
  // ��ȡ�������Ӧ���ID����������ڼ�¼����뵽����ﲢ������ӦID
  // ���ص�bool�����Ƿ�core_item�����ڲ��ɹ���������ӵ��µ��
  // insert_core_id�ǲ����ڼ�¼ʱӦ�����Core��ID
  // ��ΪCoreId::InvalidId()�����½�Core
  std::pair<CoreId, bool> GetItemCoreIdOrInsert(
      const CoreItem& core_item, CoreId insert_core_id = CoreId::InvalidId());
  // ��ȡ��ǰ�����ż�
  const std::unordered_set<ProductionNodeId>& GetForwardNodeIds(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index) {
    return GetProductionNode(production_node_id)
        .GetForwardNodeIds(production_body_id, point_index);
  }
  // �����ǰ������
  void AddForwardNodeId(ProductionNodeId production_node_id,
                        ProductionBodyId production_body_id,
                        PointIndex point_index,
                        ProductionNodeId forward_node_id) {
    GetProductionNode(production_node_id)
        .AddForwardNodeId(production_body_id, point_index, forward_node_id);
  }
  // ���һ�������ڵ�������ǰ������
  template <class Container>
  void AddForwardNodeContainer(ProductionNodeId production_node_id,
                               ProductionBodyId production_body_id,
                               PointIndex point_index,
                               Container&& forward_nodes_id_container);
  // First���ӹ��̣���ȡһ�����ս�ڵ������е�first����
  // �ڶ������������洢�Ѿ�������Ľڵ㣬��ֹ���޵ݹ飬���ε���Ӧ����ռ���
  // �������ProductionNodeId::InvalidId()�򷵻ؿռ���
  std::unordered_set<ProductionNodeId> GetNonTerminalNodeFirstNodes(
      ProductionNodeId production_node_id,
      std::unordered_set<ProductionNodeId>&& processed_nodes =
          std::unordered_set<ProductionNodeId>());
  // �հ������е�first������ǰ����������־�µ�λ��
  // ��������������Ϊ�ս�ڵ�ڵ���ܹ鲢Ϊ�սڵ㣬��Ҫ���²����ս�ڵ�
  // ��ǰ���ڵ���Թ�ԼΪ�սڵ��������Ӧ��ǰ������
  // Ȼ�������ǰ����ֱ����β�򲻿ɿչ�Լ���ս�ڵ���ս�ڵ�
  std::unordered_set<ProductionNodeId> First(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index,
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
  // ��Ӧ��CoreClosure����
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
    assert(core_id.IsValid());
    auto [iter_begin, iter_end] = core_id_goto_core_id_.equal_range(core_id);
    for (; iter_begin != iter_end; ++iter_begin) {
      core_id_goto_core_id_.erase(iter_begin);
    }
  }
  // ����һ��Goto����
  void SetGotoCacheEntry(CoreId core_id_src,
                         ProductionNodeId transform_production_node_id,
                         CoreId core_id_dst);
  // ��ȡһ��Goto���棬�������򷵻�CoreId::InvalidId()
  CoreId GetGotoCacheEntry(CoreId core_id_src,
                           ProductionNodeId transform_production_node_id);
  // Goto�����Ƿ���Ч����Ч����true
  bool IsGotoCacheAvailable(CoreId core_id) {
    return GetCore(core_id).IsClosureAvailable();
  }
  // ��ȡ������һ������ʽ�ڵ�ID������������򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId GetProductionBodyNextShiftNodeId(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index);
  // ��ȡ�������ǰ���ڵ�ID������������򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId GetProductionBodyNextNextNodeId(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index);
  // Goto���ӹ��̣��Ե��������������ItemGoto���ID���Ƿ����
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
  // ��������﷨��������ĿID��ӳ��
  void SetCoreIdToParsingEntryIdMapping(CoreId core_id,
                                        ParsingTableEntryId entry_id) {
    core_id_to_parsing_table_entry_id_[core_id] = entry_id;
  }
  // ��ȡ�Ѵ洢����﷨��������ĿID��ӳ��
  // �������򷵻�ParsingTableEntryId::InvalidId()
  ParsingTableEntryId GetParsingEntryIdCoreId(CoreId core_id);
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
  // ��һ������Ϊ�﷨�������������ս�ڵ�ID
  // �ڶ�������Ϊ�﷨�����������з��ս�ڵ�ID
  // ���ؿ��Ժϲ����﷨��������Ŀ�飬�������������������Ŀ
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

  // �����﷨������Ϊ���࣬������ʹ�ø��ֶ���
  friend class frontend::parser::lexicalmachine::LexicalMachine;

  // �ս�ڵ����ʽ����
  static const std::regex terminal_node_regex_;
  // ���������
  static const std::regex operator_node_regex_;
  // ���ս�ڵ����ʽ����
  static const std::regex nonterminal_node_regex_;
  // �����ؼ��ֶ���
  static const std::regex keyword_regex_;
  // �����ս���Ŷ���
  static const std::regex body_symbol_regex_;
  // ��������������
  static const std::regex function_regex_;
  // �����ĵ����û��ļ�����
  static const std::regex file_regex_;

  // Generator���ɵİ�װ�û����庯�����ݵ����Դ���ļ�
  FILE* process_function_class_file_ = nullptr;
  // �洢Generator���ɵ��������ɴ����ļ�
  FILE* config_construct_code_file_ = nullptr;
  // �Խڵ��ţ��������ֲ�ͬ�ڵ��Ӧ����
  // ��ֵ�������������ô��������ɰ�װ�û����庯�����ݵ���
  int node_num_;
  // �洢���õ�δ�������ʽ
  // key��δ����Ĳ���ʽ��
  // tuple�ڵ�std::string�Ƿ��ս����ʽ��
  // const std::vector<std::pair<std::string, bool>>*�洢���еĲ���ʽ����Ϣ
  // ProcessFunctionClassId�Ǹ����İ�װ�û����庯�����ݵ���Ķ���ID
  std::unordered_multimap<
      std::string,
      std::tuple<std::string, std::vector<std::pair<std::string, bool>>,
                 ProcessFunctionClassId>>
      undefined_productions_;
  // ������ʽID
  ProductionNodeId root_production_;
  // �����ս���š����ս���ŵȵĽڵ�
  ObjectManager<BaseProductionNode> manager_nodes_;
  // �������ʽ���ķ���
  UnorderedStructManager<std::string, StringHasher> manager_node_symbol_;
  // �����ս�ڵ����ʽ��ķ���
  UnorderedStructManager<std::string, StringHasher>
      manager_terminal_body_symbol_;
  // ����ʽ��ID����Ӧ����ʽ�ڵ��ӳ��
  std::unordered_map<SymbolId, ProductionNodeId> node_symbol_id_to_node_id_;
  // ����ʽ�����ID����Ӧ�ڵ�ID��ӳ��
  std::unordered_map<SymbolId, ProductionNodeId>
      production_body_symbol_id_to_node_id_;
  // ����﷨��������ĿID��ӳ��
  std::map<CoreId, ParsingTableEntryId> core_id_to_parsing_table_entry_id_;
  // �ں�+���ں��
  std::vector<Core> cores_;
  // Goto���棬�洢���е�Core�ڲ�ͬ������Goto�Ľڵ�
  // ʹ��CoreId��Ϊ��ֵ����ɾ��ʱ���Ҹ�CoreId�µ�����cache
  std::multimap<CoreId, std::pair<ProductionNodeId, CoreId>>
      core_id_goto_core_id_;
  // DFA����������������д���ļ�
  frontend::generator::dfa_generator::DfaGenerator dfa_generator_;
  // �﷨����������д���ļ�
  ParsingTableType lexical_config_parsing_table_;
  // �洢�û��Զ��庯�������ݣ�����д���ļ�
  ProcessFunctionClassManagerType manager_process_function_class_;

  //���в���ʽ�ڵ��඼Ӧ�̳��Ը���
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
          base_symbol_id_(base_production_node.base_symbol_id_) {}
    BaseProductionNode& operator=(BaseProductionNode&& base_production_node);

    struct NodeData {
      ProductionNodeType node_type;
      std::string node_symbol;
    };

    void SetType(ProductionNodeType type) { base_type_ = type; }
    ProductionNodeType Type() const { return base_type_; }
    void SetThisNodeId(ProductionNodeId id) { base_id_ = id; }
    ProductionNodeId Id() const { return base_id_; }
    void SetSymbolId(SymbolId id) { base_symbol_id_ = id; }
    SymbolId GetNodeSymbolId() const { return base_symbol_id_; }

    // ��ȡ�������Ӧ�Ĳ���ʽID������point_index�����ID
    // point_indexԽ��ʱ����ProducNodeId::InvalidId()
    // Ϊ��֧����ǰ������ڵ�����Խ��
    // ���ص��ұߵĲ���ʽID���������򷵻�ProductionNodeId::InvalidId()
    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) = 0;
    //�����ǰ���ڵ�
    virtual void AddForwardNodeId(ProductionBodyId production_body_id,
                                  PointIndex point_index,
                                  ProductionNodeId forward_node_id) = 0;
    //��ȡ��ǰ���ڵ㼯��
    virtual const std::unordered_set<ProductionNodeId>& GetForwardNodeIds(
        ProductionBodyId production_body_id, PointIndex point_index) = 0;
    //�������Ӧ�ĺ���ID
    virtual void SetCoreId(ProductionBodyId production_body_id,
                           PointIndex point_index, CoreId core_id) = 0;
    //��ȡ���Ӧ�ĺ���ID
    virtual CoreId GetCoreId(ProductionBodyId production_body_id,
                             PointIndex point_index) = 0;
    // ��ȡ��װ�û��Զ��庯�����ݵ���Ķ���ID
    virtual ProcessFunctionClassId GetBodyProcessFunctionClassId(
        ProductionBodyId production_body_id) = 0;

   private:
    // �ڵ�����
    ProductionNodeType base_type_;
    // �ڵ�ID
    ProductionNodeId base_id_;
    // �ڵ����ID
    SymbolId base_symbol_id_;
  };

  class TerminalProductionNode : public BaseProductionNode {
   public:
    TerminalProductionNode(SymbolId node_symbol_id, SymbolId body_symbol_id)
        : BaseProductionNode(ProductionNodeType::kTerminalNode,
                             node_symbol_id) {
      SetBodySymbolId(body_symbol_id);
    }
    TerminalProductionNode(const TerminalProductionNode&) = delete;
    TerminalProductionNode& operator=(const TerminalProductionNode&) = delete;
    TerminalProductionNode(TerminalProductionNode&& terminal_production_node)
        : BaseProductionNode(std::move(terminal_production_node)),
          forward_nodes_(std::move(terminal_production_node.forward_nodes_)),
          core_ids_(std::move(terminal_production_node.core_ids_)),
          body_symbol_id_(std::move(terminal_production_node.body_symbol_id_)) {
    }
    TerminalProductionNode& operator=(
        TerminalProductionNode&& terminal_production_node) {
      BaseProductionNode::operator=(std::move(terminal_production_node));
      forward_nodes_ = std::move(terminal_production_node.forward_nodes_);
      core_ids_ = std::move(terminal_production_node.core_ids_);
      body_symbol_id_ = std::move(terminal_production_node.body_symbol_id_);
      return *this;
    }
    using NodeData = BaseProductionNode::NodeData;

    // �����ǰ���ڵ�ID
    void AddFirstItemForwardNodeId(ProductionNodeId forward_node_id) {
      assert(forward_node_id.IsValid());
      forward_nodes_.first.insert(forward_node_id);
    }
    void AddSecondItemForwardNodeId(ProductionNodeId forward_node_id) {
      assert(forward_node_id.IsValid());
      forward_nodes_.second.insert(forward_node_id);
    }
    // ���һ��������������ǰ���ڵ�ID
    template <class Container>
    void AddFirstItemForwardNodeContainer(
        Container&& forward_nodes_id_container) {
      assert(!forward_nodes_id_container.empty());
      forward_nodes_.first.merge(
          std::forward<Container>(forward_nodes_id_container));
    }
    template <class Container>
    void AddSecondItemForwardNodeContainer(
        Container&& forward_nodes_id_container) {
      assert(!forward_nodes_id_container.empty());
      forward_nodes_.second.merge(
          std::forward<Container>(forward_nodes_id_container));
    }
    template <class Container>
    void AddForwardNodeContainer(ProductionBodyId production_body_id,
                                 PointIndex point_index,
                                 Container&& forward_nodes_id_container);
    const std::unordered_set<ProductionNodeId>& GetFirstItemForwardNodeIds() {
      return forward_nodes_.first;
    }
    const std::unordered_set<ProductionNodeId>& GetSecondItemForwardNodeIds() {
      return forward_nodes_.second;
    }
    // ���ø������Ӧ���ID
    void SetFirstItemCoreId(CoreId core_id) {
      assert(core_id.IsValid());
      core_ids_.first = core_id;
    }
    void SetSecondItemCoreId(CoreId core_id) {
      assert(core_id.IsValid());
      core_ids_.second = core_id;
    }
    // ��ȡ�������Ӧ���ID
    CoreId GetFirstItemCoreId() { return core_ids_.first; }
    CoreId GetSecondItemCoreId() { return core_ids_.second; }
    // ��ȡ/���ò���ʽ����
    SymbolId GetBodySymbolId() { return body_symbol_id_; }
    void SetBodySymbolId(SymbolId body_symbol_id) {
      body_symbol_id_ = body_symbol_id;
    }
    // ��Խ��ʱҲ�з���ֵΪ��֧�ֻ�ȡ��һ��/���¸����ڽڵ�Ĳ���
    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) override;
    // �����ǰ���ڵ�
    virtual void AddForwardNodeId(ProductionBodyId production_body_id,
                                  PointIndex point_index,
                                  ProductionNodeId forward_node_id);
    // ��ȡ��ǰ���ڵ㼯��
    virtual const std::unordered_set<ProductionNodeId>& GetForwardNodeIds(
        ProductionBodyId production_body_id, PointIndex point_index);
    // �������Ӧ�ĺ���ID
    virtual void SetCoreId(ProductionBodyId production_body_id,
                           PointIndex point_index, CoreId core_id);
    // ��ȡ���Ӧ�ĺ���ID
    virtual CoreId GetCoreId(ProductionBodyId production_body_id,
                             PointIndex point_index);
    virtual ProcessFunctionClassId GetBodyProcessFunctionClassId(
      ProductionBodyId body_id) {
      assert(false);
      return ProcessFunctionClassId::InvalidId();
    }

   private:
    // �ս�ڵ�����������ǰ������
    std::pair<std::unordered_set<ProductionNodeId>,
              std::unordered_set<ProductionNodeId>>
        forward_nodes_;
    // �������Ӧ�ĺ���ID
    std::pair<CoreId, CoreId> core_ids_;
    // ����ʽ����
    SymbolId body_symbol_id_;
  };

#ifdef USE_AMBIGUOUS_GRAMMAR
  class OperatorProductionNode : public TerminalProductionNode {
   public:
    OperatorProductionNode(SymbolId node_symbol_id, SymbolId body_symbol_id,
                           AssociatityType associatity_type,
                           PriorityLevel priority_level,
                           ProcessFunctionClassId process_function_class_id)
        : TerminalProductionNode(node_symbol_id, body_symbol_id),
          operator_associatity_type_(associatity_type),
          operator_priority_level_(priority_level),
          process_function_class_id_(process_function_class_id) {}
    OperatorProductionNode(const OperatorProductionNode&) = delete;
    OperatorProductionNode& operator=(const OperatorProductionNode&) = delete;
    OperatorProductionNode(OperatorProductionNode&& operator_production_node)
        : TerminalProductionNode(std::move(operator_production_node)),
          operator_associatity_type_(
              std::move(operator_production_node.operator_associatity_type_)),
          operator_priority_level_(
              std::move(operator_production_node.operator_priority_level_)),
          process_function_class_id_(
              std::move(operator_production_node.process_function_class_id_)) {}
    OperatorProductionNode& operator=(
        OperatorProductionNode&& operator_production_node);

    struct NodeData : public TerminalProductionNode::NodeData {
      std::string symbol;
    };
    void SetProcessFunctionClassId(ProcessFunctionClassId class_id) {
      process_function_class_id_ = class_id;
    }
    ProcessFunctionClassId GetProcessFunctionClassId() {
      return process_function_class_id_;
    }
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

    virtual ProcessFunctionClassId GetBodyProcessFunctionClassId(
        ProductionBodyId production_body_id) {
      assert(production_body_id == 0);
      return GetProcessFunctionClassId();
    }

   private:
    // ����������
    AssociatityType operator_associatity_type_;
    // ��������ȼ�
    PriorityLevel operator_priority_level_;
    // �û����崦�������ID
    ProcessFunctionClassId process_function_class_id_;
  };
#endif  // USE_AMBIGUOUS_GRAMMAR

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
    NonTerminalProductionNode(NonTerminalProductionNode&& node)
        : BaseProductionNode(std::move(node)),
          nonterminal_bodys_(std::move(node.nonterminal_bodys_)),
          forward_nodes_(std::move(node.forward_nodes_)),
          core_ids_(std::move(node.core_ids_)),
          process_function_class_ids_(
              std::move(node.process_function_class_ids_)),
          empty_body_(std::move(node.empty_body_)) {}
    NonTerminalProductionNode& operator=(NonTerminalProductionNode&& node) {
      BaseProductionNode::operator=(std::move(node));
      nonterminal_bodys_ = std::move(node.nonterminal_bodys_);
      forward_nodes_ = std::move(node.forward_nodes_);
      core_ids_ = std::move(node.core_ids_);
      process_function_class_ids_ = std::move(node.process_function_class_ids_);
      empty_body_ = std::move(node.empty_body_);
      return *this;
    }

    // �������ò���ʽ����Ŀ����ͬʱ�����������
    // �����µ����в���ʽ�����Ŀ����Ҫ+1
    void ResizeProductionBodyNum(size_t new_size);
    // �������ò���ʽ����ڵ���Ŀ����ͬʱ�����������
    // �����µĲ���ʽ�������нڵ����Ŀ����Ҫ+1
    void ResizeProductionBodyNodeNum(ProductionBodyId production_body_id,
                                     size_t new_size);
    // ���һ������ʽ�壬Ҫ��IdTypeΪһ��vector������洢����ʽ�ڵ�ID
    template <class IdType>
    ProductionBodyId AddBody(IdType&& body);
    // ���ø�������ʽ��ID��Ӧ��ProcessFunctionClass��ID
    void SetBodyProcessFunctionClassId(ProductionBodyId body_id,
                                       ProcessFunctionClassId class_id) {
      assert(body_id < process_function_class_ids_.size());
      process_function_class_ids_[body_id] = class_id;
    }
    const ProductionBodyType& GetBody(ProductionBodyId body_id) const {
      return nonterminal_bodys_[body_id];
    }
    const BodyContainerType& GetAllBody() const { return nonterminal_bodys_; }
    // ���һ��������������ǰ���ڵ�ID
    template <class Container>
    void AddForwardNodeContainer(ProductionBodyId production_body_id,
                                 PointIndex point_index,
                                 Container&& forward_nodes_container) {
      assert(point_index < forward_nodes_[production_body_id].size());
      forward_nodes_[production_body_id][point_index].merge(
          std::forward<Container>(forward_nodes_container));
    }
    // ���øò���ʽ������Ϊ��
    void SetProductionShouldNotEmpty() { empty_body_ = false; }
    void SetProductionCouldBeEmpty() { empty_body_ = true; }
    // ��ѯ�ò���ʽ�Ƿ����Ϊ��
    bool CouldBeEmpty() { return empty_body_; }

    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) override;
    // �����ǰ���ڵ�
    virtual void AddForwardNodeId(ProductionBodyId production_body_id,
                                  PointIndex point_index,
                                  ProductionNodeId forward_node_id) {
      assert(point_index < forward_nodes_[production_body_id].size() &&
             forward_node_id.IsValid());
      forward_nodes_[production_body_id][point_index].insert(forward_node_id);
    }
    // ��ȡ��ǰ���ڵ㼯��
    virtual const std::unordered_set<ProductionNodeId>& GetForwardNodeIds(
        ProductionBodyId production_body_id, PointIndex point_index) {
      assert(point_index < forward_nodes_[production_body_id].size());
      return forward_nodes_[production_body_id][point_index];
    }
    // �������Ӧ�ĺ���ID
    virtual void SetCoreId(ProductionBodyId production_body_id,
                           PointIndex point_index, CoreId core_id) {
      assert(production_body_id < core_ids_.size() &&
             point_index < core_ids_[production_body_id].size());
      core_ids_[production_body_id][point_index] = core_id;
    }
    // ��ȡ���Ӧ�ĺ���ID
    virtual CoreId GetCoreId(ProductionBodyId production_body_id,
                             PointIndex point_index) {
      assert(production_body_id < core_ids_.size() &&
             point_index < core_ids_[production_body_id].size());
      return core_ids_[production_body_id][point_index];
    }
    // ���ظ�������ʽ��ID��Ӧ��ProcessFunctionClass��ID
    virtual ProcessFunctionClassId GetBodyProcessFunctionClassId(
        ProductionBodyId body_id) {
      assert(body_id < process_function_class_ids_.size());
      return process_function_class_ids_[body_id];
    }

   private:
    // ���vector��÷��ս�����¸�������ʽ����
    // �ڲ�vector��ò���ʽ�����ﺬ�еĸ������Ŷ�Ӧ�Ľڵ�ID
    BodyContainerType nonterminal_bodys_;
    // ��ǰ���ڵ㣬���������
    // ����ӦProductionBodyId�����ڲ���ʽ����Ŀ
    // �ڲ��ӦPointIndex����С�ȶ�Ӧ����ʽ���ڽڵ���Ŀ��1
    std::vector<std::vector<std::unordered_set<ProductionNodeId>>>
        forward_nodes_;
    // �洢���Ӧ�ĺ���ID
    std::vector<std::vector<CoreId>> core_ids_;
    // �洢ÿ������ʽ���Ӧ�İ�װ�û��Զ��庯�������ݵ�����ѷ������ID
    std::vector<ProcessFunctionClassId> process_function_class_ids_;
    // ��־�ò���ʽ�Ƿ����Ϊ��
    bool empty_body_ = false;
  };

  //// �ļ�β�ڵ�
  //class EndNode : public BaseProductionNode {
  // public:
  //  EndNode(SymbolId symbol_id)
  //      : BaseProductionNode(ProductionNodeType::kEndNode, symbol_id) {}
  //  EndNode(const EndNode&) = delete;
  //  EndNode& operator=(const EndNode&) = delete;
  //  EndNode(EndNode&& end_node) : BaseProductionNode(std::move(end_node)) {}
  //  EndNode& operator=(EndNode&& end_node) {
  //    BaseProductionNode::operator=(std::move(end_node));
  //    return *this;
  //  }
  //};
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
    // ����Ϊ��Լʱ�洢��װ���ú�������Ķ����ID�͹�Լ��õ��ķ��ս����ʽID
    // ����ʽID����ȷ������ڹ�Լ������ķ��ս����ʽ������ת��
    // ����Ϊ����ʱ�洢�����ת�Ƶ�����ĿID
    using TerminalNodeActionAndTargetContainerType = std::unordered_map<
        ProductionNodeId,
        std::pair<ActionType, std::variant<std::pair<ProductionNodeId,
                                                     ProcessFunctionClassId>,
                                           ParsingTableEntryId>>>;

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
        std::variant<std::pair<ProductionNodeId, ProcessFunctionClassId>,
                     ParsingTableEntryId>
            target_id) {
      action_and_target_[node_id] =
                 std::make_pair(action_type, target_id);
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
  ProductionBodyId body_id(nonterminal_bodys_.size());
  // ��������뵽����ʽ�������У���ɾ����ͬ����ʽ����
  nonterminal_bodys_.emplace_back(std::forward<IdType>(body));
  // Ϊ����ʽ������в�ͬλ�õĵ��Ӧ����ǰ�����������ռ�
  ResizeProductionBodyNum(nonterminal_bodys_.size());
  // Ϊ��ͬ���λ�������ռ�
  ResizeProductionBodyNodeNum(body_id, body.size());
  return body_id;
}

template <class Container>
inline void LexicalGenerator::TerminalProductionNode::AddForwardNodeContainer(
    ProductionBodyId production_body_id, PointIndex point_index,
    Container&& forward_nodes_id_container) {
  assert(production_body_id == 0 && point_index <= 1);
  if (point_index == 0) {
    AddFirstItemForwardNodeContainer(
        std::forward<Container>(forward_nodes_id_container));
  } else {
    AddSecondItemForwardNodeContainer(
        std::forward<Container>(forward_nodes_id_container));
  }
}

template <class T>
inline void LexicalGenerator::PrintProcessFunction(FILE* function_file,
                                                   const T& data) {
#ifdef USE_INIT_FUNCTION
  fprintf(function_file, "  virtual void Init() { return %s(); }\n",
          data.init_function.c_str());
#endif  // USE_INIT_FUNCTION
#ifdef USE_SHIFT_FUNCTION
#ifdef USE_USER_DATA
  fprintf(function_file, "  virtual UserData Shift(size_t index) { \n");
#else
  fprintf(function_file, "  virtual void Shift(size_t index) { \n");
#endif  // USE_USER_DATA
  fprintf(function_file, "    switch(index) {\n");
  for (int index = 0; index < data.shift_functions.size(); index++) {
    if (!data.shift_functions[index].empty()) {
      // ��ǰ�����뺯�����ǿպ������ӡcase
      fprintf(function_file, "      case %d:\n", index);
      fprintf(function_file, "      return %s();\n",
              data.shift_functions[index].c_str());
    }
  }
  fprintf(function_file, "      default:\n      break;\n    }\n  }");
#endif  // USE_SHIFT_FUNCTION
#ifdef USE_REDUCT_FUNCTION
#ifdef USE_USER_DATA
  if (!data.reduct_function.empty()) {
    fprintf(function_file,
            " virtual void Reduct(std::vector<UserData>&& user_data) { return "
            "%s(std::move(user_data)); }\n",
            data.reduct_function.c_str());
  }
#else
  if (!data.reduct_function.empty()) {
    fprintf(function_file, " virtual void Reduct() { return %s(); }\n",
            data.reduct_function.c_str());
  }
#endif  // USE_USER_DATA
#endif  // USE_REDUCT_FUNCTION
}

#ifdef USE_AMBIGUOUS_GRAMMAR
template <class ProcessFunctionClass>
inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddOperatorNode(
    const std::string& operator_symbol, AssociatityType associatity_type,
    PriorityLevel priority_level) {
  ProcessFunctionClassId class_id =
      CreateProcessFunctionClassObject<ProcessFunctionClass>();
  return AddOperatorNode(operator_symbol, associatity_type, priority_level,
                         class_id);
}
#endif  // USE_AMBIGUOUS_GRAMMAR

template <class ProcessFunctionClass>
inline LexicalGenerator::ProductionNodeId LexicalGenerator::AddNonTerminalNode(
    const std::string& node_symbol,
    const std::vector<std::string>& subnode_symbols) {
  ProcessFunctionClassId class_id =
      CreateProcessFunctionClassObject<ProcessFunctionClass>();
  return AddNonTerminalNode(node_symbol, subnode_symbols, class_id);
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

template <class Container>
inline void LexicalGenerator::AddForwardNodeContainer(
    ProductionNodeId production_node_id, ProductionBodyId production_body_id,
    PointIndex point_index, Container&& forward_nodes_container) {
  BaseProductionNode& production_node = GetProductionNode(production_node_id);
  switch (production_node.Type()) {
    case ProductionNodeType::kTerminalNode:
    case ProductionNodeType::kOperatorNode:
      static_cast<TerminalProductionNode&>(production_node)
          .AddForwardNodeContainer(
              production_body_id, point_index,
              std::forward<Container>(forward_nodes_container));
      break;
    case ProductionNodeType::kNonTerminalNode:
      static_cast<NonTerminalProductionNode&>(production_node)
          .AddForwardNodeContainer(
              production_body_id, point_index,
              std::forward<Container>(forward_nodes_container));
      break;
    default:
      assert(false);
      break;
  }
}

}  // namespace frontend::generator::lexicalgenerator
#endif  // !GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_