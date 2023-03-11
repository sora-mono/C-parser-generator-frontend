/// @file id_wrapper.h
/// @brief ID��װ������װID�Ա���ID���õ�������ʱ����
/// @details
/// ��ͷ�ļ�����boost-serialization���ͷ�ļ����Ӷ��������л�
/// ��ͷ�ļ��������ĸ����ɲ�ͬ��ID��װ���ģ����
/// ռ�ÿռ��벻��װ��ͬ���������Ż������򲻻ᵼ�¶���Ŀ���
/// �޲ι��캯���Ὣֵ��Ϊdefault_invalid_value��IsValid()����false
/// default_invalid_valueĬ��ΪIdType(-1)
/// �����Ҫʹ��boost���л����ܣ������ͷ�ļ���id_wrapper_serializer.h
/// TODO �ػ�std::swap
#ifndef COMMON_ID_WRAPPER_H_
#define COMMON_ID_WRAPPER_H_

#include <algorithm>

namespace frontend::common {

/// @class BaseIdWrapper id_wrapper.h
/// @attention ����ID��װ���࣬��Ӧֱ��ʹ��
/// @param[in] IdType_ �� Ҫ��װ��ID��ʵ������
/// @param[in] LabelEnum_ ���ַ���ǩ��ö�����ͣ���label_��ͬ����Ψһ��ID
/// @param[in] label_
/// ���ַ���ǩö�������еľ���ö�٣���LabelEnum_��ͬ����Ψһ��ID����
/// @param[in] invalid_value_ �����ĸ�����Ϊ��Чֵ
/// @details
/// ʹ������
///  enum class ExampleEnum { kExampleType };
///  using Id = BaseIdWrapper<size_t,ExampleEnum,ExampleEnum::kExampleType,
///                               static_cast<size_t>(-1)>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_,
          const IdType_ invalid_value_>
class BaseIdWrapper {
 public:
  /// @brief �ṩʹ�õ�IDԭʼ����
  using IdType = IdType_;
  /// @brief �ṩ����ʱʹ�õ�ö������
  using LabelEnum = LabelEnum_;
  /// @brief �ṩ����ʱʹ�õ�ö���о���ö��
  static constexpr LabelEnum_ label = label_;

  BaseIdWrapper() : id_(invalid_value_) {}
  explicit BaseIdWrapper(IdType production_node_id) : id_(production_node_id) {}
  BaseIdWrapper(const BaseIdWrapper& id_wrapper) : id_(id_wrapper.id_) {}
  BaseIdWrapper& operator=(const BaseIdWrapper& id_wrapper) {
    id_ = id_wrapper.id_;
    return *this;
  }
  virtual ~BaseIdWrapper() {}

  operator IdType&() { return const_cast<IdType&>(operator const IdType&()); }
  operator const IdType&() const { return id_; }

  BaseIdWrapper& operator++() {
    ++id_;
    return *this;
  }
  BaseIdWrapper operator++(int) { return BaseIdWrapper(id_++); }
  bool operator==(const BaseIdWrapper& id_wrapper) const {
    return id_ == id_wrapper.id_;
  }
  bool operator!=(const BaseIdWrapper& id_wrapper) const {
    return !operator==(id_wrapper);
  }
  /// @brief ��ȡδ��װ��ԭʼֵ
  /// @return ԭʼ���͵�ԭʼֵ
  IdType GetRawValue() const { return id_; }
  /// @brief ����ʹ�õ�ID
  /// @param[in] production_node_id ��Ҫ�����ԭʼID
  void SetId(IdType production_node_id) { id_ = production_node_id; }
  /// @brief ��������ID����
  /// @param[in] id_wrapper ��ָ��Ҫ������������ָ��
  /// @note ������this��Ϊ����
  void Swap(BaseIdWrapper* id_wrapper) { std::swap(id_, id_wrapper->id_); }
  /// @brief �жϵ�ǰ�洢��ֵ�Ƿ���Ч
  bool IsValid() const { return id_ != invalid_value_; }
  /// @brief ��ȡԭʼ����Чֵ
  /// @return ԭʼ���͵���Чֵ
  static constexpr IdType InvalidValue() { return invalid_value_; }
  /// @brief ��ȡ�洢��Чֵ������
  /// @return �洢��Чֵ����������ЧֵͬInvalidValue
  static constexpr BaseIdWrapper InvalidId() {
    return BaseIdWrapper<IdType, LabelEnum, label, InvalidValue()>(
        InvalidValue());
  }

 private:
  /// @var id_
  /// @brief ID��ֵ
  IdType id_;
};

/// @class ExplicitIdWrapper id_wrapper.h
/// @brief ǿ����ʽ���ù��캯����ʹ��Ĭ����Чֵ��Wrapper
/// @param[in] IdType_ �� Ҫ��װ��ID��ʵ������
/// @param[in] LabelEnum_ ���ַ���ǩ��ö�����ͣ���label_��ͬ����Ψһ��ID
/// @param[in] label_
/// ���ַ���ǩö�������еľ���ö�٣���LabelEnum_��ͬ����Ψһ��ID����
/// @details
/// ʹ������
///  enum class ExampleEnum { kExampleType };
///  using Id = ExplicitIdWrapper<size_t,ExampleEnum,ExampleEnum::kExampleType>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_>
class ExplicitIdWrapper : public BaseIdWrapper<IdType_, LabelEnum_, label_,
                                               static_cast<IdType_>(-1)> {
 private:
  /// @brief ʹ�õĻ���
  using MyBase =
      BaseIdWrapper<IdType_, LabelEnum_, label_, static_cast<IdType_>(-1)>;

 public:
  /// @brief �ṩʹ�õ�IDԭʼ����
  using IdType = MyBase::IdType;
  /// @brief �ṩ����ʱʹ�õ�ö������
  using LabelEnum = MyBase::LabelEnum;
  /// @brief �ṩ����ʱʹ�õ�ö���о���ö��
  static const LabelEnum label = MyBase::label;

  ExplicitIdWrapper() {}
  explicit ExplicitIdWrapper(IdType production_node_id) {
    MyBase::SetId(production_node_id);
  }
  ExplicitIdWrapper(const MyBase& id_wrapper) : MyBase(id_wrapper) {}
  ExplicitIdWrapper& operator=(const MyBase& id_wrapper) {
    MyBase::operator=(id_wrapper);
    return *this;
  }

  ExplicitIdWrapper& operator++() {
    return static_cast<ExplicitIdWrapper&>(++static_cast<MyBase&>(*this));
  }
  ExplicitIdWrapper operator++(int) {
    return static_cast<ExplicitIdWrapper>(static_cast<MyBase>(*this)++);
  }
  bool operator==(const ExplicitIdWrapper& id_wrapper) {
    return MyBase::operator==(id_wrapper);
  }
  bool operator!=(const ExplicitIdWrapper& id_wrapper) {
    return MyBase::operator!=(id_wrapper);
  }
  /// @brief ��ȡ�洢��Чֵ������
  /// @return �洢��Чֵ����������ЧֵͬInvalidValue
  static constexpr ExplicitIdWrapper InvalidId() {
    return ExplicitIdWrapper<IdType, LabelEnum, label>(MyBase::InvalidValue());
  }
};

/// @class ExplicitIdWrapperCustomizeInvalidValue id_wrapper.h
/// @briefǿ����ʾ���ù��캯����ʹ��Ĭ����Чֵ��
/// @param[in] IdType_ �� Ҫ��װ��ID��ʵ������
/// @param[in] LabelEnum_ ���ַ���ǩ��ö�����ͣ���label_��ͬ����Ψһ��ID
/// @param[in] label_
/// ���ַ���ǩö�������еľ���ö�٣���LabelEnum_��ͬ����Ψһ��ID����
/// @param[in] invalid_value_ ���Զ������Чֵ
/// ����
///  enum class ExampleEnum { kExampleType };
///  using Id = ExplicitIdWrapper<size_t,ExampleEnum,ExampleEnum::kExampleType>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_,
          const IdType_ invalid_value_>
class ExplicitIdWrapperCustomizeInvalidValue
    : public BaseIdWrapper<IdType_, LabelEnum_, label_, invalid_value_> {
 private:
  /// @brief ʹ�õĻ���
  using MyBase = BaseIdWrapper<IdType_, LabelEnum_, label_, invalid_value_>;

 public:
  using IdType = MyBase::IdType;
  using LabelEnum = MyBase::LabelEnum;
  static const LabelEnum label = MyBase::label;

  ExplicitIdWrapperCustomizeInvalidValue() {}
  explicit ExplicitIdWrapperCustomizeInvalidValue(IdType production_node_id) {
    MyBase::SetId(production_node_id);
  }
  ExplicitIdWrapperCustomizeInvalidValue(
      const ExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    MyBase::SetId(id_wrapper.GetThisNodeId());
  }
  ExplicitIdWrapperCustomizeInvalidValue& operator=(
      const ExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    MyBase::SetId(id_wrapper.GetThisNodeId());
    return *this;
  }

  ExplicitIdWrapperCustomizeInvalidValue& operator++() {
    return static_cast<ExplicitIdWrapperCustomizeInvalidValue&>(
        ++static_cast<MyBase&>(*this));
  }
  ExplicitIdWrapperCustomizeInvalidValue operator++(int) {
    return static_cast<ExplicitIdWrapperCustomizeInvalidValue>(
        static_cast<MyBase&>(*this)++);
  }
  bool operator==(const ExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    return MyBase::operator==(id_wrapper);
  }
  bool operator!=(const ExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    return MyBase::operator!=(id_wrapper);
  }
  /// @brief ��ȡ�洢��Чֵ������
  /// @return �洢��Чֵ����������ЧֵͬInvalidValue
  static constexpr ExplicitIdWrapperCustomizeInvalidValue InvalidId() {
    return ExplicitIdWrapperCustomizeInvalidValue<IdType, LabelEnum, label,
                                                  static_cast<IdType>(-1)>(
        MyBase::InvalidValue());
  }
};

/// @class NonExplicitIdWrapper id_wrapper.h
/// @brief ��ǿ����ʾ���ù��캯��ʹ��Ĭ����Чֵ��
/// @param[in] IdType_ �� Ҫ��װ��ID��ʵ������
/// @param[in] LabelEnum_ ���ַ���ǩ��ö�����ͣ���label_��ͬ����Ψһ��ID
/// @param[in] label_
/// ���ַ���ǩö�������еľ���ö�٣���LabelEnum_��ͬ����Ψһ��ID����
/// ����
///  enum class ExampleEnum { kExampleType };
///  using Id = NonExplicitIdWrapper<size_t,ExampleEnum,
///                                  ExampleEnum::kExampleType>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_>
class NonExplicitIdWrapper : public BaseIdWrapper<IdType_, LabelEnum_, label_,
                                                  static_cast<IdType_>(-1)> {
 private:
  /// @brief ʹ�õĻ���
  using MyBase =
      BaseIdWrapper<IdType_, LabelEnum_, label_, static_cast<IdType_>(-1)>;

 public:
  /// @brief �ṩʹ�õ�IDԭʼ����
  using IdType = MyBase::IdType;
  /// @brief �ṩ����ʱʹ�õ�ö������
  using LabelEnum = MyBase::LabelEnum;
  /// @brief �ṩ����ʱʹ�õ�ö���о���ö��
  static const LabelEnum label = MyBase::label;

  NonExplicitIdWrapper() {}
  NonExplicitIdWrapper(IdType production_node_id) {
    MyBase::SetId(production_node_id);
  }
  NonExplicitIdWrapper(const NonExplicitIdWrapper& id_wrapper) {
    MyBase::SetId(id_wrapper.GetThisNodeId());
  }
  NonExplicitIdWrapper& operator=(const NonExplicitIdWrapper& id_wrapper) {
    MyBase::SetId(id_wrapper.GetThisNodeId());
    return *this;
  }

  NonExplicitIdWrapper& operator++() {
    return static_cast<NonExplicitIdWrapper&>(++static_cast<MyBase&>(*this));
  }
  NonExplicitIdWrapper operator++(int) {
    return static_cast<NonExplicitIdWrapper>(static_cast<MyBase&>(*this)++);
  }
  bool operator==(const NonExplicitIdWrapper& id_wrapper) {
    return MyBase::operator==(id_wrapper);
  }
  bool operator!=(const NonExplicitIdWrapper& id_wrapper) {
    return MyBase::operator!=(id_wrapper);
  }
  /// @brief ��ȡ�洢��Чֵ������
  /// @return �洢��Чֵ����������ЧֵͬInvalidValue
  static constexpr NonExplicitIdWrapper InvalidId() {
    return NonExplicitIdWrapper<IdType, LabelEnum, label>(
        MyBase::InvalidValue());
  }
};

/// @class NonExplicitIdWrapperCustomizeInvalidValue id_wrapper.h
/// @brief ��ǿ����ʾ���ù��캯����ʹ��Ĭ����Чֵ��
/// @param[in] IdType_ �� Ҫ��װ��ID��ʵ������
/// @param[in] LabelEnum_ ���ַ���ǩ��ö�����ͣ���label_��ͬ����Ψһ��ID
/// @param[in] label_
/// ���ַ���ǩö�������еľ���ö�٣���LabelEnum_��ͬ����Ψһ��ID����
/// @param[in] invalid_value_ ���Զ������Чֵ
/// ����
///  enum class ExampleEnum { kExampleType };
///  using Id = NonExplicitIdWrapper<size_t,ExampleEnum,
///                                  ExampleEnum::kExampleType,
///                                  static_cast<size_t>(-1)>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_,
          const IdType_ invalid_value_>
class NonExplicitIdWrapperCustomizeInvalidValue
    : public BaseIdWrapper<IdType_, LabelEnum_, label_, invalid_value_> {
 private:
  /// @brief ʹ�õĻ���
  using MyBase = BaseIdWrapper<IdType_, LabelEnum_, label_, invalid_value_>;

 public:
  /// @brief �ṩʹ�õ�IDԭʼ����
  using IdType = MyBase::IdType;
  /// @brief �ṩ����ʱʹ�õ�ö������
  using LabelEnum = MyBase::LabelEnum;
  /// @brief �ṩ����ʱʹ�õ�ö���о���ö��
  static const LabelEnum label = MyBase::label;

  NonExplicitIdWrapperCustomizeInvalidValue() {}
  NonExplicitIdWrapperCustomizeInvalidValue(IdType production_node_id) {
    MyBase::SetId(production_node_id);
  }
  NonExplicitIdWrapperCustomizeInvalidValue(
      const NonExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    MyBase::SetId(id_wrapper.GetThisNodeId());
  }
  NonExplicitIdWrapperCustomizeInvalidValue& operator=(
      const NonExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    MyBase::SetId(id_wrapper.GetThisNodeId());
    return *this;
  }

  NonExplicitIdWrapperCustomizeInvalidValue& operator++() {
    return static_cast<NonExplicitIdWrapperCustomizeInvalidValue&>(
        ++static_cast<MyBase&>(*this));
  }
  NonExplicitIdWrapperCustomizeInvalidValue operator++(int) {
    return static_cast<NonExplicitIdWrapperCustomizeInvalidValue>(
        static_cast<MyBase&>(*this)++);
  }
  bool operator==(const NonExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    return MyBase::operator==(id_wrapper);
  }
  bool operator!=(const NonExplicitIdWrapperCustomizeInvalidValue& id_wrapper) {
    return MyBase::operator!=(id_wrapper);
  }
  /// @brief ��ȡ�洢��Чֵ������
  /// @return �洢��Чֵ����������ЧֵͬInvalidValue
  static constexpr NonExplicitIdWrapperCustomizeInvalidValue InvalidId() {
    return NonExplicitIdWrapperCustomizeInvalidValue<IdType, LabelEnum, label,
                                                     static_cast<IdType>(-1)>(
        MyBase::InvalidValue());
  }
};

}  // namespace frontend::common

namespace std {
/// @brief
/// ���廯hashģ�壬����ʹ��unorderedϵ������ʱ�������޷��Զ���������ת��Ϊ
/// IdType���͵��±���
template <class IdType_, class LabelEnum_, LabelEnum_ label_>
struct hash<frontend::common::ExplicitIdWrapper<IdType_, LabelEnum_, label_>> {
  size_t operator()(
      const frontend::common::ExplicitIdWrapper<IdType_, LabelEnum_, label_>&
          id_wrapper) const {
    return _Do_hash(id_wrapper);
  }
  static size_t _Do_hash(
      const frontend::common::ExplicitIdWrapper<IdType_, LabelEnum_, label_>&
          id_wrapper) noexcept {
    return hash<typename frontend::common::ExplicitIdWrapper<
        IdType_, LabelEnum_, label_>::IdType>::_Do_hash(id_wrapper);
  }
};

/// @brief
/// ���廯hashģ�壬����ʹ��unorderedϵ������ʱ�������޷��Զ���������ת��Ϊ
/// IdType���͵��±���
template <class IdType_, class LabelEnum_, LabelEnum_ label_,
          IdType_ invalid_value_>
struct hash<frontend::common::ExplicitIdWrapperCustomizeInvalidValue<
    IdType_, LabelEnum_, label_, invalid_value_>> {
  size_t operator()(
      const frontend::common::ExplicitIdWrapperCustomizeInvalidValue<
          IdType_, LabelEnum_, label_, invalid_value_>& id_wrapper) const {
    return _Do_hash(id_wrapper);
  }
  static size_t _Do_hash(
      const frontend::common::ExplicitIdWrapperCustomizeInvalidValue<
          IdType_, LabelEnum_, label_, invalid_value_>& id_wrapper) noexcept {
    return hash<
        typename frontend::common::ExplicitIdWrapperCustomizeInvalidValue<
            IdType_, LabelEnum_, label_>::IdType>::_Do_hash(id_wrapper);
  }
};

/// @brief
/// ���廯hashģ�壬����ʹ��unorderedϵ������ʱ�������޷��Զ���������ת��Ϊ
/// IdType���͵��±���
template <class IdType_, class LabelEnum_, LabelEnum_ label_>
struct hash<
    frontend::common::NonExplicitIdWrapper<IdType_, LabelEnum_, label_>> {
  size_t operator()(
      const frontend::common::NonExplicitIdWrapper<IdType_, LabelEnum_, label_>&
          id_wrapper) const {
    return _Do_hash(id_wrapper);
  }
  static size_t _Do_hash(
      const frontend::common::NonExplicitIdWrapper<IdType_, LabelEnum_, label_>&
          id_wrapper) noexcept {
    return hash<typename frontend::common::NonExplicitIdWrapper<
        IdType_, LabelEnum_, label_>::IdType>::_Do_hash(id_wrapper);
  }
};

/// @brief
/// ���廯hashģ�壬����ʹ��unorderedϵ������ʱ�������޷��Զ���������ת��Ϊ
/// IdType���͵��±���
template <class IdType_, class LabelEnum_, LabelEnum_ label_,
          IdType_ invalid_value_>
struct hash<frontend::common::NonExplicitIdWrapperCustomizeInvalidValue<
    IdType_, LabelEnum_, label_, invalid_value_>> {
  size_t operator()(
      const frontend::common::NonExplicitIdWrapperCustomizeInvalidValue<
          IdType_, LabelEnum_, label_, invalid_value_>& id_wrapper) const {
    return _Do_hash(id_wrapper);
  }
  static size_t _Do_hash(
      const frontend::common::NonExplicitIdWrapperCustomizeInvalidValue<
          IdType_, LabelEnum_, label_, invalid_value_>& id_wrapper) noexcept {
    return hash<
        typename frontend::common::NonExplicitIdWrapperCustomizeInvalidValue<
            IdType_, LabelEnum_, label_>::IdType>::_Do_hash(id_wrapper);
  }
};
}  // namespace std

#endif  /// !COMMON_ID_WRAPPER