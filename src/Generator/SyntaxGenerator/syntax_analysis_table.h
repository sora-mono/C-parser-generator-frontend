/// @file syntax_analysis_table.h
/// @brief �﷨������
/// @details
/// �﷨���������л����ļ��У�ʹ��ʱֻ�跴���л�
#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAX_ANALYSIS_TABLE_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAX_ANALYSIS_TABLE_H_

#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <format>
#include <iostream>

#include "Generator/export_types.h"

namespace frontend::generator::syntax_generator {

/// @class SynatxAnalysisTableEntry syntax_analysis_table.h
/// @brief �﷨����������Ŀ
class SyntaxAnalysisTableEntry {
 public:
  // ǰ���������������࣬Ϊ���麯�����Է�����Ӧ������
  class ShiftAttachedData;
  class ReductAttachedData;
  class ShiftReductAttachedData;

  /// @class ActionAndAttachedDataInterface syntax_analysis_table.h
  /// @brief �����͸������ݻ���
  /// @details
  /// �����Ǹ�����ǰ�����ž�������/��Լ/����/�����
  /// ������������ɶ�����ά��״̬��������������Ҫ�����ݣ��������뵥�ʺ�ת�Ƶ���
  /// ��һ����Ŀ����Լ��õ��ķ��ս����ʽID��
  /// @attention ���еĶ����͸������ݾ�Ӧ�Ӹ�������
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

    /// @brief ��ȡ��������ĸ�������
    /// @return ������������ĸ������ݵ�const����
    /// @attention ����������븽�����ݲſ��Ե��øú���
    virtual const ShiftAttachedData& GetShiftAttachedData() const;
    /// @brief ��ȡ��Լ�����ĸ�������
    /// @return ���ع�Լ�����ĸ������ݵ�const����
    /// @attention ������ڹ�Լ�������ݲſ��Ե��øú���
    virtual const ReductAttachedData& GetReductAttachedData() const;
    /// @brief ��ȡ����͹�Լ�����ĸ�������
    /// @return ��������͹�Լ�����ĸ������ݵ�const����
    /// @note ���ڶ������ķ�����ʱ��һ��״̬�¼ȿ�������Ҳ���Թ�Լ
    /// @attention �����������͹�Լ�������ݲſ��Ե��øú���
    virtual const ShiftReductAttachedData& GetShiftReductAttachedData() const;
    /// @brief �ж����������Ƿ���Ӧ������ͬ
    /// @param[in] action_and_attached_data �������Ƚϵ���һ��������
    /// @attention
    /// ��operator==()��ShiftReductAttachedData�����ϲ�ͬ
    /// operator==()�Ƚ����������Ƿ���ȫ��ͬ��IsSameOrPart��������
    /// ShiftAttachedData��ReductAttachedData��ֻ�Ƚ���Ӧ����
    virtual bool IsSameOrPart(const ActionAndAttachedDataInterface&
                                  action_and_attached_data) const = 0;

    /// @brief ��ȡ��������
    /// @return ���ض�������
    /// @retval ActionType::kShift ������
    /// @retval ActionType::kReduct ����Լ
    /// @retval ActionType::kShiftReduct ������͹�Լ
    /// @retval ActionType::kError ������
    /// @retval ActionType::kAccept ������
    ActionType GetActionType() const { return action_type_; }
    /// @brief ���ö�������
    /// @param[in] action_type �������õĶ�������
    void SetActionType(ActionType action_type) { action_type_ = action_type; }

    /// @brief ���л�����ĺ���
    /// @param[in,out] ar �����л�ʹ�õĵ���
    /// @param[in] version �����л��ļ��汾
    /// @attention �ú���Ӧ��boost����ö����ֶ�����
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& action_type_;
    }

   private:
    /// @brief �ṩĬ�Ϲ��캯�������л�ʱ��������
    ActionAndAttachedDataInterface() = default;

    /// @brief �������л��õ������
    friend class boost::serialization::access;
    /// @brief �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;

    /// @brief ��ȡ��������ĸ�������
    /// @return ������������ĸ������ݵ�����
    /// @attention ����������븽�����ݲſ��Ե��øú���
    virtual ShiftAttachedData& GetShiftAttachedData() {
      return const_cast<ShiftAttachedData&>(
          static_cast<const ActionAndAttachedDataInterface&>(*this)
              .GetShiftAttachedData());
    }
    /// @brief ��ȡ��Լ�����ĸ�������
    /// @return ���ع�Լ�����ĸ������ݵ�����
    /// @attention ������ڹ�Լ�������ݲſ��Ե��øú���
    virtual ReductAttachedData& GetReductAttachedData() {
      return const_cast<ReductAttachedData&>(
          static_cast<const ActionAndAttachedDataInterface&>(*this)
              .GetReductAttachedData());
    }
    /// @brief ��ȡ����͹�Լ�����ĸ�������
    /// @return ��������͹�Լ�����ĸ������ݵ�����
    /// @note ���ڶ������ķ�����ʱ��һ��״̬�¼ȿ�������Ҳ���Թ�Լ
    /// @attention �����������͹�Լ�������ݲſ��Ե��øú���
    virtual ShiftReductAttachedData& GetShiftReductAttachedData() {
      return const_cast<ShiftReductAttachedData&>(
          static_cast<const ActionAndAttachedDataInterface&>(*this)
              .GetShiftReductAttachedData());
    }

    /// @brief ��������
    ActionType action_type_;
  };

  /// @class ShiftAttachedData syntax_analysis_table.h
  /// @brief ִ�����붯��ʱ�ĸ�������
  class ShiftAttachedData : public ActionAndAttachedDataInterface {
   public:
    ShiftAttachedData(SyntaxAnalysisTableEntryId next_entry_id)
        : ActionAndAttachedDataInterface(ActionType::kShift),
          next_entry_id_(next_entry_id) {}
    ShiftAttachedData(const ShiftAttachedData&) = default;

    ShiftAttachedData& operator=(const ShiftAttachedData&) = default;
    virtual bool operator==(const ActionAndAttachedDataInterface&
                                shift_attached_data) const override;

    /// @brief ��ȡ��������ĸ�������
    /// @return ������������ĸ������ݵ�const����
    /// @attention ����������븽�����ݲſ��Ե��øú���
    virtual const ShiftAttachedData& GetShiftAttachedData() const override {
      return *this;
    }
    /// @brief �ж����������Ƿ���Ӧ������ͬ
    /// @param[in] shift_attached_data �������Ƚϵ���һ��������
    /// @attention
    /// ��operator==()��thisΪShiftReductAttachedData����ʱ�����ϲ�ͬ
    /// operator==()�Ƚ����������Ƿ���ȫ��ͬ��IsSameOrPart��������
    /// ShiftAttachedData��ReductAttachedData��ֻ�Ƚ���Ӧ����
    virtual bool IsSameOrPart(const ActionAndAttachedDataInterface&
                                  shift_attached_data) const override {
      return operator==(shift_attached_data);
    }

    /// @brief ��ȡ���뵥�ʺ�ת�Ƶ����﷨��������ĿID
    /// @return �������뵥�ʺ�ת�Ƶ����﷨��������ĿID
    SyntaxAnalysisTableEntryId GetNextSyntaxAnalysisTableEntryId() const {
      return next_entry_id_;
    }
    /// @brief �������뵥�ʺ�ת�Ƶ����﷨��������ĿID
    /// @param[in] next_entry_id �����뵥�ʺ�ת�Ƶ����﷨��������ĿID
    void SetNextSyntaxAnalysisTableEntryId(
        SyntaxAnalysisTableEntryId next_entry_id) {
      next_entry_id_ = next_entry_id;
    }

   private:
    /// @brief �ṩĬ�Ϲ��캯�������л�ʱ��������
    ShiftAttachedData() = default;

    /// @brief �������л������
    friend class boost::serialization::access;
    /// @brief �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;

    /// @brief ���л�����ĺ���
    /// @param[in,out] ar �����л�ʹ�õĵ���
    /// @param[in] version �����л��ļ��汾
    /// @attention �ú���Ӧ��boost����ö����ֶ�����
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& boost::serialization::base_object<ActionAndAttachedDataInterface>(
          *this);
      ar& next_entry_id_;
    }

    /// @brief ��ȡ��������ĸ�������
    /// @return ������������ĸ������ݵ�����
    /// @attention ����������븽�����ݲſ��Ե��øú���
    virtual ShiftAttachedData& GetShiftAttachedData() override { return *this; }

    /// @brief ����õ��ʺ�ת�Ƶ����﷨��������ĿID
    SyntaxAnalysisTableEntryId next_entry_id_;
  };

  /// @class ReductAttachedData syntax_analysis_table.h
  /// @brief ִ�й�Լ����ʱ�ĸ�������
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

    /// @brief ��ȡ��Լ�����ĸ�������
    /// @return ���ع�Լ�����ĸ������ݵ�const����
    /// @attention ������ڹ�Լ�������ݲſ��Ե��øú���
    virtual const ReductAttachedData& GetReductAttachedData() const override {
      return *this;
    }
    /// @brief �ж����������Ƿ���Ӧ������ͬ
    /// @param[in] reduct_attached_data �������Ƚϵ���һ��������
    /// @attention
    /// ��operator==()��thisΪShiftReductAttachedData����ʱ�����ϲ�ͬ
    /// operator==()�Ƚ����������Ƿ���ȫ��ͬ��IsSameOrPart��������
    /// ShiftAttachedData��ReductAttachedData��ֻ�Ƚ���Ӧ����
    virtual bool IsSameOrPart(const ActionAndAttachedDataInterface&
                                  reduct_attached_data) const override {
      return operator==(reduct_attached_data);
    }

    /// @brief ��ȡ��Լ�õ��ķ��ս����ʽID
    /// @return ���ع�Լ��õ��ķ��ս����ʽID
    ProductionNodeId GetReductedNonTerminalNodeId() const {
      return reducted_nonterminal_node_id_;
    }
    /// @brief ���ù�Լ��õ��ķ��ս����ʽID
    /// @param[in] reducted_nonterminal_node_id
    /// �������õĹ�Լ��õ��ķ��ս����ʽID
    void SetReductedNonTerminalNodeId(
        ProductionNodeId reducted_nonterminal_node_id) {
      reducted_nonterminal_node_id_ = reducted_nonterminal_node_id;
    }
    /// @brief ��ȡ��װ��Լ���������ʵ��������ID
    /// @return ���ذ�װ��Լ���������ʵ��������
    ProcessFunctionClassId GetProcessFunctionClassId() const {
      return process_function_class_id_;
    }
    /// @brief ���ð�װ��Լ���������ʵ��������ID
    /// @param[in] process_function_class_id
    /// �������õİ�װ��Լ���������ʵ��������ID
    void SetProcessFunctionClassId(
        ProcessFunctionClassId process_function_class_id) {
      process_function_class_id_ = process_function_class_id;
    }
    /// @brief ��ȡ����ʽ��
    /// @return ���ز���ʽ���const����
    /// @note ����ʽ�����ں˶Ի�ȡ������Щ���ʣ��Ӷ��ж���Щ����ʽ�չ�Լ
    const std::vector<ProductionNodeId>& GetProductionBody() const {
      return production_body_;
    }
    /// @brief ���ò���ʽ��
    /// @param[in] production_body �������õĲ���ʽ��
    void SetProductionBody(std::vector<ProductionNodeId>&& production_body) {
      production_body_ = std::move(production_body);
    }

   private:
    /// @brief �ṩĬ�Ϲ��캯�������л�ʱ��������
    ReductAttachedData() = default;

    /// @brief �������л������
    friend class boost::serialization::access;
    /// @brief �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;

    /// @brief ���л�����ĺ���
    /// @param[in,out] ar �����л�ʹ�õĵ���
    /// @param[in] version �����л��ļ��汾
    /// @attention �ú���Ӧ��boost����ö����ֶ�����
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& boost::serialization::base_object<ActionAndAttachedDataInterface>(
          *this);
      ar& reducted_nonterminal_node_id_;
      ar& process_function_class_id_;
      ar& production_body_;
    }

    /// @brief ��ȡ��Լ�����ĸ�������
    /// @return ���ع�Լ�����ĸ������ݵ�����
    /// @attention ������ڹ�Լ�������ݲſ��Ե��øú���
    virtual ReductAttachedData& GetReductAttachedData() override {
      return *this;
    }

    /// @brief ��Լ��õ��ķ��ս�ڵ��ID
    ProductionNodeId reducted_nonterminal_node_id_;
    /// @brief ִ�й�Լ����ʱʹ�õĶ����ID
    ProcessFunctionClassId process_function_class_id_;
    /// @brief ��Լ���ò���ʽ�����ں˶Ըò���ʽ������Щ�ڵ�
    /// @note ��ʹ�ÿչ�Լ������ɸ�Ϊ����ʽ�ڵ���Ŀ
    std::vector<ProductionNodeId> production_body_;
  };

  /// @class ShiftReductAttachedData syntax_analysis_table.h
  /// @brief �������Լ�����ĸ�������
  /// @note ʹ�ö������ķ�ʱĳЩ״̬�¼ȿ�������Ҳ���Թ�Լ
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

    virtual bool operator==(
        const ActionAndAttachedDataInterface& attached_data) const override;

    /// @brief ��ȡ��������ĸ�������
    /// @return ������������ĸ������ݵ�const����
    /// @attention ����������븽�����ݲſ��Ե��øú���
    virtual const ShiftAttachedData& GetShiftAttachedData() const override {
      return shift_attached_data_;
    }
    /// @brief ��ȡ��Լ�����ĸ�������
    /// @return ���ع�Լ�����ĸ������ݵ�const����
    /// @attention ������ڹ�Լ�������ݲſ��Ե��øú���
    virtual const ReductAttachedData& GetReductAttachedData() const override {
      return reduct_attached_data_;
    }
    /// @brief ��ȡ����͹�Լ�����ĸ�������
    /// @return ��������͹�Լ�����ĸ������ݵ�const����
    /// @note ���ڶ������ķ�����ʱ��һ��״̬�¼ȿ�������Ҳ���Թ�Լ
    /// @attention �����������͹�Լ�������ݲſ��Ե��øú���
    virtual const ShiftReductAttachedData& GetShiftReductAttachedData()
        const override {
      return *this;
    }
    /// @brief �ж����������Ƿ���Ӧ������ͬ
    /// @param[in] attached_data �������Ƚϵ���һ��������
    /// @attention
    /// ��operator==()��thisΪShiftReductAttachedData����ʱ�����ϲ�ͬ
    /// operator==()�Ƚ����������Ƿ���ȫ��ͬ��IsSameOrPart��������
    /// ShiftAttachedData��ReductAttachedData��ֻ�Ƚ���Ӧ����
    virtual bool IsSameOrPart(
        const ActionAndAttachedDataInterface& attached_data) const override;

    /// @brief �������븽������
    /// @param[in] shift_attached_data �������õ���������
    void SetShiftAttachedData(ShiftAttachedData&& shift_attached_data) {
      shift_attached_data_ = std::move(shift_attached_data);
    }
    /// @brief ���ù�Լ��������
    /// @param[in] reduct_attached_data �������õĹ�Լ����
    void SetReductAttachedData(ReductAttachedData&& reduct_attached_data) {
      reduct_attached_data_ = std::move(reduct_attached_data);
    }

   private:
    /// @brief �ṩĬ�Ϲ��캯�������л�ʱ��������
    ShiftReductAttachedData() = default;

    /// @brief �������л������
    friend class boost::serialization::access;
    /// @brief �����﷨��������Ŀ�����ڲ��ӿ�
    friend class SyntaxAnalysisTableEntry;

    /// @brief ���л�����ĺ���
    /// @param[in,out] ar �����л�ʹ�õĵ���
    /// @param[in] version �����л��ļ��汾
    /// @attention �ú���Ӧ��boost����ö����ֶ�����
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& boost::serialization::base_object<ActionAndAttachedDataInterface>(
          *this);
      ar& shift_attached_data_;
      ar& reduct_attached_data_;
    }
    /// @brief ��ȡ��������ĸ�������
    /// @return ������������ĸ������ݵ�����
    /// @attention ����������븽�����ݲſ��Ե��øú���
    virtual ShiftAttachedData& GetShiftAttachedData() override {
      return shift_attached_data_;
    }
    /// @brief ��ȡ��Լ�����ĸ�������
    /// @return ���ع�Լ�����ĸ������ݵ�����
    /// @attention ������ڹ�Լ�������ݲſ��Ե��øú���
    virtual ReductAttachedData& GetReductAttachedData() override {
      return reduct_attached_data_;
    }

    /// @brief ��ȡ����͹�Լ�����ĸ�������
    /// @return ��������͹�Լ�����ĸ������ݵ�����
    /// @note ���ڶ������ķ�����ʱ��һ��״̬�¼ȿ�������Ҳ���Թ�Լ
    /// @attention �����������͹�Լ�������ݲſ��Ե��øú���
    virtual ShiftReductAttachedData& GetShiftReductAttachedData() override {
      return *this;
    }

    /// @brief ���븽������
    ShiftAttachedData shift_attached_data_;
    /// @brief ��Լ��������
    ReductAttachedData reduct_attached_data_;
  };

  /// @class AcceptAttachedData syntax_analysis_table.h
  /// @brief ��ʾAccept�����Ľڵ�
  class AcceptAttachedData : public ActionAndAttachedDataInterface {
   public:
    AcceptAttachedData()
        : ActionAndAttachedDataInterface(ActionType::kAccept) {}
    AcceptAttachedData(const AcceptAttachedData&) = delete;
    AcceptAttachedData(AcceptAttachedData&&) = default;

    AcceptAttachedData& operator=(const AcceptAttachedData&) = delete;
    AcceptAttachedData& operator=(AcceptAttachedData&&) = delete;

    /// @attention Accept�����²�֧�ָò���
    virtual bool operator==(
        const ActionAndAttachedDataInterface&) const override {
      assert(false);
      /// ��ֹ����
      return false;
    }
    /// @attention Accept�����²�֧�ָò���
    virtual bool IsSameOrPart(const ActionAndAttachedDataInterface&
                                  accept_attached_data) const override {
      assert(false);
      /// ��ֹ����
      return false;
    }

   private:
    /// @brief �������л������
    friend class boost::serialization::access;

    /// @brief ���л�����ĺ���
    /// @param[in,out] ar �����л�ʹ�õĵ���
    /// @param[in] version �����л��ļ��汾
    /// @attention �ú���Ӧ��boost����ö����ֶ�����
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& boost::serialization::base_object<ActionAndAttachedDataInterface>(
          *this);
    }
  };

  /// @brief �﷨��������Ŀ�д洢��ǰ�����Ŷ�Ӧ�Ķ����͸������ݵ�����
  /// @note ��ֵΪ������ڵ�ID��ֵΪָ�����͸������ݵ�ָ��
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

  /// @brief ���ø���Ŀ�ڸ����ս�ڵ��µĶ�����Ŀ��ڵ�
  /// @param[in] node_id ����������ս�ڵ�ID
  /// @param[in] attached_data ���ս�ڵ�ĸ�������
  /// @details
  /// �ú������������node_id������ս�ڵ�ʱ��Ҫ���Ĳ����Ͳ����ĸ�������
  /// �ڶ���������֧��ShiftAttachedData��ReductAttachedData��AcceptAttachedData
  /// @note ����ӵĲ������ͱ�����node_id���еĶ������Ͳ�ͬ���������ͺ�
  /// ����������ȫ��ͬ
  template <class AttachedData>
  requires std::is_same_v<std::decay_t<AttachedData>,
                          SyntaxAnalysisTableEntry::ShiftAttachedData> ||
      std::is_same_v<std::decay_t<AttachedData>,
                     SyntaxAnalysisTableEntry::ReductAttachedData> ||
      std::is_same_v<std::decay_t<AttachedData>,
                     SyntaxAnalysisTableEntry::AcceptAttachedData>
  void SetTerminalNodeActionAndAttachedData(ProductionNodeId node_id,
                                            AttachedData&& attached_data);
  /// @brief ���ø���Ŀ������ս�ڵ��ת�Ƶ����﷨��������ĿID
  /// @param[in] node_id ��������ķ��ս�ڵ�ID
  /// @param[in] next_analysis_table_entry_id
  /// ������÷��ս�ڵ��ת�Ƶ����﷨��������ĿID
  void SetNonTerminalNodeTransformId(
      ProductionNodeId node_id,
      SyntaxAnalysisTableEntryId next_analysis_table_entry_id) {
    nonterminal_node_transform_table_[node_id] = next_analysis_table_entry_id;
  }
  /// @brief �޸��﷨������������ָ�����﷨������IDΪ��ID
  /// @param[in] old_id_to_new_id ���洢���޸ĵ�ID����ID��ӳ��
  /// @note old_id_to_new_id����洢��Ҫ���ĵ�ID�����ı��ID����洢
  void ResetEntryId(
      const std::unordered_map<SyntaxAnalysisTableEntryId,
                               SyntaxAnalysisTableEntryId>& old_id_to_new_id);
  /// @brief ��ȡ�ڸ����������ս�ڵ�ID�µĶ����븽������
  /// @param[in] node_id ����������ս�ڵ�ID
  /// @return ����ָ�����븽�����ݵ�constָ��
  /// @retval nullptr �����ս�ڵ��²����ڿ���ִ�еĶ������﷨����
  const ActionAndAttachedDataInterface* AtTerminalNode(
      ProductionNodeId node_id) const {
    auto iter = action_and_attached_data_.find(node_id);
    return iter == action_and_attached_data_.end() ? nullptr
                                                   : iter->second.get();
  }
  /// @brief ��ȡ����������ս�ڵ��ת�Ƶ����﷨��������ĿID
  /// @param[in] node_id ��������ķ��ս�ڵ�ID
  /// @return ���������ת�Ƶ����﷨��������ĿID
  /// @retval SyntaxAnalysisTableEntryId::InvalidId()
  /// ���÷��ս�ڵ��޷����루�﷨����
  SyntaxAnalysisTableEntryId AtNonTerminalNode(ProductionNodeId node_id) const {
    auto iter = nonterminal_node_transform_table_.find(node_id);
    return iter == nonterminal_node_transform_table_.end()
               ? SyntaxAnalysisTableEntryId::InvalidId()
               : iter->second;
  }
  /// @brief ��ȡȫ���ս�ڵ�Ķ����͸�������
  /// @return ���ش洢ȫ���ս�ڵ�Ķ����͸������ݵ�������const����
  const ActionAndTargetContainer& GetAllActionAndAttachedData() const {
    return action_and_attached_data_;
  }
  /// @brief ��ȡȫ�����ս�ڵ�ת�Ƶ����﷨��������ĿID
  /// @return
  /// ���ش洢ȫ�����ս�ڵ������ת�Ƶ����﷨��������ĿID��������const����
  const std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>&
  GetAllNonTerminalNodeTransformTarget() const {
    return nonterminal_node_transform_table_;
  }
  /// @brief �������ļ�β��ǰ���ڵ���ִ��Accept����
  /// @param[in] eof_node_id ���ļ�β��ǰ���ڵ�
  /// @note �ļ�β�ڵ���Զ���ᱻ����
  /// @attention ����������ø���Ŀִ�иò���
  void SetAcceptInEofForwardNode(ProductionNodeId eof_node_id) {
    SetTerminalNodeActionAndAttachedData(eof_node_id, AcceptAttachedData());
  }
  /// @brief �������Ŀ����������
  void Clear() {
    action_and_attached_data_.clear();
    nonterminal_node_transform_table_.clear();
  }

 private:
  /// @brief �������л������
  friend class boost::serialization::access;

  /// @brief ���л�����ĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& action_and_attached_data_;
    ar& nonterminal_node_transform_table_;
  }

  /// @brief ��ȡȫ���ս�ڵ�Ķ����͸�������
  /// @return ���ش洢ȫ���ս�ڵ�Ķ����͸������ݵ�����������
  ActionAndTargetContainer& GetAllActionAndAttachedData() {
    return const_cast<ActionAndTargetContainer&>(
        static_cast<const SyntaxAnalysisTableEntry&>(*this)
            .GetAllActionAndAttachedData());
  }
  /// @brief ��ȡȫ�����ս�ڵ�ת�Ƶ����﷨��������ĿID
  /// @return ���ش洢ȫ�����ս�ڵ������ת�Ƶ����﷨��������ĿID������������
  std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>&
  GetAllNonTerminalNodeTransformTarget() {
    return const_cast<
        std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>&>(
        static_cast<const SyntaxAnalysisTableEntry&>(*this)
            .GetAllNonTerminalNodeTransformTarget());
  }

  /// @brief ��ǰ������ID�µĲ�����Ŀ��ڵ�
  ActionAndTargetContainer action_and_attached_data_;
  /// @brief ������ս�ڵ��ת�Ƶ��Ĳ���ʽ�����
  std::unordered_map<ProductionNodeId, SyntaxAnalysisTableEntryId>
      nonterminal_node_transform_table_;
};

template <class AttachedData>
requires std::is_same_v<std::decay_t<AttachedData>,
                        SyntaxAnalysisTableEntry::ShiftAttachedData> ||
    std::is_same_v<std::decay_t<AttachedData>,
                   SyntaxAnalysisTableEntry::ReductAttachedData> ||
    std::is_same_v<std::decay_t<AttachedData>,
                   SyntaxAnalysisTableEntry::AcceptAttachedData>
void SyntaxAnalysisTableEntry::SetTerminalNodeActionAndAttachedData(
    ProductionNodeId node_id, AttachedData&& attached_data) {
  if constexpr (std::is_same_v<std::decay_t<AttachedData>,
                               SyntaxAnalysisTableEntry::AcceptAttachedData>) {
    // ������ǰ�����ź󣬸��﷨��������Ŀ�������ø��ڵ�󵽴����Ŀ����ǰ������
    // ���ļ�β�ڵ�ʱִ�й�Լ��������Ҫ����Ϊ���ܲ���
    assert(action_and_attached_data_.find(node_id)->second->GetActionType() ==
           ActionType::kReduct);
    action_and_attached_data_[node_id] = std::make_unique<AcceptAttachedData>(
        std::forward<AttachedData>(attached_data));
    return;
  } else {
    static_assert(
        !std::is_same_v<std::decay_t<AttachedData>, ShiftReductAttachedData>,
        "�����ͽ�����ͨ��������һ�ֶ����Ļ����ϲ�ȫȱ�ٵ���һ���ת���õ�������"
        "��ֱ�Ӵ���");
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
            if (!data_already_in.IsSameOrPart(attached_data)) [[unlikely]] {
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
            if (!data_already_in.IsSameOrPart(attached_data)) [[unlikely]] {
              // ������ͬ������ͬ����ʽ�¹�Լ
              std::cerr << std::format(
                  "һ�������ͬ������ֻ�ܹ�Լһ�ֲ���ʽ����ο�����"
                  "����հ�ʱ�������Ϣ������ʽ");
              exit(-1);
            }
          }
          break;
        case ActionType::kShiftReduct: {
          if (!data_already_in.IsSameOrPart(attached_data)) [[unlikely]] {
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
}
};  // namespace frontend::generator::syntax_generator

#endif  // !GENERATOR_SYNTAXGENERATOR_SYNTAX_ANALYSIS_TABLE_H_