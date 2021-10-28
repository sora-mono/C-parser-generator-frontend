#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAX_ANALYSIS_TABLE_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAX_ANALYSIS_TABLE_H_

#include <boost/serialization/unique_ptr.hpp>
#include <format>
#include <iostream>

#include "Generator/export_types.h"

namespace frontend::generator::syntax_generator {

// �﷨��������Ŀ
class SyntaxAnalysisTableEntry {
 public:
  // ǰ���������������࣬Ϊ���麯�����Է�����Ӧ������
  class ShiftAttachedData;
  class ReductAttachedData;
  class ShiftReductAttachedData;

  class ActionAndAttachedDataInterface {
   public:
    ActionAndAttachedDataInterface(ActionType action_type)
        : action_type_(action_type) {}
    ActionAndAttachedDataInterface(const ActionAndAttachedDataInterface&) =
        default;
    virtual ~ActionAndAttachedDataInterface() {}

    ActionAndAttachedDataInterface& operator=(
        const ActionAndAttachedDataInterface&) = default;

    virtual bool operator==(const ActionAndAttachedDataInterface&
                                attached_data_interface) const = 0 {
      return action_type_ == attached_data_interface.action_type_;
    }

    virtual const ShiftAttachedData& GetShiftAttachedData() const;
    virtual const ReductAttachedData& GetReductAttachedData() const;
    virtual const ShiftReductAttachedData& GetShiftReductAttachedData() const;

    ActionType GetActionType() const { return action_type_; }
    void SetActionType(ActionType action_type) { action_type_ = action_type; }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& action_type_;
    }

   private:
    // �ṩĬ�Ϲ��캯�������л�ʱ��������
    ActionAndAttachedDataInterface() = default;

    // �������л��õ������
    friend class boost::serialization::access;
    // �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;
    // ��¶�����ڲ����ͣ��Ӷ���boost_serialization��ע��
    friend struct ExportSyntaxGeneratorInsideTypeForSerialization;

    virtual ShiftAttachedData& GetShiftAttachedData() {
      return const_cast<ShiftAttachedData&>(
          static_cast<const ActionAndAttachedDataInterface&>(*this)
              .GetShiftAttachedData());
    }
    virtual ReductAttachedData& GetReductAttachedData() {
      return const_cast<ReductAttachedData&>(
          static_cast<const ActionAndAttachedDataInterface&>(*this)
              .GetReductAttachedData());
    }
    virtual ShiftReductAttachedData& GetShiftReductAttachedData() {
      return const_cast<ShiftReductAttachedData&>(
          static_cast<const ActionAndAttachedDataInterface&>(*this)
              .GetShiftReductAttachedData());
    }

    ActionType action_type_;
  };
  // ִ�����붯��ʱ�ĸ�������
  class ShiftAttachedData : public ActionAndAttachedDataInterface {
   public:
    ShiftAttachedData(SyntaxAnalysisTableEntryId next_entry_id)
        : ActionAndAttachedDataInterface(ActionType::kShift),
          next_entry_id_(next_entry_id) {}
    ShiftAttachedData(const ShiftAttachedData&) = default;

    ShiftAttachedData& operator=(const ShiftAttachedData&) = default;
    virtual bool operator==(const ActionAndAttachedDataInterface&
                                shift_attached_data) const override;

    virtual const ShiftAttachedData& GetShiftAttachedData() const override {
      return *this;
    }

    SyntaxAnalysisTableEntryId GetNextSyntaxAnalysisTableEntryId() const {
      return next_entry_id_;
    }
    void SetNextSyntaxAnalysisTableEntryId(
        SyntaxAnalysisTableEntryId next_entry_id) {
      next_entry_id_ = next_entry_id;
    }

   private:
    // �ṩĬ�Ϲ��캯�������л�ʱ��������
    ShiftAttachedData() = default;

    // �������л������
    friend class boost::serialization::access;
    // �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& boost::serialization::base_object<ActionAndAttachedDataInterface>(
          *this);
      ar& next_entry_id_;
    }
    virtual ShiftAttachedData& GetShiftAttachedData() override {
      return const_cast<ShiftAttachedData&>(
          static_cast<const ShiftAttachedData&>(*this).GetShiftAttachedData());
    }

    // ����õ��ʺ�ת�Ƶ����﷨��������ĿID
    SyntaxAnalysisTableEntryId next_entry_id_;
  };
  // ִ�й�Լ����ʱ�ĸ�������
  class ReductAttachedData : public ActionAndAttachedDataInterface {
   public:
    template <class ProductionBody>
    ReductAttachedData(ProductionNodeId reducted_nonterminal_node_id,
                       ProcessFunctionClassId process_function_class_id,
                       ProductionBody&& production_body)
        : ActionAndAttachedDataInterface(ActionType::kReduct),
          reducted_nonterminal_node_id_(reducted_nonterminal_node_id),
          process_function_class_id_(process_function_class_id),
          production_body_(std::forward<ProductionBody>(production_body)) {}
    ReductAttachedData(const ReductAttachedData&) = default;
    ReductAttachedData(ReductAttachedData&&) = default;

    ReductAttachedData& operator=(const ReductAttachedData&) = default;
    ReductAttachedData& operator=(ReductAttachedData&&) = default;
    virtual bool operator==(const ActionAndAttachedDataInterface&
                                reduct_attached_data) const override;

    virtual const ReductAttachedData& GetReductAttachedData() const override {
      return *this;
    }

    ProductionNodeId GetReductedNonTerminalNodeId() const {
      return reducted_nonterminal_node_id_;
    }
    void SetReductedNonTerminalNodeId(
        ProductionNodeId reducted_nonterminal_node_id) {
      reducted_nonterminal_node_id_ = reducted_nonterminal_node_id;
    }
    ProcessFunctionClassId GetProcessFunctionClassId() const {
      return process_function_class_id_;
    }
    void SetProcessFunctionClassId(
        ProcessFunctionClassId process_function_class_id) {
      process_function_class_id_ = process_function_class_id;
    }
    const std::vector<ProductionNodeId>& GetProductionBody() const {
      return production_body_;
    }
    void SetProductionBody(std::vector<ProductionNodeId>&& production_body) {
      production_body_ = std::move(production_body);
    }

   private:
    // �ṩĬ�Ϲ��캯�������л�ʱ��������
    ReductAttachedData() = default;

    // �������л������
    friend class boost::serialization::access;
    // �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& boost::serialization::base_object<ActionAndAttachedDataInterface>(
          *this);
      ar& reducted_nonterminal_node_id_;
      ar& process_function_class_id_;
      ar& production_body_;
    }

    virtual ReductAttachedData& GetReductAttachedData() override {
      return const_cast<ReductAttachedData&>(
          static_cast<const ReductAttachedData&>(*this)
              .GetReductAttachedData());
    }

    // ��Լ��õ��ķ��ս�ڵ��ID
    ProductionNodeId reducted_nonterminal_node_id_;
    // ִ�й�Լ����ʱʹ�õĶ����ID
    ProcessFunctionClassId process_function_class_id_;
    // ��Լ���ò���ʽ�����ں˶Ըò���ʽ������Щ�ڵ�
    // ��ʹ�ÿչ�Լ������ɸ�Ϊ����ʽ�ڵ���Ŀ
    std::vector<ProductionNodeId> production_body_;
  };

  // ʹ�ö������ķ�ʱ��һ�����ʼȿ�������Ҳ���Թ�Լ
  class ShiftReductAttachedData : public ActionAndAttachedDataInterface {
   public:
    template <class ShiftData, class ReductData>
    ShiftReductAttachedData(ShiftData&& shift_attached_data,
                            ReductData&& reduct_attached_data)
        : ActionAndAttachedDataInterface(ActionType::kShiftReduct),
          shift_attached_data_(std::forward<ShiftData>(shift_attached_data)),
          reduct_attached_data_(
              std::forward<ReductData>(reduct_attached_data)) {}
    ShiftReductAttachedData(const ShiftReductAttachedData&) = delete;

    // ����ShiftAttachedData��ReductAttachedData�Ƚ�ʱ���Ƚ���Ӧ����
    virtual bool operator==(
        const ActionAndAttachedDataInterface& attached_data) const override;

    virtual const ShiftAttachedData& GetShiftAttachedData() const override {
      return shift_attached_data_;
    }
    virtual const ReductAttachedData& GetReductAttachedData() const override {
      return reduct_attached_data_;
    }
    virtual const ShiftReductAttachedData& GetShiftReductAttachedData()
        const override {
      return *this;
    }

    void SetShiftAttachedData(ShiftAttachedData&& shift_attached_data) {
      shift_attached_data_ = std::move(shift_attached_data);
    }
    void SetReductAttachedData(ReductAttachedData&& reduct_attached_data) {
      reduct_attached_data_ = std::move(reduct_attached_data);
    }

   private:
    // �ṩĬ�Ϲ��캯�������л�ʱ��������
    ShiftReductAttachedData() = default;

    // �������л������
    friend class boost::serialization::access;
    // �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& boost::serialization::base_object<ActionAndAttachedDataInterface>(
          *this);
      ar& shift_attached_data_;
      ar& reduct_attached_data_;
    }
    virtual ShiftAttachedData& GetShiftAttachedData() override {
      return const_cast<ShiftAttachedData&>(
          static_cast<const ShiftReductAttachedData&>(*this)
              .GetShiftAttachedData());
    }
    virtual ReductAttachedData& GetReductAttachedData() override {
      return const_cast<ReductAttachedData&>(
          static_cast<const ShiftReductAttachedData&>(*this)
              .GetReductAttachedData());
    }
    virtual ShiftReductAttachedData& GetShiftReductAttachedData() override {
      return const_cast<ShiftReductAttachedData&>(
          static_cast<const ShiftReductAttachedData&>(*this)
              .GetShiftReductAttachedData());
    }

    ShiftAttachedData shift_attached_data_;
    ReductAttachedData reduct_attached_data_;
  };

  // ��ֵΪ������ڵ�ID��ֵΪָ����Ӧ���ݵ�ָ��
  using ActionAndTargetContainer =
      std::unordered_map<ProductionNodeId,
                         std::unique_ptr<ActionAndAttachedDataInterface>>;

  SyntaxAnalysisTableEntry() {}
  SyntaxAnalysisTableEntry(const SyntaxAnalysisTableEntry&) = delete;
  SyntaxAnalysisTableEntry& operator=(const SyntaxAnalysisTableEntry&) = delete;
  SyntaxAnalysisTableEntry(
      SyntaxAnalysisTableEntry&& syntax_analysis_table_entry)
      : action_and_attached_data_(
            std::move(syntax_analysis_table_entry.action_and_attached_data_)),
        nonterminal_node_transform_table_(std::move(
            syntax_analysis_table_entry.nonterminal_node_transform_table_)) {}
  SyntaxAnalysisTableEntry& operator=(
      SyntaxAnalysisTableEntry&& syntax_analysis_table_entry);

  // ���øò���ʽ��ת�������µĶ�����Ŀ��ڵ�
  template <class AttachedData>
  requires std::is_same_v<std::decay_t<AttachedData>,
                          SyntaxAnalysisTableEntry::ShiftAttachedData> ||
      std::is_same_v<std::decay_t<AttachedData>,
                     SyntaxAnalysisTableEntry::ReductAttachedData>
  void SetTerminalNodeActionAndAttachedData(ProductionNodeId node_id,
                                            AttachedData&& attached_data);
  // ���ø���Ŀ������ս�ڵ��ת�Ƶ��Ľڵ�
  void SetNonTerminalNodeTransformId(
      ProductionNodeId node_id, SyntaxAnalysisTableEntryId production_node_id) {
    nonterminal_node_transform_table_[node_id] = production_node_id;
  }
  // �޸ĸ���Ŀ��������ĿIDΪ��ID
  // ��ǰ����½��޸�����ʱת�Ƶ�����һ����ĿID�������ս�ڵ�/���ս�ڵ㣩
  void ResetEntryId(const std::unordered_map<SyntaxAnalysisTableEntryId,
                                             SyntaxAnalysisTableEntryId>&
                        old_entry_id_to_new_entry_id);
  // ���ʸ���Ŀ�¸���ID�ս�ڵ����Ϊ��Ŀ��ID
  // ��������ڸ�ת�������򷵻ؿ�ָ��
  const ActionAndAttachedDataInterface* AtTerminalNode(
      ProductionNodeId node_id) const {
    auto iter = action_and_attached_data_.find(node_id);
    return iter == action_and_attached_data_.end() ? nullptr
                                                   : iter->second.get();
  }
  // ���ʸ���Ŀ�¸���ID���ս�ڵ�����ʱת�Ƶ�����ĿID
  // �����ڸ�ת�������򷵻�SyntaxAnalysisTableEntryId::InvalidId()
  SyntaxAnalysisTableEntryId AtNonTerminalNode(ProductionNodeId node_id) const {
    auto iter = nonterminal_node_transform_table_.find(node_id);
    return iter == nonterminal_node_transform_table_.end()
               ? SyntaxAnalysisTableEntryId::InvalidId()
               : iter->second;
  }
  // ��ȡȫ���ս�ڵ�Ĳ���
  const ActionAndTargetContainer& GetAllActionAndAttachedData() const {
    return action_and_attached_data_;
  }
  // ��ȡȫ�����ս�ڵ�ת�Ƶ��ı���
  const std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>&
  GetAllNonTerminalNodeTransformTarget() const {
    return nonterminal_node_transform_table_;
  }
  // �������Ŀ����������
  void Clear() {
    action_and_attached_data_.clear();
    nonterminal_node_transform_table_.clear();
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
        static_cast<const SyntaxAnalysisTableEntry&>(*this)
            .GetAllActionAndAttachedData());
  }
  // ��ȡȫ�����ս�ڵ�ת�Ƶ��ı���
  std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>&
  GetAllNonTerminalNodeTransformTarget() {
    return const_cast<
        std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>&>(
        static_cast<const SyntaxAnalysisTableEntry&>(*this)
            .GetAllNonTerminalNodeTransformTarget());
  }

  // ��ǰ������ID�µĲ�����Ŀ��ڵ�
  ActionAndTargetContainer action_and_attached_data_;
  // ������ս�ڵ��ת�Ƶ��Ĳ���ʽ�����
  std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>
      nonterminal_node_transform_table_;
};

template <class AttachedData>
requires std::is_same_v<std::decay_t<AttachedData>,
                        SyntaxAnalysisTableEntry::ShiftAttachedData> ||
    std::is_same_v<std::decay_t<AttachedData>,
                   SyntaxAnalysisTableEntry::ReductAttachedData>
void SyntaxAnalysisTableEntry::SetTerminalNodeActionAndAttachedData(
    ProductionNodeId node_id, AttachedData&& attached_data) {
  static_assert(
      !std::is_same_v<std::decay_t<AttachedData>, ShiftReductAttachedData>,
      "�����ͽ�����ͨ��������һ�ֶ����Ļ����ϲ�ȫȱ�ٵ���һ���ת���õ���������"
      "ֱ�Ӵ���");
  // ʹ�ö������ķ����﷨������ĳЩ�������Ҫ��ͬһ���ڵ�֧������͹�Լ��������
  auto iter = action_and_attached_data_.find(node_id);
  if (iter == action_and_attached_data_.end()) {
    // �²���ת�ƽڵ�
    action_and_attached_data_.emplace(
        node_id, std::make_unique<std::decay_t<AttachedData>>(
                     std::forward<AttachedData>(attached_data)));
  } else {
    // �������ת�������Ѵ���
    // ��ȡ�Ѿ��洢������
    ActionAndAttachedDataInterface& data_already_in = *iter->second;
    // Ҫô�޸����еĹ�Լ��õ��Ľڵ㣬Ҫô��ȫ����/��Լ����һ���֣���Լ/���룩
    // ��Ӧ�޸����е������ת�Ƶ����﷨��������Ŀ
    switch (data_already_in.GetActionType()) {
      case ActionType::kShift:
        if constexpr (std::is_same_v<std::decay_t<AttachedData>,
                                     ReductAttachedData>) {
          // �ṩ�������ǹ�Լ���ݣ�ת��ΪShiftReductAttachedData
          iter->second = std::make_unique<ShiftReductAttachedData>(
              std::move(iter->second->GetShiftAttachedData()),
              std::forward<AttachedData>(attached_data));
        } else {
          // �ṩ�����������ݣ�Ҫ����������е�������ͬ
          static_assert(
              std::is_same_v<std::decay_t<AttachedData>, ShiftAttachedData>);
          // ����ṩ�����������Ƿ������е�����������ͬ
          // ����д��������data_already_inΪShiftReductAttachedDataʱ
          // �޷�����������������ݱȽϵ���ȷ����
          // ������MSVC��BUG
          // ʹ����ʽ���ûᵼ�µ���attached_data��operator==��������ʽ����
          if (!data_already_in.operator==(attached_data)) [[unlikely]] {
            // ������ͬ������ͬ����ʽ�¹�Լ
            std::cerr << std::format(
                "һ�������ͬ������ֻ�ܹ�Լһ�ֲ���ʽ����ο�����"
                "����հ�ʱ�������Ϣ������ʽ");
            exit(-1);
          }
        }
        break;
      case ActionType::kReduct:
        if constexpr (std::is_same_v<std::decay_t<AttachedData>,
                                     ShiftAttachedData>) {
          // �ṩ������Ϊ���벿�����ݣ�ת��ΪShiftReductAttachedData
          iter->second = std::make_unique<ShiftReductAttachedData>(
              std::forward<AttachedData>(attached_data),
              std::move(iter->second->GetReductAttachedData()));
        } else {
          // �ṩ�����������ݣ���Ҫ����Ƿ������е�������ͬ
          static_assert(
              std::is_same_v<std::decay_t<AttachedData>, ReductAttachedData>);
          // ����д��������data_already_inΪShiftReductAttachedDataʱ
          // �޷�����������������ݱȽϵ���ȷ����
          // ������MSVC��BUG
          // ʹ����ʽ���ûᵼ�µ���attached_data��operator==��������ʽ����
          if (!data_already_in.operator==(attached_data)) {
            // ������ͬ������ͬ����ʽ�¹�Լ
            std::cerr << std::format(
                "һ�������ͬ������ֻ�ܹ�Լһ�ֲ���ʽ����ο�����"
                "����հ�ʱ�������Ϣ������ʽ");
            exit(-1);
          }
        }
        break;
      case ActionType::kShiftReduct: {
        // ������MSVC��BUG
        // ʹ����ʽ���ûᵼ�µ���attached_data��operator==��������ʽ����
        if (!data_already_in.operator==(attached_data)) {
          // ������ͬ������ͬ����ʽ�¹�Լ
          std::cerr << std::format(
              "һ�������ͬ������ֻ�ܹ�Լһ�ֲ���ʽ����ο�����"
              "����հ�ʱ�������Ϣ������ʽ");
          exit(-1);
        }
        break;
      }
      default:
        assert(false);
        break;
    }
  }
}
};  // namespace frontend::generator::syntax_generator

#endif  // !GENERATOR_SYNTAXGENERATOR_SYNTAX_ANALYSIS_TABLE_H_