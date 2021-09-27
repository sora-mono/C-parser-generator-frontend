#ifndef COMMON_ID_WRAPPER_H_
#define COMMON_ID_WRAPPER_H_

#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>

namespace frontend::common {
// ��ͷ�ļ������������ɲ�ͬ��ID��װ���ģ����
// ռ�ÿռ��벻��װ��ͬ���������Ż������򲻻ᵼ�¶���Ŀ���
// �޲ι��캯���Ὣֵ��Ϊdefault_invalid_value��IsValid()����false
// default_invalid_valueĬ��ΪIdType(-1)
// TODO �ػ�std::swap

// ����ID��װ���࣬�û���Ӧʹ��
// ��һ������Ϊʹ�õ�ID����
// �ڶ�������Ϊ�ַ���ǩ��ö������
// ����������Ϊ�ַ���ǩö�������еľ���ö��
// ���ĸ�����Ϊ��Чֵ
// ����
//  enum class ExampleEnum { kExampleType };
//  using Id = BaseIdWrapper<size_t,ExampleEnum,ExampleEnum::kExampleType,
//                               static_cast<size_t>(-1)>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_,
          const IdType_ invalid_value_>
class BaseIdWrapper {
 public:
  using IdType = IdType_;
  using LabelEnum = LabelEnum_;
  static const LabelEnum_ label = label_;

  BaseIdWrapper() : id_(invalid_value_) {}
  explicit BaseIdWrapper(IdType production_node_id) : id_(production_node_id) {}
  BaseIdWrapper(const BaseIdWrapper& id_wrapper) : id_(id_wrapper.id_) {}
  BaseIdWrapper& operator=(const BaseIdWrapper& id_wrapper) {
    id_ = id_wrapper.id_;
    return *this;
  }
  virtual ~BaseIdWrapper() {}

  operator IdType&() { return const_cast<IdType&>(operator const IdType_&()); }
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
  // ��ȡδ��װ��ԭʼֵ
  IdType GetThisNodeValue() const { return id_; }
  void SetId(IdType production_node_id) { id_ = production_node_id; }
  void Swap(BaseIdWrapper* id_wrapper) { std::swap(id_, id_wrapper->id_); }
  bool IsValid() const { return id_ != invalid_value_; }
  static constexpr IdType InvalidValue() { return invalid_value_; }
  static constexpr BaseIdWrapper InvalidId() {
    return BaseIdWrapper<IdType, LabelEnum, label, InvalidValue()>(
        invalid_value_);
  }

 private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& id_;
  }

  IdType id_;
};

// ǿ����ʾ���ù��캯��ʹ��Ĭ����Чֵ��
// ����
//  enum class ExampleEnum { kExampleType };
//  using Id = ExplicitIdWrapper<size_t,ExampleEnum,ExampleEnum::kExampleType>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_>
class ExplicitIdWrapper : public BaseIdWrapper<IdType_, LabelEnum_, label_,
                                               static_cast<IdType_>(-1)> {
 private:
  using MyBase =
      BaseIdWrapper<IdType_, LabelEnum_, label_, static_cast<IdType_>(-1)>;

 public:
  using IdType = MyBase::IdType;
  using LabelEnum = MyBase::LabelEnum;
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
  static constexpr ExplicitIdWrapper InvalidId() {
    return ExplicitIdWrapper<IdType, LabelEnum, label>(MyBase::InvalidValue());
  }

 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& boost::serialization::base_object<MyBase>(*this);
  }
};

// ǿ����ʾ���ù��캯����ʹ��Ĭ����Чֵ��
// ����
//  enum class ExampleEnum { kExampleType };
//  using Id = ExplicitIdWrapper<size_t,ExampleEnum,ExampleEnum::kExampleType>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_,
          const IdType_ invalid_value_>
class ExplicitIdWrapperCustomizeInvalidValue
    : public BaseIdWrapper<IdType_, LabelEnum_, label_, invalid_value_> {
 private:
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
  static constexpr ExplicitIdWrapperCustomizeInvalidValue InvalidId() {
    return ExplicitIdWrapperCustomizeInvalidValue<IdType, LabelEnum, label,
                                                  static_cast<IdType>(-1)>(
        MyBase::InvalidValue());
  }

 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& boost::serialization::base_object<MyBase>(*this);
  }
};

// ��ǿ����ʾ���ù��캯��ʹ��Ĭ����Чֵ��
// ����
//  enum class ExampleEnum { kExampleType };
//  using Id = NonExplicitIdWrapper<size_t,ExampleEnum,
//                                  ExampleEnum::kExampleType>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_>
class NonExplicitIdWrapper : public BaseIdWrapper<IdType_, LabelEnum_, label_,
                                                  static_cast<IdType_>(-1)> {
 private:
  using MyBase =
      BaseIdWrapper<IdType_, LabelEnum_, label_, static_cast<IdType_>(-1)>;

 public:
  using IdType = MyBase::IdType;
  using LabelEnum = MyBase::LabelEnum;
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
  static constexpr NonExplicitIdWrapper InvalidId() {
    return NonExplicitIdWrapper<IdType, LabelEnum, label>(
        MyBase::InvalidValue());
  }

 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& boost::serialization::base_object<MyBase>(*this);
  }
};

// ��ǿ����ʾ���ù��캯����ʹ��Ĭ����Чֵ��
// ����
//  enum class ExampleEnum { kExampleType };
//  using Id = NonExplicitIdWrapper<size_t,ExampleEnum,
//                                  ExampleEnum::kExampleType,
//                                  static_cast<size_t>(-1)>;
template <class IdType_, class LabelEnum_, const LabelEnum_ label_,
          const IdType_ invalid_value_>
class NonExplicitIdWrapperCustomizeInvalidValue
    : public BaseIdWrapper<IdType_, LabelEnum_, label_, invalid_value_> {
 private:
  using MyBase = BaseIdWrapper<IdType_, LabelEnum_, label_, invalid_value_>;

 public:
  using IdType = MyBase::IdType;
  using LabelEnum = MyBase::LabelEnum;
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
  static constexpr NonExplicitIdWrapperCustomizeInvalidValue InvalidId() {
    return NonExplicitIdWrapperCustomizeInvalidValue<IdType, LabelEnum, label,
                                                     static_cast<IdType>(-1)>(
        MyBase::InvalidValue());
  }

 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& boost::serialization::base_object<MyBase>(*this);
  }
};

}  // namespace frontend::common

namespace std {
// ���廯hashģ�壬����ʹ��unorderedϵ������ʱ�������޷��Զ���������ת��Ϊ
// IdType���͵��±���

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

#endif  // !COMMON_ID_WRAPPER