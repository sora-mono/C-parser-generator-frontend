/// @file syntax_generator.h
/// @brief �﷨����������������
/// @details
/// 1.�﷨����������������ʹ��LALR(1)�﷨
/// 2.֧�ַ��ս����ʽ�չ�Լ
/// 3.֧����������ȼ����������ķ���
/// 4.֧��˫Ŀ����൥Ŀ���干��
/// 5.֧�ֲ���ʽѭ������
/// 6.��ǰ�����Ų��������ʱ������������ԭ��
/// ����
/// IF -> if() {} else
///       if() {}
/// ����ѡ��if-else·��
#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAX_GENERATOR_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAX_GENERATOR_H_

#include <any>
#include <boost/archive/binary_oarchive.hpp>
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

/// TODO ���ɾ��δʹ�ò���ʽ�Ĺ���
namespace frontend::generator::syntax_generator {

/// @class SyntaxGenerator syntax_generator.h
/// @brief �﷨����������������
class SyntaxGenerator {
 private:
  /// @brief �ʷ������дʵ����ȼ�
  using WordPriority =
      frontend::generator::dfa_generator::DfaGenerator::WordPriority;
  /// @brief ��ʾ������ݽṹ
  using ProductionItem = ProductionItemSet::ProductionItem;
  /// @brief ��ϣProductionItemSet����
  using ProductionItemHasher = ProductionItemSet::ProductionItemHasher;
  /// @brief ������ǰ���ڵ������
  using ProductionItemAndForwardNodesContainer =
      ProductionItemSet::ProductionItemAndForwardNodesContainer;
  /// @brief �洢��ǰ���ڵ������
  using ForwardNodesContainer = ProductionItemSet::ForwardNodesContainer;

 public:
  SyntaxGenerator() = default;
  SyntaxGenerator(const SyntaxGenerator&) = delete;

  SyntaxGenerator& operator=(const SyntaxGenerator&) = delete;

  /// @brief ���������������ǰ������
  /// @note �Զ������﷨������DFA���ò�����
  void ConstructSyntaxConfig();

 private:
  /// @brief ��ʼ��
  void SyntaxGeneratorInit();
  /// @brief ��ӹ�����������ĸ�����Χ��Ϣ
  /// @note �ú���������config_construct.cpp��
  void ConfigConstruct();
  /// @brief �����﷨������
  void SyntaxAnalysisTableConstruct();

  /// @brief ��Ӳ���ʽ��
  /// @param[in] node_symbol ������ʽ��
  /// @return ǰ�벿��Ϊ����ʽ��ID����벿��Ϊ�Ƿ�������µĲ���ʽ��
  /// @note �������ʽ���Ѿ���ӹ��򷵻�ֵ��벿�ַ���false
  /// @attention ������������ַ���
  std::pair<SymbolId, bool> AddNodeSymbol(const std::string& node_symbol) {
    assert(node_symbol.size() != 0);
    return manager_node_symbol_.EmplaceObject(node_symbol);
  }
  /// @brief ��Ӳ���ʽ���ַ���
  /// @param[in] body_symbol ������ʽ���ַ���
  /// @return
  /// ǰ�벿��Ϊ����ʽ���ַ�����ID����벿��Ϊ�Ƿ�������µĲ���ʽ���ַ���
  /// @note �������ʽ���ַ����Ѿ���ӹ��򷵻�ֵ��벿�ַ���false
  /// @attention ������������ַ���
  std::pair<SymbolId, bool> AddBodySymbol(const std::string& body_symbol) {
    assert(body_symbol.size() != 0);
    return manager_terminal_body_symbol_.EmplaceObject(body_symbol);
  }
  /// @brief ���ݲ���ʽ����ȡ��ӦID
  /// @param[in] node_symbol ������ʽ��
  /// @return ���ز���ʽ����ӦID
  /// @retval SymbolId::InvalidId() �������Ĳ���ʽ��δ��ӹ�
  SymbolId GetNodeSymbolId(const std::string& node_symbol) const {
    assert(node_symbol.size() != 0);
    return manager_node_symbol_.GetObjectIdFromObject(node_symbol);
  }
  /// @brief ���ݲ���ʽ���ַ�����ȡ��ӦID
  /// @param[in] body_symbol ������ʽ���ַ���
  /// @return ���ز���ʽ���ַ�����Ӧ��ID
  /// @retval SymbolId::InvalidId() �������Ĳ���ʽ���ַ���δ��ӹ�
  SymbolId GetBodySymbolId(const std::string& body_symbol) const {
    assert(body_symbol.size() != 0);
    return manager_terminal_body_symbol_.GetObjectIdFromObject(body_symbol);
  }
  /// @brief ���ݲ���ʽ��ID��ȡ��Ӧ����ʽ��
  /// @param[in] node_symbol_id ������ʽ��ID
  /// @return ���ز���ʽ��ID��Ӧ�Ĳ���ʽ����const����
  /// @note Ҫ�������node_symbol_id��Ч
  const std::string& GetNodeSymbolStringFromId(SymbolId node_symbol_id) const {
    assert(node_symbol_id.IsValid());
    return manager_node_symbol_.GetObject(node_symbol_id);
  }
  /// @brief ���ݲ���ʽ���ַ���ID��ȡ����ʽ���ַ���
  /// @param[in] body_symbol_id ������ʽ���ַ���ID
  /// @return ���ز���ʽ���ַ�����const����
  /// @note Ҫ�������body_symbol_id��Ч
  const std::string& GetBodySymbolStringFromId(SymbolId body_symbol_id) const {
    assert(body_symbol_id.IsValid());
    return manager_terminal_body_symbol_.GetObject(body_symbol_id);
  }
  /// @brief ͨ������ʽ�ڵ�ID��ȡ����ʽ��
  /// @param[in] production_node_id ������ʽ�ڵ�ID
  /// @return ���ز���ʽ����const����
  /// @note �����Ĳ���ʽ�ڵ�ID��Ӧ�Ĳ���ʽ�ڵ������������Ч�Ĳ���ʽ��ID
  const std::string& GetNodeSymbolStringFromProductionNodeId(
      ProductionNodeId production_node_id) const {
    return GetNodeSymbolStringFromId(
        GetProductionNode(production_node_id).GetNodeSymbolId());
  }
  /// @brief ��ѯ������һ������Ĳ���ʽ��
  /// @param[in] production_node_id ������ʽ�ڵ�ID
  /// @param[in] production_body_id ������ʽ��ID
  /// @param[in] next_word_to_shift_index ����һ������Ľڵ��ڲ���ʽ����±�
  /// @return ���ظ�������һ������Ĳ���ʽ����const����
  /// @note Ҫ��������������Ҹ�������ڿ�������Ĳ���ʽ
  const std::string& GetNextNodeToShiftSymbolString(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      NextWordToShiftIndex next_word_to_shift_index) const {
    return GetNodeSymbolStringFromProductionNodeId(GetProductionNodeIdInBody(
        production_node_id, production_body_id, next_word_to_shift_index));
  }
  /// @brief ͨ�����ѯ��һ������Ľڵ���
  /// @param[in] production_item ����
  /// @return ���ظ�������һ������Ĳ���ʽ����const����
  /// @note Ҫ���������ڿ�������Ĳ���ʽ
  const std::string& GetNextNodeToShiftSymbolString(
      const ProductionItem& production_item) const {
    auto& [production_node_id, production_body_id, next_word_to_shift_index] =
        production_item;
    return GetNextNodeToShiftSymbolString(
        production_node_id, production_body_id, next_word_to_shift_index);
  }
  /// @brief ���ò���ʽ��ID������ʽ�ڵ�ID��ӳ��
  /// @param[in] node_symbol_id ������ʽ��ID
  /// @param[in] node_id ������ʽ�ڵ�ID
  /// @note ����Ѵ��ھ�ӳ����Ḳ��
  void SetNodeSymbolIdToProductionNodeIdMapping(SymbolId node_symbol_id,
                                                ProductionNodeId node_id) {
    assert(node_symbol_id.IsValid() && node_id.IsValid());
    node_symbol_id_to_node_id_[node_symbol_id] = node_id;
  }
  /// @brief ���ò���ʽ���ַ���ID������ʽ�ڵ�ID��ӳ��
  /// @param[in] body_symbol_id ������ʽ���ַ���ID
  /// @param[in] node_id ������ʽ�ڵ�ID
  /// @note ����Ѵ��ھ�ӳ����Ḳ��
  void SetBodySymbolIdToProductionNodeIdMapping(SymbolId body_symbol_id,
                                                ProductionNodeId node_id) {
    assert(body_symbol_id.IsValid() && node_id.IsValid());
    production_body_symbol_id_to_node_id_[body_symbol_id] = node_id;
  }
  /// @brief ���ݲ���ʽ��ID��ȡ��Ӧ����ʽ�ڵ�ID
  /// @param[in] node_symbol_id ������ʽ��ID
  /// @return ���ز���ʽ�ڵ�ID
  /// @retval ProductionNodeId::InvalidId() �������Ĳ���ʽ��ID������
  ProductionNodeId GetProductionNodeIdFromNodeSymbolId(SymbolId node_symbol_id);
  /// @brief ���ݲ���ʽ���ַ���ID��ȡ����ʽ�ڵ�ID
  /// @param[in] body_symbol_id ������ʽ�ַ���ID
  /// @return ���ز���ʽ�ڵ�ID
  /// @retval ProductionNodeId::InvalidId() ����������ʽ���ַ���ID������
  ProductionNodeId GetProductionNodeIdFromBodySymbolId(SymbolId body_symbol_id);
  /// @brief ���ݲ���ʽ����ȡ����ʽ�ڵ�ID
  /// @param[in] node_symbol ������ʽ��ID
  /// @return ���ز���ʽ�ڵ�ID
  /// @retval ProductionNodeId::InvalidId() ������ʽ��δ��ӹ�
  ProductionNodeId GetProductionNodeIdFromNodeSymbol(
      const std::string& node_symbol);
  /// @brief ������ʽ�����ת��Ϊ����ʽ�ڵ�ID
  /// @param[in] body_symbol ������ʽ���ַ���ID
  /// @return ���ز���ʽ�ڵ�ID
  /// @retval ProductionNodeId::InvalidId() ������ʽ���ַ���δ��ӹ�
  ProductionNodeId GetProductionNodeIdFromBodySymbol(
      const std::string& body_symbol);
  /// @brief ��ȡ����ʽ����ָ��λ�õĲ���ʽ�ڵ�ID
  /// @param[in] production_node_id ������ʽ�ڵ�ID
  /// @param[in] production_body_id ������ʽ��ID
  /// @param[in] next_word_to_shift_index ��Ҫ��ȡ�Ĳ���ʽ�ڵ�ID��λ��
  /// @return ���ز���ʽ�ڵ�ID
  /// @retval ProductionNodeId::InvalidId()
  /// ��next_word_to_shift_index���ڵ���ָ���Ĳ���ʽ���к��еĲ���ʽ��Ŀ
  /// @note production_node_id��production_body_id������Ч
  /// �ս�ڵ��������ڵ���production_body_id����ʹ��0
  ProductionNodeId GetProductionNodeIdInBody(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id,
      NextWordToShiftIndex next_word_to_shift_index) const {
    return GetProductionNode(production_node_id)
        .GetProductionNodeInBody(production_body_id, next_word_to_shift_index);
  }
  /// @brief �����û�����ĸ�����ʽ�ڵ�ID
  /// @param[in] root_production_node_id ���û�����ĸ�����ʽ�ڵ�ID
  void SetRootProductionNodeId(ProductionNodeId root_production_node_id) {
    root_production_node_id_ = root_production_node_id;
  }
  /// @brief ��ȡ�û�����ĸ�����ʽID
  /// @return �����û�����ĸ�����ʽID
  /// @retval ProductionNodeId::InvalidId()
  /// ��δ���ù�������ʽID����������Ϊ���ֵ
  ProductionNodeId GetRootProductionNodeId() {
    return root_production_node_id_;
  }
  /// @brief ʵ������װ�������������
  /// @tparam ProcessFunctionClass ����װ������������������
  /// @return ���ذ�װ�����������ʵ���������ID
  /// @note ������Ƿ��ظ�������ʹ��ʱһ���������ʵ����һ��
  template <class ProcessFunctionClass>
  ProcessFunctionClassId CreateProcessFunctionClassObject() {
    return manager_process_function_class_
        .EmplaceObject<ProcessFunctionClass>();
  }
  /// @brief ��ȡ��Լĳ������ʽʹ�õİ�װ�����������ʵ��������ID
  /// @param[in] production_node_id ������ʽID
  /// @param[in] production_body_id ������ʽ��ID
  /// @return ���ذ�װ�����������ʵ��������ID
  /// @note ������Է��ս����ʽ�ڵ�ִ�иò�����Ҫ��production_node_id��Ч
  ProcessFunctionClassId GetProcessFunctionClass(
      ProductionNodeId production_node_id,
      ProductionBodyId production_body_id) {
    NonTerminalProductionNode& production_node =
        static_cast<NonTerminalProductionNode&>(
            GetProductionNode(production_node_id));
    assert(production_node.GetType() == ProductionNodeType::kNonTerminalNode);
    return production_node.GetBodyProcessFunctionClassId(production_body_id);
  }
  /// @brief �����Ϊ����ʽ����ĳ������ʽδ��������²��ܼ�����ӵļ�¼
  /// @param[in] undefined_symbol ��δ����Ĳ���ʽ��
  /// @param[in] node_symbol ������ӵĲ���ʽ��
  /// @param[in] subnode_symbols ������ʽ��
  /// @param[in] class_id ����װ�����������ʵ��������ID
  /// @note
  /// 1.node_symbol��subnode_symbols��class_idͬAddNonTerminalNode���ò���
  /// 2.�����Ḵ��/�ƶ�����һ�ݸ��������豣��ԭ���Ĳ�������������
  /// 3.�ú����������ʽѭ�����ù����޷�����ʹ��δ��ӵĲ���ʽ��Ϊ����ʽ������
  void AddUnableContinueNonTerminalNode(
      const std::string& undefined_symbol, std::string&& node_symbol,
      std::vector<std::string>&& subnode_symbols,
      ProcessFunctionClassId class_id);
  /// @brief
  /// �������Ӹ����Ĳ���ʽ�ڵ���Ƿ����������Ϊ�ò���ʽ��δ��������õ�
  /// ���ս����ʽ��ӹ��̣��������������ȫ����ӹ���
  /// @param[in] node_symbol ������ӵĲ���ʽ��
  /// @note
  /// 1.�ú�����AddUnableContinueNonTerminalNode����ʹ��
  /// 2.������ӹ���ͨ��ʹ�ñ���Ĳ������µ���AddNonTerminalNodeʵ��
  void CheckNonTerminalNodeCanContinue(const std::string& node_symbol);
  /// @brief ����Ƿ����δ����Ĳ���ʽ��
  /// @note
  /// 1.������в���ʽ��Ӻ���
  /// 2.����������������Ϣ���˳�
  void CheckUndefinedProductionRemained();
  /// @brief ��ӹؼ���
  /// @param[in] node_symbol ������ʽ��
  /// @param[in] key_word ���ؼ����ַ���
  /// @note
  /// 1.֧������
  /// 2.�ؼ����Ѵ��������������Ϣ
  void AddKeyWord(std::string node_symbol, std::string key_word);
  /// @brief �����ͨ�ս����ʽ
  /// @param[in] node_symbol ������ʽ��
  /// @param[in] regex_string ������ʽ�������ʾ
  void AddSimpleTerminalProduction(std::string node_symbol,
                                   std::string regex_string);
  /// @brief ����ս����ʽ���ؼ��֡����������ͨ�ս����ʽ��
  /// @param[in] node_symbol ���ս����ʽ��
  /// @param[in] body_symbol ������ʽ���ַ�����������ʽ��
  /// @param[in] node_priority ������ʽ��ĵ������ȼ�
  /// @param[in] is_key_word ���ò���ʽ�Ƿ��ǹؼ���
  /// @return �����ս�ڵ�ID
  /// @retval ProductionNodeId::Invalid() ��������ս����ʽ�������ַ����Ѵ���
  /// @note
  /// 1.�ڵ��Ѵ��ڻ����symbol_id��ͬ������ID�����������Ϣ������
  /// ProductionNodeId::InvalidId()
  /// 2.�������ȼ���0����Ϊ��ͨ�ʵ����ȼ���1����Ϊ��������ȼ���2����Ϊ�ؼ���
  /// ���ȼ�����������ȼ���δָ��
  /// @attention �����ȼ�����������ȼ���ͬ����ע������
  ProductionNodeId AddTerminalProduction(std::string&& node_symbol,
                                         std::string&& body_symbol,
                                         WordPriority node_priority,
                                         bool regex_allowed);
  /// @brief AddTerminalProduction���ӹ��̣������ڴ����ս�ڵ�
  /// @param[in] node_symbol_id ���ս�ڵ���ID
  /// @param[in] body_symbol_id ���ս�ڵ����ַ���ID
  /// @return �����½����ս����ʽ�ڵ�ID
  /// @note
  /// 1.�Զ�Ϊ�ڵ������ýڵ�ID
  /// 2.�Զ����½ڵ���ID���ڵ�ID��ӳ��
  /// 3.�Զ����½ڵ���ID���ڵ�ID��ӳ��
  ProductionNodeId SubAddTerminalNode(SymbolId node_symbol_id,
                                      SymbolId body_symbol_id);
  /// @brief ���˫Ŀ�����
  /// @param[in] operator_symbol �����������֧������
  /// @param[in] binary_operator_associatity_type ��˫Ŀ����������
  /// @param[in] binary_operator_priority ��˫Ŀ���������������ȼ�
  /// @return ����˫Ŀ������ڵ�ID
  /// @note
  /// 1.�ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  /// 2.������ʷ��������ȼ�������ͨ�ս����ʽ���ڹؼ���
  ProductionNodeId AddBinaryOperator(
      std::string node_symbol, std::string operator_symbol,
      OperatorAssociatityType binary_operator_associatity_type,
      OperatorPriority binary_operator_priority);
  /// @brief ��ӵ�Ŀ�����
  /// @param[in] operator_symbol �����������֧������
  /// @param[in] unary_operator_associatity_type ����Ŀ����������
  /// @param[in] unary_operator_priority ����Ŀ���������������ȼ�
  /// @return ���ص�Ŀ������ڵ�ID
  /// @note
  /// 1.�ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  /// 2.������ʷ��������ȼ�������ͨ�ս����ʽ���ڹؼ���
  /// @attention ��֧����൥Ŀ���������֧���Ҳ൥Ŀ�����
  ProductionNodeId AddLeftUnaryOperator(
      std::string node_symbol, std::string operator_symbol,
      OperatorAssociatityType unary_operator_associatity_type,
      OperatorPriority unary_operator_priority);
  /// @brief ��Ӿ���˫Ŀ���������൥Ŀ���������������
  /// @param[in] operator_symbol �����������֧������
  /// @param[in] binary_operator_associatity_type ��˫Ŀ����������
  /// @param[in] binary_operator_priority ��˫Ŀ���������������ȼ�
  /// @param[in] unary_operator_associatity_type ����Ŀ����������
  /// @param[in] unary_operator_priority ����Ŀ���������������ȼ�
  /// @return ����������ڵ�ID
  /// @note
  /// 1.�ڵ��Ѵ����򷵻�ProductionNodeId::InvalidId()
  /// 2.������ʷ��������ȼ�������ͨ�ս����ʽ���ڹؼ���
  /// 3.һ��������ڵ㺬����������
  ProductionNodeId AddBinaryLeftUnaryOperator(
      std::string node_symbol, std::string operator_symbol,
      OperatorAssociatityType binary_operator_associatity_type,
      OperatorPriority binary_operator_priority,
      OperatorAssociatityType unary_operator_associatity_type,
      OperatorPriority unary_operator_priority);
  /// @brief AddBinaryOperatorNode���ӹ���
  /// @param[in] node_symbol_id ������ӵ����������ID
  /// @param[in] binary_associatity_type ��˫Ŀ����������
  /// @param[in] binary_priority ��˫Ŀ��������ȼ�
  /// @details
  /// ���ӹ��̽����ڴ����ڵ�
  /// �Զ�Ϊ�ڵ������ýڵ�ID
  /// �Զ����½ڵ���ID���ڵ�ID��ӳ���
  /// �Զ����½ڵ���ID���ڵ�ID��ӳ��
  /// @note
  /// ��������ȼ��뵥�����ȼ���ͬ����ע������
  ProductionNodeId SubAddBinaryOperatorNode(
      SymbolId node_symbol_id, OperatorAssociatityType binary_associatity_type,
      OperatorPriority binary_priority);
  /// @brief AddUnaryOperatorNode���ӹ���
  /// @param[in] node_symbol_id ������ӵ����������ID
  /// @param[in] unary_associatity_type ����Ŀ����������
  /// @param[in] unary_priority ����Ŀ��������ȼ�
  /// @details
  /// ���ӹ��̽����ڴ����ڵ�
  /// �Զ�Ϊ�ڵ������ýڵ�ID
  /// �Զ����½ڵ���ID���ڵ�ID��ӳ���
  /// �Զ����½ڵ���ID���ڵ�ID��ӳ��
  /// @note
  /// ��������ȼ��뵥�����ȼ���ͬ����ע������
  ProductionNodeId SubAddUnaryOperatorNode(
      SymbolId node_symbol_id, OperatorAssociatityType unary_associatity_type,
      OperatorPriority unary_priority);
  /// @brief AddBinaryUnaryOperatorNode���ӹ���
  /// @param[in] node_symbol_id ������ӵ����������ID
  /// @param[in] binary_associatity_type ��˫Ŀ����������
  /// @param[in] binary_priority ��˫Ŀ��������ȼ�
  /// @param[in] unary_associatity_type ����Ŀ����������
  /// @param[in] unary_priority ����Ŀ��������ȼ�
  /// @details
  /// ���ӹ��̽����ڴ����ڵ�
  /// �Զ�Ϊ�ڵ������ýڵ�ID
  /// �Զ����½ڵ���ID���ڵ�ID��ӳ���
  /// �Զ����½ڵ���ID���ڵ�ID��ӳ��
  /// @note
  /// ��������ȼ��뵥�����ȼ���ͬ����ע������
  ProductionNodeId SubAddBinaryUnaryOperatorNode(
      SymbolId node_symbol_id, OperatorAssociatityType binary_associatity_type,
      OperatorPriority binary_priority,
      OperatorAssociatityType unary_associatity_type,
      OperatorPriority unary_priority);
  /// @brief ��ӷ��ս����ʽ
  /// @tparam ProcessFunctionClass ����װ��Լ��������
  /// @param[in] node_symbol �����ս����ʽ��
  /// @param[in] subnode_symbols �����ս����ʽ��
  /// @return ���ط��ս����ʽ�ڵ�ID
  /// @details
  /// ���ģ�庯���ͷ�ģ�庯���Ӷ����ʹ�������������ֹ�������ͣ����ߵȼ�
  template <class ProcessFunctionClass>
  ProductionNodeId AddNonTerminalProduction(std::string node_symbol,
                                            std::string subnode_symbols);
  /// @brief ��ӷ��ս����ʽ
  /// @param[in] node_symbol �����ս����ʽ��
  /// @param[in] subnode_symbols �����ս����ʽ��
  /// @param[in] class_id ����װ��Լ���������ʵ��������ID
  /// @return ���ط��ս����ʽ�ڵ�ID
  ProductionNodeId AddNonTerminalProduction(
      std::string&& node_symbol, std::vector<std::string>&& subnode_symbols,
      ProcessFunctionClassId class_id);
  /// @brief AddNonTerminalProduction���ӹ��̣������ڴ����ڵ�
  /// @param[in] symbol_id �����ս����ʽ��ID
  /// @return ���ش����ķ��ս����ʽ�ڵ�ID
  /// @note
  /// �Զ����½ڵ���ID���ڵ�ID��ӳ���
  /// �Զ�Ϊ�ڵ������ýڵ�ID
  ProductionNodeId SubAddNonTerminalNode(SymbolId symbol_id);
  /// @brief ���÷��ս����ʽ���Կչ�Լ
  /// @param[in] nonterminal_node_symbol �����ս����ʽ��
  /// @note ָ���ķ��ս����ʽ��������ӹ����������������Ϣ���˳�
  void SetNonTerminalNodeCouldEmptyReduct(
      const std::string& nonterminal_node_symbol);
  /// @brief �½��ļ�β�ڵ�
  /// @return ����β�ڵ�ID
  ProductionNodeId AddEndNode();
  /// @brief �����û�����Ĳ���ʽ���ڵ�
  /// @param[in] production_node_name ������ʽ��
  /// @note production_node_name��Ӧ�Ĳ���ʽ��������ӣ��������������Ϣ���˳�
  void SetRootProduction(const std::string& production_node_name);

  /// @brief ���ݲ���ʽ�ڵ�ID��ȡ����ʽ�ڵ�
  /// @param[in] production_node_id ������ʽ�ڵ�ID
  /// @return ���ز���ʽ�ڵ������
  /// @note �����Ĳ���ʽ�ڵ�ID������Ч
  BaseProductionNode& GetProductionNode(ProductionNodeId production_node_id);
  /// @brief ���ݲ���ʽ�ڵ�ID��ȡ����ʽ�ڵ�
  /// @param[in] production_node_id ������ʽ�ڵ�ID
  /// @return ���ز���ʽ�ڵ��const����
  /// @note �����Ĳ���ʽ�ڵ�ID������Ч
  const BaseProductionNode& GetProductionNode(
      ProductionNodeId production_node_id) const;
  /// @brief ���ݲ���ʽ��ID��ȡ����ʽ�ڵ�
  /// @param[in] symbol_id ������ʽ��ID
  /// @return ���ز���ʽ�ڵ������
  /// @note �����Ĳ���ʽ��ID������Ч
  BaseProductionNode& GetProductionNodeFromNodeSymbolId(SymbolId symbol_id);
  /// @brief ���ݲ���ʽ���ַ���ID��ȡ����ʽ�ڵ�
  /// @param[in] symbol_id ������ʽ���ַ���ID
  /// @return ���ز���ʽ�ڵ������
  /// @note �����Ĳ���ʽ���ַ���ID������Ч
  BaseProductionNode& GetProductionNodeFromBodySymbolId(SymbolId symbol_id);
  /// @brief ��ȡ���ս�ڵ��е�һ������ʽ��
  /// @param[in] production_node_id �����ս����ʽ�ڵ�ID
  /// @param[in] production_body_id ������ʽ��ID
  /// @return ���ز���ʽ���const����
  /// @note ����ʹ����Ч�ķ��ս����ʽ�ڵ�����׵���Ч�Ĳ���ʽ��ID
  const std::vector<ProductionNodeId>& GetProductionBody(
      ProductionNodeId production_node_id, ProductionBodyId production_body_id);
  /// @brief ����ս�ڵ�����Ӳ���ʽ��
  /// @param[in] node_id ������Ӳ���ʽ�ķ��ս�ڵ�ID
  /// @param[in] body ������ʽ��
  /// @return ���ز���ʽ��ID
  /// @note ����������ս����ʽ�ڵ��ID
  template <class IdType>
  ProductionBodyId AddNonTerminalNodeBody(ProductionNodeId node_id,
                                          IdType&& body) {
    assert(GetProductionNode(node_id).GetType() ==
           ProductionNodeType::kNonTerminalNode);
    return static_cast<NonTerminalProductionNode&>(GetProductionNode(node_id))
               .AddBody,
           (std::forward<IdType>(body));
  }
  /// @brief ���һ���﷨��������Ŀ
  /// @return ��������ӵ��﷨��������ĿID
  SyntaxAnalysisTableEntryId AddSyntaxAnalysisTableEntry() {
    SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id(
        syntax_analysis_table_.size());
    syntax_analysis_table_.emplace_back();
    return syntax_analysis_table_entry_id;
  }
  /// @brief �����ID��ȡ�
  /// @param[in] production_item_set_id ���ID
  /// @return �������const����
  /// @note �����ID������Ч
  const ProductionItemSet& GetProductionItemSet(
      ProductionItemSetId production_item_set_id) const {
    assert(production_item_set_id < production_item_sets_.Size());
    return production_item_sets_[production_item_set_id];
  }
  /// @brief �����ID��ȡ�
  /// @param[in] production_item_set_id ���ID
  /// @return �����������
  /// @note �����ID������Ч
  ProductionItemSet& GetProductionItemSet(
      ProductionItemSetId production_item_set_id) {
    return const_cast<ProductionItemSet&>(
        static_cast<const SyntaxGenerator&>(*this).GetProductionItemSet(
            production_item_set_id));
  }
  /// @brief �����﷨��������ĿID���ID��ӳ��
  /// @param[in] syntax_analysis_table_entry_id ���﷨��������ĿID
  /// @param[in] production_item_set_id ���ID
  /// @note ������ھ�ӳ���򸲸�
  void SetSyntaxAnalysisTableEntryIdToProductionItemSetIdMapping(
      SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id,
      ProductionItemSetId production_item_set_id) {
    syntax_analysis_table_entry_id_to_production_item_set_id_
        [syntax_analysis_table_entry_id] = production_item_set_id;
  }
  /// @brief ��ȡ�﷨��������ĿID��Ӧ���ID
  /// @param[in] syntax_analysis_table_entry_id ���﷨��������ĿID
  /// @return �����ID
  /// @note �﷨��������ĿID������Ч
  ProductionItemSetId GetProductionItemSetIdFromSyntaxAnalysisTableEntryId(
      SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id);
  /// @brief ����µ��
  /// @return ��������ӵ����ID
  /// @note �Զ������﷨��������Ŀ���ID��ӳ��
  ProductionItemSetId EmplaceProductionItemSet();
  /// @brief ����������������ǰ������
  /// @param[in] production_item_set_id ���ID
  /// @param[in] production_item ����
  /// @param[in] forward_node_ids �������ǰ������
  /// @return ǰ�벿��Ϊ�����λ�ã���벿��Ϊ�Ƿ��������
  /// @note
  /// 1.������������ǰ�����������ñհ���Ч
  /// 2.�Ѿ�����հ������������������ǰ�����ţ��������������
  /// 3.����������Ѵ����򷵻�ֵ��벿��һ������false
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddItemAndForwardNodeIdsToProductionItem(
      ProductionItemSetId production_item_set_id,
      const ProductionItem& production_item,
      ForwardNodeIdContainer&& forward_node_ids);
  /// @brief �������Ӻ�����ͺ��������ǰ������
  /// @param[in] production_item_set_id ���ID
  /// @param[in] production_item ��������
  /// @param[in] forward_node_ids �����������ǰ������
  /// @return ǰ�벿��Ϊ�����λ�ã���벿��Ϊ�Ƿ��������
  /// @note
  /// 1.forward_node_ids֧�ִ洢ID������Ҳ֧��δ��װ��ID
  /// 2.������������ǰ�����������ñհ���Ч
  /// 3.����������Ѵ����򷵻�ֵ��벿��һ������false
  /// 4.������������Զ����¸��������ڵ����ID�ļ�¼
  /// @attention �������������հ����ִ�иò���
  template <class ForwardNodeIdContainer>
  std::pair<ProductionItemAndForwardNodesContainer::iterator, bool>
  AddMainItemAndForwardNodeIdsToProductionItem(
      ProductionItemSetId production_item_set_id,
      const ProductionItem& production_item,
      ForwardNodeIdContainer&& forward_node_ids);
  /// @brief ������������������ǰ������
  /// @param[in] production_item_set_id �������ڵ��ID
  /// @param[in] production_item ��������
  /// @param[in] forward_node_ids �����������ǰ������
  /// @return �����Ƿ�����µ���ǰ������
  /// @retval true ��������µ���ǰ������
  /// @retval false ��δ����µ���ǰ������
  /// @note
  /// 1.forward_node_ids֧�ִ洢ID������Ҳ֧��δ��װ��ID
  /// 2.Ҫ�����Ѿ����ڣ�����Ӧ����AddItemAndForwardNodeIds��ͬ�ຯ��
  /// 3.���������µ���ǰ�����������ñհ���Ч
  /// 4.production_item�����Ǻ�����
  template <class ForwardNodeIdContainer>
  bool AddForwardNodes(ProductionItemSetId production_item_set_id,
                       const ProductionItem& production_item,
                       ForwardNodeIdContainer&& forward_node_ids) {
    return GetProductionItemSet(production_item_set_id)
        .AddForwardNodes(production_item, std::forward<ForwardNodeIdContainer>(
                                              forward_node_ids));
  }
  /// @brief ��Ӻ������������ID
  /// @param[in] production_item ����
  /// @param[in] production_item_set_id ���ID
  /// @note production_item_set_id����δ����ӵ�production_item�������
  void AddProductionItemBelongToProductionItemSetId(
      const ProductionItem& production_item,
      ProductionItemSetId production_item_set_id);
  /// @brief ��ȡ���ȫ��������
  /// @param[in] production_item_set_id ���ID
  /// @return ���ش洢���ȫ���������������const����
  const std::list<ProductionItemAndForwardNodesContainer::const_iterator>
  GetProductionItemSetMainItems(
      ProductionItemSetId production_item_set_id) const {
    return GetProductionItemSet(production_item_set_id).GetMainItemIters();
  }
  /// @brief ��ȡ����������������ȫ���
  /// @param[in] production_node_id �����ս����ʽ�ڵ�ID
  /// @param[in] body_id ������ʽ��ID
  /// @param[in] next_word_to_shift_index ����һ������Ĳ���ʽ�±�
  /// @return ���ش洢������������ȫ�����������const����
  /// @note
  /// 1.�����д����ID���ظ�
  /// 2.����������ͬ����һ����
  /// @attention ����¼��������Ϊ��������ڵ��ID
  const std::list<ProductionItemSetId>&
  GetProductionItemSetIdFromProductionItem(
      ProductionNodeId production_node_id, ProductionBodyId body_id,
      NextWordToShiftIndex next_word_to_shift_index);
  /// @brief ��ȡ�����ǰ������
  /// @param[in] production_item_set_id ���ID
  /// @param[in] production_item ����
  /// @return ���ش洢��ǰ�����ŵ�������const����
  /// @note �������������ڸ����
  const ForwardNodesContainer& GetForwardNodeIds(
      ProductionItemSetId production_item_set_id,
      const ProductionItem& production_item) const {
    return GetProductionItemSet(production_item_set_id)
        .GetItemsAndForwardNodeIds()
        .at(production_item);
  }

  /// @brief First���ӹ��̣���ȡһ�����ս�ڵ�ȫ����һ��������Ĳ���ʽ�ڵ�ID
  /// @param[in] production_node_id �����ս����ʽ�ڵ�ID
  /// @param[in,out] result ���洢��ȡ���Ľڵ�ID
  /// @param[in,out] processed_nodes ������ȡ�ķ��ս����ʽ�ڵ�
  /// @details
  /// ��ȡ���Թ�ԼΪ�������ս����ʽ�����в���ʽ�����ÿ����ϵĵ�һ������ʽID
  /// ��ȡ�������нڵ�ID�浽result��
  /// @note
  /// 1.processed_nodes�洢�Ѿ���ȡ���Ĳ���ʽ�ڵ�ID����Щ�ڵ㲻�ᱻ�ٴ���ȡ
  /// 2.�ú���ֻ�Ὣ�����ӵ�result�У��������result���е�����
  /// @attention
  /// production_node_id������Ч��result������nullptr
  void GetNonTerminalNodeFirstNodeIds(
      ProductionNodeId production_node_id, ForwardNodesContainer* result,
      std::unordered_set<ProductionNodeId>&& processed_nodes =
          std::unordered_set<ProductionNodeId>());
  /// @brief �հ������е�first������������ȡ���ս����ʽ����ǰ���ڵ�
  /// @param[in] production_node_id �����ս����ʽ�ڵ�ID
  /// @param[in] production_body_id �����ս����ʽ��ID
  /// @param[in] next_word_to_shift_index
  /// ��ָ����ȡ��ǰ�����ŵ���ʼ����ʽλ�ã����ս����ʽ����һ��λ�ã�
  /// @param[in] next_node_ids
  /// ����ȡ������ս����ʽ��βʱ����������������ӵ����ؽ����
  /// ����չ���ķ��ս����ʽ���ڵķ��ս����ʽ����ǰ�����ţ�
  /// @return ���ش洢��ȡ������ǰ������ID������
  /// @details
  /// 1.production_node_id��production_body_id��next_word_to_shift_index��־
  /// �µ�λ�ã���ȡ��ǰ�����ŵ�λ�ã�
  /// 2.�Զ��ݹ鴦����Կչ�Լ�ķ��ս����ʽ
  /// 3.��Ϊ���ս�ڵ���ܿչ�Լ����Ҫ���²��Ҳ���ʽ��
  /// ����ǰ�����������ܺϲ�Ϊ��չ���Ĳ���ʽID
  ForwardNodesContainer First(ProductionNodeId production_node_id,
                              ProductionBodyId production_body_id,
                              NextWordToShiftIndex next_word_to_shift_index,
                              const ForwardNodesContainer& next_node_ids);
  /// @brief ��ȡ���ȫ����������ǰ������
  /// @param[in] production_item_set_id ���ID
  /// @return ���ش洢��Ͷ�Ӧ��ǰ�����ŵ�������const����
  /// @note Ҫ��production_item_set_id��Ч
  const ProductionItemAndForwardNodesContainer&
  GetProductionItemsAndForwardNodes(
      ProductionItemSetId production_item_set_id) {
    return GetProductionItemSet(production_item_set_id)
        .GetItemsAndForwardNodeIds();
  }
  /// @brief ��ȡ�﷨��������Ŀ
  /// @param[in] syntax_analysis_table_entry_id ���﷨��������ĿID
  /// @return �����﷨��������Ŀ������
  /// @note Ҫ��syntax_analysis_table_entry_id��Ч
  SyntaxAnalysisTableEntry& GetSyntaxAnalysisTableEntry(
      SyntaxAnalysisTableEntryId syntax_analysis_table_entry_id) {
    assert(syntax_analysis_table_entry_id < syntax_analysis_table_.size());
    return syntax_analysis_table_[syntax_analysis_table_entry_id];
  }
  /// @brief ���ø��﷨��������ĿID
  /// @param[in] root_syntax_analysis_table_entry_id �����﷨��������ĿID
  void SetRootSyntaxAnalysisTableEntryId(
      SyntaxAnalysisTableEntryId root_syntax_analysis_table_entry_id) {
    root_syntax_analysis_table_entry_id_ = root_syntax_analysis_table_entry_id;
  }
  /// @brief ������հ���Ч
  /// @param[in] production_item_set_id ���ID
  /// @note
  /// �հ���Ч����ڵ���ProductionItemSetClosureʱֱ�ӷ��أ���������հ�
  /// @attention ��Ӧ��ProductionItemSetClosure����
  void SetProductionItemSetClosureAvailable(
      ProductionItemSetId production_item_set_id) {
    GetProductionItemSet(production_item_set_id).SetClosureAvailable();
  }
  /// @brief ��ѯһ����Ƿ�հ���Ч
  /// @param[in] production_item_set_id ���ID
  /// @return ���ظ�����Ƿ�հ���Ч
  /// @retval true ���հ���Ч
  /// @retval false ���հ���Ч
  bool IsClosureAvailable(ProductionItemSetId production_item_set_id) {
    return GetProductionItemSet(production_item_set_id).IsClosureAvailable();
  }
  /// @brief �����հ���ͬʱ�����﷨������reduct��������
  /// @param[in] production_item_set_id ���ID
  /// @return �����Ƿ�����հ�
  /// @retval true ������հ�
  /// @retval false ���հ���Ч����������
  /// @note
  /// 1.�Զ�������е�ǰλ�ÿ��Կչ�Լ����ĺ�����
  /// 2.����հ�ǰ������﷨��������Ŀ�ͷǺ�����
  /// 3.��հ��������Զ���д�﷨�������пɹ�Լ����
  bool ProductionItemSetClosure(ProductionItemSetId production_item_set_id);
  /// @brief ��ȡ������������ͬ����ʽ�󹹳ɵ��
  /// @param[in] items ��ָ��ת��ǰ����ĵ�����
  /// @return ���ػ�ȡ�����ID
  /// @retval ProductionItemSetId::InvalidId()
  /// ��������������ͬ����ʽ��δ���������
  /// @details
  /// �������������������ͬ����ʽ�������ҽ�����Щ���Ǻ�����
  ProductionItemSetId GetProductionItemSetIdFromProductionItems(
      const std::list<std::unordered_map<
          ProductionItem, std::unordered_set<ProductionNodeId>,
          ProductionItemSet::ProductionItemHasher>::const_iterator>& items);
  /// @brief ������ǰ�����ţ�ͬʱ�ڴ��������й����﷨������shift�����Ĳ���
  /// @param[in] production_item_set_id ����������ǰ�����ŵ��ID
  /// @return �����Ƿ�ִ���˴�������
  /// @retval true ��ִ���˴�����ǰ�����ŵĹ���
  /// @retval false ������ִ�д�����ǰ�����ŵĹ���
  /// @details
  /// �հ���Ч˵������Ͷ�Ӧ���﷨��������Ŀδ�޸ģ��������´�����ǰ������
  bool SpreadLookForwardSymbolAndConstructSyntaxAnalysisTableEntry(
      ProductionItemSetId production_item_set_id);
  /// @brief �����в���ʽ�ڵ㰴��ProductionNodeType����
  /// @return ���ش洢��ͬ���ͽڵ������
  /// @note
  /// ProductionNodeType�����͵�ֵ��Ϊ�±����array�Եõ������͵����в���ʽ�ڵ�
  std::array<std::vector<ProductionNodeId>, 4> ClassifyProductionNodes() const;
  /// @brief �������ս����ʽ��Ϊ��ǰ������ʱ�¶�����ͬ���﷨��������Ŀ
  /// @param[in] terminal_node_ids ������������ս����ʽ�ڵ�
  /// @param[in] index
  /// ����������������ս����ʽ�ڵ�ID��terminal_node_ids�е�λ��
  /// @param[in] syntax_analysis_table_entry_ids ����������﷨��������Ŀ
  /// @param[in,out] equivalent_ids ���ȼ۵��﷨��������Ŀ
  /// @details
  /// 1.�״ε���ʱindexһ��ʹ��0
  /// 2.�ú���ֻ��equivalent_ids����д��ȼ۵��﷨��������ĿID�����ϲ���Щ��Ŀ
  /// 3.����д��ֻ��һ�������
  /// 4.�ú�����SyntaxAnalysisTableEntryClassify���ӹ���
  void SyntaxAnalysisTableTerminalNodeClassify(
      const std::vector<ProductionNodeId>& terminal_node_ids, size_t index,
      std::list<SyntaxAnalysisTableEntryId>&& syntax_analysis_table_entry_ids,
      std::vector<std::list<SyntaxAnalysisTableEntryId>>* equivalent_ids);
  /// @brief ������ս����ʽ��Ϊ��ǰ������ʱ�¶�����ͬ���﷨��������Ŀ
  /// @param[in] nonterminal_node_ids ����������ķ��ս����ʽ�ڵ�
  /// @param[in] index
  /// ��������������ķ��ս����ʽ�ڵ�ID��nonterminal_node_ids�е�λ��
  /// @param[in] syntax_analysis_table_entry_ids ����������﷨��������Ŀ
  /// @param[in,out] equivalent_ids ���ȼ۵��﷨��������Ŀ
  /// @details
  /// 1.�״ε���ʱindexһ��ʹ��0
  /// 2.�ú���ֻ��equivalent_ids����д��ȼ۵��﷨��������ĿID�����ϲ���Щ��Ŀ
  /// 3.����д��ֻ��һ�������
  /// 4.�ú�����SyntaxAnalysisTableEntryClassify���ӹ���
  void SyntaxAnalysisTableNonTerminalNodeClassify(
      const std::vector<ProductionNodeId>& nonterminal_node_ids, size_t index,
      std::list<SyntaxAnalysisTableEntryId>&& syntax_analysis_table_entry_ids,
      std::vector<std::list<SyntaxAnalysisTableEntryId>>* equivalent_ids);
  /// @brief ����ȼ۵��﷨��������Ŀ
  /// @param[in] operator_node_ids ������������ڵ�ID
  /// @param[in] terminal_node_ids �������ս�ڵ�ID
  /// @param[in] nonterminal_node_ids �����з��ս�ڵ�ID
  /// @return ���ؿ��Ժϲ����﷨��������Ŀ�飬�������������������Ŀ
  /// @details SyntaxAnalysisTableMergeOptimize���ӹ���
  std::vector<std::list<SyntaxAnalysisTableEntryId>>
  SyntaxAnalysisTableEntryClassify(
      std::vector<ProductionNodeId>&& operator_node_ids,
      std::vector<ProductionNodeId>&& terminal_node_ids,
      std::vector<ProductionNodeId>&& nonterminal_node_ids);
  /// @brief ��ӳ���﷨��������ʹ�õ�ID
  /// @param[in] old_id_to_new_id ����ID����ID��ӳ��
  /// @note old_id_to_new_id���洢��Ҫ�޸ĵ�ID�����ı��ID����洢
  void RemapSyntaxAnalysisTableEntryId(
      const std::unordered_map<SyntaxAnalysisTableEntryId,
                               SyntaxAnalysisTableEntryId>& old_id_to_new_id);
  /// @brief ������ΪͼƬ
  /// @param[in] root_production_set_id �����ID
  /// @param[in] image_output_path ��ͼƬ���·���������ļ����� ��'/'��β��
  void FormatProductionItemSetToImageGraphivz(
      ProductionItemSetId root_production_item_set_id,
      const std::string& image_output_path = "./");
  /// @brief ������Ϊmarkdown������ͼ��
  /// @param[in] root_production_set_id �����ID
  /// @param[in] image_output_path ���ļ����·���������ļ����� ��'/'��β��
  void FormatProductionItemSetToMarkdown(
      ProductionItemSetId root_production_item_set_id,
      const std::string& image_output_path = "./");
  /// @brief �ϲ��﷨�������ڵȼ���Ŀ�������﷨�������С
  void SyntaxAnalysisTableMergeOptimize();

  /// @brief ���﷨����������д���ļ�
  /// @param[in] config_file_output_path
  /// �������ļ����·���������ļ�������'/'��β��
  /// @details ��ָ��·��������﷨������ʹʷ�������
  /// �﷨�����������ļ���Ϊfrontend::common::kSyntaxConfigFileName��
  /// �ʷ������������ļ���Ϊfrontend::common::kDfaConfigFileName
  void SaveConfig(const std::string& config_file_output_path = "./") const;

  /// @brief ��ʽ������ʽ
  /// @param[in] nonterminal_node_id �����ս����ʽID
  /// @param[in] production_body_id �����ս����ʽ��ID
  /// @return ���ظ�ʽ������ַ���
  /// @details ��ʽ��nonterminal_node_id��production_body_idָ���Ĳ���ʽ
  /// @note ����ֵ���� IdOrEquivence -> IdOrEquivence [ Num ]
  std::string FormatSingleProductionBody(
      ProductionNodeId nonterminal_node_id,
      ProductionBodyId production_body_id) const;
  /// @brief ��ʽ����
  /// @param[in] production_item ������ʽ������
  /// @return ���ظ�ʽ������ַ���
  /// @details
  /// ����ֵ����IdOrEquivence -> IdOrEquivence �� [ Num ]
  /// ���Ҳ�Ϊ��һ������Ĳ���ʽ
  std::string FormatProductionItem(const ProductionItem& production_item) const;
  /// @brief ��ʽ����ǰ�����ż�
  /// @param[in] look_forward_node_ids ������ʽ������ǰ�����ż�
  /// @return ���ظ�ʽ������ַ���
  /// @details
  /// 1.��ǰ�����ż�ͨ���ո�ָ�
  /// 2.�������߲���˫����
  /// 3.���һ�����ź��޿ո�
  /// @note ����ֵ����const Id * ( [ )
  std::string FormatLookForwardSymbols(
      const ForwardNodesContainer& look_forward_node_ids) const;
  /// @brief ��ʽ����������ǰ������
  /// @param[in] production_item ����
  /// @param[in] look_forward_node_ids �������ǰ������
  /// @return ���ظ�ʽ������ַ���
  /// @details
  /// 1.��ʽ����������" ��ǰ�����ţ�"���ȫ����ǰ������
  /// 2.�������ʽͬFormatProductionItem
  /// 3.��ǰ�����Ÿ�ʽͬFormatLookForwardSymbols
  /// 4.����ֵ����StructType -> �� StructureDefine ��ǰ�����ţ�const Id * ( [ )
  std::string FormatProductionItemAndLookForwardSymbols(
      const ProductionItem& production_item,
      const ForwardNodesContainer& look_forward_node_ids) const;
  /// @brief ��ʽ�����ȫ������Ӧ����ǰ������
  /// @param[in] production_item_set_id ���ID
  /// @return ���ظ�ʽ������ַ���
  /// @details
  /// 1.����ʽ��ʽͬFormatProductionItem
  /// 2.������֮��ͨ��line_feed�ָ�
  /// 3.���һ�����line_feed
  /// 4.����ֵ����line_feedΪ"\n"����
  /// StructureDefineHead -> union �� ��ǰ�����ţ�{
  /// StructureAnnounce -> union �� Id ��ǰ�����ţ�const , Id * ( { [ )
  std::string FormatProductionItems(ProductionItemSetId production_item_set_id,
                                    std::string line_feed = "\n") const;
  /// @brief ��graphviz��label�������ַ�ת��
  static std::string EscapeLabelSpecialCharacter(const std::string& source);
  /// @brief ���Info�������Ϣ
  /// @param[in] info ��Info�������Ϣ
  /// @note ������Զ�����
  static void OutPutInfo(const std::string& info) {
     //std::cout << std::format("SyntaxGenerator Info: ") << info << std::endl;
  }
  /// @brief ���Warning�������Ϣ
  /// @param[in] warning ��Warning�������Ϣ
  /// @note ������Զ�����
  static void OutPutWarning(const std::string& warning) {
    std::cerr << std::format("SyntaxGenerator Warning: ") << warning
              << std::endl;
  }
  /// @brief ���Error�������Ϣ
  /// @param[in] error ��Error�������Ϣ
  /// @note ������Զ�����
  static void OutPutError(const std::string& error) {
    std::cerr << std::format("SyntaxGenerator Error: ") << error << std::endl;
  }

  /// @brief �������л������
  friend class boost::serialization::access;
  /// @brief boost-serialization���������﷨���������õĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const;
  /// �����л���Ϊ��������أ�Generator���������ã�������
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  /// @class SyntaxAnalysisTableEntryIdAndProcessFunctionClassIdHasher
  /// syntax_generator.h
  /// @brief ��ϣ�﷨��������ĿID���Լ����ID����
  /// @note
  /// ����SyntaxAnalysisTableTerminalNodeClassify�з���ͬʱ֧�ֹ�Լ�����������
  struct SyntaxAnalysisTableEntryIdAndProcessFunctionClassIdHasher {
    size_t operator()(
        const std::pair<SyntaxAnalysisTableEntryId, ProcessFunctionClassId>&
            data_to_hash) const {
      return data_to_hash.first * data_to_hash.second;
    }
  };

  /// @brief �洢����δ�������ʽ���Ƴ���ӵĲ���ʽ����
  /// @details
  /// ����δ����Ĳ���ʽ��
  /// tuple�ڵ�std::string�Ƿ��ս����ʽ��
  /// std::tuple<std::string, std::vector<std::string>,ProcessFunctionClassId>
  /// Ϊ����ӵĲ���ʽ����Ϣ
  /// ProcessFunctionClassId�ǰ�װ��Լ���������ʵ��������ID
  std::unordered_multimap<
      std::string,
      std::tuple<std::string, std::vector<std::string>, ProcessFunctionClassId>>
      undefined_productions_;
  /// @brief �洢����ʽ�ڵ�
  ObjectManager<BaseProductionNode> manager_nodes_;
  /// @brief �洢����ʽ�����ս�/���ս�/�������
  UnorderedStructManager<std::string, std::hash<std::string>>
      manager_node_symbol_;
  /// @brief �洢�ս�ڵ����ʽ��ķ��ţ�������ֹ������ͬһ����
  UnorderedStructManager<std::string, std::hash<std::string>>
      manager_terminal_body_symbol_;
  /// @brief ����ʽ��ID����Ӧ����ʽ�ڵ��ӳ��
  std::unordered_map<SymbolId, ProductionNodeId> node_symbol_id_to_node_id_;
  /// @brief �ս����ʽ�����ID����Ӧ�ڵ�ID��ӳ��
  std::unordered_map<SymbolId, ProductionNodeId>
      production_body_symbol_id_to_node_id_;
  /// @brief �洢�
  ObjectManager<ProductionItemSet> production_item_sets_;
  /// @brief �洢�﷨��������ĿID���ID��ӳ��
  std::unordered_map<SyntaxAnalysisTableEntryId, ProductionItemSetId>
      syntax_analysis_table_entry_id_to_production_item_set_id_;
  /// @brief �û�����ĸ����ս����ʽ�ڵ�ID
  ProductionNodeId root_production_node_id_ = ProductionNodeId::InvalidId();
  /// @brief ��ʼ�﷨��������ĿID������д���ļ�
  SyntaxAnalysisTableEntryId root_syntax_analysis_table_entry_id_;
  /// @brief �﷨����������д���ļ�
  SyntaxAnalysisTableType syntax_analysis_table_;
  /// @brief ��װ��Լ���������ʵ������������д���ļ�
  /// @details
  /// ÿ����Լ���ݶ��������Ψһ�İ�װ��Լ������ʵ�������󣬲��������ö���
  ProcessFunctionClassManagerType manager_process_function_class_;
  /// @brief DFA����������������д���ļ�
  frontend::generator::dfa_generator::DfaGenerator dfa_generator_;
};

template <class IdType>
inline ProductionBodyId NonTerminalProductionNode::AddBody(
    IdType&& body, ProcessFunctionClassId class_for_reduct_id) {
  ProductionBodyId body_id(nonterminal_bodys_.size());
  // ��������뵽����ʽ�������У���ɾ����ͬ����ʽ����
  nonterminal_bodys_.emplace_back(std::forward<IdType>(body),
                                  class_for_reduct_id);
  return body_id;
}

template <class ProcessFunctionClass>
inline ProductionNodeId SyntaxGenerator::AddNonTerminalProduction(
    std::string node_symbol, std::string subnode_symbols) {
  // ��#__VA_ARGS__�������ޣ�ֻ�ܽ�__VA_ARGS__�������������תΪһ���ַ�������Ҫ�ֶ��ָ�
  std::vector<std::string> splited_subnode_symbols(1);
  for (char c : subnode_symbols) {
    if (c == ',') {
      // ��ǰ�ַ���������׼��������һ���ַ���
      splited_subnode_symbols.emplace_back();
    } else if (!isblank(c)) {
        // [a-zA-Z_][a-zA-Z0-9_]*
      splited_subnode_symbols.back().push_back(c);
    }
  }

  ProcessFunctionClassId class_id =
      CreateProcessFunctionClassObject<ProcessFunctionClass>();
  return AddNonTerminalProduction(std::move(node_symbol),
                                  std::move(splited_subnode_symbols), class_id);
}

template <class ForwardNodeIdContainer>
inline std::pair<
    SyntaxGenerator::ProductionItemAndForwardNodesContainer::iterator, bool>
SyntaxGenerator::AddItemAndForwardNodeIdsToProductionItem(
    ProductionItemSetId production_item_set_id,
    const ProductionItem& production_item,
    ForwardNodeIdContainer&& forward_node_ids) {
  assert(production_item_set_id.IsValid());
  auto result = GetProductionItemSet(production_item_set_id)
                    .AddItemAndForwardNodeIds(
                        production_item,
                        std::forward<ForwardNodeIdContainer>(forward_node_ids));
  return result;
}

template <class ForwardNodeIdContainer>
inline std::pair<
    SyntaxGenerator::ProductionItemAndForwardNodesContainer::iterator, bool>
SyntaxGenerator::AddMainItemAndForwardNodeIdsToProductionItem(
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

#endif  /// !GENERATOR_SYNTAXGENERATOR_SYNTAXGENERATOR_H_