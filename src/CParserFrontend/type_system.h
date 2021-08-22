#ifndef CPARSERFRONTEND_TYPE_SYSTEM_H_
#define CPARSERFRONTEND_TYPE_SYSTEM_H_

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// ����ϵͳ
namespace c_parser_frontend::type_system {
// const��ǣ����뱣֤��ǰ����˳��
// ���Խ��ж��Ƿ���Ը�ֵʱ������if�Ż�Ϊһ��switch
enum class ConstTag { kNonConst, kConst };
// ���ű��
enum class SignTag { kSigned, kUnsigned };

// �������ʹ���
enum class StructOrBasicType {
  kBasic,           // ��������
  kPointer,         // ָ�루*��
  kFunction,        // ����
  kStruct,          // �ṹ��
  kUnion,           // ������
  kEnum,            // ö��
  kInitializeList,  // ��ʼ���б�
  kEnd,             // ����ϵͳ��β�������Ż��ж��߼�
  kNotSpecified  // δȷ����Ҫ�����ͣ�ʹ�ñ�׼���Ͳ��ҹ������ڲ������ƶ�Ӧ��ֵ
};
// �������ͣ����뱣֤��ǰ����˳�򣬿��������ж����������Ƿ������ʽת��
// void���ͽ�������Ϊ�����ķ������ͣ����������б�������Ϊָ��
// C����û����ʽ֧��bool��bool��Ӧ�����������ж���
// kVoid���ں�������ֵ�п�����Ϊ������֣���Ϊ��������ʱ����Ϊָ��
enum class BuiltInType {
  kVoid,
  kBool,
  kChar,
  kShort,
  kInt,
  kLong,
  kFloat,
  kDouble
};
// ����Ƿ���Ը�ֵ�Ľ��
enum class AssignableCheckResult {
  // ���Ը�ֵ�����
  kNonConvert,    // ����������ͬ������ת�����ɸ�ֵ
  kUpperConvert,  // ������ֵ������ת��Ϊ������ֵ����
  kConvertToVoidPointer,  // ����ֵ�Ķ���Ϊvoidָ�룬��ֵ�Ķ���Ϊͬ��ά��ָ��
  kZeroConvertToPointer,  // 0ֵת��Ϊָ��
  kUnsignedToSigned,  // �޷���ֵ��ֵ���з���ֵ������ʹ�þ���
  kSignedToUnsigned,  // �з���ֵ��ֵ���޷���ֵ������ʹ�þ���
  // ��Ҫ�����жϵ����
  kMayBeZeroToPointer,  // ����ǽ�0��ֵ��ָ����Ϸ�������Ƿ�
  // ���ɸ�ֵ�����
  kLowerConvert,         // ������ֵ�����ͷ�����խת��
  kCanNotConvert,        // ����ת��
  kAssignedNodeIsConst,  // ����ֵ����Ϊconst
  kAssignToRightValue,  // ����ֵ����Ϊ��ֵ��C���Բ�����ֵ����ֵ
  kArgumentsFull        // ��������������������Ӹ������
};
// �ƶ���ѧ�����ȡ����
enum class DeclineMathematicalComputeTypeResult {
  // ���Լ�������
  kComputable,              // ���Լ���
  kLeftPointerRightOffset,  // ������Ϊָ�룬������Ϊƫ����
  kLeftOffsetRightPointer,  // ������Ϊƫ������������Ϊָ��
  kConvertToLeft,           // ���������͵������ͺ�����
  kConvertToRight,          // ���������͵������ͺ�����
  // �����Լ�������
  kLeftNotComputableType,   // �����Ͳ��ǿ��Լ��������
  kRightNotComputableType,  // �����Ͳ��ǿ��Լ��������
  kLeftRightBothPointer,    // �������;�Ϊָ��
  kLeftNotIntger,  // ��������ָ�룬�����Ͳ���������������Ϊƫ����ʹ��
  kRightNotIntger  // ��������ָ�룬�����Ͳ���������������Ϊƫ����ʹ��
};
// ǰ������ָ������࣬����TypeInterface::ObtainAddress����ָ������
class PointerType;
// �������ͻ���
class TypeInterface {
 public:
  TypeInterface(StructOrBasicType type) : type_(type){};
  TypeInterface(StructOrBasicType type,
                std::shared_ptr<TypeInterface>&& next_type_node)
      : type_(type), next_type_node_(std::move(next_type_node)) {}
  virtual ~TypeInterface();

  // �Ƚ����������Ƿ���ȫ��ͬ�����Զ�������һ���ȽϺ���
  virtual bool operator==(const TypeInterface& type_interface) const {
    if (IsSameObject(type_interface)) [[likely]] {
      // �κ������඼Ӧ�ȵ�����ͼ��̳����IsSameObject������operator==()
      // ���������������StructOrBasicType::kEnd��β
      // ��Ӧ��EndType���operator==()ֱ�ӷ���true���ս������
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  // �жϸ��������Ƿ���Ը��ö���ֵ
  // ����ֵ������������Ͷ���
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const = 0;

  // operator==()���ӹ��̣����Ƚϸü����ͣ����������һ���ȽϺ���
  // �������Ӧ��д�ú������ȵ��û����ͬ�����������ж���������������
  // �Ӷ�ʵ��Ϊ����һ�����������ҿ��԰�ȫ�������ĺ���
  // ��ʹ���麯��������������
  // if��֧�࣬��������
  bool IsSameObject(const TypeInterface& type_interface) const {
    // һ�������д���룬��Ҫ�Ƚϵ����Ͷ�����ͬ�ģ�����Զ������ȷ
    return GetType() == type_interface.GetType();
  }

  void SetType(StructOrBasicType type) { type_ = type; }
  StructOrBasicType GetType() const { return type_; }

  std::shared_ptr<const TypeInterface> GetNextNodePointer() const {
    return next_type_node_;
  }
  std::shared_ptr<TypeInterface> GetNextNodePointer() {
    return next_type_node_;
  }
  void SetNextNodePointer(std::shared_ptr<TypeInterface>&& next_type_node) {
    next_type_node_ = std::move(next_type_node);
  }
  // ������Ƿ�Ϊ��ָ�룬��EndType���������
  const TypeInterface& GetNextNodeReference() const { return *next_type_node_; }
  TypeInterface& GetNextNodeReference() { return *next_type_node_; }

  // ��ȡȡ��ַ��õ������ͣ������Ҫȡ��ַ������
  static std::shared_ptr<const PointerType> ObtainAddress(
      std::shared_ptr<const TypeInterface>&& type_interface) {
    // ת��type_interfaceΪ�˹���PointerType�������޸ĸ�ָ��ָ�������
    return std::make_shared<const PointerType>(
        ConstTag::kConst,
        std::const_pointer_cast<TypeInterface>(type_interface), 0);
  }
  // �ƶ���ѧ���������
  // �������ͺ��ƶϵ����
  static std::pair<std::shared_ptr<const TypeInterface>,
                   DeclineMathematicalComputeTypeResult>
  DeclineMathematicalComputeResult(
      std::shared_ptr<const TypeInterface>&& left_compute_type,
      std::shared_ptr<const TypeInterface>&& right_compute_type);

 private:
  // ��ǰ�ڵ�����
  StructOrBasicType type_;
  // ָ����һ�����ͳɷֵ�ָ�룬֧�ֹ������ͣ���ʡ�ڴ�
  // ����ʹ��shared_ptr�����ƻ�����֧�ֹ���һ�����͵Ĳ�ͬ���ֺͲ����ͷ�
  std::shared_ptr<TypeInterface> next_type_node_;
};

// �ú����ṩ�Ƚ������ڵĽڵ�ָ��ķ���
// ����FunctionType�ȽϺ��������б�
bool operator==(const std::shared_ptr<TypeInterface>& type_interface1,
                const std::shared_ptr<TypeInterface>& type_interface2);

// Ԥ����Ļ�������
class BasicType : public TypeInterface {
 public:
  BasicType(BuiltInType built_in_type, SignTag sign_tag)
      : TypeInterface(StructOrBasicType::kBasic),
        built_in_type_(built_in_type),
        sign_tag_(sign_tag) {}
  BasicType(BuiltInType built_in_type, SignTag sign_tag,
            std::shared_ptr<TypeInterface>&& next_type_node)
      : TypeInterface(StructOrBasicType::kBasic, std::move(next_type_node)),
        built_in_type_(built_in_type),
        sign_tag_(sign_tag) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override;

  void SetBuiltInType(BuiltInType built_in_type) {
    built_in_type_ = built_in_type;
  }
  BuiltInType GetBuiltInType() const { return built_in_type_; }
  void SetSignTag(SignTag sign_tag) { sign_tag_ = sign_tag; }
  SignTag GetSignTag() const { return sign_tag_; }

  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  BuiltInType built_in_type_;
  SignTag sign_tag_;
};

// ָ�����ͣ�һ��*��Ӧһ���ڵ�
class PointerType : public TypeInterface {
 public:
  PointerType(ConstTag const_tag, size_t array_size)
      : TypeInterface(StructOrBasicType::kPointer),
        variety_const_tag_(const_tag),
        array_size_(array_size) {}
  PointerType(ConstTag const_tag,
              std::shared_ptr<TypeInterface>&& next_type_node,
              size_t array_size)
      : TypeInterface(StructOrBasicType::kPointer, std::move(next_type_node)),
        variety_const_tag_(const_tag),
        array_size_(array_size) {}

  // �Ƚ����������Ƿ���ȫ��ͬ�����Զ�������һ���ȽϺ���
  virtual bool operator==(const TypeInterface& type_interface) const {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  };
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override;

  void SetConstTag(ConstTag const_tag) { variety_const_tag_ = const_tag; }
  ConstTag GetConstTag() const { return variety_const_tag_; }
  void SetArraySize(size_t array_size) { array_size_ = array_size; }
  size_t GetArraySize() const { return array_size_; }
  bool IsSameObject(const TypeInterface& type_interface) const;
  // ��ȡ�����õõ�������
  // ���صõ��Ķ����Ƿ�Ϊconst�Ͷ��������
  std::pair<std::shared_ptr<const TypeInterface>, ConstTag> DeReference()
      const {
    return std::make_pair(GetNextNodePointer(), GetConstTag());
  }

 private:
  // const���
  ConstTag variety_const_tag_;
  // ָ��ָ�����������������0����ָ�벻ָ������
  size_t array_size_;
};

// ��������
class FunctionType : public TypeInterface {
 public:
  // �洢������Ϣ
  struct ArgumentInfo {
    bool operator==(const ArgumentInfo& argument_info) const {
      // ������������Ӱ���Ƿ�Ϊͬһ���������Ƚ����ͺ��Ƿ�Ϊconst
      return argument_const_tag == argument_info.argument_const_tag &&
             *argument_type == *argument_info.argument_type;
    }
    const std::string* argument_name;
    ConstTag argument_const_tag;
    std::shared_ptr<const TypeInterface> argument_type;
  };

  template <class FunctionName, class ReturnType>
  FunctionType(FunctionName&& function_name, ConstTag return_type_const_tag,
               ReturnType&& return_type)
      : TypeInterface(StructOrBasicType::kFunction),
        function_name_(std::forward<FunctionName>(function_name)),
        return_type_const_tag_(return_type_const_tag),
        return_type_(std::forward<ReturnType>(return_type)) {}
  template <class FunctionName, class ReturnType>
  FunctionType(FunctionName&& function_name, ConstTag return_type_const_tag,
               ReturnType&& return_type,
               std::shared_ptr<TypeInterface>&& next_node_pointer)
      : TypeInterface(StructOrBasicType::kFunction,
                      std::move(next_node_pointer)),
        function_name_(std::forward<FunctionName>(function_name)),
        return_type_const_tag_(return_type_const_tag),
        return_type_(std::forward<ReturnType>(return_type)) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override {
    // �������ܱ���ֵ
    return AssignableCheckResult::kCanNotConvert;
  }

  bool IsSameObject(const TypeInterface& type_interface) const;

  template <class ReturnTypePointer>
  void SetReturnTypePointer(ReturnTypePointer&& return_type) {
    return_type_ = std::forward<ReturnTypePointer>(return_type);
  }
  std::shared_ptr<const TypeInterface> GetReturnTypePointer() const {
    return return_type_;
  }
  const TypeInterface& GetReturnTypeReference() const { return *return_type_; }
  void SetReturnTypeConstTag(ConstTag return_type_const_tag) {
    return_type_const_tag_ = return_type_const_tag;
  }
  ConstTag GetReturnTypeConstTag() const { return return_type_const_tag_; }
  void AddArgumentNameAndType(
      const std::string* argument_name, ConstTag const_tag,
      std::shared_ptr<const TypeInterface>&& argument_type) {
    argument_infos_.emplace_back(
        ArgumentInfo{.argument_name = argument_name,
                     .argument_const_tag = const_tag,
                     .argument_type = std::move(argument_type)});
  }
  const auto& GetArgumentTypes() const { return argument_infos_; }
  template <class FunctionName>
  void SetFunctionName(FunctionName&& function_name) {
    function_name_ = function_name;
  }
  const std::string& GetFunctionName() const { return function_name_; }
  // ��ȡ������һ������ָ����ʽ
  // ����shared_ptrָ��һ��ָ��ڵ㣬���ָ��ڵ�ָ��ú���
  static std::shared_ptr<const PointerType> ConvertToFunctionPointer(
      std::shared_ptr<const TypeInterface>&& function_type) {
    assert(function_type->GetType() == StructOrBasicType::kFunction);
    // ת��constΪ�˿��Թ���ָ��ڵ㣬�����޸�function_type
    return std::make_shared<const PointerType>(
        ConstTag::kConst, std::const_pointer_cast<TypeInterface>(function_type),
        0);
  }

 private:
  // ������
  // ��Ϊ����ָ��Ľڵ����ʱ�������ÿ�
  std::string function_name_;
  // ����ֵ��const���
  ConstTag return_type_const_tag_;
  // ������������
  std::shared_ptr<const TypeInterface> return_type_;
  // �������ͺͲ�����
  std::vector<ArgumentInfo> argument_infos_;
};

class StructType : public TypeInterface {
 public:
  // �ṹ����ڲ��洢����
  // ConstTag��ʶ��Ա��const��
  using StructContainerType = std::unordered_map<
      std::string, std::pair<ConstTag, std::shared_ptr<const TypeInterface>>>;

  template <class EnumMembers>
  StructType(const std::string* struct_name, EnumMembers&& members)
      : TypeInterface(StructOrBasicType::kStruct),
        struct_name_(struct_name),
        struct_members_(std::forward<EnumMembers>(members)) {}
  template <class EnumMembers>
  StructType(const std::string* struct_name, EnumMembers&& members,
             std::shared_ptr<TypeInterface>&& next_type_node_pointer)
      : TypeInterface(StructOrBasicType::kStruct,
                      std::move(next_type_node_pointer)),
        struct_name_(struct_name),
        struct_members_(std::forward<EnumMembers>(members)) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() ==
             static_cast<const StructType&>(type_interface)
                 .GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override {
    return *this == *type_interface ? AssignableCheckResult::kNonConvert
                                    : AssignableCheckResult::kCanNotConvert;
  }

  template <class EnumMembers>
  void SetStructMembers(EnumMembers&& member_datas) {
    struct_members_ = std::forward<EnumMembers>(member_datas);
  }
  // ����ָ�����ݵĵ��������Ƿ�ɹ�����
  // ���Ѵ��ڸ�����ʱ�����벢����false
  template <class MemberName>
  auto AddStructMember(
      MemberName&& member_name,
      std::shared_ptr<const TypeInterface>&& member_type_pointer) {
    return struct_members_.insert(std::make_pair(
        std::forward<MemberName>(member_name), std::move(member_type_pointer)));
  }
  // ��ȡ�ṹ���ڳ�Ա�����ͣ����������Ա���������򷵻�false
  std::pair<StructContainerType::const_iterator, bool> GetStructMemberInfo(
      const std::string& member_name) const {
    auto iter = GetStructMembers().find(member_name);
    return std::make_pair(iter, iter != GetStructMembers().end());
  }
  void SetStructName(const std::string* struct_name) {
    struct_name_ = struct_name;
  }
  const std::string* GetStructName() const { return struct_name_; }

  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  const StructContainerType& GetStructMembers() const {
    return struct_members_;
  }
  StructContainerType& GetStructMembers() { return struct_members_; }

  const std::string* struct_name_;
  StructContainerType struct_members_;
};

class UnionType : public TypeInterface {
 public:
  // �������ڲ��洢�ṹ
  // ConstTag��ʶ��Ա��const��
  using UnionContainerType = std::unordered_map<
      std::string, std::pair<ConstTag, std::shared_ptr<const TypeInterface>>>;

  UnionType() : TypeInterface(StructOrBasicType::kUnion) {}
  UnionType(const std::string* union_name,
            std::shared_ptr<TypeInterface>&& next_node_pointer)
      : TypeInterface(StructOrBasicType::kUnion, std::move(next_node_pointer)),
        union_name_(union_name) {}
  template <class UnionMembers>
  UnionType(const std::string* union_name, UnionMembers&& union_members,
            std::shared_ptr<TypeInterface>&& next_node_pointer)
      : TypeInterface(StructOrBasicType::kUnion, std::move(next_node_pointer)),
        union_name_(union_name),
        union_members_(std::forward<UnionMembers>(union_members)) {}

  virtual bool operator==(const TypeInterface& type_interface) {
    if (IsSameObject(type_interface)) [[likely]] {
      return type_interface.GetNextNodeReference() ==
             type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override {
    return *this == *type_interface ? AssignableCheckResult::kNonConvert
                                    : AssignableCheckResult::kCanNotConvert;
  }

  template <class UnionMembers>
  void SetUnionMembers(UnionMembers&& union_members) {
    union_members_ = std::forward<UnionMembers>(union_members);
  }
  // ����ָ�����ݵĵ��������Ƿ�ɹ�����
  // ���Ѵ��ڸ�����ʱ�����벢����false
  template <class MemberName>
  auto AddUnionMember(
      MemberName&& member_name,
      std::shared_ptr<const TypeInterface>&& member_type_pointer) {
    return union_members_.insert(std::make_pair(
        std::forward<MemberName>(member_name), std::move(member_type_pointer)));
  }
  // ��ȡ�������ڳ�Ա��Ϣ�������Ա���������򷵻�false
  std::pair<UnionContainerType::const_iterator, bool> GetUnionMemberInfo(
      const std::string& member_name) const {
    auto iter = GetUnionMembers().find(member_name);
    return std::make_pair(iter, iter != GetUnionMembers().end());
  }
  void SetUnionName(const std::string* union_name) { union_name_ = union_name; }
  const std::string* GetUnionName() const { return union_name_; }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  const UnionContainerType& GetUnionMembers() const { return union_members_; }
  UnionContainerType& GetUnionMembers() { return union_members_; }

  // ��������
  const std::string* union_name_;
  // �ù��������ʹ�õ����ͣ���ֵΪ��Ա����ֵΪָ�����͵�ָ��
  UnionContainerType union_members_;
};

class EnumType : public TypeInterface {
 public:
  // ö���ڲ��洢�ṹ
  using EnumContainerType = std::unordered_map<std::string, long long>;

  EnumType(const std::string* enum_name,
           std::shared_ptr<TypeInterface>&& next_node_pointer)
      : TypeInterface(StructOrBasicType::kEnum, std::move(next_node_pointer)),
        enum_name_(enum_name) {}
  template <class EnumMembers>
  EnumType(const std::string* enum_name, EnumMembers&& enum_members,
           std::shared_ptr<TypeInterface>&& next_node_pointer)
      : TypeInterface(StructOrBasicType::kEnum, std::move(next_node_pointer)),
        enum_name_(enum_name),
        enum_members_(std::forward<EnumMembers>(enum_members)) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override {
    return *this == *type_interface ? AssignableCheckResult::kNonConvert
                                    : AssignableCheckResult::kCanNotConvert;
  }

  template <class EnumMembers>
  void SetEnumMembers(EnumMembers&& enum_members) {
    enum_members_ = std::forward<EnumMembers>(enum_members);
  }
  // ����ָ�����ݵĵ��������Ƿ�ɹ�����
  // ���Ѵ��ڸ�����ʱ�����벢����false
  template <class EnumMemberName>
  auto AddEnumMember(EnumMemberName&& enum_member_name, long long value) {
    return enum_members_.emplace(
        std::make_pair(std::forward<EnumMemberName>(enum_member_name), value));
  }
  // ��ȡö���ڳ�Ա��Ϣ�������Ա���������򷵻�false
  std::pair<EnumContainerType::const_iterator, bool> GetEnumMemberInfo(
      const std::string& member_name) const {
    auto iter = GetEnumMembers().find(member_name);
    return std::make_pair(iter, iter != GetEnumMembers().end());
  }
  // ���ݳ�Ա���������index����
  void CalculateContainerType();
  std::shared_ptr<const TypeInterface> GetContainerTypePointer() const {
    return container_type_;
  }
  const TypeInterface& GetContainerTypeReference() const {
    return *container_type_;
  }
  void SetEnumName(const std::string* enum_name) { enum_name_ = enum_name; }
  const std::string* GetEnumName() const { return enum_name_; }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  const EnumContainerType& GetEnumMembers() const { return enum_members_; }
  EnumContainerType& GetEnumMembers() { return enum_members_; }
  void SetContainerType(std::shared_ptr<const TypeInterface>&& container_type) {
    container_type_ = std::move(container_type);
  }
  // ö������
  const std::string* enum_name_;
  // ǰ�벿��Ϊ��Ա������벿��Ϊö�ٶ�Ӧ��ֵ
  EnumContainerType enum_members_;
  // �洢ö��ֵ�õ�����
  std::shared_ptr<const TypeInterface> container_type_;
};

class InitializeListType : public TypeInterface {
 public:
  InitializeListType() : TypeInterface(StructOrBasicType::kInitializeList) {}
  InitializeListType(std::shared_ptr<TypeInterface>&& next_node_pointer)
      : TypeInterface(StructOrBasicType::kInitializeList,
                      std::move(next_node_pointer)) {}

  virtual bool operator==(const TypeInterface& type_interface) {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override {
    // ��ʼ���б��ܱ���ֵ
    return AssignableCheckResult::kCanNotConvert;
  }

  void AddListType(std::shared_ptr<const TypeInterface>&& type_pointer) {
    list_types_.emplace_back(std::move(type_pointer));
  }
  const auto& GetListTypes() const { return list_types_; }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  std::vector<std::shared_ptr<const TypeInterface>> list_types_;
};

class EndType : public TypeInterface {
 public:
  EndType()
      : TypeInterface(StructOrBasicType::kEnd,
                      std::shared_ptr<TypeInterface>(nullptr)) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    // EndType�в��������һ�����ͽڵ��operator==()
    // �������ǽ�β��Ҳû����һ�����ͽڵ�
    return IsSameObject(type_interface);
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      std::shared_ptr<const TypeInterface>&& type_interface) const override {
    assert(false);
    // EndType�иú�����Ӧ������
    // ��ֹ����
    return AssignableCheckResult::kCanNotConvert;
  }
  // ��ȡEndType�ڵ�ָ�룬ȫ�ֹ���һ���ڵ㣬��ʡ�ڴ�
  static std::shared_ptr<TypeInterface> GetEndType() {
    static std::shared_ptr<TypeInterface> end_type_pointer(new EndType());
    return end_type_pointer;
  }

  bool IsSameObject(const TypeInterface& type_interface) const {
    // this == &type_interface���Ż��ֶ�
    // ����ϵͳ���˼·�Ǿ����ܶ�Ĺ���һ��������
    // �������׳���ָ��ͬһ���ڵ�����
    return this == &type_interface ||
           TypeInterface::IsSameObject(type_interface);
  }
};

// �������ͽڵ����������ȫ�ֹ���һ�ݽڵ��Խ�ʡ�ڴ�ͱ����η��������
class CommonlyUsedTypeGenerator {
 public:
  // ��ȡ��������
  // void���ͺ�bool����ʹ��SignTag::kUnsigned
  template <BuiltInType built_in_type, SignTag sign_tag>
  static std::shared_ptr<TypeInterface> GetBasicType() {
    static std::shared_ptr<TypeInterface> basic_type(
        new BasicType(built_in_type, sign_tag, EndType::GetEndType()));
    return basic_type;
  }

  static std::shared_ptr<TypeInterface> GetBasicTypeNotTemplate(
      BuiltInType built_in_type, SignTag sign_tag);

  // ��ȡ��ʼ�����ַ������ͣ�const char*��
  static std::shared_ptr<TypeInterface> GetConstExprStringType() {
    static std::shared_ptr<TypeInterface> basic_type(new PointerType(
        ConstTag::kConst, GetBasicType<BuiltInType::kChar, SignTag::kSigned>(),
        0));
    return basic_type;
  }
};

class TypeSystem {
 public:
  // �������ʱ���ص�״̬
  enum class AddTypeResult {
    // �ɹ���ӵ����
    kNew,  // ��ǰ�����ڸ�������
    kShiftToVector,  // ���ǰ����������Ӧһ�����ͣ���Ӻ�ת��Ϊvector�洢
    kAddToVector,  // ���ǰ����������Ӧ����2������
    // ���ʧ�ܵ����
    kTypeAlreadyIn  // ����ӵ��������������Ѿ���һ��ͬ������
  };
  enum class GetTypeResult {
    // �ɹ�ƥ������
    kSuccess,  // �ɹ�ƥ��
    // ƥ��ʧ�ܵ����
    kTypeNameNotFound,   // ������������
    kNoMatchTypePrefer,  // ������ƥ������ѡ�������������ָ��
    kSeveralSameLevelMatches  // ƥ�䵽���ƽ�����
  };
  // ������ϵͳ���������
  // type_name����������type_pointer��ָ����������ָ��
  // ����ɹ���������򷵻�true
  // ��������������´���ӵ������������Ĵ����Ѵ��������ʧ�ܣ�����false
  // �����StructOrBasicType������kEnd��kNotSpecified
  template <class TypeName>
  AddTypeResult AddType(TypeName&& type_name,
                        std::shared_ptr<const TypeInterface>&& type_pointer);
  // ���ݸ�����������ѡ�����͵������ȡ����
  // ����ѡ���������StructOrBasicType::kEnd����ѡ��
  // ����ѡ������ΪStructOrBasicType::kNotSpecifiedʱ����ƥ��kBasic
  // ��������ƽ�������ڶ��ƽ������ʱ�޷���ȡ������ָ��
  // ����������ָ��ͻ�ȡ�������ȡ����Ľ��ͼ��䶨��
  std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
      const std::string& type_name, StructOrBasicType type_prefer);

 private:
  auto& GetTypeNameToNode() { return type_name_to_node_; }
  // ��������������������ӳ��
  // �����ṹ������������ɸýṹ�����ı���ά��������
  // ���洢���ͬ������ʱֵ��ת��Ϊָ��vector��ָ�룬ͨ��vector����ͬ������
  // ʹ��vector����ʱӦ��StructOrBasicType::kBasic���ͷ��ڵ�һ��λ��
  // ���Ա������ȫ��ָ�룬���ٱ�׼���Ͳ��ҹ�������ٶ�
  std::unordered_map<
      std::string,
      std::variant<
          std::monostate, std::shared_ptr<const TypeInterface>,
          std::unique_ptr<std::vector<std::shared_ptr<const TypeInterface>>>>>
      type_name_to_node_;
};

template <class TypeName>
inline TypeSystem::AddTypeResult TypeSystem::AddType(
    TypeName&& type_name, std::shared_ptr<const TypeInterface>&& type_pointer) {
  assert(type_pointer->GetType() != StructOrBasicType::kEnd);
  assert(type_pointer->GetType() != StructOrBasicType::kNotSpecified);
  auto& type_pointers = GetTypeNameToNode()[std::forward<TypeName>(type_name)];
  std::monostate* empty_status_pointer =
      std::get_if<std::monostate>(&type_pointers);
  if (empty_status_pointer != nullptr) [[likely]] {
    // �ýڵ�δ�����κ�ָ��
    type_pointers = std::move(type_pointer);
    return AddTypeResult::kNew;
  } else {
    std::shared_ptr<const TypeInterface>* shared_pointer =
        std::get_if<std::shared_ptr<const TypeInterface>>(&type_pointers);
    if (shared_pointer != nullptr) [[likely]] {
      // �ýڵ㱣����һ��ָ�룬����һ����תΪvector�洢
      if ((*shared_pointer)->GetType() == type_pointer->GetType())
          [[unlikely]] {
        // ����ӵ�����������������ͬ��������ͻ
        return AddTypeResult::kTypeAlreadyIn;
      }
      auto vector_pointer =
          std::make_unique<std::vector<std::shared_ptr<const TypeInterface>>>();
      // ��StructOrBasicType::kBasic���ͷ�����ǰ��
      if (type_pointer->GetType() == StructOrBasicType::kBasic) {
        vector_pointer->emplace_back(std::move(type_pointer));
        vector_pointer->emplace_back(std::move(*shared_pointer));
      } else {
        // ԭ���洢�����Ϳ�����StructOrBasicType::kBasic
        // ���Ǵ���ӵ�����һ������
        vector_pointer->emplace_back(std::move(*shared_pointer));
        vector_pointer->emplace_back(std::move(type_pointer));
      }
      type_pointers = std::move(vector_pointer);
      return AddTypeResult::kShiftToVector;
    } else {
      // �ýڵ��Ѿ�ʹ��vector�洢
      std::unique_ptr<std::vector<std::shared_ptr<const TypeInterface>>>&
          vector_pointer = *std::get_if<std::unique_ptr<
              std::vector<std::shared_ptr<const TypeInterface>>>>(
              &type_pointers);
      assert(vector_pointer != nullptr);
      for (auto& stored_pointer : *vector_pointer) {
        if (stored_pointer->GetType() == type_pointer->GetType()) [[unlikely]] {
          // ����ӵ�����������������ͬ��������ͻ
          return AddTypeResult::kTypeAlreadyIn;
        }
      }
      vector_pointer->emplace_back(std::move(type_pointer));
      if (type_pointer->GetType() == StructOrBasicType::kBasic) {
        // �²��������ΪStructOrBasicType::kBasic
        // �������ͷŵ���һ��λ���Ա��Ż�������������ʱ�Ĳ����߼�
        std::swap(vector_pointer->front(), vector_pointer->back());
      }
      return AddTypeResult::kAddToVector;
    }
  }
}

}  // namespace c_parser_frontend::type_system
#endif  // !CPARSERFRONTEND_PARSE_CLASSES_H_