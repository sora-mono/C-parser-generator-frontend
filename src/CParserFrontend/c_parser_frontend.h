#ifndef CPARSERFRONTEND_C_PARSER_FRONTEND_H_
#define CPARSERFRONTEND_C_PARSER_FRONTEND_H_

#include <unordered_map>

#include "Generator/SyntaxGenerator/process_function_interface.h"
#include "action_scope_system.h"
#include "flow_control.h"
#include "operator_node.h"
#include "type_system.h"

namespace c_parser_frontend {
// ����һЩ���͵Ķ���
using c_parser_frontend::action_scope_system::VarietyScopeSystem;
using AddVarietyResult = c_parser_frontend::action_scope_system::
    VarietyScopeSystem::AddVarietyResult;
using c_parser_frontend::flow_control::FunctionDefine;
using c_parser_frontend::operator_node::VarietyOperatorNode;
using c_parser_frontend::type_system::StructOrBasicType;
using c_parser_frontend::type_system::TypeInterface;
using c_parser_frontend::type_system::TypeSystem;

class CParserFrontend {
  using AddTypeResult = TypeSystem::AddTypeResult;
  using GetTypeResult = TypeSystem::GetTypeResult;

 public:
  enum class AddFunctionDefineResult {
    // ������ӵ����
    kSuccess,  // �ɹ����
    // ��������ӵ����
    kReDefine,  // �ض��庯��
    kOverLoad   // ���غ���
  };
  // �������
  // ������ӽ��
  AddTypeResult AddType(std::string&& type_name,
                        std::shared_ptr<const TypeInterface>&& type_pointer) {
    return type_system_.AddType(std::move(type_name), std::move(type_pointer));
  }
  std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
      const std::string& type_name, StructOrBasicType type_prefer) {
    return type_system_.GetType(type_name, type_prefer);
  }
  // ����һ��������ȼ�
  void AddActionScopeLevel() { variety_system.AddActionScopeLevel(); }
  // ����һ�������򣬻ᵯ���������������еı���
  void PopActionScope() { variety_system.PopActionScope(); }
  size_t GetActionScopeLevel() const {
    return variety_system.GetActionScopeLevel();
  }
  // ��ӱ���
  // ������ӽ��
  AddVarietyResult AddVariety(
      std::string&& variety_name,
      std::shared_ptr<VarietyOperatorNode>&& operator_node) {
    return variety_system.AddVariety(std::move(variety_name),
                                     std::move(operator_node));
  }
  std::pair<std::shared_ptr<VarietyOperatorNode>, bool> GetVariety(
      const std::string& variety_name) {
    return variety_system.GetVariety(variety_name);
  }
  // ��Ӻ���������������������ͬ����
  // �����Ƿ���ӳɹ�
  // C���Բ�֧�ֺ������أ��������������ʧ��
  // ������ʧ���򷵻غ������ݵĿ���Ȩ
  template <class FunctionName>
  std::pair<std::unique_ptr<FunctionDefine>, bool> AddFunctionAnnounce(
      FunctionName&& function_name,
      std::unique_ptr<FunctionDefine>&& function_announce_data);
  // ��Ӻ������壬ֻ�����һ�ζ���
  // ������ӽ��
  // ������ʧ���򷵻غ������ݵĿ���Ȩ
  template <class FunctionName>
  std::pair<std::unique_ptr<FunctionDefine>, AddFunctionDefineResult>
  AddFunctionDefine(FunctionName&& function_name,
                    std::unique_ptr<FunctionDefine>&& function_define_data);
  // ��ȡ������Ϣ
  // ��������ڸ������Ƶĺ����򷵻ؿ�ָ��
  FunctionDefine* GetFunction(const std::string& function_name);

 private:
  // ����ϵͳ
  TypeSystem type_system_;
  // ����������ϵͳ
  VarietyScopeSystem variety_system;
  // �������壬��ֵΪ������
  std::unordered_map<std::string, std::unique_ptr<FunctionDefine>> functions_;
};

// ��Ӻ���������������������ͬ����
// �����Ƿ���ӳɹ�
// C���Բ�֧�ֺ������أ��������������ʧ��
// ������ʧ���򷵻غ������ݵĿ���Ȩ

template <class FunctionName>
inline std::pair<std::unique_ptr<FunctionDefine>, bool>
CParserFrontend::AddFunctionAnnounce(
    FunctionName&& function_name,
    std::unique_ptr<FunctionDefine>&& function_announce_data) {
  auto [iter, inserted] =
      functions_.emplace(std::forward<FunctionName>(function_name),
                         std::move(function_announce_data));
  if (!inserted) [[unlikely]] {
    // �Ѵ���ͬ�������������Ƿ�Ϊͬһ����
    if (function_announce_data->GetFunctionTypeReference().GetArgumentTypes() !=
        iter->second->GetFunctionTypeReference().GetArgumentTypes())
        [[unlikely]] {
      // ͬ������ͬ����
      // C���Բ�֧�ֺ������أ��������
      return std::make_pair(std::move(function_announce_data), false);
    }
  }
  return std::make_pair(std::unique_ptr<FunctionDefine>(), true);
}

// ��Ӻ������壬ֻ�����һ�ζ���
// ������ӽ��
// ������ʧ���򷵻غ������ݵĿ���Ȩ

template <class FunctionName>
inline std::pair<std::unique_ptr<FunctionDefine>,
                 CParserFrontend::AddFunctionDefineResult>
CParserFrontend::AddFunctionDefine(
    FunctionName&& function_name,
    std::unique_ptr<FunctionDefine>&& function_define_data) {
  auto [iter, inserted] =
      functions_.emplace(std::forward<FunctionName>(function_name),
                         std::move(function_define_data));
  if (!inserted) {
    // �Ѿ����ں������������������
    // �жϴ���ӵĺ��������еĺ����ǲ���ͬһ������
    // ��֪��������ͬ��ֻ���жϲ�������
    if (function_define_data->GetFunctionTypeReference().GetArgumentTypes() !=
        iter->second->GetFunctionTypeReference().GetArgumentTypes())
        [[unlikely]] {
      // ͬ������ͬ����
      // C���Բ�֧�ֺ������أ��������
      return std::make_pair(std::move(function_define_data),
                            AddFunctionDefineResult::kOverLoad);
    }
    // �ж����в������������Ƕ���
    if (iter->second->GetSentences().size() != 0) [[unlikely]] {
      // �ض���
      return std::make_pair(std::move(function_define_data),
                            AddFunctionDefineResult::kReDefine);
    }
  }
  return std::make_pair(std::unique_ptr<FunctionDefine>(),
                        AddFunctionDefineResult::kSuccess);
}

}  // namespace c_parser_frontend

#endif  // !CPARSERFRONTEND_C_PARSER_FRONTEND_H_
