#ifndef CPARSERFRONTEND_C_PARSER_FRONTEND_H_
#define CPARSERFRONTEND_C_PARSER_FRONTEND_H_

#include <unordered_map>

#include "Generator/SyntaxGenerator/process_function_interface.h"
#include "Parser/DfaMachine/dfa_machine.h"
#include "action_scope_system.h"
#include "flow_control.h"
#include "operator_node.h"
#include "type_system.h"

// ���ý�����������������
namespace frontend::parser {
extern thread_local size_t line_;
extern thread_local size_t column_;
}  // namespace frontend::parser

namespace c_parser_frontend {
// ��ȡ��ǰ����
size_t GetLine();
// ��ȡ��ǰ����
size_t GetColumn();

class CParserFrontend {
  // ����һЩ���͵Ķ���
  using DefineVarietyResult =
      c_parser_frontend::action_scope_system::DefineVarietyResult;
  using ActionScopeSystem =
      c_parser_frontend::action_scope_system::ActionScopeSystem;
  using FunctionDefine = c_parser_frontend::flow_control::FunctionDefine;
  using VarietyOperatorNode =
      c_parser_frontend::operator_node::VarietyOperatorNode;
  using StructOrBasicType = c_parser_frontend::type_system::StructOrBasicType;
  using TypeInterface = c_parser_frontend::type_system::TypeInterface;
  using TypeSystem = c_parser_frontend::type_system::TypeSystem;
  using AddTypeResult = c_parser_frontend::type_system::AddTypeResult;
  using GetTypeResult = c_parser_frontend::type_system::GetTypeResult;
  using FlowControlSystem = c_parser_frontend::flow_control::FlowControlSystem;
  using FunctionType = c_parser_frontend::type_system::FunctionType;
  using FlowInterface = c_parser_frontend::flow_control::FlowInterface;

 public:
  enum class AddFunctionDefineResult {
    // ������ӵ����
    kSuccess,  // �ɹ����
    // ��������ӵ����
    kReDefine,  // �ض��庯��
    kOverLoad   // ���غ���
  };

  // �������
  // ����ָ�����λ�õĵ���������ӽ��
  // �����������붨����߸ýӿڣ�����������ʱ���Զ��������������Ը�������
  // �Ѵ��ڶ����򷵻�AddTypeResult::kAlreadyIn
  template <class TypeName>
  std::pair<TypeSystem::TypeNodeContainerIter, AddTypeResult> DefineType(
      TypeName&& type_name,
      const std::shared_ptr<const TypeInterface>& type_pointer) {
    return type_system_.DefineType(std::forward<TypeName>(type_name),
                                   type_pointer);
  }
  // �������ͣ���֤��������Ч������ڵ��Ѵ���
  template <class TypeName>
  TypeSystem::TypeNodeContainerIter AnnounceTypeName(TypeName&& type_name) {
    return type_system_.AnnounceTypeName(std::forward<TypeName>(type_name));
  }
  std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
      const std::string& type_name, StructOrBasicType type_prefer) {
    return type_system_.GetType(type_name, type_prefer);
  }
  // ������������ʹ�øýӿ�
  std::pair<TypeSystem::TypeNodeContainerIter, AddTypeResult>
  AnnounceFunctionType(
      const std::shared_ptr<const FunctionType>& function_type) {
    return type_system_.AnnounceFunctionType(function_type);
  }
  // ���庯������ʹ�øýӿ�
  std::pair<TypeSystem::TypeNodeContainerIter, AddTypeResult>
  DefineFunctionType(const std::shared_ptr<const FunctionType>& function_type) {
    return type_system_.DefineFunctionType(function_type);
  }
  // ��ӱ���
  // ����ָ�����λ�õĵ���������ӽ������ӽ�����������
  template <class VarietyName>
  std::pair<ActionScopeSystem::ActionScopeContainerType::const_iterator,
            DefineVarietyResult>
  DefineVariety(
      VarietyName&& variety_name,
      const std::shared_ptr<const VarietyOperatorNode>& operator_node) {
    return action_scope_system_.DefineVariety(
        std::forward<VarietyName>(variety_name), operator_node);
  }
  // ��������������ָ�����λ�õĵ�����
  // ��֤����ڵ���Ч�ҹ���ڵ����
  template <class VarietyName>
  ActionScopeSystem::ActionScopeContainerType::const_iterator
  AnnounceVarietyName(VarietyName&& variety_name) {
    return action_scope_system_.AnnounceVarietyName(
        std::forward<VarietyName>(variety_name));
  }
  std::pair<std::shared_ptr<const VarietyOperatorNode>, bool> GetVariety(
      const std::string& variety_name) {
    return action_scope_system_.GetVariety(variety_name);
  }
  // ����һ��������ȼ�
  void AddActionScopeLevel() { action_scope_system_.AddActionScopeLevel(); }
  // ����һ���������Զ�����������������еı��������̿������
  // ������������ʱ������øú��������������������
  void PopActionScope() { action_scope_system_.PopActionScope(); }
  size_t GetActionScopeLevel() const {
    return action_scope_system_.GetActionScopeLevel();
  }
  // �Ƴ�����ϵͳ�սڵ㣬 �ڵ㲻�����Ƴ�
  // �����Ƿ��Ƴ��˽ڵ�
  bool RemoveTypeSystemEmptyNode(const std::string& empty_node_to_remove_name) {
    return type_system_.RemoveEmptyNode(empty_node_to_remove_name);
  }
  // �Ƴ�����ϵͳ�սڵ㣬 �ڵ㲻�����Ƴ�
  // �����Ƿ��Ƴ��˽ڵ�
  bool RemoveVarietySystemEmptyNode(
      const std::string& empty_node_to_remove_name) {
    return action_scope_system_.RemoveEmptyNode(empty_node_to_remove_name);
  }
  // ���õ�ǰ����������
  // ��������������������ȫ�������ͺ������͵Ķ������Զ�����������ȼ�
  void SetFunctionToConstruct(
      const std::shared_ptr<FunctionType>& function_to_construct) {
    flow_control_system_.SetFunctionToConstruct(function_to_construct);
    action_scope_system_.SetFunctionToConstruct(function_to_construct);
  }
  // �����̿������ѹ��ջ
  // �����Ƿ���ӳɹ�
  // ����޷������������������Ϣ
  void PushFlowControlSentence(
      std::unique_ptr<FlowInterface>&& flow_control_sentence);
  // ��ȡ���Ŀ������
  // ��������do-while���β����for���ͷ��
  FlowInterface& GetTopFlowControlSentence() {
    return action_scope_system_.GetTopFlowControlSentence();
  }
  // ��ǰ��Ծ�ĺ�����ִ�е����β���������
  // �����Ƿ���ӳɹ������ʧ�����޸Ĳ���
  // �����ǰ�����̿�������򷵻�false
  bool AddSentence(std::unique_ptr<FlowInterface>&& sentence) {
    return action_scope_system_.AddSentence(std::move(sentence));
  }
  // ��Ӵ�ִ�����ļ���
  // �����Ƿ���ӳɹ������ʧ�����޸Ĳ���
  // �����ǰ�����̿�������򷵻�false
  bool AddSentences(
      std::list<std::unique_ptr<FlowInterface> >&& sentence_container) {
    return action_scope_system_.AddSentences(std::move(sentence_container));
  }
  bool ConvertIfSentenceToIfElseSentence() {
    return action_scope_system_.ConvertIfSentenceToIfElseSentence();
  }
  // ���switch��ͨ��֧ѡ��
  // �����Ƿ���ӳɹ�
  // �����ǰ���̿�����䲻Ϊswitch�򷵻�false
  bool AddSwitchSimpleCase(
      const std::shared_ptr<
          c_parser_frontend::flow_control::BasicTypeInitializeOperatorNode>&
          case_value) {
    return action_scope_system_.AddSwitchSimpleCase(case_value);
  }
  // ���switchĬ�Ϸ�֧��ǩ
  // �����Ƿ���ӳɹ�
  // �����ǰ���̿�����䲻Ϊswitch�򷵻�false
  bool AddSwitchDefaultCase() {
    return action_scope_system_.AddSwitchDefaultCase();
  }

 private:
  // ���ڸ������FinishFunctionConstruct
  friend ActionScopeSystem;

  // ��һЩ��ɺ�����������������
  void FinishFunctionConstruct(
      const std::shared_ptr<c_parser_frontend::type_system::FunctionType>&
          function_type) {
    flow_control_system_.FinishFunctionConstruct();
    // ������ϵͳ�в��뺯������
    DefineFunctionType(function_type);
  }
  FunctionType& GetActiveFunctionReference() const {
    return flow_control_system_.GetActiveFunctionReference();
  }
  std::shared_ptr<FunctionType> GetActiveFunctionPointer() const {
    return flow_control_system_.GetActiveFunctionPointer();
  }
  // ����ϵͳ
  TypeSystem type_system_;
  // ������ϵͳ
  ActionScopeSystem action_scope_system_;
  // ���̿���ϵͳ
  FlowControlSystem flow_control_system_;
};

extern thread_local CParserFrontend parser_frontend;

}  // namespace c_parser_frontend

#endif  // !CPARSERFRONTEND_C_PARSER_FRONTEND_H_
