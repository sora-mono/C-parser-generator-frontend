#ifndef CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_
#define CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_
#include <memory>
#include <stack>
#include <unordered_map>
#include <variant>

#include "Common/id_wrapper.h"
#include "flow_control.h"
#include "operator_node.h"

// ������ϵͳ
namespace c_parser_frontend::action_scope_system {
enum class DefineVarietyResult {
  // ������ӵ����
  kNew,           // ����ָ�룬�ýڵ㲻���洢ָ��
  kShiftToStack,  // �ýڵ���ǰ�洢��һ��ָ�룬���ڴ洢�ڶ�����ת��Ϊָ��ջ
  kAddToStack,    // ��ָ��ջ�����һ��ָ��
  // ��������ӵ����
  kReDefine,  // �ض���
};
class ActionScopeSystem {
  using OperatorNodeInterface =
      c_parser_frontend::operator_node::OperatorNodeInterface;
  using VarietyOperatorNode =
      c_parser_frontend::operator_node::VarietyOperatorNode;
  enum IdWrapperLabel { kActionScopeLevelType };
  // �洢��ǰ������ȼ�������
  using ActionScopeLevelType = frontend::common::ExplicitIdWrapper<
      size_t, IdWrapperLabel, IdWrapperLabel::kActionScopeLevelType>;
  // ���������Ϣ�Ĵ洢������Ҫ��ʱ��ת�����ִ洢��ʽ
  class VarietyData {
   public:
    // �洢����ָ�������
    using SinglePointerType =
        std::pair<std::shared_ptr<const VarietyOperatorNode>,
                  ActionScopeLevelType>;
    // �洢���ָ�������
    using PointerStackType = std::stack<SinglePointerType>;

    VarietyData() = default;
    template <class Data>
    VarietyData(Data&& data) : variety_data_(std::forward<Data>(data)) {}

    // ���һ�����ݣ�����ת����ջ���Զ�ִ��
    // �����ȴ����սڵ����øú�����������������
    // ����ֵ����������Ͷ��崦
    DefineVarietyResult AddVarietyData(
        const std::shared_ptr<const VarietyOperatorNode>& operator_node,
        ActionScopeLevelType action_scope_level_);
    // ����������ݣ��Զ�����ת��
    // ������������һ�������򷵻�true��Ӧ�Ƴ��ýڵ�
    bool PopTopData();
    // ��ȡ������ָ��
    // ���ر���ָ��͸ô洢�ڵ��Ƿ���б���ָ��
    std::pair<SinglePointerType, bool> GetTopData();
    // �ж������Ƿ�δ�洢�κ�ָ��
    bool Empty() {
      return std::get_if<std::monostate>(&variety_data_) != nullptr;
    }

   private:
    auto& GetVarietyData() { return variety_data_; }
    // �洢ָ��ڵ��ָ�룬std::monostateΪĬ�Ϲ���ʱ��ӵ�еĶ���
    std::variant<std::monostate, SinglePointerType,
                 std::unique_ptr<PointerStackType>>
        variety_data_;
    // �ϴε����ñ�����Ϣʱ���ڵĲ���
  };

 public:
  // �洢��ͬ����������Ľṹ
  using ActionScopeContainerType = std::unordered_map<std::string, VarietyData>;

  ActionScopeSystem() { VarietyScopeSystemInit(); }

  // ��ʼ��
  void VarietyScopeSystemInit() {
    variety_name_to_operator_node_pointer_.clear();
    while (!variety_stack_.empty()) {
      variety_stack_.pop();
    }
    // ѹ���ڱ����������и��ڸ������������ŵı���ʱ�����ж�ջ�Ƿ�Ϊ��
    auto [iter, inserted] = variety_name_to_operator_node_pointer_.emplace(
        std::string(),
        VarietyData::SinglePointerType(nullptr, ActionScopeLevelType(0)));
    assert(inserted == true);
    variety_stack_.emplace(std::move(iter));
  }
  // ����һ��������level����ñ����⻨���ŵĲ�����0����ȫ�ֱ���
  // ����ָ�����λ�õĵ����������Ľ����������������
  template <class VarietyName>
  std::pair<ActionScopeContainerType::const_iterator, DefineVarietyResult>
  DefineVariety(
      VarietyName&& variety_name,
      const std::shared_ptr<const VarietyOperatorNode>& operator_node_pointer);
  // ����һ����������֤���صĵ�������Ч�ҹ���ڵ���ڣ���������ӱ���ָ��
  // ��ӱ���ָ����ʹ��DefineVariety()
  // ����ָ�����λ�õĵ�����
  template <class VarietyName>
  ActionScopeContainerType::const_iterator AnnounceVarietyName(
      VarietyName&& variety_name);
  // ����ָ������ڵ������ָ��ͱ����Ƿ����
  std::pair<std::shared_ptr<const VarietyOperatorNode>, bool> GetVariety(
      const std::string& variety_name);
  // ���õ�ǰ����������
  // �Զ����������򣬴����������ͱ�����
  // �����Ƿ���ӳɹ�
  bool SetFunctionToConstruct(
      const std::shared_ptr<c_parser_frontend::type_system::FunctionType>&
          function_type);
  ActionScopeLevelType GetActionScopeLevel() const {
    return action_scope_level_;
  }
  // �������е����̿��ƽڵ�ѹջ���Զ�����һ��������ȼ�
  // ������̿������ջΪ�գ���ǰ�޻�Ծ�������򷵻�false
  bool PushFlowControlSentence(
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
          flow_control_sentence);
  // ����һ��������ȼ�
  void AddActionScopeLevel() { ++action_scope_level_; }
  // ����һ�������򣬻ᵯ���������������еı���
  void PopActionScope() {
    assert(action_scope_level_ != 0);
    --action_scope_level_;
    PopVarietyOverLevel(GetActionScopeLevel());
  }
  // �Ƴ��սڵ㣬����ڵ㲻�����Ƴ�
  // �����Ƿ��Ƴ��˽ڵ�
  // ����ֻannounce�˱����������û����ӱ��������������ָ���������̣�
  bool RemoveEmptyNode(const std::string& empty_node_to_remove_name);
  // ��ȡ������̿������
  auto& GetTopFlowControlSentence() const {
    return *flow_control_stack_.top().first;
  }
  // ��if���̿������ת��Ϊif-else���
  // ������������䲻Ϊif�򷵻�false
  // ��if���̿������ת��Ϊif-else���
  // ������������䲻Ϊif�򷵻�false
  void ConvertIfSentenceToIfElseSentence() {
    assert(flow_control_stack_.top().first->GetFlowType() ==
           c_parser_frontend::flow_control::FlowType::kIfSentence);
    static_cast<c_parser_frontend::flow_control::IfSentence&>(
        *flow_control_stack_.top().first)
        .ConvertToIfElse();
  }
  // ��switch����������ͨcase
  // �����Ƿ���ӳɹ�
  // �����ǰ���������䲻Ϊswitch�򷵻�false
  bool AddSwitchSimpleCase(
      const std::shared_ptr<
          c_parser_frontend::flow_control::BasicTypeInitializeOperatorNode>&
          case_value);
  // ���switch����е�default��ǩ
  // �����Ƿ���ӳɹ�
  // �����ǰ���������䲻Ϊswitch�򷵻�false
  bool AddSwitchDefaultCase();
  // ��ǰ��Ծ�ĺ�����ִ�е����β���������
  // �����Ƿ���ӳɹ������ʧ�����޸Ĳ���
  // �����ǰ�����̿�������򷵻�false
  bool AddSentence(
      std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>&&
          sentence) {
    if (flow_control_stack_.empty()) [[unlikely]] {
      return false;
    }
    return flow_control_stack_.top().first->AddMainSentence(
        std::move(sentence));
  }
  // ��Ӵ�ִ�����ļ���
  // �����Ƿ���ӳɹ������ʧ�����޸Ĳ���
  // �����ǰ�����̿�������򷵻�false
  bool AddSentences(
      std::list<
          std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>>&&
          sentence_container);

 private:
  // �������и��ڸ���������ȼ��ı���
  void PopVarietyOverLevel(ActionScopeLevelType level);
  ActionScopeContainerType& GetVarietyNameToOperatorNodePointer() {
    return variety_name_to_operator_node_pointer_;
  }
  auto& GetVarietyStack() { return variety_stack_; }
  // �����������ͱ�������ӵ��������У������Ƿ���ӳɹ�
  // �������ͱ������ں���ָ�븳ֵ
  bool CreateFunctionTypeVarietyAndPush(
      const std::shared_ptr<c_parser_frontend::type_system::FunctionType>&
          function_type);

  // �洢�������Ͷ�Ӧͬ����ͬ���������
  // ��ֵΪ������
  // �ڷ�������ʱ��������ջת���ṹ���Խ����ڴ�ռ�ú�stack����������
  ActionScopeContainerType variety_name_to_operator_node_pointer_;
  // ��˳��洢��ӵı���������֧��������������������������ȫ����������
  // iteratorָ������Ͷ�Ӧ����
  std::stack<ActionScopeContainerType::iterator> variety_stack_;
  // �洢��ǰ������ȼ���ȫ�ֱ���Ϊ0����ÿ��{}����һ��
  ActionScopeLevelType action_scope_level_;
  // �洢��ǰ�������ڹ���������������������������ȼ�����ײ�Ϊ��ǰ���ں���
  std::stack<
      std::pair<std::unique_ptr<c_parser_frontend::flow_control::FlowInterface>,
                ActionScopeLevelType>>
      flow_control_stack_;
};
template <class VarietyName>
inline std::pair<ActionScopeSystem::ActionScopeContainerType::const_iterator,
                 DefineVarietyResult>
ActionScopeSystem::DefineVariety(
    VarietyName&& variety_name,
    const std::shared_ptr<const VarietyOperatorNode>& operator_node_pointer) {
  auto [iter, inserted] = GetVarietyNameToOperatorNodePointer().emplace(
      std::forward<VarietyName>(variety_name), VarietyData());
  DefineVarietyResult add_variety_result =
      iter->second.AddVarietyData(operator_node_pointer, GetActionScopeLevel());
  switch (add_variety_result) {
    case DefineVarietyResult::kNew:
    case DefineVarietyResult::kAddToStack:
    case DefineVarietyResult::kShiftToStack:
      // ȫ�ֱ������ᱻ������������ջ
      if (GetActionScopeLevel() != 0) [[likely]] {
        // ��Ӹýڵ����Ϣ���Ա�������ʧЧʱ��ȷ����
        // ���Ա����û��ṩ��Ҫ���������У��򻯲���
        // ͬʱ��������������������ڵ�ӳ���Ҳ����ÿ��ָ�붼�洢��Ӧ��level
        GetVarietyStack().emplace(std::move(iter));
      }
      break;
    case DefineVarietyResult::kReDefine:
      break;
    default:
      assert(false);
      break;
  }
  return std::make_pair(std::move(iter), add_variety_result);
}
template <class VarietyName>
inline ActionScopeSystem::ActionScopeContainerType::const_iterator
ActionScopeSystem::AnnounceVarietyName(VarietyName&& variety_name) {
  return GetVarietyNameToOperatorNodePointer()
      .emplace(std::forward<VarietyName>(variety_name), VarietyData())
      .first;
}
}  // namespace c_parser_frontend::action_scope_system

#endif  // !CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_