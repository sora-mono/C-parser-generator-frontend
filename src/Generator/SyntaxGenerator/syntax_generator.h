#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAXGENERATOR_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAXGENERATOR_H_

#include <any>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <cassert>
#include <fstream>
#include <functional>
#include <regex>
#include <tuple>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/object_manager.h"
#include "Common/unordered_struct_manager.h"
#include "Config/ProductionConfig/user_defined_header_files.h"
#include "Generator/DfaGenerator/dfa_generator.h"
#include "process_functions_classes.h"
#include "user_defined_function_and_data_register.h"

namespace frontend::parser::syntaxmachine {
class SyntaxMachine;
}

// TODO ���ɾ��δʹ�ò���ʽ�Ĺ���
namespace frontend::generator::syntaxgenerator {
using frontend::common::ObjectManager;
using frontend::common::UnorderedStructManager;

// �ӿ��࣬�����û����庯�����Ӹ�������
class ProcessFunctionInterface {
 public:
  using UserData = std::any;
  // �սᵥ�ʵ�����
  struct TerminalWordData {
    // ����
    std::string word;
    // ������������
    size_t line;
  };
  // ���ս�ڵ������
  struct NonTerminalWordData {
    NonTerminalWordData() = default;
    NonTerminalWordData(UserData&& user_data)
        : user_data_(std::move(user_data)) {}
    NonTerminalWordData(NonTerminalWordData&&) = default;
    NonTerminalWordData(const NonTerminalWordData&) = default;
    NonTerminalWordData& operator=(const NonTerminalWordData&) = default;
    UserData user_data_;
  };
  // �洢����ʽ�е����ڵ���Я������Ϣ
  struct WordDataToUser {
    // ���з��ս�ڵ���ս�ڵ����ֿ���
    frontend::common::ProductionNodeType node_type;
    // �õ��ʵ�����
    boost::variant<TerminalWordData, NonTerminalWordData> word_data_to_user_;
    TerminalWordData& GetTerminalWordData() {
      return boost::get<TerminalWordData>(word_data_to_user_);
    }
    NonTerminalWordData& GetNonTerminalWordData() {
      return boost::get<NonTerminalWordData>(word_data_to_user_);
    }
    template <class T>
    void SetWordData(T&& word_data) {
      word_data_to_user_ = word_data;
    }
  };
  // �������Խ����ջʱ��Խ��
  // ���ص�ֵ��Ϊ����ò���ʽ��Լ�õ��ķ��ս������һ�β����Լ���ݵĲ���
  // �չ�Լ�ڵ��node_type��ProductionNodeType::kEndNode
  virtual UserData Reduct(std::vector<WordDataToUser>&& user_data_) = 0;
  // ���л���
  virtual void serialize(boost::archive::binary_oarchive& ar,
                         const unsigned int version) {}
};

class SyntaxGenerator {
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
  using CoreId = ObjectManager<Core>::ObjectId;
  // �﷨��������ĿID
  using ParsingTableEntryId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kParsingTableEntryId>;

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ��������ȼ�������Խ�����ȼ�Խ�ߣ�����������ڵ���Ч
  // ��TailNodePriority���岻ͬ�������ȼ�Ӱ���﷨��������
  // �����������������ʱ�������뻹�ǹ鲢
  using OperatorPriority =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPriorityLevel>;
#endif  // USE_AMBIGUOUS_GRAMMAR

  // �ʷ������дʵ����ȼ�
  using WordPriority =
      frontend::generator::dfa_generator::DfaGenerator::WordPriority;
  // ���λ��
  using PointIndex =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kPointIndex>;
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
  using SymbolId =
      UnorderedStructManager<std::string, std::hash<std::string>>::ObjectId;
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
  using ProcessFunctionClassId = frontend::common::ObjectManager<
      frontend::generator::syntaxgenerator::ProcessFunctionInterface>::ObjectId;
  // �����װ�û��Զ��庯�������ݵ�����ѷ�����������
  using ProcessFunctionClassManagerType =
      ObjectManager<ProcessFunctionInterface>;
  // �����������ͣ����ϣ��ҽ��
  using AssociatityType = frontend::common::AssociatityType;
  // �����������ͣ���Լ�����룬����͹�Լ����������
  enum class ActionType { kReduct, kShift, kShiftReduct, kError, kAccept };

#ifdef USE_AMBIGUOUS_GRAMMAR
  // ���������
  struct OperatorData {
    std::string operator_symbol;
    std::string operator_priority;
    std::string associatity_type;
    std::string reduct_function;
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
    std::string reduct_function;
#ifdef USE_USER_DEFINED_FILE
    std::vector<std::string> include_files;
#endif  // USE_USER_DEFINED_FILE

    // �ò���ʽ�ڵ��Ƿ�ֻ��������ͬ���ս����ʽ
    // ����Example -> "int"|"char"|"double"
    // ��Щ�ս����ʽ�����ô�����
    bool use_same_process_function_class = false;
  };

 public:
  SyntaxGenerator() { SyntaxGeneratorInit(); }
  ~SyntaxGenerator() { CloseConfigFile(); }

  // �����ؼ���
  void AnalysisKeyWord(const std::string& str);
  // �����ս�ڵ����ʽ�ļ�
  void AnalysisProductionConfig(const std::string& file_name);
  // �����ս�ڵ����ʽ�������ұ�������һ�������Ĳ���ʽ
  // ��ͨ�ս�ڵ�Ĭ�����ȼ�Ϊ0����ͣ�
  void AnalysisTerminalProduction(const std::string& str,
                                  size_t operator_priority = 0);
  // �������������ʽ�������ұ�������һ�������Ĳ���ʽ
  void AnalysisOperatorProduction(const std::string& str);
  // �������ս�ڵ����ʽ�������ұ�������һ�������Ĳ���ʽ
  void AnalysisNonTerminalProduction(const std::string& str);
  // ����LALR����ĸ�����Χ��Ϣ
  void ConfigConstruct();
  // ����LALR(1)����
  void ParsingTableConstruct();
  // ��ʼ��
  void SyntaxGeneratorInit();

 private:
  FILE*& GetConfigConstructFilePointer() { return config_construct_code_file_; }
  FILE*& GetProcessFunctionClassFilePointer() {
    return process_function_class_file_;
  }
  FILE*& GetUserDefinedFunctionAndDataRegisterFilePointer() {
    return user_defined_function_and_data_register_file_;
  }
  // �������ļ�
  void OpenConfigFile();
  // �ر������ļ�
  void CloseConfigFile();
  // ��ȡһ���ڵ���
  int GetNodeNum() { return node_num_++; }
  // ��λ�ڵ���
  void NodeNumInit() { node_num_ = 0; }
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
                                      size_t operator_priority);
  // �ӹ��̣��������ļ�д��Reduct����
  // data�к�����Ϊ���򷵻�false
  template <class T>
  bool PrintProcessFunction(FILE* function_file, const T& data);
#ifdef USE_AMBIGUOUS_GRAMMAR
  // �������ļ���д������������Ĳ���
  // data�е����ȼ�����Ϊ�����ַ�L��R
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
  // ��ȡ����ʽ��Ĳ���ʽ�ڵ�ID��Խ��ʱ����ProductionNodeId::InvalidId()
  ProductionNodeId GetProductionNodeIdInBody(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      PointIndex point_index) {
    return GetProductionNode(production_node_id)
        .GetProductionNodeInBody(production_body_id, point_index);
  }
  // ���ø�����ʽID
  void SetRootProductionNodeId(ProductionNodeId root_production_node_id) {
    root_production_node_id_ = root_production_node_id;
  }
  // ��ȡ������ʽID
  ProductionNodeId GetRootProductionNodeId() {
    return root_production_node_id_;
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
    NonTerminalProductionNode& production_node =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id));
    assert(production_node.Type() == ProductionNodeType::kNonTerminalNode);
    return production_node.GetBodyProcessFunctionClassId(production_body_id);
  }
  // �����Ϊ����ʽ��δ��������²��ܼ�����ӵķ��ս�ڵ�
  // ��һ������Ϊδ����Ĳ���ʽ��
  // ������������ͬAddNonTerminalNode
  // �����Ḵ��һ�ݸ��������豣��ԭ���Ĳ�������������
  void AddUnableContinueNonTerminalNode(
      const std::string& undefined_symbol, std::string&& node_symbol,
      std::vector<std::string>&& subnode_symbols,
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
  // �ú������������ս����ʽ��������
  // �ڵ��Ѵ����Ҹ���symbol_id��ͬ������ID�򷵻�ProductionNodeId::InvalidId()
  // �����������Ǵ����ȼ���0����Ϊ��ͨ�ʵ����ȼ���1����Ϊ��������ȼ�
  // 2����Ϊ�ؼ������ȼ�����������ȼ���δָ��
  // �����������ȼ�����������ȼ���ͬ����ע�����֣�����
  ProductionNodeId AddTerminalNode(
      const std::string& node_symbol, const std::string& body_symbol,
      WordPriority node_priority = WordPriority(0));
  // �ӹ��̣������ڴ����ڵ�
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddTerminalNode(SymbolId node_symbol_id,
                                      SymbolId body_symbol_id);

#ifdef USE_AMBIGUOUS_GRAMMAR
  // �½�������ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  // Ĭ����ӵ�������ʷ��������ȼ�������������ȼ�
  ProductionNodeId AddOperatorNode(const std::string& operator_symbol,
                                   AssociatityType associatity_type,
                                   OperatorPriority priority_level);
  // �ӹ��̣������ڴ����ڵ�
  // ������ڵ���ͬ�������
  // �Զ����½ڵ���ID���ڵ�ID��ӳ���
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddOperatorNode(SymbolId node_symbol_id,
                                      AssociatityType associatity_type,
                                      OperatorPriority priority_level);
#endif  // USE_AMBIGUOUS_GRAMMAR

  // �½����ս�ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򲻻ᴴ���µĽڵ�
  // node_symbolΪ����ʽ����subnode_symbols�ǲ���ʽ��
  // class_id������ӵİ�װ�û��Զ��庯�������ݵ���Ķ���ID
  // ���ģ�庯���ͷ�ģ�庯��Ϊ�˽��ʹ�������������ֹ��������
  // ����������������ֱ�ӵ��ã�class_id�ǰ�װ�û����庯�������ݵ���Ķ���ID
  template <class ProcessFunctionClass>
  ProductionNodeId AddNonTerminalNode(
      std::string&& node_symbol, std::vector<std::string>&& subnode_symbols);
  ProductionNodeId AddNonTerminalNode(
      std::string&& node_symbol, std::vector<std::string>&& subnode_symbols,
      ProcessFunctionClassId class_id);
  // �ӹ��̣������ڴ����ڵ�
  // �Զ����½ڵ���ID���ڵ�ID��ӳ���
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddNonTerminalNode(SymbolId symbol_id);
  // �½��ļ�β�ڵ㣬���ؽڵ�ID
  ProductionNodeId AddEndNode();
  // ���ò���ʽ���ڵ�
  void SetRootProduction(const std::string& production_node_name);

  // ��ȡ�ڵ�
  BaseProductionNode& GetProductionNode(ProductionNodeId production_node_id) {
    return manager_nodes_[production_node_id];
  }
  BaseProductionNode& GetProductionNodeFromNodeSymbolId(SymbolId symbol_id) {
    ProductionNodeId production_node_id =
        GetProductionNodeIdFromNodeSymbolId(symbol_id);
    assert(symbol_id.IsValid());
    return GetProductionNode(production_node_id);
  }
  BaseProductionNode& GetProductionNodeBodyFromSymbolId(SymbolId symbol_id) {
    ProductionNodeId production_node_id =
        GetProductionNodeIdFromBodySymbolId(symbol_id);
    assert(symbol_id.IsValid());
    return GetProductionNode(production_node_id);
  }
  // ��ȡ���ս�ڵ��е�һ������ʽ��
  const std::vector<ProductionNodeId>& GetProductionBody(
      ProductionNodeId production_node_id,
      ProductionBodyId production_body_id) {
    NonTerminalProductionNode& nonterminal_node =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id));
    // ֻ�з��ս�ڵ���ж������ʽ�壬�������ڵ���øú���������
    assert(nonterminal_node.Type() == ProductionNodeType::kNonTerminalNode);
    return nonterminal_node.GetBody(production_body_id);
  }
  // �����ս�ڵ���Ӳ���ʽ��
  template <class IdType>
  void AddNonTerminalNodeBody(ProductionNodeId node_id, IdType&& body) {
    static_cast<NonTerminalProductionNode&>(GetProductionNode(node_id))
        .AddBody(std::forward<IdType>(body));
  }
  // ���һ���﷨��������Ŀ
  ParsingTableEntryId AddParsingTableEntry() {
    ParsingTableEntryId parsing_table_entry_id(
        syntax_config_parsing_table_.size());
    syntax_config_parsing_table_.emplace_back();
    return parsing_table_entry_id;
  }
  // ��ȡcore_id��Ӧ����ʽ�
  Core& GetCore(CoreId core_id) {
    assert(core_id < cores_.Size());
    return cores_[core_id];
  }
  // �����﷨��������ĿID������ID��ӳ��
  void SetParsingTableEntryIdToCoreIdMapping(
      ParsingTableEntryId parsing_table_entry_id, CoreId core_id) {
    parsing_table_entry_id_to_core_id_[parsing_table_entry_id] = core_id;
  }
  // ��ȡ�﷨��������ĿID��Ӧ�ĺ���ID
  CoreId GetCoreIdFromParsingTableEntryId(
      ParsingTableEntryId parsing_table_entry_id) {
    auto iter = parsing_table_entry_id_to_core_id_.find(parsing_table_entry_id);
    assert(iter != parsing_table_entry_id_to_core_id_.end());
    return iter->second;
  }
  // ����µĺ��ģ���Ҫ�����﷨��������Ŀ������ID��ӳ��
  CoreId EmplaceCore() {
    ParsingTableEntryId parsing_table_entry_id = AddParsingTableEntry();
    CoreId core_id = cores_.EmplaceObject(parsing_table_entry_id);
    cores_[core_id].SetCoreId(core_id);
    SetParsingTableEntryIdToCoreIdMapping(parsing_table_entry_id, core_id);
    return core_id;
  }
  // �������������Ӧ����ǰ�����ţ����Դ��뵥��δ��װID
  // ��������Ѵ�����������ǰ������
  template <class ForwardNodeIdContainer>
  std::pair<std::map<CoreItem, std::unordered_set<ProductionNodeId>>::iterator,
            bool>
  AddItemAndForwardNodeIdsToCore(CoreId core_id, const CoreItem& core_item,
                                 ForwardNodeIdContainer&& forward_node_ids);
  // ��ȡ��ǰ�����ż�
  const std::unordered_set<ProductionNodeId>& GetForwardNodeIds(
      CoreId core_id, const CoreItem& core_item) {
    return GetCore(core_id).GetItemsAndForwardNodeIds().at(core_item);
  }
  // �����ǰ�����ţ�����ֻ���뵥��δ����װ��ID
  template <class ForwardNodeIdContainer>
  void AddForwardNode(CoreId core_id, const CoreItem& core_item,
                      ForwardNodeIdContainer&& forward_node_ids) {
    GetCore(core_id).AddForwardNode(
        core_item, std::forward<ForwardNodeIdContainer>(forward_node_ids));
  }
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
      const InsideForwardNodesContainerType& next_node_ids =
          InsideForwardNodesContainerType());
  // ��ȡ���ĵ�ȫ����
  const std::map<CoreItem, std::unordered_set<ProductionNodeId>>&
  GetCoreItemsAndForwardNodes(CoreId core_id) {
    return GetCore(core_id).GetItemsAndForwardNodeIds();
  }
  // ��ȡ�﷨��������Ŀ
  ParsingTableEntry& GetParsingTableEntry(
      ParsingTableEntryId production_node_id) {
    assert(production_node_id < syntax_config_parsing_table_.size());
    return syntax_config_parsing_table_[production_node_id];
  }
  // ���ø��﷨��������ĿID
  void SetRootParsingTableEntryId(
      ParsingTableEntryId root_parsing_table_entry_id) {
    root_parsing_table_entry_id_ = root_parsing_table_entry_id;
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
  // �Զ�������е�ǰλ�ÿ��Կչ�Լ����ĺ�����
  void CoreClosure(CoreId core_id);
  // ������ǰ�����ţ�ͬʱ�ڴ��������й����﷨������
  void SpreadLookForwardSymbolAndConstructParsingTableEntry(CoreId core_id);
  // �����в���ʽ�ڵ㰴��ProductionNodeType����
  // ����array�ڵ�vector�����Ͷ�Ӧ���±�ΪProductionNodeType�����͵�ֵ
  std::array<std::vector<ProductionNodeId>, sizeof(ProductionNodeType)>
  ClassifyProductionNodes();
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
          moved_entry_id_to_new_entry_id);
  // �ϲ��﷨����������ͬ���ͬʱ�����﷨�������С
  // �Ὣ�﷨�������ڵ����޸�Ϊ�µ���
  void ParsingTableMergeOptimize();

  // boost-serialization���������﷨���������õĺ���
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const;
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  // ���﷨����������д���ļ�
  void SaveConfig() {
    std::ofstream config_file("user_defined_function_and_data_register.h");
    boost::archive::binary_oarchive oarchive(config_file);
    dfa_generator_.SaveConfig();
    oarchive << *this;
  }

  // �����﷨������Ϊ���࣬������ʹ�ø��ֶ���
  friend class frontend::parser::syntaxmachine::SyntaxMachine;
  // ���л���Ҫ
  friend class boost::serialization::access;

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
  // boost���л�������ʱ��Ҫע���������������
  // ���ļ��ﴢ���û�����ĺ������ݶ������͵�����
  FILE* user_defined_function_and_data_register_file_ = nullptr;
  // Generator���ɵ���Χ���ݼ��ش����ļ�(ConfigConstruct�����ľ���ʵ��)
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
      std::tuple<std::string, std::vector<std::string>, ProcessFunctionClassId>>
      undefined_productions_;
  // �����ս���š����ս���ŵȵĽڵ�
  ObjectManager<BaseProductionNode> manager_nodes_;
  // �洢����ʽ��(�ս�/���ս�/��������ķ���
  UnorderedStructManager<std::string, std::hash<std::string>>
      manager_node_symbol_;
  // �洢�ս�ڵ����ʽ��ķ��ţ�������ֹ������ͬһ����
  UnorderedStructManager<std::string, std::hash<std::string>>
      manager_terminal_body_symbol_;
  // ����ʽ��ID����Ӧ����ʽ�ڵ��ӳ��
  std::unordered_map<SymbolId, ProductionNodeId> node_symbol_id_to_node_id_;
  // �ս����ʽ�����ID����Ӧ�ڵ�ID��ӳ��
  std::unordered_map<SymbolId, ProductionNodeId>
      production_body_symbol_id_to_node_id_;
  // �ں�+���ں��
  ObjectManager<Core> cores_;
  // �﷨������ID������ID��ӳ��
  std::unordered_map<ParsingTableEntryId, CoreId>
      parsing_table_entry_id_to_core_id_;
  // ������ʽ��ĿID
  ProductionNodeId root_production_node_id_;
  // ��ʼ�﷨��������ĿID������д���ļ�
  ParsingTableEntryId root_parsing_table_entry_id_;
  // DFA����������������д���ļ�
  frontend::generator::dfa_generator::DfaGenerator dfa_generator_;
  // �﷨����������д���ļ�
  ParsingTableType syntax_config_parsing_table_;
  // �û��Զ��庯�������ݵ���Ķ�������д���ļ�
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
    virtual ~BaseProductionNode() {}

    struct NodeData {
      ProductionNodeType node_type;
      std::string node_symbol;
    };

    void SetType(ProductionNodeType type) { base_type_ = type; }
    ProductionNodeType Type() const { return base_type_; }
    void SetThisNodeId(ProductionNodeId production_node_id) {
      base_id_ = production_node_id;
    }
    ProductionNodeId Id() const { return base_id_; }
    void SetSymbolId(SymbolId production_node_id) {
      base_symbol_id_ = production_node_id;
    }
    SymbolId GetNodeSymbolId() const { return base_symbol_id_; }

    // ��ȡ�������Ӧ�Ĳ���ʽID������point_index�����ID
    // point_indexԽ��ʱ����ProducNodeId::InvalidId()
    // Ϊ��֧����ǰ������ڵ�����Խ��
    // ���ص��ұߵĲ���ʽID���������򷵻�ProductionNodeId::InvalidId()
    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) = 0;

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
          body_symbol_id_(std::move(terminal_production_node.body_symbol_id_)) {
    }
    TerminalProductionNode& operator=(
        TerminalProductionNode&& terminal_production_node) {
      BaseProductionNode::operator=(std::move(terminal_production_node));
      body_symbol_id_ = std::move(terminal_production_node.body_symbol_id_);
      return *this;
    }
    using NodeData = BaseProductionNode::NodeData;

    // ��ȡ/���ò���ʽ����
    SymbolId GetBodySymbolId() { return body_symbol_id_; }
    void SetBodySymbolId(SymbolId body_symbol_id) {
      body_symbol_id_ = body_symbol_id;
    }
    // ��Խ��ʱҲ�з���ֵΪ��֧�ֻ�ȡ��һ��/���¸����ڽڵ�Ĳ���
    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) override;

   private:
    // ����ʽ����
    SymbolId body_symbol_id_;
  };

#ifdef USE_AMBIGUOUS_GRAMMAR
  class OperatorProductionNode : public BaseProductionNode {
   public:
    OperatorProductionNode(SymbolId node_symbol_id,
                           AssociatityType associatity_type,
                           OperatorPriority priority_level)
        : BaseProductionNode(ProductionNodeType::kOperatorNode, node_symbol_id),
          operator_associatity_type_(associatity_type),
          operator_priority_level_(priority_level) {}
    OperatorProductionNode(const OperatorProductionNode&) = delete;
    OperatorProductionNode& operator=(const OperatorProductionNode&) = delete;
    OperatorProductionNode(OperatorProductionNode&& operator_production_node)
        : BaseProductionNode(std::move(operator_production_node)),
          operator_associatity_type_(
              std::move(operator_production_node.operator_associatity_type_)),
          operator_priority_level_(
              std::move(operator_production_node.operator_priority_level_)) {}
    OperatorProductionNode& operator=(
        OperatorProductionNode&& operator_production_node);

    struct NodeData : public TerminalProductionNode::NodeData {
      std::string symbol_;
    };
    void SetAssociatityType(AssociatityType type) {
      operator_associatity_type_ = type;
    }
    AssociatityType GetAssociatityType() const {
      return operator_associatity_type_;
    }
    void SetPriorityLevel(OperatorPriority level) {
      operator_priority_level_ = level;
    }
    OperatorPriority GetPriorityLevel() const {
      return operator_priority_level_;
    }
    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) {
      assert(false);
    }

   private:
    // ����������
    AssociatityType operator_associatity_type_;
    // ��������ȼ�
    OperatorPriority operator_priority_level_;
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
          process_function_class_ids_(
              std::move(node.process_function_class_ids_)),
          empty_body_(std::move(node.empty_body_)) {}
    NonTerminalProductionNode& operator=(NonTerminalProductionNode&& node) {
      BaseProductionNode::operator=(std::move(node));
      nonterminal_bodys_ = std::move(node.nonterminal_bodys_);
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
    // ��ȡ����ʽ��һ����
    const std::vector<ProductionNodeId>& GetProductionBody(
        ProductionBodyId production_body_id) {
      assert(production_body_id < nonterminal_bodys_.size());
      return nonterminal_bodys_[production_body_id];
    }
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
    // ��ȡȫ������ʽ��ID
    std::vector<ProductionBodyId> GetAllBodyIds() const;
    // ���øò���ʽ������Ϊ��
    void SetProductionShouldNotEmpty() { empty_body_ = false; }
    void SetProductionCouldBeEmpty() { empty_body_ = true; }
    // ��ѯ�ò���ʽ�Ƿ����Ϊ��
    bool CouldBeEmpty() { return empty_body_; }

    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) override;
    // ���ظ�������ʽ��ID��Ӧ��ProcessFunctionClass��ID
    ProcessFunctionClassId GetBodyProcessFunctionClassId(
        ProductionBodyId body_id) {
      assert(body_id < process_function_class_ids_.size());
      return process_function_class_ids_[body_id];
    }

   private:
    // ���vector��÷��ս�����¸�������ʽ����
    // �ڲ�vector��ò���ʽ�����ﺬ�еĸ������Ŷ�Ӧ�Ľڵ�ID
    BodyContainerType nonterminal_bodys_;
    // �洢ÿ������ʽ���Ӧ�İ�װ�û��Զ��庯�������ݵ�����ѷ������ID
    std::vector<ProcessFunctionClassId> process_function_class_ids_;
    // ��־�ò���ʽ�Ƿ����Ϊ��
    bool empty_body_ = false;
  };

  // �ļ�β�ڵ�
  class EndNode : public BaseProductionNode {
   public:
    EndNode()
        : BaseProductionNode(ProductionNodeType::kEndNode,
                             SymbolId::InvalidId()) {}
    EndNode(const EndNode&) = delete;
    EndNode& operator=(const EndNode&) = delete;
    EndNode(EndNode&& end_node) : BaseProductionNode(std::move(end_node)) {}
    EndNode& operator=(EndNode&& end_node) {
      BaseProductionNode::operator=(std::move(end_node));
      return *this;
    }
    virtual ProductionNodeId GetProductionNodeInBody(
        ProductionBodyId production_body_id, PointIndex point_index) {
      assert(false);
      return ProductionNodeId::InvalidId();
    }
    // ��ȡ��װ�û��Զ��庯�����ݵ���Ķ���ID
    virtual ProcessFunctionClassId GetBodyProcessFunctionClassId(
        ProductionBodyId production_body_id) {
      assert(false);
      return ProcessFunctionClassId::InvalidId();
    }
  };

  // �����ǰ������
  class Core {
   public:
    Core() {}
    Core(ParsingTableEntryId parsing_table_entry_id)
        : parsing_table_entry_id_(parsing_table_entry_id) {}
    template <class ItemAndForwardNodes>
    Core(ItemAndForwardNodes&& item_and_forward_node_ids,
         ParsingTableEntryId parsing_table_entry_id)
        : core_closure_available_(false),
          parsing_table_entry_id_(parsing_table_entry_id),
          item_and_forward_node_ids_(
              std::forward<ItemAndForwardNodes>(item_and_forward_node_ids)) {}
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;
    Core(Core&& core)
        : core_closure_available_(std::move(core.core_closure_available_)),
          core_id_(std::move(core.core_id_)),
          parsing_table_entry_id_(std::move(core.parsing_table_entry_id_)),
          item_and_forward_node_ids_(
              std::move(core.item_and_forward_node_ids_)) {}
    Core& operator=(Core&& core);
    // ���ظ���Item������iterator���Ƿ�ɹ�����bool���
    // ���Item�Ѵ�����������ǰ������
    // bool�ڲ����ڸ���item�Ҳ���ɹ�ʱΪtrue
    // ����ʹ�õ���δ��װID
    template <class ForwardNodeIdContainer>
    std::pair<
        std::map<CoreItem, std::unordered_set<ProductionNodeId>>::iterator,
        bool>
    AddItemAndForwardNodeIds(const CoreItem& item,
                             ForwardNodeIdContainer&& forward_node_ids);
    // �жϸ���item�Ƿ��ڸ���ڣ����򷵻�true
    bool IsItemIn(const CoreItem& item) const {
      return item_and_forward_node_ids_.find(item) !=
             item_and_forward_node_ids_.end();
    }
    // �жϸ����ıհ��Ƿ���Ч
    bool IsClosureAvailable() const { return core_closure_available_; }
    // ����core_id
    void SetCoreId(CoreId core_id) { core_id_ = core_id; }
    // ��ȡcore_id
    CoreId GetCoreId() const { return core_id_; }

    // ���ø����ıհ���Ч����Ӧ�ɱհ���������
    void SetClosureAvailable() { core_closure_available_ = true; }
    // ���ø������ıհ���Ч��Ӧ��ÿ���޸���core_items_�ĺ�������
    void SetClosureNotAvailable() { core_closure_available_ = false; }
    // ��ȡȫ����Ͷ�Ӧ����ǰ���ڵ�
    const std::map<CoreItem, std::unordered_set<ProductionNodeId>>&
    GetItemsAndForwardNodeIds() const {
      return item_and_forward_node_ids_;
    }
    // �������Ӧ���﷨��������ĿID
    void SetParsingTableEntryId(ParsingTableEntryId parsing_table_entry_id) {
      parsing_table_entry_id_ = parsing_table_entry_id;
    }
    // ��ȡ���Ӧ���﷨��������ĿID
    ParsingTableEntryId GetParsingTableEntryId() const {
      return parsing_table_entry_id_;
    }
    // ��������������ǰ�����ţ��Ե����ڵ��ػ�
    void AddForwardNode(
        const std::map<CoreItem,
                       std::unordered_set<ProductionNodeId>>::iterator& iter,
        ProductionNodeId forward_node_id) {
      iter->second.insert(forward_node_id);
    }
    // ��������������ǰ�����ţ��������ػ�
    template <class ForwardNodeIdContainer>
    void AddForwardNode(
        const std::map<CoreItem,
                       std::unordered_set<ProductionNodeId>>::iterator& iter,
        const ForwardNodeIdContainer& forward_node_id_container) {
      iter->second.insert(forward_node_id_container.begin(),
                          forward_node_id_container.end());
    }
    size_t Size() const { return item_and_forward_node_ids_.size(); }

   private:
    // �����ıհ��Ƿ���Ч������հ���֮��û�����κθ�����Ϊtrue��
    bool core_closure_available_ = false;
    // �ID
    CoreId core_id_ = CoreId::InvalidId();
    // ���Ӧ���﷨��������ĿID
    ParsingTableEntryId parsing_table_entry_id_ =
        ParsingTableEntryId::InvalidId();
    // ��Ͷ�Ӧ����ǰ������
    std::map<CoreItem, InsideForwardNodesContainerType>
        item_and_forward_node_ids_;
  };

  // �﷨��������Ŀ
  class ParsingTableEntry {
   public:
    // ִ�����붯��ʱ�ĸ�������
    struct ShiftAttachedData {
      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar& next_entry_id_;
      }

      // ����õ��ʺ�ת�Ƶ����﷨��������ĿID
      ParsingTableEntryId next_entry_id_;
    };
    // ִ�й�Լ����ʱ�ĸ�������
    struct ReductAttachedData {
      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar& reducted_nonterminal_node_id_;
        ar& process_function_class_id_;
        ar& production_body_;
      }

      // ��Լ��õ��ķ��ս�ڵ��ID
      ProductionNodeId reducted_nonterminal_node_id_;
      // ִ�й�Լ����ʱʹ�õĶ����ID
      ProcessFunctionClassId process_function_class_id_;
      // ��Լ���ò���ʽ�����ں˶Ըò���ʽ������Щ�ڵ�
      // ��ʹ�ÿչ�Լ������ɸ�Ϊ����ʽ�ڵ���Ŀ
      std::vector<ProductionNodeId> production_body_;
    };

#ifdef USE_AMBIGUOUS_GRAMMAR
    // ʹ�ö������ķ�ʱ��һ�����ʼȿ�������Ҳ���Թ�Լ
    struct ShiftReductAttachedData {
      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar& shift_attached_data_;
        ar& reduct_attached_data_;
      }

      ShiftAttachedData shift_attached_data_;
      ReductAttachedData reduct_attached_data_;
    };
#endif  // USE_AMBIGUOUS_GRAMMAR

    // ������һ������ʱ�Ķ����͸ö�������������
    struct ActionAndAttachedData {
      // ��ȡ��������ĸ�������
      // ���Ի�ȡ�Ѿ�ת��kShiftReduct���͵����������������
      const ShiftAttachedData& GetShiftAttachedData() const;
      // ��ȡ��Լ�����ĸ�������
      // ���Ի�ȡ�Ѿ�ת��kShiftReduct���͵Ĺ�Լ������������
      const ReductAttachedData& GetReductAttachedData() const;

#ifdef USE_AMBIGUOUS_GRAMMAR
      const ShiftReductAttachedData& GetShiftReductAttachedData() const {
        return boost::get<ShiftReductAttachedData>(attached_data_);
      }
      // ���е�ReductAttachedData����ShiftAttachedData
      // ת����ShiftReductAttachedData
      void ConvertToShiftReductAttachedData(
          const ShiftAttachedData& shift_attached_data) {
        assert(action_type_ == ActionType::kReduct);
        action_type_ = ActionType::kShiftReduct;
        SetAttachedData(ShiftReductAttachedData(
            shift_attached_data, std::move(GetReductAttachedData())));
      }
      // �����е�ShiftAttachedData����ReductAttachedData
      // ת����ShiftReductAttachedData
      void ConvertToShiftReductAttachedData(
          const ReductAttachedData& reduct_attached_data) {
        assert(action_type_ == ActionType::kShift);
        action_type_ = ActionType::kShiftReduct;
        SetAttachedData(ShiftReductAttachedData(
            std::move(GetShiftAttachedData()), reduct_attached_data));
      }
#endif  // USE_AMBIGUOUS_GRAMMAR

      template <class T>
      void SetAttachedData(T&& attached_data) {
        attached_data_ = std::forward<T>(attached_data);
      }
      // ����֧�ֽ�ActionAndAttachedData��Ϊmap��ֵ�Ĳ���
      bool operator<(const ActionAndAttachedData& right) const {
        return memcmp(this, &right, sizeof(ActionAndAttachedData));
      }

      ActionType action_type_;
      boost::variant<
#ifdef USE_AMBIGUOUS_GRAMMAR
          ShiftReductAttachedData,
#endif  // USE_AMBIGUOUS_GRAMMAR
          ShiftAttachedData, ReductAttachedData>
          attached_data_;
     private:
      // ֻ��ParsingTableEntry���Ե��÷�const����
      friend class ParsingTableEntry;
      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar& action_type_;
        ar& attached_data_;
      }

      ShiftAttachedData& GetShiftAttachedData() {
        return const_cast<ShiftAttachedData&>(
            static_cast<const ActionAndAttachedData&>(*this)
                .GetShiftAttachedData());
      }
      ReductAttachedData& GetReductAttachedData() {
        return const_cast<ReductAttachedData&>(
            static_cast<const ActionAndAttachedData&>(*this)
                .GetReductAttachedData());
      }
#ifdef USE_AMBIGUOUS_GRAMMAR
      ShiftReductAttachedData& GetShiftReductAttachedData() {
        return const_cast<ShiftReductAttachedData&>(
            static_cast<const ActionAndAttachedData&>(*this)
                .GetShiftReductAttachedData());
      }
#endif  // USE_AMBIGUOUS_GRAMMAR
    };
    // ����Ϊ��Լʱ�洢��װ���ú�������Ķ����ID�͹�Լ��õ��ķ��ս����ʽID
    // ����ʽID����ȷ������ڹ�Լ������ķ��ս����ʽ������ת��
    // ����Ϊ����ʱ�洢�����ת�Ƶ�����ĿID
    using ActionAndTargetContainer =
        std::unordered_map<ProductionNodeId, ActionAndAttachedData>;

    ParsingTableEntry() {}
    ParsingTableEntry(const ParsingTableEntry&) = delete;
    ParsingTableEntry& operator=(const ParsingTableEntry&) = delete;
    ParsingTableEntry(ParsingTableEntry&& parsing_table_entry)
        : action_and_attached_data_(
              std::move(parsing_table_entry.action_and_attached_data_)),
          nonterminal_node_transform_table_(std::move(
              parsing_table_entry.nonterminal_node_transform_table_)) {}
    ParsingTableEntry& operator=(ParsingTableEntry&& parsing_table_entry);

    // ���øò���ʽ��ת�������µĶ�����Ŀ��ڵ�
    template <class AttachedData>
    void SetTerminalNodeActionAndAttachedData(ProductionNodeId node_id,
                                              ActionType action_type_,
                                              AttachedData&& attached_data);
    // ���ø���Ŀ������ս�ڵ��ת�Ƶ��Ľڵ�
    void SetNonTerminalNodeTransformId(ProductionNodeId node_id,
                                       ParsingTableEntryId production_node_id) {
      nonterminal_node_transform_table_[node_id] = production_node_id;
    }
    // �޸ĸ���Ŀ��������ĿIDΪ��ID
    // ��ǰ����½��޸�����ʱת�Ƶ�����һ����ĿID�������ս�ڵ�/���ս�ڵ㣩
    void ResetEntryId(
        const std::unordered_map<ParsingTableEntryId, ParsingTableEntryId>&
            old_entry_id_to_new_entry_id);
    // ���ʸ���Ŀ�¸���ID�ս�ڵ����Ϊ��Ŀ��ID
    // ��������ڸ�ת�������򷵻ؿ�ָ��
    const ActionAndAttachedData* AtTerminalNode(
        ProductionNodeId node_id) const {
      auto iter = action_and_attached_data_.find(node_id);
      return iter == action_and_attached_data_.end() ? nullptr : &iter->second;
    }
    // ���ʸ���Ŀ�¸���ID���ս�ڵ�����ʱת�Ƶ�����ĿID
    // �����ڸ�ת�������򷵻�ParsingTableEntryId::InvalidId()
    ParsingTableEntryId AtNonTerminalNode(ProductionNodeId node_id) const {
      auto iter = nonterminal_node_transform_table_.find(node_id);
      return iter == nonterminal_node_transform_table_.end()
                 ? ParsingTableEntryId::InvalidId()
                 : iter->second;
    }
    // ��ȡȫ���ս�ڵ�Ĳ���
    const ActionAndTargetContainer& GetAllActionAndAttachedData() const {
      return action_and_attached_data_;
    }
    // ��ȡȫ�����ս�ڵ�ת�Ƶ��ı���
    const std::unordered_map<ProductionNodeId, ParsingTableEntryId>&
    GetAllNonTerminalNodeTransformTarget() const {
      return nonterminal_node_transform_table_;
    }

   private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& action_and_attached_data_;
      ar& nonterminal_node_transform_table_;
    }

    // ֻ����������޸�ԭʼ���ݽṹ����������Ҫ�߽ӿ�
    ActionAndTargetContainer& GetAllActionAndAttachedData() {
      return const_cast<ActionAndTargetContainer&>(
          static_cast<const ParsingTableEntry&>(*this)
              .GetAllActionAndAttachedData());
    }
    // ��ȡȫ�����ս�ڵ�ת�Ƶ��ı���
    std::unordered_map<ProductionNodeId, ParsingTableEntryId>&
    GetAllNonTerminalNodeTransformTarget() {
      return const_cast<
          std::unordered_map<ProductionNodeId, ParsingTableEntryId>&>(
          static_cast<const ParsingTableEntry&>(*this)
              .GetAllNonTerminalNodeTransformTarget());
    }
    // ��ǰ������ID�µĲ�����Ŀ��ڵ�
    // ����Ϊ����ʱvariant�������ת�Ƶ���ID��ProductionBodyId��
    // ����Ϊ��Լʱvariant��ʹ�õĲ���ʽID�Ͳ���ʽ��ID��ProductionNodeId��
    // ����Ϊ���ܺͱ���ʱvariantδ����
    ActionAndTargetContainer action_and_attached_data_;
    // ������ս�ڵ��ת�Ƶ��Ĳ���ʽ�����
    std::unordered_map<ProductionNodeId, ParsingTableEntryId>
        nonterminal_node_transform_table_;
  };
};
template <class IdType>
inline SyntaxGenerator::ProductionBodyId
SyntaxGenerator::NonTerminalProductionNode::AddBody(IdType&& body) {
  ProductionBodyId body_id(nonterminal_bodys_.size());
  // ��������뵽����ʽ�������У���ɾ����ͬ����ʽ����
  nonterminal_bodys_.emplace_back(std::forward<IdType>(body));
  // Ϊ����ʽ������в�ͬλ�õĵ��Ӧ����ǰ�����������ռ�
  ResizeProductionBodyNum(nonterminal_bodys_.size());
  // Ϊ��ͬ���λ�������ռ�
  ResizeProductionBodyNodeNum(body_id, body.size());
  return body_id;
}

template <class T>
inline bool SyntaxGenerator::PrintProcessFunction(FILE* function_file,
                                                  const T& data) {
  assert(function_file != nullptr);
  if (data.reduct_function.empty()) {
    fprintf(stderr, "Warning: ����ʽδ�ṩ��Լ����\n");
    return false;
  }
  fprintf(function_file,
          " virtual ProcessFunctionInterface::UserData "
          "Reduct(std::vector<WordDataToUser>&& user_data_) { return "
          "%s(user_data_); }\n",
          data.reduct_function.c_str());
  return true;
}

template <class ProcessFunctionClass>
inline SyntaxGenerator::ProductionNodeId SyntaxGenerator::AddNonTerminalNode(
    std::string&& node_symbol, std::vector<std::string>&& subnode_symbols) {
  ProcessFunctionClassId class_id =
      CreateProcessFunctionClassObject<ProcessFunctionClass>();
  return AddNonTerminalNode(std::move(node_symbol), std::move(subnode_symbols),
                            class_id);
}

template <class ParsingTableEntryIdContainer>
inline void SyntaxGenerator::ParsingTableTerminalNodeClassify(
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
    std::map<ParsingTableEntry::ActionAndAttachedData,
             std::vector<ParsingTableEntryId>>
        classify_table;
    ProductionNodeId transform_id = terminal_node_ids[index];
    for (auto production_node_id : entry_ids) {
      // ����map���з���
      classify_table[*GetParsingTableEntry(production_node_id)
                          .AtTerminalNode(transform_id)]
          .push_back(production_node_id);
    }
    size_t next_index = index + 1;
    for (auto& vec : classify_table) {
      if (vec.second.size() > 1) {
        // ���ຬ�ж����Ŀ����ʣ��δ�Ƚϵ�ת����������Ҫ��������
        ParsingTableTerminalNodeClassify(terminal_node_ids, next_index,
                                         std::move(vec.second), equivalent_ids);
      }
    }
  }
}

template <class ParsingTableEntryIdContainer>
inline void SyntaxGenerator::ParsingTableNonTerminalNodeClassify(
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
    std::map<ParsingTableEntryId, std::vector<ParsingTableEntryId>>
        classify_table;
    ProductionNodeId transform_id = nonterminal_node_ids[index];
    for (auto production_node_id : entry_ids) {
      // ����map���з���
      classify_table[GetParsingTableEntry(production_node_id)
                         .AtNonTerminalNode(transform_id)]
          .push_back(production_node_id);
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

template <class Archive>
inline void SyntaxGenerator::save(Archive& ar,
                                  const unsigned int version) const {
  ar << root_parsing_table_entry_id_;
  ar << syntax_config_parsing_table_.size();
  for (auto& item : syntax_config_parsing_table_) {
    ar << item;
  }
  ar << manager_process_function_class_;
}

template <class AttachedData>
void SyntaxGenerator::ParsingTableEntry::SetTerminalNodeActionAndAttachedData(
    ProductionNodeId node_id, ActionType action_type,
    AttachedData&& attached_data) {
#ifdef USE_AMBIGUOUS_GRAMMAR
  // ʹ�ö������ķ����﷨������ĳЩ�������Ҫ��ͬһ���ڵ�֧������͹�Լ��������
  auto iter = action_and_attached_data_.find(node_id);
  if (iter == action_and_attached_data_.end()) {
    // �²���ת�ƽڵ�
    action_and_attached_data_.emplace(std::make_pair(
        node_id, ActionAndAttachedData(
                     action_type, std::forward<AttachedData>(attached_data))));
    return;
  }
  // �������ת�������Ѵ���
  ActionAndAttachedData& data = iter->second;
  // �﷨�������������в�Ӧ��д�Ѵ��ڵ�ת������
  // Ҫô������ת��������Ҫô��ȫ����/��Լ����һ���֣���Լ/���룩
  assert(action_type == ActionType::kShift &&
             data.action_type_ == ActionType::kReduct ||
         action_type == ActionType::kReduct &&
             data.action_type_ == ActionType::kShift);
  data.ConvertToShiftReductAttachedData(
      std::forward<AttachedData>(attached_data));
#else
  ActionAndAttachedData& data = action_and_attached_data_[node_id];
  data.action_type_ = action_type;
  data.SetAttachedData(std::forward<AttachedData>(attached_data));
#endif  // USE_AMBIGUOUS_GRAMMAR
}

template <class ForwardNodeIdContainer>
inline std::pair<
    std::map<SyntaxGenerator::CoreItem,
             std::unordered_set<SyntaxGenerator::ProductionNodeId>>::iterator,
    bool>
SyntaxGenerator::Core::AddItemAndForwardNodeIds(
    const CoreItem& item, ForwardNodeIdContainer&& forward_node_ids) {
  SetClosureNotAvailable();
  auto iter = item_and_forward_node_ids_.find(item);
  if (iter == item_and_forward_node_ids_.end()) {
    return item_and_forward_node_ids_.insert(std::make_pair(
        item, std::unordered_set<ProductionNodeId>(
                  std::forward<ForwardNodeIdContainer>(forward_node_ids))));
  } else {
    AddForwardNode(iter,
                   std::forward<ForwardNodeIdContainer>(forward_node_ids));
    return std::make_pair(iter, false);
  }
}

inline const SyntaxGenerator::ParsingTableEntry::ShiftAttachedData&
SyntaxGenerator::ParsingTableEntry::ActionAndAttachedData::
    GetShiftAttachedData() const {
  switch (action_type_) {
    break;
    case ActionType::kShift:
      return boost::get<ShiftAttachedData>(attached_data_);
      break;

#ifdef USE_AMBIGUOUS_GRAMMAR
    case ActionType::kShiftReduct:
      return boost::get<ShiftReductAttachedData>(attached_data_)
          .shift_attached_data_;
      break;
#endif  // USE_AMBIGUOUS_GRAMMAR

    case ActionType::kError:
    case ActionType::kAccept:
    case ActionType::kReduct:
    default:
      assert(false);
      // ��ֹ���棺�������еĿؼ�·��������ֵ
      return boost::get<ShiftAttachedData>(attached_data_);
      break;
  }
}

inline const SyntaxGenerator::ParsingTableEntry::ReductAttachedData&
SyntaxGenerator::ParsingTableEntry::ActionAndAttachedData::
    GetReductAttachedData() const {
  switch (action_type_) {
    case ActionType::kReduct:
      return boost::get<ReductAttachedData>(attached_data_);
      break;

#ifdef USE_AMBIGUOUS_GRAMMAR
    case ActionType::kShiftReduct:
      return boost::get<ShiftReductAttachedData>(attached_data_)
          .reduct_attached_data_;
      break;
#endif  // USE_AMBIGUOUS_GRAMMAR

    case ActionType::kShift:
    case ActionType::kError:
    case ActionType::kAccept:
    default:
      assert(false);
      // ��ֹ���棺�������еĿؼ�·��������ֵ
      return boost::get<ReductAttachedData>(attached_data_);
      break;
  }
}
}  // namespace frontend::generator::syntaxgenerator
#endif  // !GENERATOR_SYNTAXGENERATOR_SYNTAXGENERATOR_H_