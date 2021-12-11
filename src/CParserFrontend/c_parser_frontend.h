/// @file c_parser_frontend.h
/// @brief C���Ա�����ǰ�˿�����
/// @details
/// CParserFrontend�������й���Լ�������õĽӿڣ�����TypeSystem��
/// ActionScopeSystem��FlowControlSystem���ܣ��ṩ�ϲ�ӿ�
#ifndef CPARSERFRONTEND_C_PARSER_FRONTEND_H_
#define CPARSERFRONTEND_C_PARSER_FRONTEND_H_

#include <iostream>
#include <unordered_map>

#include "Parser/line_and_column.h"
#include "action_scope_system.h"
#include "flow_control.h"
#include "operator_node.h"
#include "type_system.h"

namespace c_parser_frontend {
/// @brief ��ȡ��ǰ����
/// @note ��0��ʼ����
inline size_t GetLine() { return frontend::parser::GetLine(); }
/// @brief ��ȡ��ǰ����
/// @note ��0��ʼ����
inline size_t GetColumn() { return frontend::parser::GetColumn(); }

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
  /// @brief ��������
  /// @param[in] type_name ��������
  /// @param[in] type_pointer ��������ͷ���ָ��
  /// @return ǰ�벿��Ϊָ�����λ�õĵ���������벿��Ϊ��ӽ��
  /// @note
  /// �����������붨����߸ýӿڣ�����������ʱ���Զ��������������Ը�������
  /// �Ѵ��ڶ����򷵻�AddTypeResult::kAlreadyIn
  template <class TypeName>
  std::pair<TypeSystem::TypeNodeContainerIter, AddTypeResult> DefineType(
      TypeName&& type_name,
      const std::shared_ptr<const TypeInterface>& type_pointer) {
    return type_system_.DefineType(std::forward<TypeName>(type_name),
                                   type_pointer);
  }
  /// @brief ������������ȡ������
  /// @param[in] type_name ��������
  /// @param[in] type_prefer ������ѡ������
  /// @return ǰ�벿��Ϊָ���ȡ����������ͷ���ָ�룬��벿��Ϊ��ȡ���
  /// @ref c_parser_frontend::type_system::TypeSystem::GetType
  std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
      const std::string& type_name, StructOrBasicType type_prefer) {
    return type_system_.GetType(type_name, type_prefer);
  }
  /// @brief ��������ʹ�øýӿ�
  /// @param[in] function_type ���������ĺ�������
  /// @return ǰ�벿��Ϊָ�����λ�õĵ���������벿��Ϊ��ӽ��
  /// @note �����������Ὣ�ú�������Ϊ��ǰ��Ծ����
  std::pair<TypeSystem::TypeNodeContainerIter, AddTypeResult> AnnounceFunction(
      const std::shared_ptr<const FunctionType>& function_type);
  /// @brief �������
  /// @param[in] operator_node �������ڵ�
  /// @return ǰ�벿��Ϊָ�����λ�õĵ���������벿��Ϊ��ӽ��
  /// @note operator_node�����Ѿ����ñ�����
  std::pair<ActionScopeSystem::ActionScopeContainerType::const_iterator,
            DefineVarietyResult>
  DefineVariety(
      const std::shared_ptr<const VarietyOperatorNode>& operator_node) {
    return action_scope_system_.DefineVariety(operator_node);
  }
  /// @brief �������ֻ�ȡ����������
  /// @param[in] variety_name ����ѯ������
  /// @return ǰ�벿��Ϊָ����������ڵ��ָ�룬��벿��Ϊ�ڵ��Ƿ����
  std::pair<std::shared_ptr<const operator_node::OperatorNodeInterface>, bool>
  GetVarietyOrFunction(const std::string& variety_name) const {
    return action_scope_system_.GetVarietyOrFunction(variety_name);
  }
  /// @brief ����һ��������ȼ�
  void AddActionScopeLevel() { action_scope_system_.AddActionScopeLevel(); }
  /// @brief ����һ��������
  /// @details
  /// �Զ�����ʧЧ�����������еı��������̿�����䣬�����ϼ������ӵ�����
  /// ��������ɵģ����̿������
  /// @note ������������ʱ������øú��������������������
  void PopActionScope() { action_scope_system_.PopActionScope(); }
  size_t GetActionScopeLevel() const {
    return action_scope_system_.GetActionScopeLevel();
  }
  /// @brief ���õ�ǰ����������
  /// @param[in] function_to_construct ������������������
  /// @return �����Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false ���������ڹ����ĺ�����������Ƕ�׶��庯�����ض���/����������
  /// @details
  /// ��������������������ȫ�������ͺ������͵ĳ�ʼ���������Զ�����1��������ȼ�
  /// @attention ��ֹʹ��PushFlowControlSentence���ô���������
  bool SetFunctionToConstruct(
      const std::shared_ptr<const FunctionType>& function_to_construct);
  /// @brief �����̿������ѹ��ջ
  /// @param[in] flow_control_sentence ������ջ�Ŀ������
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false �������ڹ����ĺ������޷�ѹ�����̿��ƽڵ�
  /// @attention ����ѹ����Դ洢���̿��ƽڵ�����̿������
  bool PushFlowControlSentence(
      std::unique_ptr<FlowInterface>&& flow_control_sentence);
  /// @brief ��ȡ���Ŀ������
  /// @return �������̿�����������
  /// @note ��������do-while���β����for���ͷ��
  /// @attention ����������ڹ��������̿���������
  FlowInterface& GetTopFlowControlSentence() {
    return action_scope_system_.GetTopFlowControlSentence();
  }
  /// @brief ��ȡ��ǰ��Ծ���������ڹ����ĺ�����
  /// @return ����ָ��ǰ��Ծ������constָ��
  /// @retval nullptr ����ǰû�л�Ծ����
  const FunctionDefine* GetActiveFunctionPointer() const {
    return flow_control_system_.GetActiveFunctionPointer();
  }
  /// @brief ��ȡ��ǰ��Ծ���������ڹ����ĺ�����
  /// @return ���ص�ǰ��Ծ������const����
  /// @note ������ڻ�Ծ����
  const FunctionDefine& GetActiveFunctionReference() const {
    return flow_control_system_.GetActiveFunctionReference();
  }
  /// @brief �򶥲����̿��������������
  /// @param[in] sentence ������ӵ����
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�����ȡsentence����Ȩ
  /// @retval false
  /// ���޻�Ծ�����̿��������������޷���ӵ���ǰ�����̿��ƽڵ��У����޸Ĳ���
  /// @note ��ӵ����е�����
  bool AddSentence(std::unique_ptr<FlowInterface>&& sentence) {
    return action_scope_system_.AddSentence(std::move(sentence));
  }
  /// @brief ��ǰ��Ծ�����̿����������Ӷ������
  /// @param[in] sentence_container ���洢����ӵ���������
  /// @return �����Ƿ���ӳɹ�
  /// @retval true
  /// ����ӳɹ�����sentence_container����������ƶ�����Ծ���̿���������������
  /// @retval false ����������޷���ӵ���ǰ�����̿��ƽڵ��У����޸Ĳ���
  /// @note ��begin->end��˳����ӣ���ӵ����е���������
  bool AddSentences(
      std::list<std::unique_ptr<FlowInterface> >&& sentence_container) {
    return action_scope_system_.AddSentences(std::move(sentence_container));
  }

  /// @brief ���������̿�������if���ת��Ϊif-else���
  /// @note ת����ִ��AddSentenceϵ�к���ʱ�����䵽�ٷ�֧��
  /// @attention �������̿�����������if���
  void ConvertIfSentenceToIfElseSentence() {
    action_scope_system_.ConvertIfSentenceToIfElseSentence();
  }
  /// @brief ���switch��ͨcase
  /// @param[in] case_value ����֧��ֵ
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ������ӵ�ֵ�Ѵ��ڻ򶥲����̿�����䲻Ϊswitch
  bool AddSwitchSimpleCase(
      const std::shared_ptr<
          c_parser_frontend::flow_control::BasicTypeInitializeOperatorNode>&
          case_value) {
    return action_scope_system_.AddSwitchSimpleCase(case_value);
  }
  /// @brief ���switch��default��֧��ǩ
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ���Ѿ���ӹ�default��ǩ�򶥲����̿�����䲻Ϊswitch
  bool AddSwitchDefaultCase() {
    return action_scope_system_.AddSwitchDefaultCase();
  }

 private:
  /// @brief ���ڸ������FinishFunctionConstruct
  friend ActionScopeSystem;

  /// @brief ��һЩ��ɺ�����������������
  void FinishFunctionConstruct() {
    // �����Ƿ����������������øú����󶼻ᱣ֤�ú������ʹ���
    type_system_.AnnounceFunctionType(
        flow_control_system_.GetActiveFunctionReference()
            .GetFunctionTypePointer());
    flow_control_system_.FinishFunctionConstruct();
  }

  /// @brief ����ϵͳ
  TypeSystem type_system_;
  /// @brief ������ϵͳ
  ActionScopeSystem action_scope_system_;
  /// @brief ���̿���ϵͳ
  FlowControlSystem flow_control_system_;
};

/// @brief ȫ��CParserFrontend����
/// @details
/// ������ϵͳ�������û�����
/// �̼߳����
extern thread_local CParserFrontend c_parser_frontend;

}  // namespace c_parser_frontend

#endif  /// !CPARSERFRONTEND_C_PARSER_FRONTEND_H_
