#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAX_GENERATOR_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAX_GENERATOR_H_

#include <any>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <cassert>
#include <format>
#include <fstream>
#include <regex>
#include <tuple>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/object_manager.h"
#include "Common/unordered_struct_manager.h"
#include "Generator/DfaGenerator/dfa_generator.h"
#include "Generator/export_types.h"
#include "process_function_interface.h"
#include "production_item_set.h"
#include "production_node.h"
#include "syntax_analysis_table.h"

// TODO ���ɾ��δʹ�ò���ʽ�Ĺ���
namespace frontend::generator::syntax_generator {

class SyntaxGenerator {
 private:
  // �ʷ������дʵ����ȼ�
  using WordPriority =
      frontend::generator::dfa_generator::DfaGenerator::WordPriority;
  // ��ʾ������ݽṹ
  using ProductionItem = ProductionItemSet::ProductionItem;
  // hash��ʾ������ݽṹ�Ľṹ
  using ProductionItemHasher = ProductionItemSet::ProductionItemHasher;
  // ������ǰ���ڵ������
  using ProductionItemAndForwardNodesContainer =
      ProductionItemSet::ProductionItemAndForwardNodesContainer;
  // �洢��ǰ���ڵ������
  using ForwardNodesContainer = ProductionItemSet::ForwardNodesContainer;

 public:
  SyntaxGenerator() = default;
  SyntaxGenerator(const SyntaxGenerator&) = delete;

  SyntaxGenerator& operator=(const SyntaxGenerator&) = delete;

  // ���������������ǰ������
  void ConstructSyntaxConfig();

 private:
  // ��ʼ��
  void SyntaxGeneratorInit();
  // ����LALR��1��������ĸ�����Χ��Ϣ
  void ConfigConstruct();
  // ����LALR(1)����
  void SyntaxAnalysisTableConstruct();

  // ��ȡһ���ڵ���
  int GetNodeNum() { return node_num_++; }
  // ��λ�ڵ���
  void NodeNumInit() { node_num_ = 0; }
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
  SymbolId GetNodeSymbolId(const std::string& node_symbol) const {
    assert(node_symbol.size() != 0);
    return manager_node_symbol_.GetObjectId(node_symbol);
  }
  // ��ȡ����ʽ����Ŷ�Ӧ��ID���������򷵻�SymbolId::InvalidId()
  SymbolId GetBodySymbolId(const std::string& body_symbol) const {
    assert(body_symbol.size() != 0);
    return manager_terminal_body_symbol_.GetObjectId(body_symbol);
  }
  // ͨ������ʽ��ID��ѯ��Ӧ����ʽ��
  const std::string& GetNodeSymbolStringFromId(SymbolId node_symbol_id) const {
    assert(node_symbol_id.IsValid());
    return manager_node_symbol_.GetObject(node_symbol_id);
  }
  // ͨ������ʽ�����ID��ѯ����ʽ��ԭʼ����
  const std::string& GetBodySymbolStringFromId(SymbolId body_symbol_id) const {
    assert(body_symbol_id.IsValid());
    return manager_terminal_body_symbol_.GetObject(body_symbol_id);
  }
  // ͨ������ʽID��ѯ��Ӧ����ʽ��
  const std::string& GetNodeSymbolStringFromProductionNodeId(
      ProductionNodeId production_node_id) const {
    return GetNodeSymbolStringFromId(
        GetProductionNode(production_node_id).GetNodeSymbolId());
  }
  // ͨ�����ѯ��һ������Ľڵ���
  const std::string& GetNextNodeToShiftSymbolString(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      NextWordToShiftIndex next_word_to_shift_index) const {
    return GetNodeSymbolStringFromProductionNodeId(GetProductionNodeIdInBody(
        production_node_id, production_body_id, next_word_to_shift_index));
  }
  // ͨ�����ѯ��һ������Ľڵ���
  const std::string& GetNextNodeToShiftSymbolString(
      const ProductionItem& production_item) const {
    auto& [production_node_id, production_body_id, next_word_to_shift_index] =
        production_item;
    return GetNextNodeToShiftSymbolString(
        production_node_id, production_body_id, next_word_to_shift_index);
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
  // ��������ڸò���ʽ�ڵ��򷵻�ProductionNodeId::InvalidId()
  ProductionNodeId GetProductionNodeIdFromNodeSymbolId(
      SymbolId node_symbol_id) {
    auto iter = node_symbol_id_to_node_id_.find(node_symbol_id);
    if (iter == node_symbol_id_to_node_id_.end()) {
      return ProductionNodeId::InvalidId();
    } else {
      return iter->second;
    }
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
      NextWordToShiftIndex next_word_to_shift_index) const {
    return GetProductionNode(production_node_id)
        .GetProductionNodeInBody(production_body_id, next_word_to_shift_index);
  }
  // �����û�����ĸ�����ʽID
  void SetRootProductionNodeId(ProductionNodeId root_production_node_id) {
    root_production_node_id_ = root_production_node_id;
  }
  // ��ȡ�û�����ĸ�����ʽID
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
  // ����������������Ϣ����������
  void CheckUndefinedProductionRemained();
  // ��ӹؼ��֣��Զ�����ͬ���ս�ڵ�
  void AddKeyWord(std::string&& key_word);
  // �½��ս�ڵ㣬���ؽڵ�ID
  // �ú������������ս����ʽ��������
  // �ڵ��Ѵ����Ҹ���symbol_id��ͬ������ID�򷵻�ProductionNodeId::InvalidId()
  // �����������Ǵ����ȼ���0����Ϊ��ͨ�ʵ����ȼ���1����Ϊ��������ȼ�
  // 2����Ϊ�ؼ������ȼ�����������ȼ���δָ��
  // �����������ȼ�����������ȼ���ͬ����ע�����֣�����
  ProductionNodeId AddTerminalNode(std::string&& node_symbol,
                                   std::string&& body_symbol,
                                   WordPriority node_priority = WordPriority(0),
                                   bool is_key_word = false);
  // �ӹ��̣������ڴ����ڵ�
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddTerminalNode(SymbolId node_symbol_id,
                                      SymbolId body_symbol_id);
  // �½�˫Ŀ������ڵ㣬���ؽڵ�ID
  // �ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  // Ĭ����ӵ�������ʷ��������ȼ�������ͨ�ս����ʽ���ڹؼ���
  ProductionNodeId AddBinaryOperatorNode(
      std::string&& operator_symbol,
      OperatorAssociatityType binary_operator_associatity_type,
      OperatorPriority binary_operator_priority_level);
  // �½���൥Ŀ������ڵ㣬���ؽڵ�ID
  // �ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  // Ĭ����ӵ�������ʷ��������ȼ�������ͨ�ս����ʽ���ڹؼ���
  // �ṩ��൥Ŀ������汾
  ProductionNodeId AddLeftUnaryOperatorNode(
      std::string&& operator_symbol,
      OperatorAssociatityType unary_operator_associatity_type,
      OperatorPriority unary_operator_priority_level);
  // �½����õ���൥Ŀ��˫Ŀ������ڵ㣬���ؽڵ�ID
  // �ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  // Ĭ����ӵ�������ʷ��������ȼ�������ͨ�ս����ʽ���ڹؼ���
  // �ṩ��൥Ŀ������汾
  ProductionNodeId AddBinaryUnaryOperatorNode(
      std::string&& operator_symbol,
      OperatorAssociatityType binary_operator_associatity_type,
      OperatorPriority binary_operator_priority_level,
      OperatorAssociatityType unary_operator_associatity_type,
      OperatorPriority unary_operator_priority_level);
  // �ӹ��̣������ڴ����ڵ�
  // ������ڵ���ͬ�������
  // �Զ����½ڵ���ID���ڵ�ID��ӳ���
  // �Զ����½ڵ���ID���ڵ�ID��ӳ��
  // �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddOperatorNode(SymbolId node_symbol_id,
                                      OperatorAssociatityType associatity_type,
                                      OperatorPriority priority_level);
  // �½����ս�ڵ㣬���ؽڵ�ID���ڵ��Ѵ����򲻻ᴴ���µĽڵ�
  // node_symbolΪ����ʽ����subnode_symbols�ǲ���ʽ��
  // could_empty_reduct�����Ƿ���Կչ�Լ
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
  // ���÷��ս�ڵ���Կչ�Լ
  // ���뱣֤�������ƽڵ������Ϊ���ս����ʽ��
  void SetNonTerminalNodeCouldEmptyReduct(
      const std::string& nonterminal_node_symbol);
  // �½��ļ�β�ڵ㣬���ؽڵ�ID
  ProductionNodeId AddEndNode();
  // ���ò���ʽ���ڵ�
  // �������ʽ��
  void SetRootProduction(const std::string& production_node_name);

  // ��ȡ�ڵ�
  BaseProductionNode& GetProductionNode(ProductionNodeId production_node_id);
  const BaseProductionNode& GetProductionNode(
      ProductionNodeId production_node_id) const;
  BaseProductionNode& GetProductionNodeFromNodeSymbolId(SymbolId symbol_id);
  BaseProductionNode& GetProductionNodeBodyFromSymbolId(SymbolId symbol_id);
  // ��ȡ���ս�ڵ��е�һ������ʽ��
  const std::vector<ProductionNodeId>& GetProductionBody(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id);
  // �����ս�ڵ���Ӳ���ʽ��
  template <class IdType>
  void AddNonTerminalNodeBody(ProductionNodeId node_id, IdType&& body) {
    static_cast<NonTerminalProductionNode&>(GetProductionNode(node_id)).AddBody,
        (std::forward<IdType>(body));
  }
  // ���һ���﷨��������Ŀ
  SyntaxAnalysisTableEntryId AddSyntaxAnalysisTableEntry() {
    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id(
        syntax_analysis_table_.size());
    syntax_analysis_table_.emplace_back();
    return syntax_analysis_table_entry_id;
  }
  // ��ȡproduction_item_set_id��Ӧ����ʽ�
  const ProductionItemSet& GetProductionItemSet(
      ProductionItemSetId production_item_set_id) const {
    assert(production_item_set_id < production_item_sets_.Size());
    return production_item_sets_[production_item_set_id];
  }
  // ��ȡproduction_item_set_id��Ӧ����ʽ�
  ProductionItemSet& GetProductionItemSet(
      ProductionItemSetId production_item_set_id) {
    return const_cast<ProductionItemSet&>(
        static_cast<const SyntaxGenerator&>(*this).GetProductionItemSet(
            production_item_set_id));
  }
  // �����﷨��������ĿID���ID��ӳ��
  void SetSyntaxAnalysisTableEntryIdToProductionItemSetIdMapping(
      SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id,
      ProductionItemSetId production_item_set_id) {
    syntax_analysis_table_entry_id_to_production_item_set_id_
        [syntax_analysis_table_entry_id] = production_item_set_id;
  }
  // ��ȡ�﷨��������ĿID��Ӧ���ID
  ProductionItemSetId GetProductionItemSetIdFromSyntaxAnalysisTableEntryId(
      SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id) {
    auto iter = syntax_analysis_table_entry_id_to_production_item_set_id_.find(
        syntax_analysis_table_entry_id);
    assert(iter !=
           syntax_analysis_table_entry_id_to_production_item_set_id_.end());
    return iter->second;
  }
  // ����µ������Ҫ�����﷨��������Ŀ���ID��ӳ��
  ProductionItemSetId EmplaceProductionItemSet() {
    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id =
        AddSyntaxAnalysisTableEntry();
    ProductionItemSetId production_item_set_id =
        production_item_sets_.EmplaceObject(syntax_analysis_table_entry_id);
    production_item_sets_[production_item_set_id].SetProductionItemSetId(
        production_item_set_id);
    SetSyntaxAnalysisTableEntryIdToProductionItemSetIdMapping(
        syntax_analysis_table_entry_id, production_item_set_id);
    return production_item_set_id;
  }
  // �������������Ӧ����ǰ�����ţ����Դ��뵥��δ��װID
  // ���ز���λ�ú��Ƿ����
  // ���������������ǰ�����������ñհ���Ч
  // ��������Ѵ�����������ǰ������
  // ��������������������Զ���Ӹ�������ӵ������ID�ļ�¼
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddItemAndForwardNodeIdsToCore(ProductionItemSetId production_item_set_id,
                                 const ProductionItem& production_item,
                                 ForwardNodeIdContainer&& forward_node_ids);
  // ��AddItemAndForwardNodeIdsToCore������������ӵ���Ϊ������
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddMainItemAndForwardNodeIdsToCore(ProductionItemSetId production_item_set_id,
                                     const ProductionItem& production_item,
                                     ForwardNodeIdContainer&& forward_node_ids);
  // ��������������ǰ�����ţ�ͬʱ֧�����뵥�����źͷ�������
  // �����Ƿ����
  // Ҫ�����Ѿ����ڣ����������AddItemAndForwardNodeIds�����ƺ���
  // ���������µ���ǰ�����������ñհ���Ч
  template <class ForwardNodeIdContainer>
  bool AddForwardNodes(ProductionItemSetId production_item_set_id,
                       const ProductionItem& production_item,
                       ForwardNodeIdContainer&& forward_node_ids) {
    return GetProductionItemSet(production_item_set_id)
        .AddForwardNodes(production_item, std::forward<ForwardNodeIdContainer>(
                                              forward_node_ids));
  }
  // ��¼�������������ID
  void AddProductionItemBelongToProductionItemSetId(
      const ProductionItem& production_item,
      ProductionItemSetId production_item_set_id);
  // ��ȡ���ȫ��������
  const std::list<ProductionItemAndForwardNodesContainer::const_iterator>
  GetProductionItemSetMainItems(
      ProductionItemSetId production_item_set_id) const {
    return GetProductionItemSet(production_item_set_id).GetMainItems();
  }
  // ��ȡ����������������ȫ���
  const std::list<ProductionItemSetId>&
  GetProductionItemSetIdFromProductionItem(
      ProductionNodeId production_node_id, ProductionBodyId body_id,
      NextWordToShiftIndex next_word_to_shift_index);
  // ��ȡ��ǰ�����ż�
  const ForwardNodesContainer& GetForwardNodeIds(
      ProductionItemSetId production_item_set_id,
      const ProductionItem& production_item) const {
    return GetProductionItemSet(production_item_set_id)
        .GetItemsAndForwardNodeIds()
        .at(production_item);
  }

  // First���ӹ��̣���ȡһ�����ս�ڵ������е�first����
  // �ڶ�������ָ��洢����ļ���
  // ���������������洢�Ѿ�������Ľڵ㣬��ֹ���޵ݹ飬���ε���Ӧ����ռ���
  // �������ProductionNodeId::InvalidId()�򷵻ؿռ���
  void GetNonTerminalNodeFirstNodeIds(
      ProductionNodeId production_node_id, ForwardNodesContainer* result,
      std::unordered_set<ProductionNodeId>&& processed_nodes =
          std::unordered_set<ProductionNodeId>());
  // �հ������е�first������ǰ����������־�µ�λ��
  // ��������������Ϊ���ս�ڵ���ܹ�ԼΪ�սڵ㣬��Ҫ���²����ս�ڵ�
  // ��ǰ���ڵ���Թ�ԼΪ�սڵ��������Ӧ��ǰ������
  // Ȼ�������ǰ����ֱ����β�򲻿ɿչ�Լ���ս�ڵ���ս�ڵ�
  ForwardNodesContainer First(ProductionNodeId production_node_id,
                              ProductionBodyId production_body_id,
                              NextWordToShiftIndex next_word_to_shift_index,
                              const ForwardNodesContainer& next_node_ids);
  // ��ȡ���ȫ����
  const ProductionItemAndForwardNodesContainer&
  GetProductionItemsAndForwardNodes(
      ProductionItemSetId production_item_set_id) {
    return GetProductionItemSet(production_item_set_id)
        .GetItemsAndForwardNodeIds();
  }
  // ��ȡ�﷨��������Ŀ
  SyntaxAnalysisTableEntry& GetSyntaxAnalysisTableEntry(
      SyntaxAnalysisTableEntryId production_node_id) {
    assert(production_node_id < syntax_analysis_table_.size());
    return syntax_analysis_table_[production_node_id];
  }
  // ���ø��﷨��������ĿID
  void SetRootSyntaxAnalysisTableEntryId(
      SyntaxAnalysisTableEntryId root_syntax_analysis_table_entry_id) {
    root_syntax_analysis_table_entry_id_ = root_syntax_analysis_table_entry_id;
  }
  // ������հ���Ч������ÿ��Goto����հ���
  // ��Ӧ��CoreClosure����
  void SetCoreClosureAvailable(ProductionItemSetId production_item_set_id) {
    GetProductionItemSet(production_item_set_id).SetClosureAvailable();
  }
  bool IsClosureAvailable(ProductionItemSetId production_item_set_id) {
    return GetProductionItemSet(production_item_set_id).IsClosureAvailable();
  }
  // �Ը�����������հ����������ԭλ��
  // �Զ�������е�ǰλ�ÿ��Կչ�Լ����ĺ�����
  // �����Ƿ�����հ�
  // ����հ�������﷨��������Ŀ
  bool ProductionItemSetClosure(ProductionItemSetId production_item_set_id);
  // ��ȡ��������ɵ��������������򷵻�ProductionItemSetId::InvalidId()
  // ����ָ��ת��ǰ��ĵ�����
  ProductionItemSetId GetProductionItemSetIdFromProductionItems(
      const std::list<std::unordered_map<
          ProductionItem, std::unordered_set<ProductionNodeId>,
          ProductionItemSet::ProductionItemHasher>::const_iterator>& items);
  // ������ǰ�����ţ�ͬʱ�ڴ��������й����﷨������
  // �����Ƿ�ִ���˴�������
  // ���δ����հ��򲻻�ִ�д����Ĳ���ֱ�ӷ���
  bool SpreadLookForwardSymbolAndConstructSyntaxAnalysisTableEntry(
      ProductionItemSetId production_item_set_id);
  // �����в���ʽ�ڵ㰴��ProductionNodeType����
  // ����array�ڵ�vector�����Ͷ�Ӧ���±�ΪProductionNodeType�����͵�ֵ
  std::array<std::vector<ProductionNodeId>, 4> ClassifyProductionNodes() const;
  // SyntaxAnalysisTableMergeOptimize���ӹ��̣����������ͬ�ս�ڵ�����﷨��������Ŀ
  // ��equivalent_idsд����ͬ�ս�ڵ�ת�Ʊ�Ľڵ�ID���飬����ִ��ʵ�ʺϲ�����
  // ����д��ֻ��һ�������
  void SyntaxAnalysisTableTerminalNodeClassify(
      const std::vector<ProductionNodeId>& terminal_node_ids, size_t index,
      std::list<SyntaxAnalysisTableEntryId>&& syntax_analysis_table_entry_ids,
      std::vector<std::list<SyntaxAnalysisTableEntryId>>* equivalent_ids);
  // SyntaxAnalysisTableMergeOptimize���ӹ��̣����������ͬ���ս�ڵ�����﷨��������Ŀ
  // ��equivalent_idsд����ͬ���ս�ڵ�ת�Ʊ�Ľڵ�ID���飬����ִ��ʵ�ʺϲ�����
  // ����д��ֻ��һ�������
  void SyntaxAnalysisTableNonTerminalNodeClassify(
      const std::vector<ProductionNodeId>& nonterminal_node_ids, size_t index,
      std::list<SyntaxAnalysisTableEntryId>&& entry_ids,
      std::vector<std::list<SyntaxAnalysisTableEntryId>>* equivalent_ids);
  // SyntaxAnalysisTableMergeOptimize���ӹ��̣����������ͬ����﷨��������Ŀ
  // ��һ������Ϊ�﷨������������������ڵ�ID
  // �ڶ�������Ϊ�﷨�������������ս�ڵ�ID
  // ����������Ϊ�﷨�����������з��ս�ڵ�ID
  // ���ؿ��Ժϲ����﷨��������Ŀ�飬�������������������Ŀ
  std::vector<std::list<SyntaxAnalysisTableEntryId>>
  SyntaxAnalysisTableEntryClassify(
      std::vector<ProductionNodeId>&& operator_node_ids,
      std::vector<ProductionNodeId>&& terminal_node_ids,
      std::vector<ProductionNodeId>&& nonterminal_node_ids);
  // ���ݸ����ı�����ӳ���﷨��������ID
  void RemapSyntaxAnalysisTableEntryId(
      const std::unordered_map<SyntaxAnalysisTableEntryId,
                               SyntaxAnalysisTableEntryId>&
          moved_entry_id_to_new_entry_id);
  // �ϲ��﷨����������ͬ���ͬʱ�����﷨�������С
  // �Ὣ�﷨�������ڵ����޸�Ϊ�µ���
  void SyntaxAnalysisTableMergeOptimize();

  // boost-serialization���������﷨���������õĺ���
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const;
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  // ���﷨����������д���ļ�
  void SaveConfig() const {
    dfa_generator_.SaveConfig();
    std::ofstream config_file(frontend::common::kSyntaxConfigFileName,
                              std::ios_base::binary | std::ios_base::out);
    // oarchiveҪ��config_file����ǰ�����������ļ��������ڷ����л�ʱ�����쳣
    {
      boost::archive::binary_oarchive oarchive(config_file);
      oarchive << *this;
    }
  }

  // ����������ʽת��Ϊ�ַ���
  // ����ֵ���� IdOrEquivence -> IdOrEquivence [ Num ]
  std::string FormatSingleProductionBody(
      ProductionNodeId nonterminal_node_id,
      ProductionBodyId production_body_id) const;
  // ���������ս�ڵ�ȫ������ʽת��Ϊ�ַ�������ͬ����ʽ��ʹ�û��з���'\n'���ָ�
  // �����������ʽ��ʽͬFormatSingleProductionBody
  std::string FormatProductionBodys(ProductionNodeId nonterminal_node_id);
  // ���������ʽ��
  // ����ֵ����IdOrEquivence -> IdOrEquivence �� [ Num ]
  std::string FormatProductionItem(const ProductionItem& production_item) const;
  // ����������ǰ�����Ÿ�ʽ��
  // ��ǰ�����ż�ͨ���ո�ָ����������߲���˫����
  std::string FormatLookForwardSymbols(
      const ForwardNodesContainer& look_forward_node_ids) const;
  // ͬʱ��ʽ�����������ǰ������
  // ��ʽ����������" ��ǰ�����ţ�"���ȫ����ǰ������
  // �������ʽͬFormatProductionItem����ǰ�����Ÿ�ʽͬFormatLookForwardSymbols
  std::string FormatProductionItemAndLookForwardSymbols(
      const ProductionItem& production_item,
      const ForwardNodesContainer& look_forward_node_ids) const;
  // ���������ȫ�����ʽ��
  // ����ʽ��ʽͬFormatItem������ʽ���" ��ǰ�����ţ�"���������ǰ������
  // ��ǰ�����Ÿ�ʽͬFormatLookForwardSymbols
  // ������֮��ͨ��'\n'�ָ�
  std::string FormatProductionItems(
      ProductionItemSetId production_item_set_id) const;
  // ���Info�������Ϣ
  static void OutPutInfo(const std::string& info) {
    //std::cout << std::format("SyntaxGenerator Info: ") << info << std::endl;
  }
  // ���Warning�������Ϣ
  static void OutPutWarning(const std::string& warning) {
    std::cerr << std::format("SyntaxGenerator Warning: ") << warning
              << std::endl;
  }
  // ���Error�������Ϣ
  static void OutPutError(const std::string& error) {
    std::cerr << std::format("SyntaxGenerator Error: ") << error << std::endl;
  }

  // �������л������
  friend class boost::serialization::access;

  // �Խڵ��ţ��������ֲ�ͬ�ڵ��Ӧ����
  // ��ֵ�������������ô��������ɰ�װ�û����庯�����ݵ���
  int node_num_;
  // �洢���õ�δ�������ʽ
  // key��δ����Ĳ���ʽ��
  // tuple�ڵ�std::string�Ƿ��ս����ʽ��
  // std::tuple<std::string, std::vector<std::string>,ProcessFunctionClassId>
  //     �洢���еĲ���ʽ����Ϣ
  // ProcessFunctionClassId�Ǹ����İ�װ�û����庯�����ݵ���Ķ���ID
  // bool���Ƿ���Կչ�Լ���
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
  // �洢�
  ObjectManager<ProductionItemSet> production_item_sets_;
  // �﷨������ID���ID��ӳ��
  std::unordered_map<SyntaxAnalysisTableEntryId, ProductionItemSetId>
      syntax_analysis_table_entry_id_to_production_item_set_id_;
  // �û�����ĸ����ս����ʽ�ڵ�ID
  ProductionNodeId root_production_node_id_ = ProductionNodeId::InvalidId();
  // ��ʼ�﷨��������ĿID������д���ļ�
  SyntaxAnalysisTableEntryId root_syntax_analysis_table_entry_id_;
  // DFA����������������д���ļ�
  frontend::generator::dfa_generator::DfaGenerator dfa_generator_;
  // �﷨����������д���ļ�
  SyntaxAnalysisTableType syntax_analysis_table_;
  // �û��Զ��庯�������ݵ���Ķ�������д���ļ�
  // ÿ����Լ���ݶ��������Ψһ�İ�װReduct��������Ķ��󣬲��������ö���
  ProcessFunctionClassManagerType manager_process_function_class_;

  // ��ϣ����ProductionNodeId�õ���
  // ����SyntaxAnalysisTableTerminalNodeClassify�з���ͬʱ֧�ֹ�Լ�����������
  struct PairOfSyntaxAnalysisTableEntryIdAndProcessFunctionClassIdHasher {
    size_t operator()(
        const std::pair<SyntaxAnalysisTableEntryId, ProcessFunctionClassId>&
            data_to_hash) const {
      return data_to_hash.first * data_to_hash.second;
    }
  };
};
template <class IdType>
inline ProductionBodyId NonTerminalProductionNode::AddBody(
    IdType&& body, ProcessFunctionClassId class_for_reduct_id_) {
  ProductionBodyId body_id(nonterminal_bodys_.size());
  // ��������뵽����ʽ�������У���ɾ����ͬ����ʽ����
  nonterminal_bodys_.emplace_back(std::forward<IdType>(body),
                                  class_for_reduct_id_);
  return body_id;
}

template <class ProcessFunctionClass>
inline ProductionNodeId SyntaxGenerator::AddNonTerminalNode(
    std::string&& node_symbol, std::vector<std::string>&& subnode_symbols) {
  ProcessFunctionClassId class_id =
      CreateProcessFunctionClassObject<ProcessFunctionClass>();
  return AddNonTerminalNode(std::move(node_symbol), std::move(subnode_symbols),
                            class_id);
}

template <class ForwardNodeIdContainer>
inline std::pair<
    SyntaxGenerator::ProductionItemAndForwardNodesContainer::iterator, bool>
SyntaxGenerator::AddItemAndForwardNodeIdsToCore(
    ProductionItemSetId production_item_set_id,
    const ProductionItem& production_item,
    ForwardNodeIdContainer&& forward_node_ids) {
  assert(production_item_set_id.IsValid());
  auto result = GetProductionItemSet(production_item_set_id)
                    .AddItemAndForwardNodeIds(
                        production_item,
                        std::forward<ForwardNodeIdContainer>(forward_node_ids));
  if (result.second) {
    // ����²����������¼�������ڵ����ID
    AddProductionItemBelongToProductionItemSetId(production_item,
                                                 production_item_set_id);
  }
  return result;
}

// ��AddItemAndForwardNodeIdsToCore������������ӵ���Ϊ������

template <class ForwardNodeIdContainer>
inline std::pair<
    SyntaxGenerator::ProductionItemAndForwardNodesContainer::iterator, bool>
SyntaxGenerator::AddMainItemAndForwardNodeIdsToCore(
    ProductionItemSetId production_item_set_id,
    const ProductionItem& production_item,
    ForwardNodeIdContainer&& forward_node_ids) {
  auto result = GetProductionItemSet(production_item_set_id)
                    .AddMainItemAndForwardNodeIds(
                        production_item,
                        std::forward<ForwardNodeIdContainer>(forward_node_ids));
  if (result.second) {
    // ����в������µ�������¸������ڵ��
    AddProductionItemBelongToProductionItemSetId(production_item,
                                                 production_item_set_id);
  }
  return result;
}

template <class Archive>
inline void SyntaxGenerator::save(Archive& ar,
                                  const unsigned int version) const {
  ar << root_syntax_analysis_table_entry_id_;
  ar << syntax_analysis_table_;
  ar << manager_process_function_class_;
}
}  // namespace frontend::generator::syntax_generator

#endif  // !GENERATOR_SYNTAXGENERATOR_SYNTAXGENERATOR_H_