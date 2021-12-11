/// @file flow_control.h
/// @brief ���̿���
/// @details
/// ���ļ��ж������̿��ƽڵ㣬����ڵ����Ͳο�FlowType
#ifndef CPARSERFRONTEND_FLOW_CONTROL_H_
#define CPARSERFRONTEND_FLOW_CONTROL_H_

#include <format>
#include <list>
#include <memory>
#include <unordered_map>

#include "operator_node.h"
#include "type_system.h"

namespace c_parser_frontend::flow_control {
/// @brief ���̿��ƽڵ�����
enum class FlowType {
  kFunctionDefine,   ///< ��������
  kSimpleSentence,   ///< ���������̿������
  kLabel,            ///< ��ӱ�ǩ���
  kJmp,              ///< ��ת���
  kReturn,           ///< �������
  kIfSentence,       ///< if���
  kIfElseSentence,   ///< if-else���
  kWhileSentence,    ///< while���
  kDoWhileSentence,  ///< do-while���
  kForSentence,      ///< for���
  kSwitchSentence    ///< switch���
};

// ���ò����ڵ�͸����������͵Ķ���
using c_parser_frontend::operator_node::AllocateOperatorNode;
using c_parser_frontend::operator_node::AssignOperatorNode;
using c_parser_frontend::operator_node::BasicTypeInitializeOperatorNode;
using c_parser_frontend::operator_node::DereferenceOperatorNode;
using c_parser_frontend::operator_node::FunctionCallOperatorNode;
using c_parser_frontend::operator_node::GeneralOperationType;
using c_parser_frontend::operator_node::InitializeOperatorNodeInterface;
using c_parser_frontend::operator_node::InitializeType;
using c_parser_frontend::operator_node::LogicalOperation;
using c_parser_frontend::operator_node::LogicalOperationOperatorNode;
using c_parser_frontend::operator_node::MathematicalOperatorNode;
using c_parser_frontend::operator_node::MemberAccessOperatorNode;
using c_parser_frontend::operator_node::OperatorNodeInterface;
using c_parser_frontend::operator_node::VarietyOperatorNode;
using c_parser_frontend::type_system::AssignableCheckResult;
using c_parser_frontend::type_system::BuiltInType;
using c_parser_frontend::type_system::CommonlyUsedTypeGenerator;
using c_parser_frontend::type_system::ConstTag;
using c_parser_frontend::type_system::FunctionType;
using c_parser_frontend::type_system::StructOrBasicType;
using c_parser_frontend::type_system::TypeInterface;

/// @class FlowInterface flow_control.h
/// @brief ���̿��ƽڵ����
/// @details
/// �������̿��ƽڵ���Ӹ�������
class FlowInterface {
  /// @brief ���̽ڵ�ID�ķַ���ǩ
  enum class IdWrapper { kFlowNodeId };

 public:
  /// @brief ���̽ڵ�ID
  using FlowId = frontend::common::ExplicitIdWrapper<size_t, IdWrapper,
                                                     IdWrapper::kFlowNodeId>;

  FlowInterface(FlowType flow_type) : flow_type_(flow_type){};
  virtual ~FlowInterface() {}

  /// @brief �������������������ӵ������
  /// @param[in] sentence ������ӵ����
  /// @return ��������Ƿ�ɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ��sentence������ӵ����̿��������
  /// @details
  /// 1.��do-while/while/for���øú���ʱ��ӵ�ѭ������
  /// 2.��if������ʱ��ӵ�if����ʱִ�е������
  /// 3.��if-else������ʱ��ӵ�else����ʱִ�е������
  /// 4.��switch������ʱ��ӵ�switch������
  /// @note ������ʱ��ӵ����е�����
  virtual bool AddMainSentence(std::unique_ptr<FlowInterface>&& sentence) = 0;
  /// @brief �������������������Ӷ������
  /// @param[in] sentences ������ӵ����
  /// @return ��������Ƿ�ɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ��sentences�к��в�����ӵ����̿����������
  /// @details
  /// 1.��do-while/while/for���øú���ʱ��ӵ�ѭ������
  /// 2.��if������ʱ��ӵ�if����ʱִ�е������
  /// 3.��if-else������ʱ��ӵ�else����ʱִ�е������
  /// 4.��switch������ʱ��ӵ�switch������
  /// @note ������ʱ��ӵ����е�����
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) = 0;

  /// @brief ��ȡ���̿�������
  /// @return �������̿�������
  FlowType GetFlowType() const { return flow_type_; }
  /// @brief ��ȡ���̿��ƽڵ�ID
  /// @return �������̿��ƽڵ�ID
  /// @note �������̿��ƽڵ㶼��һ���̶߳�����ID
  FlowId GetFlowId() const { return flow_id_; }

 protected:
  /// @brief �������̿�������
  void SetFlowType(FlowType flow_type) { flow_type_ = flow_type; }

 private:
  /// @brief ��ȡһ����δ������������̿��ƽڵ�ID
  /// @return �������̿��ƽڵ�ID
  /// @note ���̿��ƽڵ��̶߳���
  static FlowId GetNewFlowId() {
    static thread_local FlowId flow_id(0);
    return flow_id++;
  }

  /// @brief ���̿��ƽڵ�ID
  const FlowId flow_id_ = GetNewFlowId();
  /// @brief ���̿��ƽڵ�����
  FlowType flow_type_;
};

/// @class Label flow_control.h
/// @brief ��ǩ
/// @details
/// 1.��ǩ������ת��䣬ÿ����ת��䶼Ӧ��һ����ǩ
/// 2.ʹ�÷��������ڻ���ǩ/goto�����ת
/// 3.ÿ����ǩֻ�б�ǩ����һ��Ա
/// @note ��ͬ��ǩ���ı�ǩ����һ����ǩ
class Label : public FlowInterface {
 public:
  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    /// ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    /// ��ֹ����
    return false;
  }

  template <class LabelName>
  Label(LabelName&& label_name)
      : FlowInterface(FlowType::kLabel),
        label_name_(std::forward<LabelName>(label_name)) {}
  Label(const Label&) = delete;
  Label(Label&&) = delete;

  Label& operator=(const Label&) = default;
  Label& operator=(Label&&) = default;

  /// @brief ���ñ�ǩ��
  /// @param[in] label_name ����ǩ��
  template <class LabelName>
  void SetLabelName(LabelName&& label_name) {
    label_name_ = std::forward<LabelName>(label_name);
  }
  /// @brief ��ȡ��ǩ��
  /// @return ���ر�ǩ����const����
  const std::string& GetLabelName() const { return label_name_; }

 private:
  /// @brief ��ǩ��
  std::string label_name_;
};

/// @class Jmp flow_control.h
/// @brief ��ת
/// @details
/// ÿ����ת��䶼����ת���ı�ǩ
class Jmp : public FlowInterface {
 public:
  Jmp(std::unique_ptr<Label>&& target_label)
      : FlowInterface(FlowType::kJmp), target_label_(std::move(target_label)) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    /// ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    /// ��ֹ����
    return false;
  }

  /// @brief ��ȡ��ת���󶨵ı�ǩ
  /// @return ������ת���󶨵ı�ǩ��const����
  const Label& GetTargetLabel() const { return *target_label_; }

 private:
  /// ��תָ��Ҫ�����ı�ǩ
  std::unique_ptr<Label> target_label_;
};

/// @class FunctionDefine flow_control.h
/// @brief ��������
/// @attention �����岻��ִ�����ʱӦ����һ��return���ռλ��������Ϊ��������
class FunctionDefine : public FlowInterface {
 public:
  FunctionDefine(const std::shared_ptr<const FunctionType>& function_type)
      : FlowInterface(FlowType::kFunctionDefine),
        function_type_(function_type) {}

  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }
  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }

  /// @brief ���һ��������ִ�е����
  /// @param[in] sentence_to_add ������ӵ����
  /// @return ��������Ƿ�ɹ�
  /// @retval true ��ӳɹ�����ȡsentences_to_add����Ȩ
  /// @retval false ������ӣ����޸����
  /// @details
  /// 1.������Ҫ��ο�CheckSentenceInFunctionValid
  /// 2.��ӵ�����ӵ����β��
  bool AddSentence(std::unique_ptr<FlowInterface>&& sentence_to_add);
  /// @brief ���һ�������ڵ�ȫ�����
  /// @param[in] sentence_container ���洢�������������
  /// @return ��������Ƿ�ɹ�
  /// @retval true �ɹ���ӣ��ƶ�sentence_container�����нڵ㵽����ִ���������
  /// @retval false �����д��ڷǷ���䣬������ӣ����޸�sentence_container
  /// @details
  /// 1.���ո���������begin->end˳�����
  /// 2.������Ҫ��ο�CheckSentenceInFunctionValid
  bool AddSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentence_container);
  /// @brief ��ȡ������ִ�е����
  /// @return ���ش洢����������const����
  const std::list<std::unique_ptr<FlowInterface>>& GetSentences() const {
    return sentences_in_function_;
  }
  /// @brief ����Ƿ�Ϊ��������
  /// @return �����Ƿ�Ϊ��������
  /// @details
  /// ���������в������κ����̿������
  /// @attention �պ�������Ӧ����һ��return���
  bool IsFunctionAnnounce() const { return GetSentences().empty(); }
  /// @brief ���ú�������
  /// @param[in] function_type ����������
  void SetFunctionType(
      const std::shared_ptr<const FunctionType>& function_type) {
    function_type_ = function_type;
  }
  /// @brief ��ȡ��������
  /// @return ����ָ�������͵�constָ��
  std::shared_ptr<const FunctionType> GetFunctionTypePointer() const {
    return function_type_;
  }
  /// @brief ��ȡ��������
  /// @return ���غ������͵�const����
  const FunctionType& GetFunctionTypeReference() const {
    return *function_type_;
  }

  /// @brief ����������Ƿ������Ϊ������ִ�е����
  /// @param[in] flow_interface �����������
  /// @return ���ظ�������Ƿ������Ϊ������ִ�е����
  /// @retval true �����ں�����ִ��
  /// @retval false �������ں�����ִ��
  /// @details
  /// ��鷵�����ķ���ֵ�Ƿ��뺯������ķ���ֵƥ��
  /// ������Ƕ����������
  bool CheckSentenceInFunctionValid(const FlowInterface& flow_interface) const;

 private:
  /// @brief ����������
  std::shared_ptr<const FunctionType> function_type_;
  /// @brief ��������ִ�е����
  std::list<std::unique_ptr<FlowInterface>> sentences_in_function_;
};

/// @class Return flow_control.h
/// @brief �Ӻ����з���
/// @note ����鷵�ص�ֵ����鲽������ӵ�������ʱִ��
class Return : public FlowInterface {
 public:
  Return(const std::shared_ptr<const OperatorNodeInterface>& return_target =
             nullptr)
      : FlowInterface(FlowType::kReturn), return_target_(return_target) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    /// ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    /// ��ֹ����
    return false;
  }

  /// @brief ���÷���ֵ
  /// @param[in] return_target ������ֵ����������ʹ��nullptr
  void SetReturnValue(const std::shared_ptr<const OperatorNodeInterface>&
                          return_target = nullptr) {
    return_target_ = return_target;
  }
  /// @brief ��ȡ���ص�ֵ
  /// @return ����ָ�򷵻ص�ֵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetReturnValuePointer() const {
    return return_target_;
  }
  /// @brief ��ȡ���ص�ֵ
  /// @return ����ָ�򷵻ص�ֵ��constָ��
  const OperatorNodeInterface& GetReturnValueReference() const {
    return *return_target_;
  }

 private:
  /// @brief ����ֵ��nullptr�����޷���ֵ
  std::shared_ptr<const OperatorNodeInterface> return_target_;
};

/// @class ConditionBlockInterface flow_control.h
/// @brief �������ƿ���ࣨif��for��while��switch�ȣ�
/// @details
/// 1.���ƿ���������
///   if(condition) { // ���� } else { // ���� }
///   for(int i = 0;i < x;i++){ // ���� }
/// 2.���ƿ�����ָ�жϿ��ƿ��Ƿ�ִ��/��ת�������䣬��1��if����condition��
///   for�е�i < x;
/// @attention �����಻Ӧ�޸Ľӿ������Ҳ������κη�ʽ��д���ƹ��ӿ�
class ConditionBlockInterface : public FlowInterface {
 public:
  ConditionBlockInterface(FlowType flow_type) : FlowInterface(flow_type) {}

  /// @brief �趨�������������������ȡ�����Ĳ���
  /// @param[in] condition �������ڵ�
  /// @param[in] sentences_to_get_condition ����ȡ�����Ĳ���
  /// @return �����Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false ��condition������Ϊ����������������
  /// @details
  /// 1.�޷������򲻻�����
  /// 2.�ɹ����ú��ȡcondition�Ŀ���Ȩ
  /// 3.�����������޸�condition
  /// 4.�������ڵ��Ҫ��ο�DefaultConditionCheck
  bool SetCondition(
      const std::shared_ptr<const OperatorNodeInterface>& condition,
      std::list<std::unique_ptr<FlowInterface>>&& sentences_to_get_condition);
  /// @brief ��ȡ�õ������ڵ�Ĳ���
  /// @return ���ش洢������������const����
  const std::list<std::unique_ptr<FlowInterface>>& GetSentenceToGetCondition()
      const {
    return sentence_to_get_condition_;
  }
  /// @brief ��ȡ�����ڵ�
  /// @return ����ָ�������ڵ��constָ��
  std::shared_ptr<const OperatorNodeInterface> GetConditionPointer() const {
    return condition_;
  }
  /// @brief ��ȡ�����ڵ�
  /// @return ���������ڵ��const����
  const OperatorNodeInterface& GetConditionReference() const {
    return *condition_;
  }
  /// @brief �����������һ�����
  /// @param[in] sentence ������ӵ����
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ������䲻����ӵ������������������
  /// @details
  /// 1.�޷�������޸�sentence
  /// 2.�ɹ�������ȡsentence����Ȩ
  /// 3.���������ӵ����е�����
  /// 4.������Ҫ��ο�DefaultMainBlockSentenceCheck
  bool AddSentence(std::unique_ptr<FlowInterface>&& sentence);
  /// @brief ����������Ӷ������
  /// @param[in] sentences ���洢����ӵ���������
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false ������䲻����ӵ������������������
  /// @details
  /// 1.�޷�������޸�sentence
  /// 2.�ɹ�������ȡsentence����Ȩ
  /// 3.���������ӵ����е�����
  /// 4.��begin->end��˳����������ڵ����
  /// 5.������Ҫ��ο�DefaultMainBlockSentenceCheck
  bool AddSentences(std::list<std::unique_ptr<FlowInterface>>&& sentences);
  /// @brief ��ȡ��������ӵ����
  /// @return ���ش洢����������const����
  const auto& GetSentences() const { return main_block_; }
  /// @brief ��ȡ����ʼ����ǩ
  /// @return ���ؿ���ʼ����ǩ
  std::unique_ptr<Label> GetSentenceStartLabel() const {
    return std::make_unique<Label>(
        std::format("condition_block_{:}_start", GetFlowId().GetRawValue()));
  }
  /// @brief ��ȡ���������ǩ
  /// @return ���ؿ��������ǩ
  std::unique_ptr<Label> GetSentenceEndLabel() const {
    return std::make_unique<Label>(
        std::format("condition_block_{:}_end", GetFlowId().GetRawValue()));
  }

  /// @brief Ĭ�϶�ѭ�������ڵ�������ͼ��ĺ���
  /// @param[in] condition_node �������������ڵ�
  /// @return ���ظ����ڵ��Ƿ������Ϊ����������������
  /// @retval true �����ڵ�������Ϊ����������������
  /// @retval false �����ڵ㲻����Ϊ����������������
  static bool DefaultConditionCheck(
      const OperatorNodeInterface& condition_node);
  /// @brief Ĭ�ϼ�����������ĺ���
  /// @param[in] flow_interface �����������
  /// @return ���ظ������̿��ƽڵ��Ƿ������Ϊ��������������������
  /// @retval true �����ڵ�������Ϊ��������������������
  /// @retval false �����ڵ㲻����Ϊ��������������������
  static bool DefaultMainBlockSentenceCheck(
      const FlowInterface& flow_interface);

 private:
  /// @brief ���ƿ������
  std::shared_ptr<const OperatorNodeInterface> condition_;
  /// @brief ��ȡ���ƿ����������
  std::list<std::unique_ptr<FlowInterface>> sentence_to_get_condition_;
  /// @brief ���ƿ����������
  std::list<std::unique_ptr<FlowInterface>> main_block_;
};

/// @class SimpleSentence flow_control.h
/// @brief �������
/// @details
/// ���м̳���OperatorNodeInterface�����ڸ���
class SimpleSentence : public FlowInterface {
 public:
  SimpleSentence() : FlowInterface(FlowType::kSimpleSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    assert(false);
    /// ��ֹ����
    return false;
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    assert(false);
    /// ��ֹ����
    return false;
  }

  /// @brief ���ø���䱣������
  /// @param[in] sentence_operate_node ��������Ĳ���
  /// @return �����Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false ����֧�ִ�����Ĳ������޷�����
  /// @note �Ա���Ĳ�����Ҫ��ο�CheckOperatorNodeValid
  bool SetSentenceOperateNode(
      const std::shared_ptr<OperatorNodeInterface>& sentence_operate_node);
  /// @brief ��ȡ����Ĳ���
  /// @return ����ָ�򱣴�Ĳ����ڵ��constָ��
  std::shared_ptr<OperatorNodeInterface> GetSentenceOperateNodePointer() const {
    return sentence_operate_node_;
  }
  /// @brief ��ȡ����Ĳ���
  /// @return ���ر���Ĳ����ڵ��const����
  OperatorNodeInterface& GetSentenceOperateNodeReference() const {
    return *sentence_operate_node_;
  }

  /// @brief �������ڵ��Ƿ���Ա���
  /// @param[in] operator_node ������Ĳ���
  /// @return �����Ƿ���Ա���
  /// @retval true �����Ա���
  /// @retval false �������Ա���
  static bool CheckOperatorNodeValid(
      const OperatorNodeInterface& operator_node);

 private:
  /// @brief �����ڵ�
  std::shared_ptr<OperatorNodeInterface> sentence_operate_node_;
};

/// @class IfSentence flow_control.h
/// @brief if��if-else���
class IfSentence : public ConditionBlockInterface {
 public:
  // �մ���ʱֻ��true��֧
  IfSentence() : ConditionBlockInterface(FlowType::kIfSentence) {}

  /// @details
  /// ����if��������ж�Ӧ����ӵ��ĸ�����
  /// �����FlowType::kIfSentence����ӵ�true_branch
  /// �����FlowType::kIfElseSentence����ӵ�false_branch
  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override;
  /// @details
  /// ����if��������ж�Ӧ����ӵ��ĸ�����
  /// �����FlowType::kIfSentence����ӵ�true_branch
  /// �����FlowType::kIfElseSentence����ӵ�false_branch
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override;

  /// @brief ���ýڵ�ת��Ϊif-else���
  /// @note �Զ�����else��Ĵ洢�ṹ
  void ConvertToIfElse();
  /// @brief ������֧��ִ�е�һ�����
  /// @param[in] if_body_sentence ������ӵ����
  /// @return �����Ƿ�������
  /// @retval true ��ӳɹ�����ȡif_body_sentence����Ȩ
  /// @retval false ������䲻����ӵ����֧�ڣ���������Ҳ��޸Ĳ���
  /// @note ��ӵ��Ѵ��ڵ�����
  /// ������Ҫ��ο�AddSentence��Ҫ��
  bool AddTrueBranchSentence(
      std::unique_ptr<FlowInterface>&& if_body_sentence) {
    // ��AddMainBlockSentence�м��
    return AddSentence(std::move(if_body_sentence));
  }
  /// @brief ������֧��ִ�еĶ������
  /// @param[in] if_body_sentences ���洢����ӵ���������
  /// @return �����Ƿ�������
  /// @retval true ����ӳɹ�����if_body_sentences������ƶ������֧���������
  /// @retval false ��������ĳ����䲻����ӵ����֧�ڣ���������Ҳ��޸Ĳ���
  /// @note ��begin->end˳����ӣ���ӵ��Ѵ��ڵ�����
  /// ������Ҫ��ο�AddSentence��Ҫ��
  bool AddTrueBranchSentences(
      std::list<std::unique_ptr<FlowInterface>>&& if_body_sentences) {
    return AddSentences(std::move(if_body_sentences));
  }
  /// @brief ��Ӽٷ�֧��ִ�е�һ�����
  /// @param[in] else_body_sentence ������ӵ����
  /// @return �����Ƿ�������
  /// @retval true ��ӳɹ�����ȡelse_body_sentence����Ȩ
  /// @retval false ������䲻����ӵ��ٷ�֧�ڣ���������Ҳ��޸Ĳ���
  /// @note ��ӵ��Ѵ��ڵ�����
  /// ������Ҫ��ο�CheckElseBodySentenceValid
  /// @attention ������ù�ConvertToIfElse
  bool AddFalseBranchSentence(
      std::unique_ptr<FlowInterface>&& else_body_sentence);
  /// @brief ��Ӽٷ�֧��ִ�еĶ������
  /// @param[in] else_body_sentences ���洢����ӵ���������
  /// @return �����Ƿ�������
  /// @retval true ����ӳɹ�����else_body_sentences������ƶ����ٷ�֧���������
  /// @retval false ��������ĳ����䲻����ӵ��ٷ�֧�ڣ���������Ҳ��޸Ĳ���
  /// @note ��begin->end˳����ӣ���ӵ��Ѵ��ڵ�����
  /// ������Ҫ��ο�CheckElseBodySentenceValid
  /// @attention ������ù�ConvertToIfElse
  bool AddFalseBranchSentences(
      std::list<std::unique_ptr<FlowInterface>>&& else_body_sentences);
  /// @brief ��ȡ���ִ֧�е�ȫ�����
  /// @return ����ָ��洢���ִ֧�е�����������constָ��
  const auto& GetTrueBranchSentences() const { return GetSentences(); }
  /// @brief ��ȡ�ٷ�ִ֧�е�ȫ�����
  /// @return ���ش洢�ٷ�ִ֧�е�����������const����
  const auto& GetFalseBranchSentences() const { return else_body_; }

  /// @brief ����������Ƿ������ӵ��ٷ�֧��
  /// @param[in] else_body_sentence �����������
  /// @return �����Ƿ�������
  /// @retval true ���������
  /// @retval false �����������
  static bool CheckElseBodySentenceValid(
      const FlowInterface& else_body_sentence) {
    // if�����else����Ҫ����ͬ
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        else_body_sentence);
  }

 private:
  /// @brief �ٷ�֧��ִ�е����ݣ�������else���ƿ�����Ϊnullptr
  std::unique_ptr<std::list<std::unique_ptr<FlowInterface>>> else_body_ =
      nullptr;
};

/// @class LoopSentenceInterface flow_control.h
/// @brief ѭ��������
class LoopSentenceInterface : public ConditionBlockInterface {
 protected:
  LoopSentenceInterface(FlowType flow_type)
      : ConditionBlockInterface(flow_type) {
    assert(flow_type == FlowType::kDoWhileSentence ||
           flow_type == FlowType::kWhileSentence ||
           flow_type == FlowType::kForSentence);
  }

 public:
  /// @brief ��ȡѭ����ѭ�������жϿ�ʼ����ǩ��������ʼ�ж�ѭ��������
  /// @return ����ѭ����ѭ�������жϿ�ʼ����ǩ
  virtual std::unique_ptr<Label> GetLoopConditionStartLabel() const = 0;
  /// @brief ��ȡѭ����ѭ�������жϽ�������ǩ���ո����ѭ�������жϣ�
  /// @return ����ѭ����ѭ�������жϽ�������ǩ
  virtual std::unique_ptr<Label> GetLoopConditionEndLabel() const = 0;

  /// @brief ��ȡѭ�������鿪ʼ����ǩ��������ʼִ���������ݣ�
  /// @return ����ѭ�������鿪ʼ����ǩ
  std::unique_ptr<Label> GetLoopMainBlockStartLabel() const {
    return std::make_unique<Label>(
        std::format("loop_main_block_{:}_start", GetFlowId().GetRawValue()));
  }
  /// @brief ��ȡѭ�������β����ǩ���ոս�������ִ�У�
  /// @return ����ѭ�������β����ǩ
  std::unique_ptr<Label> GetLoopMainBlockEndLabel() const {
    return std::make_unique<Label>(
        std::format("loop_main_block_{:}_end", GetFlowId().GetRawValue()));
  }
};

/// @class WhileSentenceInterface flow_control.h
/// @brief whileѭ����do-whileѭ���Ļ���
class WhileSentenceInterface : public LoopSentenceInterface {
 protected:
  WhileSentenceInterface(FlowType while_type)
      : LoopSentenceInterface(while_type) {
    assert(while_type == FlowType::kWhileSentence ||
           while_type == FlowType::kDoWhileSentence);
  }
};

/// @class WhileSentence flow_control.h
/// @brief whileѭ��
class WhileSentence : public WhileSentenceInterface {
 public:
  WhileSentence() : WhileSentenceInterface(FlowType::kWhileSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }
  virtual std::unique_ptr<Label> GetLoopConditionStartLabel() const override {
    return GetSentenceStartLabel();
  }
  virtual std::unique_ptr<Label> GetLoopConditionEndLabel() const override {
    return GetLoopMainBlockStartLabel();
  }
};

/// @class DoWhileSentence flow_control.h
/// @brief do-whileѭ��
class DoWhileSentence : public WhileSentenceInterface {
 public:
  DoWhileSentence() : WhileSentenceInterface(FlowType::kDoWhileSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }
  virtual std::unique_ptr<Label> GetLoopConditionStartLabel() const override {
    return GetLoopMainBlockEndLabel();
  }
  virtual std::unique_ptr<Label> GetLoopConditionEndLabel() const override {
    return GetLoopMainBlockEndLabel();
  }
};

/// @class ForSentence flow_control.h
/// @brief forѭ��
class ForSentence : public LoopSentenceInterface {
 public:
  ForSentence() : LoopSentenceInterface(FlowType::kForSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }
  virtual std::unique_ptr<Label> GetLoopConditionStartLabel() const override {
    return GetSentenceStartLabel();
  }
  virtual std::unique_ptr<Label> GetLoopConditionEndLabel() const override {
    return GetLoopMainBlockStartLabel();
  }

  /// @brief ���һ��for���ִ��ǰ�ĳ�ʼ�����
  /// @param[in] init_body_sentence ��forѭ����ʼ�����
  /// @return �����Ƿ�������
  /// @retval true ����ӳɹ�����ȡinit_body_sentence����Ȩ
  /// @retval false ����������ӣ���������Ҳ��޸Ĳ���
  /// @note ������Ҫ��ο�AddSentence
  /// ��ӵ����еĳ�ʼ������
  bool AddForInitSentence(std::unique_ptr<FlowInterface>&& init_body_sentence);
  /// @brief ��Ӷ���for���ִ��ǰ�ĳ�ʼ�����
  /// @param[in] init_body_sentences ���洢forѭ����ʼ����������
  /// @return �����Ƿ�������
  /// @retval true ����ӳɹ�����init_body_sentences����������ƶ�����ʼ��������
  /// @retval false ����������ӣ���������Ҳ��޸Ĳ���
  /// @note ������Ҫ��ο�AddSentence
  /// ���˳��begin->end����ӵ����еĳ�ʼ������
  bool AddForInitSentences(
      std::list<std::unique_ptr<FlowInterface>>&& init_body_sentences);
  /// @brief ���һ��for���ִ�к����ѭ�����������
  /// @param[in] after_body_sentence ��forִ�к����ѭ�����������
  /// @return �����Ƿ�������
  /// @retval true ����ӳɹ�����ȡafter_body_sentences�Ŀ���Ȩ
  /// @retval false ����������ӣ���������Ҳ��޸Ĳ���
  /// @note ������Ҫ��ο�CheckForBodySentenceValid
  /// ��ӵ����еĳ�ʼ������
  bool AddForRenewSentence(
      std::unique_ptr<FlowInterface>&& after_body_sentence);
  /// @brief ��Ӷ���for���ִ�к����ѭ�����������
  /// @param[in] after_body_sentences ���洢forִ�к����ѭ������������
  /// @return �����Ƿ�������
  /// @retval true ����ӳɹ�����after_body_sentences����������ƶ�����ʼ������
  /// @retval false ����������ӣ���������Ҳ��޸Ĳ���
  /// @note ������Ҫ��ο�CheckForBodySentenceValid
  /// ���˳��begin->end����ӵ����еĳ�ʼ������
  bool AddForRenewSentences(
      std::list<std::unique_ptr<FlowInterface>>&& after_body_sentences);
  /// @brief ��ȡfor�������������
  /// @return ���ش洢����������const����
  const auto& GetForBody() const { return GetSentences(); }
  /// @brief ��ȡfor��ʼ�����
  /// @return ���ش洢��ʼ������������const����
  const auto& GetForInitSentences() const { return init_block_; }
  /// @brief ��ȡ����ѭ�����������
  /// @return ���ش洢����ѭ������������������const����
  const auto& GetForAfterBodySentences() const { return renew_sentences_; }

  /// @brief �������ڵ��Ƿ������Ϊfor����
  /// @param[in] for_condition �������������ڵ�
  /// @return �����Ƿ�������
  /// @retval true ���������
  /// @retval false �����������
  static bool CheckForConditionValid(
      const OperatorNodeInterface& for_condition) {
    return ConditionBlockInterface::DefaultConditionCheck(for_condition);
  }
  /// @brief �������ڵ��Ƿ������Ϊfor������ִ�е����
  /// @param[in] for_body_sentence �������������ڵ�
  /// @return �����Ƿ�������
  /// @retval true ���������
  /// @retval false �����������
  static bool CheckForBodySentenceValid(
      const FlowInterface& for_body_sentence) {
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        for_body_sentence);
  }

 private:
  /// @brief ��ʼ�����
  std::list<std::unique_ptr<FlowInterface>> init_block_;
  /// @brief ѭ�����������ѭ�����������
  std::list<std::unique_ptr<FlowInterface>> renew_sentences_;
};

/// @class SwitchSentence flow_control.h
/// @brief switch��֧���
class SwitchSentence : public ConditionBlockInterface {
 public:
  SwitchSentence() : ConditionBlockInterface(FlowType::kSwitchSentence) {}

  virtual bool AddMainSentence(
      std::unique_ptr<FlowInterface>&& sentence) override {
    return AddSentence(std::move(sentence));
  }
  virtual bool AddMainSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentences) override {
    return AddSentences(std::move(sentences));
  }

  /// @brief �����ͨ��case
  /// @param[in] case_value ����֧�����������ڳ�����
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ��ӳɹ�����ȡcase_value����Ȩ
  /// @retval false ���ʧ�ܣ�������Ҳ��޸Ĳ���
  /// @note ��case��Ҫ��ο�CheckSwitchCaseAbleToAdd
  bool AddSimpleCase(
      const std::shared_ptr<const BasicTypeInitializeOperatorNode>& case_value);
  /// @brief ���default��ǩ
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ��ӳɹ�
  /// @retval false ���ʧ�ܣ��ú����Ѿ������ù�
  bool AddDefaultCase();
  /// @brief ��ȡȫ����ͨcase��ǩ
  /// @return ���ش洢��ͨcase��ǩ��������const����
  const auto& GetSimpleCases() const { return simple_cases_; }
  /// @brief ��ȡdefault��֧��ǩ
  /// @return ����ָ��default��֧��ǩ��constָ��
  /// @retval nullptr ��δ����AddDefaultCase���������default��ǩ
  const Label* GetDefaultCase() const { return default_case_.get(); }

  /// @brief �������ڵ��Ƿ������Ϊswitch������ִ�е����
  /// @param[in] switch_body_sentence �������������ڵ�
  /// @return �����Ƿ�������
  /// @retval true ���������
  /// @retval false �����������
  static bool CheckSwitchBodySentenceValid(
      const FlowInterface& switch_body_sentence) {
    return ConditionBlockInterface::DefaultMainBlockSentenceCheck(
        switch_body_sentence);
  }
  /// @brief ������case�Ƿ������Ϊswitch�ڵı�ǩ
  /// @param[in] switch_node ����ӵ���switch�ڵ�
  /// @param[in] case_value ����ǩ��ֵ
  /// @return �����ܷ���Ϊ��Ϊswitch�ڵı�ǩʹ��
  /// @retval true ���������
  /// @retval false ����ǩ�����������������е�caseֵ�ظ�
  static bool CheckSwitchCaseAbleToAdd(
      const SwitchSentence& switch_node,
      const BasicTypeInitializeOperatorNode& case_value);

 private:
  /// @brief ����case��ֵ������Ӧ�ı�ǩ
  /// @param[in] case_value ��case��ֵ
  /// @return ǰ�벿��Ϊ��ת�����ʹ�õı�ǩ����벿��Ϊswitch������ʹ�õı�ǩ
  /// @note ֻ����������
  std::pair<std::unique_ptr<Label>, std::unique_ptr<Label>>
  ConvertCaseValueToLabel(
      const BasicTypeInitializeOperatorNode& case_value) const;
  /// @brief ����default��ǩ
  /// @brief ǰ�벿��Ϊ��ת�����ʹ�õı�ǩ����벿��Ϊswitch������ʹ�õı�ǩ
  /// @note ֻ����������
  std::pair<std::unique_ptr<Label>, std::unique_ptr<Label>> CreateDefaultLabel()
      const;

  /// @brief switch�ķ�֧(case)
  /// @details
  /// ��Ϊcase��ֵ
  /// ֵǰ�벿��Ϊ�����ڳ����ڵ㣬��벿��Ϊ��case�ı�ǩ
  std::unordered_map<
      std::string,
      std::pair<std::shared_ptr<const BasicTypeInitializeOperatorNode>,
                std::unique_ptr<Label>>>
      simple_cases_;
  /// @brief default��֧��ǩ
  std::unique_ptr<Label> default_case_ = nullptr;
};

/// @class FlowControlSystem flow_control.h
/// @brief ���̿���ϵͳ
/// @details
/// 1.��ϵͳ��ǰ�����ڴ洢����
/// 2.�Ѿ�������ɵĺ��������ڹ����ĺ�����������ע��
/// 3.��Ծ����ָ���ڹ����ĺ���
/// @attention ��������ʱ����ȡ��ǰ����������flow_control_node����ӵ�
/// ActionScopeSystem�У�FunctionDefine���̽ڵ�Ŀ���Ȩ
/// ������FlowControlSystem����ϸ��Ϣ�ο�ActionScopeSystem
class FlowControlSystem {
 public:
  /// @brief ����/���û�Ծ����ʱ���صĽ��
  enum class FunctionCheckResult {
    kSuccess,  ///< �ɹ�����/���õ�ǰ��Ծ����
    ///< ʧ�ܵ����
    kOverrideFunction,  ///< ��ͼ���غ���
    kDoubleAnnounce,    ///< �����Ѿ�����/����ĺ���
    kFunctionConstructed  ///< ��ͼ�����Ѿ�������ɵĺ���Ϊ����������
  };
  /// @brief ���õ�ǰ�����ĺ���
  /// @param[in] active_function ����ǰ�����ĺ���������
  /// @return ���ع�����������������������
  /// @note ���������򲻻�����
  FunctionCheckResult SetFunctionToConstruct(
      const std::shared_ptr<const FunctionType>& active_function);
  /// @brief ��ȡָ��ǰ��Ծ�����ڵ��ָ��
  /// @return ����ָ��ǰ��Ծ�����ڵ��ָ��
  /// @retval nullptr ����ǰ�޻�Ծ����
  /// @warning ��ֹdelete���ص�ָ��
  FunctionDefine* GetActiveFunctionPointer() const {
    return active_function_ == functions_.end() ? nullptr
                                                : &active_function_->second;
  }
  /// @brief ��ȡ��ǰ��Ծ�����ڵ������
  /// @return ���ص�ǰ��Ծ�����ڵ������
  /// @note ������ڻ�Ծ����
  FunctionDefine& GetActiveFunctionReference() const {
    return active_function_->second;
  }
  /// @brief ��ɺ�������
  /// @note ��ɺ�����������øú������к�������
  void FinishFunctionConstruct() { active_function_ = functions_.end(); }
  /// @brief ��ѯ��������
  /// @param[in] function_name ��������
  /// @return ����ָ�����ڵ��constָ��
  /// @retval nullptr ��������function_nameָ���ĺ���
  const FunctionDefine* GetFunction(const std::string& function_name) {
    auto iter = functions_.find(function_name);
    if (iter == functions_.end()) [[unlikely]] {
      return nullptr;
    } else {
      return &iter->second;
    }
  }

  /// @brief ��������
  /// @param[in] function_type ������������������
  /// @note �������õ�ǰ��ĺ���
  FunctionCheckResult AnnounceFunction(
      const std::shared_ptr<const FunctionType>& function_type);

 private:
  /// @brief ���к�������Ϊ��������ֵΪ��������FunctionDefine��
  std::unordered_map<std::string, FunctionDefine> functions_;
  /// @brief ��ǰ��Ծ�ĺ��������ڱ������ĺ�����
  std::unordered_map<std::string, FunctionDefine>::iterator active_function_ =
      functions_.end();
};
}  // namespace c_parser_frontend::flow_control

#endif