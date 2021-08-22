#ifndef CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_
#define CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_
#include <memory>
#include <stack>
#include <unordered_map>
#include <variant>

#include "Common/id_wrapper.h"
#include "operator_node.h"
// ������ϵͳ
namespace c_parser_frontend::action_scope_system {
class VarietyScopeSystem {
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
        std::pair<std::shared_ptr<VarietyOperatorNode>, ActionScopeLevelType>;
    // �洢���ָ�������
    using PointerStackType = std::stack<SinglePointerType>;
    enum class AddVarietyResult {
      // ������ӵ����
      kNew,           // ����ָ�룬�ýڵ㲻���洢ָ��
      kShiftToStack,  // �ýڵ���ǰ�洢��һ��ָ�룬���ڴ洢�ڶ�����ת��Ϊָ��ջ
      kAddToStack,    // ��ָ��ջ�����һ��ָ��
      // ��������ӵ����
      kReDefine  // �ض���
    };
    VarietyData() = default;
    template <class Data>
    VarietyData(Data&& data) : variety_data_(std::forward<Data>(data)) {}

    // ���һ�����ݣ�����ת����ջ���Զ�ִ��
    // �����ȴ����սڵ����øú�����������������
    // ����ֵ����������Ͷ��崦
    AddVarietyResult AddVarietyData(
        std::shared_ptr<VarietyOperatorNode>&& operator_node,
        ActionScopeLevelType action_scope_level);
    // ����������ݣ��Զ�����ת��
    // ������������һ�������򷵻�true��Ӧ�Ƴ��ýڵ�
    bool PopTopData();
    // ��ȡ��������
    VarietyScopeSystem::VarietyData::SinglePointerType& GetTopData();

   private:
    auto& GetVarietyData() { return variety_data_; }
    // �洢ָ��ڵ��ָ�룬std::monostateΪĬ�Ϲ���ʱ��ӵ�еĶ���
    std::variant<std::monostate, SinglePointerType,
                 std::unique_ptr<PointerStackType>>
        variety_data_;
    // �ϴε����ñ�����Ϣʱ���ڵĲ���
  };

 public:
  using AddVarietyResult = VarietyData::AddVarietyResult;
  // �洢��ͬ����������Ľṹ
  using ActionScopeType = std::unordered_map<std::string, VarietyData>;

  VarietyScopeSystem() { VarietyScopeSystemInit(); }

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
  // ���һ��������level����ñ����⻨���ŵĲ�����0����ȫ�ֱ���
  // ����true����ɹ���ӣ�false�����ض���
  template <class VarietyName>
  AddVarietyResult AddVariety(
      VarietyName&& variety_name,
      std::shared_ptr<VarietyOperatorNode>&& operator_node_pointer);
  // ����ָ������ڵ������ָ��ͱ����Ƿ����
  std::pair<std::shared_ptr<VarietyOperatorNode>, bool> GetVariety(
      const std::string& variety_name);

  ActionScopeLevelType GetActionScopeLevel() const {
    return action_scope_level;
  }
  // ����һ��������ȼ�
  void AddActionScopeLevel() { ++action_scope_level; }
  // ����һ�������򣬻ᵯ���������������еı���
  void PopActionScope() {
    assert(action_scope_level != 0);
    --action_scope_level;
    PopVarietyOverLevel(GetActionScopeLevel());
  }

 private:
  // �������и��ڸ���������ȼ��ı���
  void PopVarietyOverLevel(ActionScopeLevelType level);
  ActionScopeType& GetVarietyNameToOperatorNodePointer() {
    return variety_name_to_operator_node_pointer_;
  }
  auto& GetVarietyStack() { return variety_stack_; }

  // �洢�������Ͷ�Ӧͬ����ͬ���������
  // ��ֵΪ��������ֵʹ��variant��Ϊ���������������
  // �ڷ�������ʱ��������ջת���ṹ���Խ����ڴ�ռ�ú�stack����������
  ActionScopeType variety_name_to_operator_node_pointer_;
  // ��˳��洢��ӵı���������֧��������������������������ȫ����������
  // iteratorָ������Ͷ�Ӧ����
  std::stack<ActionScopeType::iterator> variety_stack_;
  // �洢��ǰ������ȼ���ȫ�ֱ���Ϊ0����ÿ��{}����һ��
  ActionScopeLevelType action_scope_level;
};
template <class VarietyName>
inline VarietyScopeSystem::AddVarietyResult VarietyScopeSystem::AddVariety(
    VarietyName&& variety_name,
    std::shared_ptr<VarietyOperatorNode>&& operator_node_pointer) {
  auto [iter, inserted] = GetVarietyNameToOperatorNodePointer().emplace(
      std::forward<VarietyName>(variety_name), VarietyData());
  AddVarietyResult add_variety_result = iter->second.AddVarietyData(
      std::move(operator_node_pointer), GetActionScopeLevel());
  switch (add_variety_result) {
    case VarietyData::AddVarietyResult::kNew:
    case VarietyData::AddVarietyResult::kAddToStack:
    case VarietyData::AddVarietyResult::kShiftToStack:
      // ȫ�ֱ������ᱻ������������ջ
      if (GetActionScopeLevel() != 0) [[likely]] {
        // ��Ӹýڵ����Ϣ���Ա�������ʧЧʱ��ȷ����
        // ���Ա����û��ṩ��Ҫ���������У��򻯲���
        // ͬʱ��������������������ڵ�ӳ���Ҳ����ÿ��ָ�붼�洢��Ӧ��level
        GetVarietyStack().emplace(std::move(iter));
      }
      break;
    case VarietyData::AddVarietyResult::kReDefine:
      break;
    default:
      assert(false);
      break;
  }
  return add_variety_result;
}
}  // namespace c_parser_frontend::action_scope_system

#endif  // !CPARSESRFRONTEND_ACTION_SCOPE_SYSTEM_H_