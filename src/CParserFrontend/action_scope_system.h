/// @file action_scope_system.h
/// @brief ������ϵͳ
#ifndef CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_
#define CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_
#include <memory>
#include <stack>
#include <unordered_map>
#include <variant>

#include "Common/id_wrapper.h"
#include "flow_control.h"
#include "operator_node.h"

namespace c_parser_frontend::action_scope_system {
/// @brief ��������ļ����
enum class DefineVarietyResult {
  ///< ������ӵ����
  kNew,           ///< ����ָ�룬�ýڵ㲻���洢ָ��
  kShiftToStack,  ///< �ýڵ���ǰ�洢��һ��ָ�룬���ڴ洢�ڶ�����ת��Ϊָ��ջ
  kAddToStack,    ///< ��ָ��ջ�����һ��ָ��
  ///< ��������ӵ����
  kReDefine,  ///< �ض���
};
/// @class ActionScopeSystem action_scope_system.h
/// @brief ������ϵͳ
/// @details
/// 1.������ϵͳ���������������/�ɼ�������̿������Ĺ���
/// 2.����������ı仯�Զ����������򳬳���Χ�ı����͹�����ɵ����̿������
/// 3.ÿ������/���̿�����䶼���Լ���������ȼ���������ȼ��������������������
///   ������䣨�����ţ�������ȫ�ֱ���Ϊ0�������ڹ����ĺ���Ϊ1��
/// @attention
/// ������ϵͳ�ڹ�������ʱ��FlowControlSystem��ȡָ��ǰ��Ծ�����ڵ��ָ�벢��
/// unique_ptr�洢��ָ�룬����ѹ�����̿���ջ�У���ǰ����������Ϻ󵯳���ָ�벻
/// �ͷ��ڴ棬�������Ա���ÿ��������ʱ����Ҫ�ж����̿���ջ�Ƿ�Ϊ�գ������ڵ�
/// �Ŀ���Ȩһֱ����FlowControlSystem��������ϵͳֻӵ�з���Ȩ
class ActionScopeSystem {
  using OperatorNodeInterface =
      c_parser_frontend::operator_node::OperatorNodeInterface;
  using VarietyOperatorNode =
      c_parser_frontend::operator_node::VarietyOperatorNode;
  /// @brief ������ȼ��ķַ���ǩ
  enum IdWrapperLabel { kActionScopeLevelType };
  /// @brief �洢��ǰ������ȼ�������
  using ActionScopeLevel = frontend::common::ExplicitIdWrapper<
      size_t, IdWrapperLabel, IdWrapperLabel::kActionScopeLevelType>;
  /// @class VarietyData action_scope_system.h
  /// @brief ���������Ϣ�Ĵ洢
  /// @details
  /// 1.Ĭ�ϴ洢ָ������ڵ��ָ��
  /// 2.����ͬ����������ǳ����������ʱת��Ϊջ
  /// 3.����ͬ���������Զ��ָ�����ָ��洢
  class VarietyData {
   public:
    /// @brief �洢����ָ�������
    using SinglePointerType =
        std::pair<std::shared_ptr<const OperatorNodeInterface>,
                  ActionScopeLevel>;
    /// @brief �洢���ָ���ջ����
    using PointerStackType = std::stack<SinglePointerType>;

    VarietyData() = default;
    template <class Data>
    VarietyData(Data&& data) : variety_data_(std::forward<Data>(data)) {}

    /// @brief ���һ���������ݻ��ʼ������
    /// @param[in] operator_node ���������ݻ��ʼ�����ݽڵ�
    /// @param[in] action_scope_level ��������ȼ�
    /// @return ���ؼ���������������
    /// @details
    /// �����ȴ����սڵ����øú�����������������
    /// @note ����ת����ջ���Զ�ִ��
    DefineVarietyResult AddVarietyOrInitData(
        const std::shared_ptr<const OperatorNodeInterface>& operator_node,
        ActionScopeLevel action_scope_level);
    /// @brief �����������
    /// @return �����Ƿ�Ӧ��ɾ��VarietyData�ڵ�
    /// @retval true ���������ݾ�������Ӧɾ���ýڵ�
    /// @retval false ���ڵ�����������
    /// @note �Զ�����ת��
    bool PopTopData();
    /// @brief ��ȡ������ָ��
    /// @return ���ر���ָ��
    SinglePointerType GetTopData() const;
    /// @brief �ж������Ƿ�δ�洢�κ�ָ��
    /// @return ���������Ƿ�δ�洢�κ�ָ��
    /// @retval true ������δ�洢�κ�ָ��
    /// @retval false �����ڴ洢����һ��ָ��
    bool Empty() const {
      return std::get_if<std::monostate>(&variety_data_) != nullptr;
    }

   private:
    /// @brief ��ȡ�洢ָ�������
    /// @return ���ش洢ָ�������������
    auto& GetVarietyData() { return variety_data_; }
    /// @brief ��ȡ�洢ָ�������
    /// @return ���ش洢ָ�������������
    const auto& GetVarietyData() const { return variety_data_; }

    /// @brief �洢ָ��ڵ��ָ�룬std::monostateΪ������ʱ���������
    std::variant<std::monostate, SinglePointerType,
                 std::unique_ptr<PointerStackType>>
        variety_data_;
  };

 public:
  /// @brief �洢��ͬ����������Ľṹ
  using ActionScopeContainerType = std::unordered_map<std::string, VarietyData>;

  ActionScopeSystem() { VarietyScopeSystemInit(); }
  ~ActionScopeSystem();

  /// @brief ��ʼ������ϵͳ
  void VarietyScopeSystemInit() {
    action_scope_level_ = ActionScopeLevel(0);
    variety_or_function_name_to_operator_node_pointer_.clear();
    while (!variety_stack_.empty()) {
      variety_stack_.pop();
    }
    // ѹ���ڱ����������и��ڸ������������ŵı���ʱ�����ж�ջ�Ƿ�Ϊ��
    // ����ջ�����ҽ�����һ��������ȼ�Ϊ0�ı���
    auto [iter, inserted] =
        variety_or_function_name_to_operator_node_pointer_.emplace(
            std::string(),
            VarietyData::SinglePointerType(nullptr, ActionScopeLevel(0)));
    assert(inserted == true);
    variety_stack_.emplace(std::move(iter));
  }

  /// @brief ����һ��������ȼ�
  void AddActionScopeLevel() { ++action_scope_level_; }
  /// @brief �������
  /// @param[in] variety_node_pointer ��ָ������ڵ��ָ��
  /// @return ǰ�벿��Ϊ����λ�õĵ���������벿��Ϊ�����
  /// @note ���������Ѿ����ñ�����
  /// ��DefineVarietyOrInitValue�ڶ������ʱ�ȼ�
  std::pair<ActionScopeContainerType::const_iterator, DefineVarietyResult>
  DefineVariety(
      const std::shared_ptr<const VarietyOperatorNode>& variety_node_pointer);
  /// @brief ����������ʼ������
  /// @param[in] name ��������/��ʼ������
  /// @param[in] operator_node_pointer ��ָ������ڵ��ָ��
  /// @return ǰ�벿��Ϊ����λ�õĵ���������벿��Ϊ�����
  /// @note ���������Ѿ����ñ�����
  /// DefineVariety�޷������ʼ��ֵ����Ϊ��ʼ��ֵ���������֣���Ҫ���⴫��
  /// �������ʱ�ṩ�����ֱ���������ڵ��д����������ͬ
  std::pair<ActionScopeContainerType::const_iterator, DefineVarietyResult>
  DefineVarietyOrInitValue(const std::string& name,
                           const std::shared_ptr<const OperatorNodeInterface>&
                               operator_node_pointer);
  /// @brief �������ֻ�ȡ����������
  /// @param[in] variety_name ����ѯ������
  /// @return ǰ�벿��Ϊָ����������ڵ��ָ�룬��벿��Ϊ�ڵ��Ƿ����
  std::pair<std::shared_ptr<const OperatorNodeInterface>, bool>
  GetVarietyOrFunction(const std::string& variety_name) const;
  /// @brief ���õ�ǰ����������
  /// @param[in] function_data ��ָ��������ĺ����ڵ��ָ��
  /// @return �����Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false ����ǰ������ȼ�����0����������Ƕ�׶��庯��
  /// @details
  /// 1.function_data�Ŀ���Ȩ������FlowControlSystem��ActionScopeSystem
  /// ����function_data�ķ���Ȩ��û�п���Ȩ����ֹdelete��ָ��
  /// 2.Ϊ���ܽ���ָ��������̿��ƽڵ�ջ�У�ʹ��std::unique_ptr<FlowInterface>
  ///   ��װ��ѹ��ջ���ڵ���ʱreleaseָ���ֹ���ͷ�
  /// 3.���ú��Զ�����һ��������ȼ�
  bool SetFunctionToConstruct(
      c_parser_frontend::flow_control::FunctionDefine* function_data);
  /// @brief ��ȡ��ǰ������ȼ�
  /// @return ���ص�ǰ������ȼ�
  ActionScopeLevel GetActionScopeLevel() const { return action_scope_level_; }
  /// @brief ѹ�����̿��ƽڵ�
  /// @param[in] flow_control_sentence ����ѹ������̿��ƽڵ�
  /// @return �����Ƿ�ѹ��ɹ�
  /// @retval true ��ѹ��ɹ�
  /// @retval false ��δѹ�뺯���ڵ���Ϊ���̿���ջ�ײ���ѹ���������̿��ƽڵ��
  /// ջ����ʱѹ�뺯���ڵ㣨Ƕ�׶��庯����
  /// @details
  /// 1.�Զ�����һ��������ȼ�
  /// 2.������һ��������ȼ���ѹ�����̿��ƽڵ㣬�����ڵ����ü�������ʱ���ܹ�һ��
  ///   ����������ɵ����̿��ƽڵ�
  /// @attention ����ѹ����Դ洢���̿��ƽڵ�����̿������
  bool PushFlowControlSentence(
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
          flow_control_sentence);
  /// @brief ����һ��������
  /// @details
  /// 1.�����������������еı��������̿�����䣬�Զ���ӵ��������̿������
  /// 2.���ú�������ȼ���1
  void PopActionScope() {
    assert(action_scope_level_ > 0);
    --action_scope_level_;
    PopOverLevel(GetActionScopeLevel());
  }
  /// @brief ��ȡ������̿������
  /// @return ����������̿�����������
  auto& GetTopFlowControlSentence() const {
    assert(!flow_control_stack_.empty());
    return *flow_control_stack_.top().first;
  }
  /// @brief �����̿������ջ����if���̿������ת��Ϊif-else���
  /// @note ���������̿�����������if���̿������
  void ConvertIfSentenceToIfElseSentence() {
    assert(flow_control_stack_.top().first->GetFlowType() ==
           c_parser_frontend::flow_control::FlowType::kIfSentence);
    static_cast<c_parser_frontend::flow_control::IfSentence&>(
        *flow_control_stack_.top().first)
        .ConvertToIfElse();
  }
  /// @brief ��switch����������ͨcase
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ��ǰ���������䲻Ϊswitch�������е�ֵ�ظ�
  bool AddSwitchSimpleCase(
      const std::shared_ptr<
          c_parser_frontend::flow_control::BasicTypeInitializeOperatorNode>&
          case_value);
  /// @brief ��switch��������default��ǩ
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ������ӹ�default��ǩ��ǰ���������䲻Ϊswitch
  bool AddSwitchDefaultCase();
  /// @brief ��ǰ��Ծ�����̿�����������һ�����
  /// @param[in] sentence ������ӵ����
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�����ȡsentence����Ȩ
  /// @retval false
  /// ���޻�Ծ�����̿��������������޷���ӵ���ǰ�����̿��ƽڵ��У����޸Ĳ���
  /// @note ��ӵ����е���������
  bool AddSentence(
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
          sentence) {
    if (flow_control_stack_.empty()) [[unlikely]] {
      return false;
    }
    return flow_control_stack_.top().first->AddMainSentence(
        std::move(sentence));
  }
  /// @brief ��ǰ��Ծ�����̿����������Ӷ������
  /// @param[in] sentence_container ���洢����ӵ���������
  /// @return �����Ƿ���ӳɹ�
  /// @retval true
  /// ����ӳɹ�����sentence_container����������ƶ�����Ծ���̿���������������
  /// @retval false ����������޷���ӵ���ǰ�����̿��ƽڵ��У����޸Ĳ���
  /// @note ��begin->end��˳����ӣ���ӵ����е���������
  bool AddSentences(
      std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&&
          sentence_container);

 private:
  /// @brief �������и��ڸ���������ȼ��ı��������̿������
  /// @param[in] level ���������и��ڸõȼ��ı��������̿������
  void PopOverLevel(ActionScopeLevel level);
  /// @brief ��ȡ�洢����/���������ڵ�ָ���ӳ�������
  /// @return ��������������
  ActionScopeContainerType& GetVarietyOrFunctionNameToOperatorNodePointer() {
    return variety_or_function_name_to_operator_node_pointer_;
  }
  /// @brief ��ȡ�洢����/���������ڵ�ָ���ӳ�������
  /// @return ����������const����
  const ActionScopeContainerType&
  GetVarietyOrFunctionNameToOperatorNodePointer() const {
    return variety_or_function_name_to_operator_node_pointer_;
  }
  /// @brief ��ȡ����ջ
  /// @return ���ر���ջ������
  auto& GetVarietyStack() { return variety_stack_; }
  /// @brief ��������������ƿ鲢ѹ�����̿��ƽڵ�ջ��
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ����ǰ������ȼ�����0����������Ƕ�׶��庯��
  /// @details
  /// 1.�ú����������������͵ĳ�ʼ��ֵ�����Ϊȫ�ֱ���������ֵʹ��
  /// 2.��������ȼ�0������1
  bool PushFunctionFlowControlNode(
      c_parser_frontend::flow_control::FunctionDefine* function_data);

  /// @brief �洢����/�������͵��ڵ�ָ���ӳ��
  /// @details
  /// ��ֵΪ������
  /// �ڷ�������ʱ��������ջת���ṹ���Խ����ڴ�ռ�ú�stack����������
  ActionScopeContainerType variety_or_function_name_to_operator_node_pointer_;
  /// @brief ��˳��洢��ӵı���
  /// @details
  /// ����֧���������ս���������������ȫ������
  /// iteratorָ������Ͷ�Ӧ����
  std::stack<ActionScopeContainerType::iterator> variety_stack_;
  /// @brief ��ǰ������ȼ�
  /// @note ȫ�ֱ���Ϊ0����ÿ��{�����̿����������1��
  ActionScopeLevel action_scope_level_ = ActionScopeLevel(0);
  /// @brief �洢��ǰ�������ڹ���������������������������ȼ�
  /// @note ��ײ�Ϊ��ǰ���ں�����������ȼ�Ϊ1
  std::stack<
      std::pair<std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>,
                ActionScopeLevel>>
      flow_control_stack_;
};

}  // namespace c_parser_frontend::action_scope_system

#endif  /// !CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_