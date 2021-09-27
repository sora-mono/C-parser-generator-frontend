#ifndef CPARSERFRONTEND_TYPE_SYSTEM_H_
#define CPARSERFRONTEND_TYPE_SYSTEM_H_

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Common/id_wrapper.h"

namespace c_parser_frontend::operator_node {
// ǰ������ListInitializeOperatorNode������ʵ��AssignedByInitializeList
class ListInitializeOperatorNode;
// ǰ������VarietyOperatorNode�����ڶ��庯������
class VarietyOperatorNode;
}  // namespace c_parser_frontend::operator_node

namespace c_parser_frontend::flow_control {
// ǰ������FlowInterface�����ڶ��庯����ִ�е����
// ǰ������FlowType������ʵ����Ӻ�����ִ�е�������ع���
class FlowInterface;
enum class FlowType;
}  // namespace c_parser_frontend::flow_control

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
  kInt1,     // bool
  kInt8,     // char
  kInt16,    // short
  kInt32,    // int,long
  kFloat32,  // float
  kFloat64,  // double
  kVoid      // void
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
  kInitializeList,  // ʹ�ó�ʼ���б���Ҫ��ϸ���ֵ�����ж�
                    // ����CanBeAssignedBy�з���
  // ���ɸ�ֵ�����
  kLowerConvert,         // ������ֵ�����ͷ�����խת��
  kCanNotConvert,        // ����ת��
  kAssignedNodeIsConst,  // ����ֵ����Ϊconst
  kAssignToRightValue,  // ����ֵ����Ϊ��ֵ��C���Բ�����ֵ����ֵ
  kArgumentsFull,       // ��������������������Ӹ������
  kInitializeListTooLarge  // ��ʼ���б��и�����������Ŀ����ָ������ʱ��С
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
// �������ʱ���ص�״̬
enum class AddTypeResult {
  // �ɹ���ӵ����
  kAbleToAdd,  // ������ӣ�����CheckFunctionDefineAddResult��ʹ��
               // ���շ���ʱ��ת��Ϊ�������ֵ
  kNew,        // ��ǰ�����ڸ�����������kFunctionDefine���ȷ���
  kFunctionDefine,  // ����˺�������
  kShiftToVector,  // ���ǰ����������Ӧһ�����ͣ���Ӻ�ת��Ϊvector�洢
  kAddToVector,  // ���ǰ����������Ӧ����2������
  kAnnounceOrDefineBeforeFunctionAnnounce,  // ��������ǰ��������/����
  // ���ʧ�ܵ����
  kTypeAlreadyIn,  // ����ӵ��������������Ѿ���һ��ͬ������
  kRedefineFunction,  // �ض��庯��
};
enum class GetTypeResult {
  // �ɹ�ƥ������
  kSuccess,  // �ɹ�ƥ��
  // ƥ��ʧ�ܵ����
  kTypeNameNotFound,   // ������������
  kNoMatchTypePrefer,  // ������ƥ������ѡ�������������ָ��
  kSeveralSameLevelMatches  // ƥ�䵽���ƽ�����
};

// ����ָ�����ʹ�С
constexpr size_t kPointerSize = 4;
// �����������ʹ�С
constexpr size_t kBuiltInTypeSize[7] = {1, 1, 2, 4, 4, 8, 4};

// ���ݸ�����ֵ����洢�������С����
// �����ƶϳ�int1����
// ����BuiltInType::kVoid���������ֵ�������֧�ֵķ�Χ
BuiltInType CalculateBuiltInType(const std::string& value);

// ǰ������ָ������࣬����TypeInterface::ObtainAddress����ָ������
class PointerType;

using c_parser_frontend::flow_control::FlowInterface;
using c_parser_frontend::flow_control::FlowType;

// �������ͻ���
class TypeInterface {
 public:
  TypeInterface(StructOrBasicType type) : type_(type){};
  TypeInterface(StructOrBasicType type,
                const std::shared_ptr<const TypeInterface>& next_type_node)
      : type_(type), next_type_node_(next_type_node) {}
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
      const TypeInterface& type_interface) const = 0;
  // ��ȡ�洢�����ͱ�������ռ�Ĵ�С��ָ���Ϊ4�ֽڣ�
  virtual size_t GetTypeStoreSize() const = 0;
  // ��ȡsizeof���������͵Ĵ�С��ָ�������ָ��Ҫ��ָ���С���Ը�ά�ȴ�С��
  virtual size_t TypeSizeOf() const = 0;

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
  void SetNextNode(const std::shared_ptr<const TypeInterface>& next_type_node) {
    next_type_node_ = next_type_node;
  }
  // ������Ƿ�Ϊ��ָ�룬��EndType���������
  const TypeInterface& GetNextNodeReference() const { return *next_type_node_; }

  // ��ȡȡ��ַ��õ������ͣ������Ҫȡ��ַ������
  static std::shared_ptr<const PointerType> ObtainAddressOperatorNode(
      const std::shared_ptr<const TypeInterface>& type_interface) {
    // ת��type_interface��constΪ�˹���PointerType�������޸ĸ�ָ��ָ�������
    return std::make_shared<PointerType>(
        ConstTag::kConst, 0,
        std::const_pointer_cast<TypeInterface>(type_interface));
  }
  // �ƶ���ѧ���������
  // �������ͺ��ƶϵ����
  static std::pair<std::shared_ptr<const TypeInterface>,
                   DeclineMathematicalComputeTypeResult>
  DeclineMathematicalComputeResult(
      const std::shared_ptr<const TypeInterface>& left_compute_type,
      const std::shared_ptr<const TypeInterface>& right_compute_type);

 private:
  // ��ǰ�ڵ�����
  StructOrBasicType type_;
  // ָ����һ�����ͳɷֵ�ָ�룬֧�ֹ������ͣ���ʡ�ڴ�
  // ����ʹ��shared_ptr�����ƻ�����֧�ֹ���һ�����͵Ĳ�ͬ���ֺͲ����ͷ�
  std::shared_ptr<const TypeInterface> next_type_node_;
};

// �ú����ṩ�Ƚ������ڵĽڵ�ָ��ķ���
// ����FunctionType�ȽϺ��������б�
bool operator==(const std::shared_ptr<TypeInterface>& type_interface1,
                const std::shared_ptr<TypeInterface>& type_interface2);
// ������β���ڱ��ڵ㣬����Ƚ������Ƿ���ͬʱ��Ҫ�ж���һ���ڵ�ָ���Ƿ�Ϊ��
// ��������һ���ڵ�Ϊnullptr
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
      const TypeInterface& type_interface) const override {
    return AssignableCheckResult::kNonConvert;
  }
  virtual size_t GetTypeStoreSize() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }
  virtual size_t TypeSizeOf() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }

  // ��ȡEndType�ڵ�ָ�룬ȫ�ֹ���һ���ڵ㣬��ʡ�ڴ�
  static std::shared_ptr<EndType> GetEndType() {
    static std::shared_ptr<EndType> end_type_pointer =
        std::make_shared<EndType>();
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

// Ԥ����Ļ�������
class BasicType : public TypeInterface {
 public:
  BasicType(BuiltInType built_in_type, SignTag sign_tag)
      : TypeInterface(StructOrBasicType::kBasic),
        built_in_type_(built_in_type),
        sign_tag_(sign_tag) {}
  BasicType(BuiltInType built_in_type, SignTag sign_tag,
            const std::shared_ptr<TypeInterface>& next_node)
      : TypeInterface(StructOrBasicType::kBasic, next_node),
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
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override;
  virtual size_t TypeSizeOf() const override {
    return BasicType::GetTypeStoreSize();
  }

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
  PointerType(ConstTag const_tag, size_t array_size,
              const std::shared_ptr<const TypeInterface>& next_node)
      : TypeInterface(StructOrBasicType::kPointer, next_node),
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
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override { return 4; }
  virtual size_t TypeSizeOf() const override;

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
  // ָ��ָ�������������
  // 0����ָ�벻ָ������
  // -1����ָ��ָ��������С����������������
  size_t array_size_;
};

// ��������
class FunctionType : public TypeInterface {
  // �洢������Ϣ
  struct ArgumentInfo {
    bool operator==(const ArgumentInfo& argument_info) const;

    // ָ���������ָ�벻�ȶ������Ϊ�������������������������
    // ���Ϊ�����������ڶ�����������
    std::shared_ptr<const c_parser_frontend::operator_node::VarietyOperatorNode>
        variety_operator_node;
  };
  // �洢������Ϣ������
  using ArgumentInfoContainer = std::vector<ArgumentInfo>;

 public:
  FunctionType(const std::string* function_name)
      : TypeInterface(StructOrBasicType::kFunction, EndType::GetEndType()),
        function_name_(function_name) {}
  template <class ArgumentContainter>
  FunctionType(const std::string* function_name,
               ArgumentContainter&& argument_infos)
      : TypeInterface(StructOrBasicType::kFunction, EndType::GetEndType()),
        function_name_(function_name),
        argument_infos_(std::forward<ArgumentContainter>(argument_infos)) {}
  template <class ArgumentContainter>
  FunctionType(const std::string* function_name, ConstTag return_type_const_tag,
               const std::shared_ptr<const TypeInterface>& return_type,
               ArgumentContainter&& argument_infos)
      : TypeInterface(StructOrBasicType::kFunction, EndType::GetEndType()),
        function_name_(function_name),
        return_type_const_tag_(return_type_const_tag),
        return_type_(return_type),
        argument_infos_(std::forward<ArgumentContainter>(argument_infos)) {}
  ~FunctionType();

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }
  virtual size_t TypeSizeOf() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }

  // �����Ƿ�Ϊ�������������������в������κ����̿�����䣩
  bool IsFunctionAnnounce() const;
  // ֻ��麯��ǩ���Ƿ���ͬ������麯�����ͺ�����ִ�е����
  bool IsSameSignature(const FunctionType& function_type) const {
    return GetFunctionName() == function_type.GetFunctionName() &&
           GetArguments() == function_type.GetArguments() &&
           GetReturnTypeReference() == function_type.GetReturnTypeReference();
  }
  // ���ȽϺ�����ִ�е����
  bool IsSameObject(const TypeInterface& type_interface) const;

  void SetReturnTypePointer(
      const std::shared_ptr<const TypeInterface>& return_type) {
    return_type_ = return_type;
  }
  std::shared_ptr<const TypeInterface> GetReturnTypePointer() const {
    return return_type_;
  }
  const TypeInterface& GetReturnTypeReference() const { return *return_type_; }
  void SetReturnTypeConstTag(ConstTag return_type_const_tag) {
    return_type_const_tag_ = return_type_const_tag;
  }
  ConstTag GetReturnTypeConstTag() const { return return_type_const_tag_; }
  // �����Ƿ���ӳɹ���������ɹ������
  void AddFunctionCallArgument(
      const std::shared_ptr<
          const c_parser_frontend::operator_node::VarietyOperatorNode>&
          argument);
  const ArgumentInfoContainer& GetArguments() const { return argument_infos_; }
  void SetFunctionName(const std::string* function_name) {
    function_name_ = function_name;
  }
  const std::string& GetFunctionName() const { return *function_name_; }
  const std::list<std::unique_ptr<FlowInterface>>& GetSentences() const;
  // ���һ��������ִ�е���䣨������˳����ӣ�
  // �ɹ���ӷ���true������������Ȩ
  // �����������򷵻�false�����޸����
  bool AddSentence(std::unique_ptr<FlowInterface>&& sentence_to_add);
  // ���һ�������ڵ�ȫ����䣨���ո���������begin->end˳����ӣ�
  // �ɹ���Ӻ���Ϊ������������
  // ����з���false��CheckSentenceInFunctionValid����򷵻�false�Ҳ��޸Ĳ���
  // �ɹ�����򷵻�true
  bool AddSentences(
      std::list<std::unique_ptr<FlowInterface>>&& sentence_container);
  // ��ȡ������һ������ָ����ʽ
  // ����shared_ptrָ��һ��ָ��ڵ㣬���ָ��ڵ�ָ��ú���
  static std::shared_ptr<const PointerType> ConvertToFunctionPointer(
      const std::shared_ptr<const TypeInterface>& function_type) {
    assert(function_type->GetType() == StructOrBasicType::kFunction);
    // ת��constΪ�˿��Թ���ָ��ڵ㣬�����޸�function_type
    return std::make_shared<const PointerType>(
        ConstTag::kConst, 0,
        std::const_pointer_cast<TypeInterface>(function_type));
  }
  // ����������Ƿ������Ϊ�����ڳ��ֵ����
  static bool CheckSentenceInFunctionValid(const FlowInterface& flow_interface);

 private:
  // ������
  // ��Ϊ����ָ��Ľڵ����ʱ��������nullptr
  const std::string* function_name_;
  // ����ֵ��const���
  ConstTag return_type_const_tag_;
  // ������������
  std::shared_ptr<const TypeInterface> return_type_;
  // �������ͺͲ�����
  ArgumentInfoContainer argument_infos_;
  // ������ִ�е����
  std::shared_ptr<std::list<std::unique_ptr<FlowInterface>>>
      sentences_in_function_;
};

// �ṹ�����ݵĻ���
class StructureTypeInterface : public TypeInterface {
 protected:
  // ��ֵΪ��Ա����ֵǰ�벿��Ϊ��Ա���ͣ���벿��Ϊ��Աconst���
  class StructureMemberContainer {
    enum class IdWrapper { kMemberIndex };

   public:
    using MemberIndex =
        frontend::common::ExplicitIdWrapper<size_t, IdWrapper,
                                            IdWrapper::kMemberIndex>;
    // ��ȡ��Աindex
    // ����MemberIndex::InvalidId()�������ڸó�Ա
    MemberIndex GetMemberIndex(const std::string& member_name) const;
    const auto& GetMemberInfo(MemberIndex member_index) const {
      assert(member_index.IsValid());
      return members_[member_index];
    }
    // ����MemberIndex���������ӳ�Ա���Ѵ����򷵻�MemberIndex::InvalidId()
    template <class MemberName>
    MemberIndex AddMember(
        MemberName&& member_name,
        const std::shared_ptr<const TypeInterface>& member_type,
        ConstTag member_const_tag);
    const auto& GetMembers() const { return members_; }

   private:
    // ��Ա����index��ӳ��
    std::unordered_map<std::string, MemberIndex> member_name_to_index_;
    // �ṹ�����ݵĳ�Ա
    std::vector<std::pair<std::shared_ptr<const TypeInterface>, ConstTag>>
        members_;
  };
  using StructureMemberIndex = StructureMemberContainer::MemberIndex;

 public:
  using MemberIndex = StructureMemberContainer::MemberIndex;

  StructureTypeInterface(const std::string* structure_name,
                         StructOrBasicType structure_type)
      : TypeInterface(structure_type, EndType::GetEndType()),
        structure_name_(structure_name) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() ==
             static_cast<const StructureTypeInterface&>(type_interface)
                 .GetNextNodeReference();
    } else {
      return false;
    }
  }

  const std::string* GetStructureName() const { return structure_name_; }
  void SetStructureName(const std::string* structure_name) {
    structure_name_ = structure_name;
  }

  void SetStructureMembers(StructureMemberContainer&& structure_members) {
    structure_member_container_ = structure_members;
  }
  // ��ȡ��Ա��Ϣ����
  const auto& GetStructureMemberContainer() const {
    return structure_member_container_;
  }
  // �����Ա�Ѵ����򷵻�StructureMemberIndex::InvalidId()
  template <class MemberName>
  StructureMemberIndex AddStructureMember(
      MemberName&& member_name,
      const std::shared_ptr<const TypeInterface>& member_type,
      ConstTag member_const_tag) {
    return GetStructureMemberContainer().AddMember(
        std::forward<MemberName>(member_name), member_type, member_const_tag);
  }
  // ��ȡ�ṹ�������ڳ�Ա��index�����������Ա���������򷵻�
  // StructMemberIndex::InvalidId()
  StructureMemberIndex GetStructureMemberIndex(
      const std::string& member_name) const {
    return GetStructureMemberContainer().GetMemberIndex(member_name);
  }
  const auto& GetStructureMemberInfo(StructureMemberIndex member_index) const {
    assert(member_index.IsValid());
    return GetStructureMemberContainer().GetMemberInfo(member_index);
  }
  // ��ȡ�ײ�洢��Ա��Ϣ�Ľṹ���������ڱ���
  const auto& GetStructureMembers() const {
    return GetStructureMemberContainer().GetMembers();
  }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  StructureMemberContainer& GetStructureMemberContainer() {
    return structure_member_container_;
  }
  // �ṹ�����ݵ�������
  const std::string* structure_name_;
  // �ṹ����������
  StructureMemberContainer structure_member_container_;
};

class StructType : public StructureTypeInterface {
 public:
  // �ṹ��洢��Ա�Ľṹ����
  // ������ע�⣺reduct_functionΪ��ʵ�ֶ�̬�ڹ���struct��unionʱʹ���˴�����
  // StructureMemberContainerType������StructMemberContainerType
  using StructMemberContainerType = StructureMemberContainer;
  using StructMemberIndex = StructMemberContainerType::MemberIndex;

  StructType(const std::string* struct_name)
      : StructureTypeInterface(struct_name, StructOrBasicType::kStruct) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    return StructureTypeInterface::operator==(type_interface);
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override;
  virtual size_t TypeSizeOf() const override {
    return StructType::GetTypeStoreSize();
  }

  template <class StructMembers>
  void SetStructMembers(StructMembers&& member_datas) {
    SetStructureMembers(std::forward<StructMembers>(member_datas));
  }
  // ����Index��������������Ѵ����򷵻�
  template <class MemberName>
  StructMemberIndex AddStructMember(
      MemberName&& member_name,
      const std::shared_ptr<const TypeInterface>& member_type_pointer,
      ConstTag member_const_tag) {
    return AddStructureMember(std::forward<MemberName>(member_name),
                              member_type_pointer, member_const_tag);
  }
  // ��ȡ�ṹ���ڳ�Ա��index�����������Ա���������򷵻�
  // StructMemberIndex::InvalidId()
  StructMemberIndex GetStructMemberIndex(const std::string& member_name) const {
    return GetStructureMemberIndex(member_name);
  }
  const auto& GetStructMemberInfo(StructMemberIndex member_index) const {
    assert(member_index.IsValid());
    return GetStructureMemberInfo(member_index);
  }
  void SetStructName(const std::string* struct_name) {
    SetStructureName(struct_name);
  }
  const std::string* GetStructName() const { return GetStructureName(); }

  bool IsSameObject(const TypeInterface& type_interface) const {
    return StructureTypeInterface::IsSameObject(type_interface);
  }
};

class UnionType : public StructureTypeInterface {
 public:
  // ������洢��Ա�Ľṹ����
  // ������ע�⣺reduct_functionΪ��ʵ�ֶ�̬�ڹ���struct��unionʱʹ���˴�����
  // StructureMemberContainerType������UnionMemberContainerType
  using UnionMemberContainerType = StructureMemberContainer;
  using UnionMemberIndex = UnionMemberContainerType::MemberIndex;

  UnionType(const std::string* union_name)
      : StructureTypeInterface(union_name, StructOrBasicType::kUnion) {}

  virtual bool operator==(const TypeInterface& type_interface) {
    return StructureTypeInterface::operator==(type_interface);
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override;
  virtual size_t TypeSizeOf() const override {
    return UnionType::GetTypeStoreSize();
  }

  template <class UnionMembers>
  void SetUnionMembers(UnionMembers&& union_members) {
    SetStructureMembers(std::forward<UnionMembers>(union_members));
  }
  // ����ָ�����ݵĵ��������Ƿ�ɹ�����
  // ���Ѵ��ڸ�����ʱ�����벢����false
  template <class MemberName>
  auto AddUnionMember(
      MemberName&& member_name,
      const std::shared_ptr<const TypeInterface>& member_type_pointer,
      ConstTag member_const_tag) {
    return AddStructureMember(std::forward<MemberName>(member_name),
                              member_type_pointer, member_const_tag);
  }
  // ��ȡ�������ڳ�Աindex�������Ա���������򷵻�UnionMemberIndex::InvalidId()
  UnionMemberIndex GetUnionMemberIndex(const std::string& member_name) const {
    return GetStructureMemberIndex(member_name);
  }
  const auto& GetUnionMemberInfo(UnionMemberIndex member_index) const {
    assert(member_index.IsValid());
    return GetStructureMemberInfo(member_index);
  }
  void SetUnionName(const std::string* union_name) {
    SetStructureName(union_name);
  }
  const std::string* GetUnionName() const { return GetStructureName(); }
  bool IsSameObject(const TypeInterface& type_interface) const {
    return StructureTypeInterface::IsSameObject(type_interface);
  }
};

class EnumType : public TypeInterface {
 public:
  // ö���ڲ��洢�ṹ
  using EnumContainerType = std::unordered_map<std::string, long long>;

  EnumType(const std::string* enum_name)
      : TypeInterface(StructOrBasicType::kEnum, EndType::GetEndType()),
        enum_name_(enum_name) {}
  template <class StructMembers>
  EnumType(const std::string* enum_name, StructMembers&& enum_members)
      : TypeInterface(StructOrBasicType::kEnum, EndType::GetEndType()),
        enum_name_(enum_name),
        enum_members_(std::forward<StructMembers>(enum_members)) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override {
    return GetContainerTypeReference().GetTypeStoreSize();
  }
  virtual size_t TypeSizeOf() const override {
    return EnumType::GetTypeStoreSize();
  }

  template <class StructMembers>
  void SetEnumMembers(StructMembers&& enum_members) {
    enum_members_ = std::forward<StructMembers>(enum_members);
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
  void SetContainerType(
      const std::shared_ptr<const TypeInterface>& container_type) {
    container_type_ = container_type;
  }
  // ö����
  const std::string* enum_name_;
  // ǰ�벿��Ϊ��Ա������벿��Ϊö�ٶ�Ӧ��ֵ
  EnumContainerType enum_members_;
  // �洢ö��ֵ�õ�����
  std::shared_ptr<const TypeInterface> container_type_;
};

class InitializeListType : public TypeInterface {
 public:
  // �����ʼ����Ϣ�õ��б�����
  using InitializeListContainerType =
      std::vector<std::shared_ptr<const TypeInterface>>;

  InitializeListType()
      : TypeInterface(StructOrBasicType::kInitializeList,
                      EndType::GetEndType()) {}
  template <class MemberType>
  InitializeListType(MemberType&& list_types)
      : list_types_(std::forward<MemberType>(list_types)) {}

  virtual bool operator==(const TypeInterface& type_interface) {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override {
    // ��ʼ���б��ܱ���ֵ
    return AssignableCheckResult::kCanNotConvert;
  }
  virtual size_t GetTypeStoreSize() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }
  virtual size_t TypeSizeOf() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }

  void AddListType(const std::shared_ptr<const TypeInterface>& type_pointer) {
    list_types_.emplace_back(type_pointer);
  }
  const auto& GetListTypes() const { return list_types_; }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  InitializeListContainerType list_types_;
};

// �������ͽڵ����������ȫ�ֹ���һ�ݽڵ��Խ�ʡ�ڴ�ͱ����η��������
class CommonlyUsedTypeGenerator {
 public:
  // ��ȡ��������
  // void���ͺ�bool����ʹ��SignTag::kUnsigned
  template <BuiltInType built_in_type, SignTag sign_tag>
  static std::shared_ptr<BasicType> GetBasicType() {
    thread_local static std::shared_ptr<BasicType> basic_type =
        std::make_shared<BasicType>(built_in_type, sign_tag,
                                    EndType::GetEndType());
    return basic_type;
  }

  static std::shared_ptr<BasicType> GetBasicTypeNotTemplate(
      BuiltInType built_in_type, SignTag sign_tag);

  // ��ȡ��ʼ�����ַ������ͣ�const char*��
  static std::shared_ptr<PointerType> GetConstExprStringType() {
    thread_local static std::shared_ptr<PointerType> constexpr_string_type =
        std::make_shared<PointerType>(
            ConstTag::kConst, 0,
            std::move(GetBasicType<BuiltInType::kInt8, SignTag::kSigned>()));
    return constexpr_string_type;
  }
};

class TypeSystem {
  // �洢ͬһ�������µĲ�ͬ������Ϣ
  class TypeData {
   public:
    // ���һ�����ͣ��Զ�����洢�ṹ��ת��
    // �������޴�ִ�����ĺ�������Ϊ�Ϸ�����
    // ģ������ں�������/����ʱ��Ҫ�趨
    template <bool is_function_announce, bool is_function_define>
    requires(is_function_announce == false ||
             is_function_define == false) AddTypeResult
        AddType(const std::shared_ptr<const TypeInterface>& type_to_add);
    // ��������ƫ�û�ȡ����
    std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
        StructOrBasicType type_prefer) const;
    // ���������Ƿ�Ϊ��
    bool Empty() const {
      return std::get_if<std::monostate>(&type_data_) != nullptr;
    }

    // ������������Ƿ�����ͬһ����
    // StructOrBasicType::kBasic��StructOrBasicType::kPointer����ͬһ����
    // �����������͵�����һ����
    static bool IsSameKind(StructOrBasicType type1, StructOrBasicType type2);
    // ����Ѵ���ͬ�����������������Ƿ������Ӻ�������/����ľ������
    // �������һ����������Ӻ�������
    template <bool is_function_announce, bool is_function_define>
    requires(is_function_announce == false ||
             is_function_define == false) static AddTypeResult
        CheckFunctionDefineAddResult(const FunctionType& function_type_exist,
                                     const FunctionType& function_type_to_add);

   private:
    // ��ʼ����ʱΪ�գ��������ָ��������ӵ�����ʹ��ֱ�Ӵ洢��vector�洢
    // ��StructOrBasicType::kBasic/kPointer���͵�ָ��ŵ���ǰ��
    // ��������ƫ��ʱ���ٲ��ң��������ȫ���洢��ָ��
    // ָ���������������ֻ��������һ�֣���ֹ����
    std::variant<
        std::monostate, std::shared_ptr<const TypeInterface>,
        std::unique_ptr<std::vector<std::shared_ptr<const TypeInterface>>>>
        type_data_;
  };
  using TypeNodeContainerType = std::unordered_map<std::string, TypeData>;

 public:
  using TypeNodeContainerIter = TypeNodeContainerType::const_iterator;
  // ������ϵͳ���������
  // type_name����������type_pointer��ָ����������ָ��
  // ����ָ�����λ�õĵ������Ͳ�����
  // ��������������´���ӵ������������Ĵ����Ѵ��������ʧ�ܣ�����false
  // �����StructOrBasicType������kEnd��kNotSpecified
  template <bool is_function_announce = false, bool is_function_define = false,
            class TypeName>
  requires(is_function_announce == false || is_function_define == false) std::
      pair<TypeNodeContainerType::const_iterator, AddTypeResult> DefineType(
          TypeName&& type_name,
          const std::shared_ptr<const TypeInterface>& type_pointer);
  // ������������ʹ�øýӿ�
  std::pair<TypeNodeContainerType::const_iterator, AddTypeResult>
  AnnounceFunctionType(
      const std::shared_ptr<const FunctionType>& function_type) {
    return DefineType<true, false>(function_type->GetFunctionName(),
                                   function_type);
  }
  // ���庯������ʹ�øýӿ�
  std::pair<TypeNodeContainerType::const_iterator, AddTypeResult>
  DefineFunctionType(const std::shared_ptr<const FunctionType>& function_type) {
    return DefineType<false, true>(function_type->GetFunctionName(),
                                   function_type);
  }

  // �������ͣ�����ָ������ֶ�Ӧ���ͼ��ϵĵ�����
  // ��֤��������Ч�����ͼ��ϱ�����
  template <class TypeName>
  TypeNodeContainerType::const_iterator AnnounceTypeName(TypeName&& type_name);
  // ���ݸ�����������ѡ�����͵������ȡ����
  // ����ѡ���������StructOrBasicType::kEnd����ѡ��
  // ����ѡ������ΪStructOrBasicType::kNotSpecifiedʱ����ƥ��kBasic
  // ��������ƽ�������ڶ��ƽ������ʱ�޷���ȡ������ָ��
  // ����������ָ��ͻ�ȡ�������ȡ����Ľ��ͼ��䶨��
  std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
      const std::string& type_name, StructOrBasicType type_prefer);
  // �Ƴ��սڵ㣬����ڵ㲻�����Ƴ�
  // �����Ƿ��Ƴ��˽ڵ�
  // ����ֻannounce�˱����������û����ӱ��������������ָ���������̣�
  bool RemoveEmptyNode(const std::string& empty_node_to_remove_name);

 private:
  auto& GetTypeNameToNode() { return type_name_to_node_; }
  // ��������������������ӳ��
  // �����ṹ������������ɸýṹ�����ı���ά��������
  // ���洢���ͬ������ʱֵ��ת��Ϊָ��vector��ָ�룬ͨ��vector����ͬ������
  // ʹ��vector����ʱ���������Ӧ��StructOrBasicType::kBasic���ͷ��ڵ�һ��λ��
  // ���Ա������ȫ��ָ�룬���ٱ�׼���Ͳ��ҹ�������ٶ�
  TypeNodeContainerType type_name_to_node_;
};

template <bool is_function_announce, bool is_function_define, class TypeName>
requires(is_function_announce == false ||
         is_function_define == false) inline std::
    pair<TypeSystem::TypeNodeContainerType::const_iterator,
         AddTypeResult> TypeSystem::
        DefineType(TypeName&& type_name,
                   const std::shared_ptr<const TypeInterface>& type_pointer) {
  assert(type_pointer->GetType() != StructOrBasicType::kEnd);
  assert(type_pointer->GetType() != StructOrBasicType::kNotSpecified);
  auto [iter, inserted] = GetTypeNameToNode().emplace(
      std::forward<TypeName>(type_name), TypeData());
  return std::make_pair(
      iter, iter->second.AddType<is_function_announce, is_function_define>(
                type_pointer));
}

template <class TypeName>
inline TypeSystem::TypeNodeContainerType::const_iterator
TypeSystem::AnnounceTypeName(TypeName&& type_name) {
  return GetTypeNameToNode()
      .emplace(std::forward<TypeName>(type_name), TypeData())
      .first;
}

template <class MemberName>
inline StructureTypeInterface::StructureMemberContainer::MemberIndex
StructureTypeInterface::StructureMemberContainer::AddMember(
    MemberName&& member_name,
    const std::shared_ptr<const TypeInterface>& member_type,
    ConstTag member_const_tag) {
  auto [iter, inserted] = member_name_to_index_.emplace(
      std::forward<MemberName>(member_name), MemberIndex(members_.size()));
  // �Ѵ��ڸ������ֵĳ�Ա
  if (!inserted) [[unlikely]] {
    return MemberIndex::InvalidId();
  } else {
    return iter->second;
  }
}
template <bool is_function_announce, bool is_function_define>
requires(is_function_announce == false || is_function_define == false)
    AddTypeResult TypeSystem::TypeData::AddType(
        const std::shared_ptr<const TypeInterface>& type_to_add) {
  std::monostate* empty_status_pointer =
      std::get_if<std::monostate>(&type_data_);
  if (empty_status_pointer != nullptr) [[likely]] {
    // �ýڵ�δ�����κ�ָ��
    // �����һ��ָ��
    type_data_ = type_to_add;
    return AddTypeResult::kNew;
  } else {
    std::shared_ptr<const TypeInterface>* shared_pointer =
        std::get_if<std::shared_ptr<const TypeInterface>>(&type_data_);
    if (shared_pointer != nullptr) [[likely]] {
      // �ýڵ㱣����һ��ָ�룬����һ����תΪvector�洢
      // ���Ҫ��ӵ������Ƿ������������ظ������ظ����kPointer/kBasicһ��
      if (IsSameKind((*shared_pointer)->GetType(), type_to_add->GetType()))
          [[unlikely]] {
        // ����Ƿ�����Ӻ�������
        if (type_to_add->GetType() == StructOrBasicType::kFunction)
            [[unlikely]] {
          AddTypeResult function_define_add_result =
              CheckFunctionDefineAddResult<is_function_announce,
                                           is_function_define>(
                  static_cast<const FunctionType&>(**shared_pointer),
                  static_cast<const FunctionType&>(*type_to_add));
          if (function_define_add_result == AddTypeResult::kAbleToAdd)
              [[likely]] {
            // ʹ���µ�ָ���滻ԭ��ָ�룬�Ӷ����º����������ͺ����ڴ洢�����
            *shared_pointer = type_to_add;
            function_define_add_result = AddTypeResult::kFunctionDefine;
          }
          return function_define_add_result;
        } else {
          // ����ӵ�����������������ͬ��������ͻ
          return AddTypeResult::kTypeAlreadyIn;
        }
      }
      auto vector_pointer =
          std::make_unique<std::vector<std::shared_ptr<const TypeInterface>>>();
      // ��StructOrBasicType::kBasic��StructOrBasicType::kPointer���ͷ�����ǰ��
      // �Ӷ��Ż��������͵��ٶ�
      if (IsSameKind(type_to_add->GetType(), StructOrBasicType::kBasic)) {
        vector_pointer->emplace_back(type_to_add);
        vector_pointer->emplace_back(std::move(*shared_pointer));
      } else {
        // ԭ�������Ͳ�һ�����ڸô��࣬������ӵ�����һ�������ڸô���
        vector_pointer->emplace_back(std::move(*shared_pointer));
        vector_pointer->emplace_back(type_to_add);
      }
      type_data_ = std::move(vector_pointer);
      return AddTypeResult::kShiftToVector;
    } else {
      // �ýڵ��Ѿ�ʹ��vector�洢
      std::unique_ptr<std::vector<std::shared_ptr<const TypeInterface>>>&
          vector_pointer = *std::get_if<std::unique_ptr<
              std::vector<std::shared_ptr<const TypeInterface>>>>(&type_data_);
      assert(vector_pointer != nullptr);
      // ������ӵ��������Ѵ��ڵ������Ƿ�����ͬһ����
      for (auto& stored_pointer : *vector_pointer) {
        if (IsSameKind(stored_pointer->GetType(), type_to_add->GetType()))
            [[unlikely]] {
          // ����Ƿ�����Ӻ�������
          if (type_to_add->GetType() == StructOrBasicType::kFunction)
              [[unlikely]] {
            AddTypeResult function_define_add_result =
                CheckFunctionDefineAddResult<is_function_announce,
                                             is_function_define>(
                    static_cast<const FunctionType&>(*stored_pointer),
                    static_cast<const FunctionType&>(*type_to_add));
            if (function_define_add_result == AddTypeResult::kAbleToAdd)
                [[likely]] {
              // �������Ϊ�ں�������ǰ����Ӻ�������
              // ʹ���µ�ָ���滻ԭ��ָ��
              stored_pointer = type_to_add;
              return AddTypeResult::kFunctionDefine;
            }
            return function_define_add_result;
          }
          // ����ӵ�����������������ͬ��������ͻ
          return AddTypeResult::kTypeAlreadyIn;
        }
      }
      vector_pointer->emplace_back(type_to_add);
      if (IsSameKind(type_to_add->GetType(), StructOrBasicType::kBasic)) {
        // �²��������ΪStructOrBasicType::kBasic/kPointer
        // �������ͷŵ���һ��λ���Ա��Ż�������������ʱ�Ĳ����߼�
        std::swap(vector_pointer->front(), vector_pointer->back());
      }
      return AddTypeResult::kAddToVector;
    }
  }
}

// ����Ѵ���ͬ�����������������Ƿ������Ӻ�������/����ľ������
// �������һ����������Ӻ�������

template <bool is_function_announce, bool is_function_define>
requires(is_function_announce == false || is_function_define == false)
    AddTypeResult TypeSystem::TypeData::CheckFunctionDefineAddResult(
        const FunctionType& function_type_exist,
        const FunctionType& function_type_to_add) {
  if constexpr (is_function_announce) {
    // �Ѿ�����ͬ����������/����
    assert(function_type_to_add.IsFunctionAnnounce());
    if (function_type_exist.IsSameSignature(function_type_to_add)) {
      return AddTypeResult::kAnnounceOrDefineBeforeFunctionAnnounce;
    } else {
      return AddTypeResult::kTypeAlreadyIn;
    }
  } else if constexpr (is_function_define) {
    assert(!function_type_to_add.IsFunctionAnnounce());
    // ������Ӻ������Ѵ��ڵĺ��������Ƿ�Ϊ��ͬǩ��
    if (function_type_exist.IsSameSignature(function_type_to_add)) [[likely]] {
      // ���֮ǰ���ڵ�����Ϊ������������Ը��Ǻ�������
      if (function_type_exist.IsFunctionAnnounce()) [[likely]] {
        return AddTypeResult::kAbleToAdd;
      } else {
        // ֮ǰ�Ѵ��ں������壬�����ض������
        return AddTypeResult::kRedefineFunction;
      }
    } else {
      // ֮ǰ����ǩ����ͬ�ĺ�������
      return AddTypeResult::kTypeAlreadyIn;
    }
  }
  assert(false);
  // ��ֹ����
  return AddTypeResult();
}

}  // namespace c_parser_frontend::type_system
#endif  // !CPARSERFRONTEND_PARSE_CLASSES_H_