/// @file type_system.h
/// @brief ����ϵͳ
/// @details
/// 1.����ϵͳͨ��������������ʾһ�����������
/// 2.��ʾ���͵���С��λΪ�ڵ㣬һ���ڵ������һ��ָ��(����)������ͷ��
///   �������ͣ���������/�Զ���ṹ���ȣ��ڵ�ͨ������������һ���õ�������
/// 3.����������˳��ͬ��������ʱ��Լ��˳��ͷ���������������Ĳ���
/// 4.����������β��������һ���ڱ�
/// 5.����������
///       const double* (*func(unsigned int x, const short y))(float z);
/// ע��argument_infos_���ڼ򻯣����������Ͳ���
///
/// ********************************                      [0].variety_type_
/// * type_ =                      *argument_infos_*****************************
/// * StructOrBasicType::kFunction *==============>* type_ =                   *
/// * next_type_node_ =            *               * StructOrBasicType::kBasic *
/// *       EndType::GetEndType()  *               * next_type_node_ =         *
/// * function_name_ = "func"      *               *     EndType::GetEndType() *
/// * return_type_const_tag_ =     *               * built_in_type_ =          *
/// *         ConstTag::kNonConst  *               *       BuiltInType::kInt32 *
/// ********************************               * sign_tag_ =               *
///             ��                                 *        SignTag::kUnSigned *
///             ��return_type_                     *****************************
///             ��                                       [1].variety_type_
/// ********************************               *****************************
/// * type_ =                      *               * type_ =                   *
/// * StructOrBasicType::kPointer  *               * StructOrBasicType::kBasic *
/// * array_size_ = 0              *               * next_type_node_ =         *
/// * variety_const_tag_ =         *               *     EndType::GetEndType() *
/// *          ConstTag::kNonConst *               * built_in_type_ =          *
/// ********************************               *       BuiltInType::kInt16 *
///              ��                                * sign_tag_ =               *
///              ��next_type_node_                 *          SignTag::kSigned *
///              ��                                *****************************
/// ********************************
/// * type_ =                      *                    [0].variety_type_
/// *  StructOrBasicType::kFunction*argument_infos_*****************************
/// * next_type_node =             *==============>* type_ =                   *
/// *        EndType::GetEndType() *               * StructOrBasicType::kBasic *
/// * function_name_ = ""          *               * next_type_node_ =         *
/// * return_type_const_tag_ =     *               *     EndType::GetEndType() *
/// *             ConstTag::kConst *               * built_in_type_ =          *
/// ********************************               *     BuiltInType::kFloat32 *
///              ��                                * sign_tag_ =               *
///              ��return_type_                    *          SignTag::kSigned *
///              ��                                *****************************
/// ********************************
/// * type_ =                      *
/// *  StructOrBasicType::kPointer *
/// * array_size_ = 0              *
/// * variety_const_tag_ =         *
/// *          ConstTag::kNonConst *
/// ********************************
///              ��
///              ��next_type_node_
///              ��
/// ********************************
/// * type_ =                      *
/// * StructOrBasicType::kBasic    *
/// * next_type_node_ =            *
/// *       EndType::GetEndType()  *
/// * built_in_type_ =             *
/// *       BuiltInType::kFloat64  *
/// * sign_tag_ = SignTag::kSigned *
/// ********************************
///
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

namespace c_parser_frontend::type_system {
/// @brief const���
enum class ConstTag { kNonConst, kConst };
/// @brief ���ű��
enum class SignTag { kSigned, kUnsigned };

/// @brief ���ı�������
enum class StructOrBasicType {
  kBasic,           ///< ��������
  kPointer,         ///< ָ�루*��
  kFunction,        ///< ����
  kStruct,          ///< �ṹ��
  kUnion,           ///< ������
  kEnum,            ///< ö��
  kInitializeList,  ///< ��ʼ���б�
  kEnd,             ///< ����ϵͳ��β�������Ż��ж��߼�
  kNotSpecified  ///< δȷ�����ͣ�ʹ�ñ�׼���Ͳ��ҹ������ڲ������ƶ�Ӧ�ı���
};
/// @brief ���ñ�������
/// @details
/// void���ͽ�������Ϊ�����ķ������ͣ����������б�������Ϊָ��
/// C����û����ʽ֧��bool��bool��Ӧ�����������жϽ����
/// void���ں�������ֵ�п�����Ϊ������֣��������Ϊָ�����͵���һ���ڵ�
/// kBuiltInTypeSize�洢����������ռ�ռ��С
/// @attention ����ά�ֵ�ֵö���������ʽת��Ϊ��ֵ��ԭ������������ڼ����ж�
/// һ�������Ƿ������ʽת��Ϊ��һ������
enum class BuiltInType {
  kInt1,     /// bool
  kInt8,     /// char
  kInt16,    /// short
  kInt32,    /// int,long
  kFloat32,  /// float
  kFloat64,  /// double
  kVoid      /// void
};
/// @brief ����Ƿ���Ը�ֵ�Ľ��
enum class AssignableCheckResult {
  /// ���Ը�ֵ�����
  kNonConvert,    /// ����������ͬ������ת�����ɸ�ֵ
  kUpperConvert,  /// ������ֵ������ת��Ϊ������ֵ����
  kConvertToVoidPointer,  /// ����ֵ�Ķ���Ϊvoidָ�룬��ֵ�Ķ���Ϊͬ��ά��ָ��
  kZeroConvertToPointer,  /// 0ֵת��Ϊָ��
  kUnsignedToSigned,  /// �޷���ֵ��ֵ���з���ֵ������ʹ�þ���
  kSignedToUnsigned,  /// �з���ֵ��ֵ���޷���ֵ������ʹ�þ���
  /// ��Ҫ�����жϵ����
  kMayBeZeroToPointer,  /// ����ǽ�0��ֵ��ָ����Ϸ�������Ƿ�
  kInitializeList,  /// ʹ�ó�ʼ���б���Ҫ��ϸ���ֵ�����ж�
                    /// ����CanBeAssignedBy�з���
  /// ���ɸ�ֵ�����
  kLowerConvert,         /// ������ֵ�����ͷ�����խת��
  kCanNotConvert,        /// ����ת��
  kAssignedNodeIsConst,  /// ����ֵ����Ϊconst
  kAssignToRightValue,  /// ����ֵ����Ϊ��ֵ��C���Բ�����ֵ����ֵ
  kArgumentsFull,  /// ��������������������Ӹ������
  kInitializeListTooLarge  /// ��ʼ���б��и�����������Ŀ����ָ������ʱ��С
};
/// @brief �ƶ���ѧ�������͵Ľ��
enum class DeclineMathematicalComputeTypeResult {
  /// ���Լ�������
  kComputable,              /// ���Լ���
  kLeftPointerRightOffset,  /// ������Ϊָ�룬������Ϊƫ����
  kLeftOffsetRightPointer,  /// ������Ϊƫ������������Ϊָ��
  kConvertToLeft,           /// ���������͵������ͺ�����
  kConvertToRight,          /// ���������͵������ͺ�����
  /// �����Լ�������
  kLeftNotComputableType,   /// �����Ͳ��ǿ��Լ��������
  kRightNotComputableType,  /// �����Ͳ��ǿ��Լ��������
  kLeftRightBothPointer,    /// �������;�Ϊָ��
  kLeftNotIntger,  /// ��������ָ�룬�����Ͳ���������������Ϊƫ����ʹ��
  kRightNotIntger  /// ��������ָ�룬�����Ͳ���������������Ϊƫ����ʹ��
};
/// @brief ������͵Ľ��
enum class AddTypeResult {
  /// �ɹ���ӵ����
  kAbleToAdd,  /// ������ӣ�����CheckFunctionDefineAddResult��ʹ��
               /// ���շ���ʱ��ת��Ϊ�������ֵ
  kNew,        /// ��ǰ�����ڸ�����������kFunctionDefine���ȷ���
  kFunctionDefine,  /// ����˺�������
  kShiftToVector,  /// ���ǰ����������Ӧһ�����ͣ���Ӻ�ת��Ϊvector�洢
  kAddToVector,  /// ���ǰ����������Ӧ����2������
  kAnnounceOrDefineBeforeFunctionAnnounce,  /// ��������ǰ��������/����
  /// ���ʧ�ܵ����
  kTypeAlreadyIn,  /// ����ӵ��������������Ѿ���һ��ͬ������
  kDoubleAnnounceFunction,  /// �����Ѿ�����/�����ĺ���
  kOverrideFunction         /// ��ͼ���غ���
};
/// @brief �������ƻ�ȡ���͵Ľ��
enum class GetTypeResult {
  /// �ɹ�ƥ������
  kSuccess,  /// �ɹ�ƥ��
  /// ƥ��ʧ�ܵ����
  kTypeNameNotFound,   /// ������������
  kNoMatchTypePrefer,  /// ������ƥ������ѡ�������������ָ��
  kSeveralSameLevelMatches  /// ƥ�䵽���ƽ�����
};

/// @brief ָ�����ʹ�С
constexpr size_t kPointerSize = 4;
/// @brief �������ʹ�С����λ���ֽڣ�
constexpr size_t kBuiltInTypeSize[7] = {1, 1, 2, 4, 4, 8, 4};

/// @brief ����洢����ֵ�������С����
/// @param[in] value ��������洢������С���͵�ֵ
/// @return ���ش洢����ֵ�������С����
/// @retval BuiltInType::kVoid ��value�������ɴ洢ֵ��С
/// @note
/// �����ƶϳ�BuiltInType::kInt1���ͣ���С�ƶϳ�BuiltInType::kInt8
BuiltInType CalculateBuiltInType(const std::string& value);

// ǰ������ָ������࣬����TypeInterface::ObtainAddress����ָ������
class PointerType;

/// @class TypeInterface type_system.h
/// @brief ���ͻ���
/// @note �������;��Ӹû�������
class TypeInterface {
 public:
  TypeInterface(StructOrBasicType type) : type_(type){};
  TypeInterface(StructOrBasicType type,
                const std::shared_ptr<const TypeInterface>& next_type_node)
      : type_(type), next_type_node_(next_type_node) {}
  virtual ~TypeInterface() {}

  /// @brief �Ƚ����������Ƿ���ȫ��ͬ
  /// @param[in] type_interface �����Ƚϵ���������
  /// @return �������������Ƿ���ȫ��ͬ
  /// @retval true ������������ȫ��ͬ
  /// @retval false ���������Ͳ���ȫ��ͬ
  /// @note �Զ�������һ���ȽϺ���
  virtual bool operator==(const TypeInterface& type_interface) const = 0;
  /// @brief �жϸ��������Ƿ���Ը��ö���ֵ
  /// @param[in] type_interface �����жϵ�����
  /// @return ���ؼ����
  /// @note ����ֵ��������䶨��
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const = 0;
  /// @brief ��ȡ�洢�����ͱ�������ռ�Ĵ�С
  /// @return ���ش洢�����ͱ�������ռ��С
  /// @note ָ���ΪkPointerSize�ֽ�
  virtual size_t GetTypeStoreSize() const = 0;
  /// @brief ��ȡsizeof���������͵Ĵ�С
  /// @return ����sizeof���������͵Ĵ�С
  /// @note ָ�������ָ����Ϊ���ʹ�С���Ը�ά�ȴ�С
  virtual size_t TypeSizeOf() const = 0;

  /// @note operator==()���ӹ��̣����Ƚϸü����ͣ���������һ���ȽϺ���
  /// @param[in] type_interface �����Ƚϵ�����
  /// @details
  /// �������Ӧ��д�ú������ȵ��û����ͬ�����������ж���������������
  /// �Ӷ�ʵ��Ϊ����һ�����������ҿ��԰�ȫ�������ĺ���
  /// ��ʹ���麯��������������
  /// @note if��֧�࣬��������
  bool IsSameObject(const TypeInterface& type_interface) const {
    /// һ�������д���룬��Ҫ�Ƚϵ����Ͷ�����ͬ�ģ�����Զ������ȷ
    return GetType() == type_interface.GetType();
  }

  /// @brief �������ͽڵ������
  /// @param[in] type �����ͽڵ������
  void SetType(StructOrBasicType type) { type_ = type; }
  /// @brief ��ȡ���ͽڵ������
  /// @return �������ͽڵ������
  StructOrBasicType GetType() const { return type_; }

  /// @brief ��ȡ��һ�����ͽڵ��ָ��
  /// @return ����ָ����һ�����ͽڵ��constָ��
  std::shared_ptr<const TypeInterface> GetNextNodePointer() const {
    return next_type_node_;
  }
  /// @brief ������һ���ڵ�
  /// @param[in] next_type_node ��ָ����һ���ڵ��constָ��
  void SetNextNode(const std::shared_ptr<const TypeInterface>& next_type_node) {
    next_type_node_ = next_type_node;
  }
  /// @brief ��ȡ��һ���ڵ������
  /// @return ������һ���ڵ��const����
  const TypeInterface& GetNextNodeReference() const { return *next_type_node_; }

  /// @brief ��ȡ��������ȡ��ַ��õ�������
  /// @param[in] type_interface ��ȡ��ַǰ������
  /// @param[in] variety_const_tag ����ȡ��ַ�ı��������const���
  /// @return ����ȡ��ַ�������
  static std::shared_ptr<const PointerType> ObtainAddressOperatorNode(
      const std::shared_ptr<const TypeInterface>& type_interface,
      ConstTag variety_const_tag) {
    // ת��type_interface��constΪ�˹���PointerType�������޸ĸ�ָ��ָ�������
    return std::make_shared<PointerType>(variety_const_tag, 0, type_interface);
  }
  /// @brief �ƶ��������ͽ�����ѧ�����õ������ͺ��������
  /// @param[in] left_compute_type ������������������
  /// @param[in] right_compute_type ��������Ҳ���������
  /// @return ǰ�벿��Ϊ�����õ������ͣ���벿��Ϊ�������
  /// @note �������Ͳ�һ���������㣬�����ȼ���ƶϽ��
  static std::pair<std::shared_ptr<const TypeInterface>,
                   DeclineMathematicalComputeTypeResult>
  DeclineMathematicalComputeResult(
      const std::shared_ptr<const TypeInterface>& left_compute_type,
      const std::shared_ptr<const TypeInterface>& right_compute_type);

 private:
  /// @brief ���ͽڵ������
  StructOrBasicType type_;
  /// @brief ָ����һ�����ͽڵ��ָ��
  /// @details
  /// �����á�ȡ��ַ��������ͬһ����������ͬ���֣����ýڵ���Խ�ʡ�ڴ�
  /// ʹ��shared_ptr��֧�ֹ����������ڵ�ͽ��ͷ����������ֽڵ�Ĺ���
  std::shared_ptr<const TypeInterface> next_type_node_;
};

/// @brief �ȽϽڵ�ָ��ָ��Ľڵ��Ƿ���ͬ
/// @param[in] type_interface1 ��һ�����ͽڵ�
/// @param[in] type_interface2 ����һ�����ͽڵ�
/// @return ���������ڵ��Ƿ���ȫ��ͬ
/// @retval true �������ڵ���ȫ��ͬ
/// @retval false �������ڵ㲻��ȫ��ͬ
/// @note ����FunctionType�ȽϺ��������б��ȽϽڵ���ǱȽ�ָ��ڵ��ָ��
bool operator==(const std::shared_ptr<TypeInterface>& type_interface1,
                const std::shared_ptr<TypeInterface>& type_interface2);

/// @class EndType type_system.h
/// @brief ������β���ڱ��ڵ�
/// @details ����Ƚ��������Ƿ���ͬʱ��Ҫ�ж���һ���ڵ�ָ���Ƿ�Ϊ��
/// @note �ýڵ����һ���ڵ�Ϊnullptr
class EndType : public TypeInterface {
 public:
  EndType() : TypeInterface(StructOrBasicType::kEnd, nullptr) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    // EndType�в��������һ�����ͽڵ��operator==()
    // �������ǽ�β��Ҳû����һ�����ͽڵ�
    return IsSameObject(type_interface);
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override {
    return AssignableCheckResult::kNonConvert;
  }
  /// @attention �ú�����Ӧ������
  virtual size_t GetTypeStoreSize() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }
  /// @attention �ú�����Ӧ������
  virtual size_t TypeSizeOf() const override {
    assert(false);
    // ��ֹ����
    return size_t();
  }

  /// @brief ��ȡEndType�ڵ�ָ��
  /// @return ����ָ��ȫ�ֹ��õ�EndType�ڵ��constָ��
  /// @note ȫ�ֹ���һ���ڵ㣬��ʡ�ڴ�
  static std::shared_ptr<const EndType> GetEndType() {
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

/// @class BasicType type_system.h
/// @brief Ԥ����Ļ�������
/// @details �ýڵ�����ӵ���EndType�ڵ�
class BasicType : public TypeInterface {
 public:
  BasicType(BuiltInType built_in_type, SignTag sign_tag)
      : TypeInterface(StructOrBasicType::kBasic, EndType::GetEndType()),
        built_in_type_(built_in_type),
        sign_tag_(sign_tag) {}

  virtual bool operator==(const TypeInterface& type_interface) const override;
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override;
  virtual size_t TypeSizeOf() const override {
    return BasicType::GetTypeStoreSize();
  }

  /// @brief ������������
  /// @param[in] built_in_type �������õ���������
  void SetBuiltInType(BuiltInType built_in_type) {
    built_in_type_ = built_in_type;
  }
  /// @brief ��ȡ��������
  /// @return ������������
  BuiltInType GetBuiltInType() const { return built_in_type_; }
  /// @brief �����������͵ķ������ԣ��з���/�޷��ţ�
  /// @param[in] sign_tag �����ű��
  void SetSignTag(SignTag sign_tag) { sign_tag_ = sign_tag; }
  /// @brief ��ȡ�������͵ķ��ű��
  /// @return ���ط��ű��
  SignTag GetSignTag() const { return sign_tag_; }

  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  /// @brief ��������
  BuiltInType built_in_type_;
  /// @brief ���ű��
  SignTag sign_tag_;
};

/// ָ�����ͣ�һ��*��Ӧһ���ڵ�
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

  virtual bool operator==(const TypeInterface& type_interface) const override;
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override { return kPointerSize; }
  virtual size_t TypeSizeOf() const override;

  /// @brief ����ָ��ָ��Ķ����const״̬
  /// @param[in] const_tag ��const���
  /// @note �ñ�Ǳ�ʾ�����ú�õ��������Ƿ�����޸�
  void SetConstTag(ConstTag const_tag) { variety_const_tag_ = const_tag; }
  /// @brief ��ȡָ��ָ��Ķ����const״̬
  /// @return ���ؽ����ú�õ��Ķ����const���
  /// @note �ñ�Ǳ�ʾ�����ú�õ��������Ƿ�����޸�
  ConstTag GetConstTag() const { return variety_const_tag_; }
  /// @brief ����ָ��ָ��������С
  /// @param[in] array_size �������С
  /// @details
  /// 0 ����ָ��
  /// -1 �������ݺ���������ʼ���б���д�����С
  /// ��������СΪarray_size������
  void SetArraySize(size_t array_size) { array_size_ = array_size; }
  /// @brief ��ȡָ��ָ��������С
  /// @return ����ָ��ָ��������С
  /// @retval 0 ����ָ��
  /// @retval -1 �������ݺ���������ʼ���б���д�����С
  /// @retval ��������СΪarray_size������
  size_t GetArraySize() const { return array_size_; }
  bool IsSameObject(const TypeInterface& type_interface) const;
  /// @brief ��ȡ��ָ������ú�õ�������
  /// @return ǰ�벿��Ϊ�����ú�����ͣ���벿��Ϊ�����õõ��Ķ����Ƿ�Ϊconst
  std::pair<std::shared_ptr<const TypeInterface>, ConstTag> DeReference()
      const {
    return std::make_pair(GetNextNodePointer(), GetConstTag());
  }

 private:
  /// @brief �����ú�Ķ����Ƿ�Ϊconst���
  ConstTag variety_const_tag_;
  /// @brief ָ��ָ��������ж������
  /// 0 ����ָ��
  /// -1 �������ݺ���������ʼ���б���д�����С�����������У�
  /// ���� ����СΪarray_size_������
  size_t array_size_;
};

/// @class FunctionType type_system.h
/// @brief ��������
class FunctionType : public TypeInterface {
  /// @class ArgumentInfo type_system.h
  /// @brief �洢����������Ϣ
  struct ArgumentInfo {
    bool operator==(const ArgumentInfo& argument_info) const;

    /// @brief ָ���������ָ�벻�ȶ�
    /// @details
    /// �����������������������
    /// ���������ڶ�����������
    std::shared_ptr<const c_parser_frontend::operator_node::VarietyOperatorNode>
        variety_operator_node;
  };
  /// @brief �洢������Ϣ������
  using ArgumentInfoContainer = std::vector<ArgumentInfo>;

 public:
  FunctionType(const std::string& function_name)
      : TypeInterface(StructOrBasicType::kFunction, EndType::GetEndType()),
        function_name_(function_name) {}
  template <class ArgumentContainter>
  FunctionType(const std::string& function_name,
               ArgumentContainter&& argument_infos)
      : TypeInterface(StructOrBasicType::kFunction, EndType::GetEndType()),
        function_name_(function_name),
        argument_infos_(std::forward<ArgumentContainter>(argument_infos)) {}
  template <class ArgumentContainter>
  FunctionType(const std::string& function_name, ConstTag return_type_const_tag,
               const std::shared_ptr<const TypeInterface>& return_type,
               ArgumentContainter&& argument_infos)
      : TypeInterface(StructOrBasicType::kFunction, EndType::GetEndType()),
        function_name_(function_name),
        return_type_const_tag_(return_type_const_tag),
        return_type_(return_type),
        argument_infos_(std::forward<ArgumentContainter>(argument_infos)) {}

  virtual bool operator==(const TypeInterface& type_interface) const override;
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
  bool IsSameObject(const TypeInterface& type_interface) const {
    // ��������Ψһ��ֻ��ָ��ͬһ��FunctionType�������ͬһ������
    return this == &type_interface;
  }
  /// @brief �Ƚ���������ǩ���Ƿ���ͬ
  /// @param[in] function_type �������Ƚϵ���һ������
  /// @return ������������ǩ���Ƿ���ͬ
  /// @retval true ����������ǩ����ͬ
  /// @retval false ����������ǩ����ͬ
  /// @details
  /// ��������������͵ĺ��������������������������Ƿ���ͬ
  /// @note ֻ��麯��ǩ���Ƿ���ͬ������麯����ִ�е����
  bool IsSameSignature(const FunctionType& function_type) const {
    return GetFunctionName() == function_type.GetFunctionName() &&
           GetArguments() == function_type.GetArguments() &&
           GetReturnTypeReference() == function_type.GetReturnTypeReference();
  }
  /// @brief ���ú�����������
  /// @param[in] return_type ��ָ������������������ͷ����ָ��
  void SetReturnTypePointer(
      const std::shared_ptr<const TypeInterface>& return_type) {
    return_type_ = return_type;
  }
  /// @brief ��ȡ�����ķ���������
  /// @return ����ָ�����ķ���������ͷ����ָ��
  std::shared_ptr<const TypeInterface> GetReturnTypePointer() const {
    return return_type_;
  }
  /// @brief ��ȡ�����ķ���������ͷ�������
  /// @return ���غ����ķ���������ͷ����const����
  const TypeInterface& GetReturnTypeReference() const { return *return_type_; }
  /// @brief ���ú����������͵�const���
  /// @param[in] return_type_const_tag �������������͵�const���
  /// @details
  /// �����Ծ������صĶ����Ƿ񲻿��޸�
  void SetReturnTypeConstTag(ConstTag return_type_const_tag) {
    return_type_const_tag_ = return_type_const_tag;
  }
  /// @brief ��ȡ�������ض����const���
  /// @return ���غ������ض����const���
  ConstTag GetReturnTypeConstTag() const { return return_type_const_tag_; }
  /// @brief ��Ӻ�������
  /// @param[in] argument �����ݲ������ͺͲ����������Ĳ��������ڵ�
  void AddFunctionCallArgument(
      const std::shared_ptr<
          const c_parser_frontend::operator_node::VarietyOperatorNode>&
          argument);
  /// @brief ��ȡ�洢�����ڵ������
  /// @return ���ش洢�����ڵ��������const����
  const ArgumentInfoContainer& GetArguments() const { return argument_infos_; }
  /// @brief ���ú�����
  /// @param[in] function_name ��������
  void SetFunctionName(const std::string& function_name) {
    function_name_ = function_name;
  }
  /// @brief ��ȡ������
  /// @return ���غ�������const����
  const std::string& GetFunctionName() const { return function_name_; }
  /// @brief ��ȡ�������͵ĺ���ָ����ʽ
  /// @param[in] function_type ������ȡָ����ʽ�ĺ������ͽڵ�
  /// @return ���غ�����һ�غ���ָ����ʽ
  /// @details
  /// ����һ��constָ��ڵ㣬���ָ��ڵ�ָ��function_type�ڵ�
  static std::shared_ptr<const PointerType> ConvertToFunctionPointer(
      const std::shared_ptr<const TypeInterface>& function_type) {
    assert(function_type->GetType() == StructOrBasicType::kFunction);
    /// ת��constΪ�˿��Թ���ָ��ڵ㣬�����޸�function_type
    return std::make_shared<const PointerType>(ConstTag::kConst, 0,
                                               function_type);
  }

 private:
  /// @brief ������
  std::string function_name_;
  /// @brief ����ֵ��const���
  ConstTag return_type_const_tag_;
  /// @brief ������������
  std::shared_ptr<const TypeInterface> return_type_;
  /// @brief �������ͺͲ�����
  ArgumentInfoContainer argument_infos_;
};

/// @class StructureTypeInterface type_system.h
/// @brief �ṹ�����ͣ��ṹ��/�����壩�Ļ���
/// @note ö�ٲ��Ӹ�������
class StructureTypeInterface : public TypeInterface {
 protected:
  /// @class StructureMemberContainer type_system.h
  /// @brief �洢�ṹ�����ͳ�Ա
  /// @details
  /// ��ֵΪ��Ա����ֵǰ�벿��Ϊ��Ա���ͣ���벿��Ϊ��Աconst���
  class StructureMemberContainer {
    enum class IdWrapper { kMemberIndex };

   public:
    /// @brief �洢��Ա��Ϣ������
    using MemberContainer =
        std::vector<std::pair<std::shared_ptr<const TypeInterface>, ConstTag>>;
    /// @brief ��Ա�ڽṹ�������е��±꣬���ϵ��µ���
    /// @details
    /// struct {
    ///   int x;    // �±꣺0
    ///   char y;   // �±꣺1
    ///   double z; // �±꣺2
    /// }
    using MemberIndex =
        frontend::common::ExplicitIdWrapper<size_t, IdWrapper,
                                            IdWrapper::kMemberIndex>;
    /// @brief ��ѯ��Ա�ڽṹ�������е��±�
    /// @param[in] member_name ����Ա��
    /// @return ���س�Ա�ڽṹ�������е��±�
    /// @retval MemberIndex::InvalidId() �������ڸó�Ա
    MemberIndex GetMemberIndex(const std::string& member_name) const;
    /// @brief ���ݳ�Ա�±��ȡ��Ա��Ϣ
    /// @param[in] member_index ����Ա�±�
    /// @return ǰ�벿��Ϊ��Ա���ͣ���벿��Ϊ��Ա��const���
    /// @note member_index������Ч
    const std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&
    GetMemberInfo(MemberIndex member_index) const {
      assert(member_index.IsValid());
      assert(member_index < members_.size());
      return members_[member_index];
    }
    /// @brief ��ӳ�Ա
    /// @param[in] member_name ����Ա��
    /// @param[in] member_type ����Ա����
    /// @param[in] member_const_tag ����Ա��const���
    /// @return ���س�Ա���±�
    /// @retval MemberIndex::InvalidId() ������ӳ�Ա���Ѵ���
    template <class MemberName>
    MemberIndex AddMember(
        MemberName&& member_name,
        const std::shared_ptr<const TypeInterface>& member_type,
        ConstTag member_const_tag);
    /// @brief ��ȡ�洢��Ա��Ϣ������
    /// @return ���ش洢��Ա��Ϣ��������const����
    const auto& GetMembers() const { return members_; }

   private:
    /// @brief ��Ա�����±��ӳ��
    std::unordered_map<std::string, MemberIndex> member_name_to_index_;
    /// @brief �洢��Ա������const���
    MemberContainer members_;
  };

  StructureTypeInterface(const std::string& structure_name,
                         StructOrBasicType structure_type)
      : TypeInterface(structure_type, EndType::GetEndType()),
        structure_name_(structure_name) {}

 public:
  using MemberIndex = StructureMemberContainer::MemberIndex;

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() ==
             static_cast<const StructureTypeInterface&>(type_interface)
                 .GetNextNodeReference();
    } else {
      return false;
    }
  }

  /// @brief ��ȡ�ṹ��������
  /// @return ���ؽṹ����������const����
  const std::string& GetStructureName() const { return structure_name_; }
  /// @brief ���ýṹ��������
  /// @param[in] structure_name ���ṹ��������
  void SetStructureName(const std::string& structure_name) {
    structure_name_ = structure_name;
  }
  /// @brief ���������ýṹ�����͵ĳ�Ա����
  /// @tparam MemberContainer ��StructureMemberContainer�Ĳ�ͬ����
  /// @param[in] structure_members ���洢�ṹ�����ͳ�Ա���ݵ�����
  template <class MemberContainer>
  requires std::is_same_v<std::decay_t<MemberContainer>,
                          StructureMemberContainer>
  void SetStructureMembers(MemberContainer&& structure_members) {
    structure_member_container_ = structure_members;
  }
  /// @brief ��ȡ�ṹ�����ʹ洢��Ա��Ϣ������
  /// @return ���ؽṹ�����ʹ洢��Ա��Ϣ��������const����
  const auto& GetStructureMemberContainer() const {
    return structure_member_container_;
  }
  /// @brief ��ӽṹ�����͵ĳ�Ա
  /// @param[in] member_name ����Ա��
  /// @param[in] member_type ����Ա����
  /// @param[in] member_const_tag ����Ա��const���
  /// @return ���س�Ա���±�
  /// @retval MemberIndex::InvalidId() ������ӳ�Ա���Ѵ���
  /// @note �����г�Ա�б��β�����
  template <class MemberName>
  MemberIndex AddStructureMember(
      MemberName&& member_name,
      const std::shared_ptr<const TypeInterface>& member_type,
      ConstTag member_const_tag) {
    return GetStructureMemberContainer().AddMember(
        std::forward<MemberName>(member_name), member_type, member_const_tag);
  }
  /// @brief ��ѯ�ṹ�����ͳ�Ա���±�
  /// @param[in] member_name ����Ա��
  /// @return ���ؽṹ�����ͳ�Ա���±�
  /// @retval StructMemberIndex::InvalidId() ��������Ա��������
  MemberIndex GetStructureMemberIndex(const std::string& member_name) const {
    return GetStructureMemberContainer().GetMemberIndex(member_name);
  }
  /// @brief ��ȡ�ṹ�����ͳ�Ա����Ϣ
  /// @param[in] member_index ����Ա�±�
  /// @return ǰ�벿��Ϊ��Ա���ͣ���벿��Ϊ��Աconst���
  /// @note member_index������Ч
  const std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&
  GetStructureMemberInfo(MemberIndex member_index) const {
    assert(member_index.IsValid());
    return GetStructureMemberContainer().GetMemberInfo(member_index);
  }
  /// @brief ��ȡ�洢��Ա��Ϣ������
  /// @return ���ش洢��Ա��Ϣ��������const����
  const StructureMemberContainer::MemberContainer& GetStructureMembers() const {
    return GetStructureMemberContainer().GetMembers();
  }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  /// @brief ��ȡ�洢��Ա��Ϣ������
  /// @return ���ش洢��Ա��Ϣ������������
  StructureMemberContainer& GetStructureMemberContainer() {
    return structure_member_container_;
  }
  /// @brief �ṹ��������
  std::string structure_name_;
  /// @brief �ṹ�����ͳ�Ա����
  StructureMemberContainer structure_member_container_;
};

/// @class StructType type_system.h
/// @brief �ṹ������
class StructType : public StructureTypeInterface {
 public:
  /// @brief �洢��Ա�Ľṹ����
  using StructMemberContainerType = StructureMemberContainer;
  /// @brief ��Ա�������е��±�
  using StructMemberIndex = StructMemberContainerType::MemberIndex;

  StructType(const std::string& struct_name)
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

  /// @brief ���������ýṹ���Ա��Ϣ
  /// @tparam StructMembers ��StructMemberContainerType��ͬ����
  /// @param[in] members ����Ա��Ϣ
  template <class StructMembers>
  requires std::is_same_v<std::decay_t<StructMembers>,
                          StructMemberContainerType>
  void SetStructMembers(StructMembers&& members) {
    SetStructureMembers(std::forward<StructMembers>(members));
  }
  /// @brief ��ӽṹ���Ա
  /// @param[in] member_name ����Ա��
  /// @param[in] member_type_pointer ����Ա����
  /// @param[in] member_const_tag ����Ա��const���
  /// @return ���س�Ա���±�
  /// @retval MemberIndex::InvalidId() ������ӳ�Ա���Ѵ���
  /// @note �����г�Ա�б��β�����
  template <class MemberName>
  StructMemberIndex AddStructMember(
      MemberName&& member_name,
      const std::shared_ptr<const TypeInterface>& member_type_pointer,
      ConstTag member_const_tag) {
    return AddStructureMember(std::forward<MemberName>(member_name),
                              member_type_pointer, member_const_tag);
  }
  /// @brief ��ѯ��Ա���±�
  /// @param[in] member_name ����Ա��
  /// @return ���س�Ա�±�
  /// @retval StructMemberIndex::InvalidId() ������Ա��������
  StructMemberIndex GetStructMemberIndex(const std::string& member_name) const {
    return GetStructureMemberIndex(member_name);
  }
  /// @brief ���ݳ�Ա�±��ȡ��Ա��Ϣ
  /// @param[in] member_index ����Ա�±�
  /// @return ǰ�벿��Ϊ��Ա���ͣ���벿��Ϊ��Ա��const���
  /// @note member_index������Ч
  const std::pair<std::shared_ptr<const TypeInterface>, ConstTag>&
  GetStructMemberInfo(StructMemberIndex member_index) const {
    assert(member_index.IsValid());
    return GetStructureMemberInfo(member_index);
  }
  /// @brief ���ýṹ����
  /// @param[in] struct_name ���ṹ����
  /// @note ����ʹ�ÿ������Ա�ʾ�����ṹ��
  void SetStructName(const std::string& struct_name) {
    SetStructureName(struct_name);
  }
  /// @brief ��ȡ�ṹ����
  /// @return ���ؽṹ������const����
  const std::string& GetStructName() const { return GetStructureName(); }

  bool IsSameObject(const TypeInterface& type_interface) const {
    return StructureTypeInterface::IsSameObject(type_interface);
  }
};

class UnionType : public StructureTypeInterface {
 public:
  /// @brief �洢��Ա�Ľṹ����
  using UnionMemberContainerType = StructureMemberContainer;
  /// @brief ��Ա�������е��±�
  using UnionMemberIndex = UnionMemberContainerType::MemberIndex;

  UnionType(const std::string& union_name)
      : StructureTypeInterface(union_name, StructOrBasicType::kUnion) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    return StructureTypeInterface::operator==(type_interface);
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override;
  virtual size_t GetTypeStoreSize() const override;
  virtual size_t TypeSizeOf() const override {
    return UnionType::GetTypeStoreSize();
  }

  /// @brief ���������ù������Ա��Ϣ
  /// @tparam UnionMembers ��UnionMemberContainerType��ͬ����
  /// @param[in] union_members ����Ա��Ϣ
  template <class UnionMembers>
  requires std::is_same_v<std::decay_t<UnionMembers>, UnionMemberContainerType>
  void SetUnionMembers(UnionMembers&& union_members) {
    SetStructureMembers(std::forward<UnionMembers>(union_members));
  }
  /// @brief ��ӹ������Ա
  /// @param[in] member_name ����Ա��
  /// @param[in] member_type_pointer ����Ա����
  /// @param[in] member_const_tag ����Ա��const���
  /// @return ���س�Ա���±�
  /// @retval MemberIndex::InvalidId() ������ӳ�Ա���Ѵ���
  /// @note �����г�Ա�б��β�����
  template <class MemberName>
  auto AddUnionMember(
      MemberName&& member_name,
      const std::shared_ptr<const TypeInterface>& member_type_pointer,
      ConstTag member_const_tag) {
    return AddStructureMember(std::forward<MemberName>(member_name),
                              member_type_pointer, member_const_tag);
  }
  /// @brief ��ѯ��Ա���±�
  /// @param[in] member_name ����Ա��
  /// @return ���س�Ա�±�
  /// @retval StructMemberIndex::InvalidId() ������Ա��������
  UnionMemberIndex GetUnionMemberIndex(const std::string& member_name) const {
    return GetStructureMemberIndex(member_name);
  }
  /// @brief ���ݳ�Ա�±��ȡ��Ա��Ϣ
  /// @param[in] member_index ����Ա�±�
  /// @return ǰ�벿��Ϊ��Ա���ͣ���벿��Ϊ��Ա��const���
  /// @note member_index������Ч
  const auto& GetUnionMemberInfo(UnionMemberIndex member_index) const {
    assert(member_index.IsValid());
    return GetStructureMemberInfo(member_index);
  }
  /// @brief ���ù�������
  /// @param[in] union_name ����������
  /// @note ����ʹ�ÿ������Ա�ʾ����������
  void SetUnionName(const std::string& union_name) {
    SetStructureName(union_name);
  }
  /// @brief ��ȡ��������
  /// @return ���ع���������const����
  const std::string& GetUnionName() const { return GetStructureName(); }
  bool IsSameObject(const TypeInterface& type_interface) const {
    return StructureTypeInterface::IsSameObject(type_interface);
  }
};

/// @class EnumType type_system.h
/// @brief ö������
class EnumType : public TypeInterface {
 public:
  /// @brief ö���ڲ��洢��Ա���Ͷ�Ӧֵ�Ľṹ
  using EnumContainerType = std::unordered_map<std::string, long long>;

  EnumType(const std::string& enum_name)
      : TypeInterface(StructOrBasicType::kEnum, EndType::GetEndType()),
        enum_name_(enum_name) {}
  template <class StructMembers>
  EnumType(const std::string& enum_name, StructMembers&& enum_members)
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

  /// @brief �������趨ö�ٳ�Ա
  /// @tparam EnumMembers ��EnumContainerType�Ĳ�ͬ����
  /// @param[in] enum_members ���洢ö�ٳ�Ա��Ϣ������
  template <class EnumMembers>
  requires std::is_same_v<std::decay_t<EnumMembers>, EnumContainerType>
  void SetEnumMembers(EnumMembers&& enum_members) {
    enum_members_ = std::forward<EnumMembers>(enum_members);
  }
  /// @brief ���ö�ٳ�Ա
  /// @param[in] enum_member_name ��ö�ٳ�Ա��
  /// @param[in] value ��ö�ٳ�Աֵ
  /// @return ǰ�벿��Ϊָ�����ݵĵ���������벿��Ϊ�Ƿ����
  /// @retval (...,false) enum_member_name�Ѵ���
  template <class EnumMemberName>
  auto AddEnumMember(EnumMemberName&& enum_member_name, long long value) {
    return enum_members_.emplace(
        std::make_pair(std::forward<EnumMemberName>(enum_member_name), value));
  }
  /// @brief ��ȡö�ٳ�Ա��Ϣ
  /// @param[in] member_name ����Ա��
  /// @return ǰ�벿��Ϊָ���Ա��Ϣ�ĵ���������벿��Ϊ��Ա�Ƿ����
  /// @retval (...,false) ��Ա��������
  std::pair<EnumContainerType::const_iterator, bool> GetEnumMemberInfo(
      const std::string& member_name) const {
    auto iter = GetEnumMembers().find(member_name);
    return std::make_pair(iter, iter != GetEnumMembers().end());
  }
  /// @brief ��ȡ����ö��ֵ�����Ͷ�Ӧ��������
  /// @return ����ָ��洢ö��ֵ��������ͷ����constָ��
  std::shared_ptr<const TypeInterface> GetContainerTypePointer() const {
    return container_type_;
  }
  /// @brief ��ȡ����ö��ֵ�����Ͷ�Ӧ��������
  /// @return ����ָ��洢ö��ֵ��������ͷ����const����
  const TypeInterface& GetContainerTypeReference() const {
    return *container_type_;
  }
  /// @brief ����ö����
  /// @param[in] enum_name ��ö����
  void SetEnumName(const std::string& enum_name) { enum_name_ = enum_name; }
  /// @brief ��ȡö����
  /// @return ����ö������const����
  const std::string& GetEnumName() const { return enum_name_; }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  /// @brief ��ȡ�洢��Ա������
  /// @return ���ش洢��Ա��������const����
  const EnumContainerType& GetEnumMembers() const { return enum_members_; }
  /// @brief ��ȡ�洢��Ա������
  /// @return ���ش洢��Ա������������
  EnumContainerType& GetEnumMembers() { return enum_members_; }
  /// @brief ���ô洢ö��ֵ�����Ͷ�Ӧ��������
  /// @param[in] container_type ���洢ö��ֵ�����Ͷ�Ӧ������ͷ���ָ��
  /// @note �����ʹ�õ������ܷ�洢����ö��ֵ
  void SetContainerType(
      const std::shared_ptr<const TypeInterface>& container_type) {
    container_type_ = container_type;
  }

  /// @brief ö����
  std::string enum_name_;
  /// @brief �洢ö�ٳ�Ա������
  /// @note ǰ�벿��Ϊ��Ա������벿��Ϊö�ٶ�Ӧ��ֵ
  EnumContainerType enum_members_;
  /// @brief �洢ö��ֵ������
  /// @note ����ö�ٵ�����
  std::shared_ptr<const TypeInterface> container_type_;
};

/// @class InitializeListType type_system.h
/// @brief ��ʼ���б�����
class InitializeListType : public TypeInterface {
 public:
  /// @brief �����ʼ����Ϣ���б�����
  using InitializeListContainerType =
      std::list<std::shared_ptr<const TypeInterface>>;

  InitializeListType()
      : TypeInterface(StructOrBasicType::kInitializeList,
                      EndType::GetEndType()) {}
  template <class MemberType>
  InitializeListType(MemberType&& list_types)
      : TypeInterface(StructOrBasicType::kInitializeList),
        list_types_(std::forward<MemberType>(list_types)) {}

  virtual bool operator==(const TypeInterface& type_interface) const override {
    if (IsSameObject(type_interface)) [[likely]] {
      return GetNextNodeReference() == type_interface.GetNextNodeReference();
    } else {
      return false;
    }
  }
  virtual AssignableCheckResult CanBeAssignedBy(
      const TypeInterface& type_interface) const override {
    /// ��ʼ���б��ܱ���ֵ
    return AssignableCheckResult::kCanNotConvert;
  }
  virtual size_t GetTypeStoreSize() const override {
    assert(false);
    /// ��ֹ����
    return size_t();
  }
  virtual size_t TypeSizeOf() const override {
    assert(false);
    /// ��ֹ����
    return size_t();
  }

  /// @brief ��ӳ�ʼ���б��ڵ�����
  /// @param[in] type_pointer ��ָ��������ͷ����ָ��
  /// @note ��ӵ��������͵ĺ���
  void AddListType(const std::shared_ptr<const TypeInterface>& type_pointer) {
    list_types_.emplace_back(type_pointer);
  }
  /// @brief ��ȡ��ʼ���б������е�����
  /// @return ���ش洢���͵�����
  const auto& GetListTypes() const { return list_types_; }
  bool IsSameObject(const TypeInterface& type_interface) const;

 private:
  /// @brief ��ʼ���б��е�����
  InitializeListContainerType list_types_;
};

/// @brief �������ͽڵ��������
/// @note ȫ�ֹ���һ�ݽڵ��Խ�ʡ�ڴ�ͱ����η��������
class CommonlyUsedTypeGenerator {
 public:
  /// @brief ��ȡ�������͵�������
  /// @tparam built_in_type ����������
  /// @tparam sign_tag �����͵ķ��ű��
  /// @return ����ָ��������ͷ����constָ��
  /// @details
  /// ���ص�ָ��ָ��ȫ�ֹ����һ�����ͽڵ����
  /// @note void���ͺ�bool����ʹ��SignTag::kUnsigned
  template <BuiltInType built_in_type, SignTag sign_tag>
  static std::shared_ptr<const BasicType> GetBasicType() {
    static std::shared_ptr<BasicType> basic_type =
        std::make_shared<BasicType>(built_in_type, sign_tag);
    return basic_type;
  }
  /// @brief ��ȡ�������͵�������
  /// @param[in] built_in_type ����������
  /// @param[in] sign_tag �����͵ķ��ű��
  /// @return ����ָ��������ͷ����constָ��
  /// @details
  /// ���ص�ָ��ָ��ȫ�ֹ����һ�����ͽڵ����
  /// �ú������������ڵ���ͬ��ģ�庯��
  /// @note void���ͺ�bool����ʹ��SignTag::kUnsigned
  static std::shared_ptr<const BasicType> GetBasicTypeNotTemplate(
      BuiltInType built_in_type, SignTag sign_tag);

  /// @brief ��ȡ��ʼ�����ַ������ͣ�const char*��
  /// @return ����ָ����������constָ��
  /// @note ȫ�ֹ���һ��������
  static std::shared_ptr<const PointerType> GetConstExprStringType() {
    static std::shared_ptr<PointerType> constexpr_string_type =
        std::make_shared<PointerType>(
            ConstTag::kConst, 0,
            std::move(GetBasicType<BuiltInType::kInt8, SignTag::kSigned>()));
    return constexpr_string_type;
  }
};

/// @class TypeSystem type_system.h
/// @brief ����ϵͳ
/// @details
/// 1.����ϵͳ�洢�Զ�������������������Ӧ����������ӳ��
/// 2.����һ���������󶨶�����ͣ�������������������������ȡ������ʱ��ָ������
///   �������޷���ȡ
/// 3.����TypeSystem::TypeData::IsSameKind�ж����������Ƿ�����ͬһ���࣬
///   StructOrBasicType�г���BasicType��PointerType����ͬһ�������⣬��������
///   ���Գ�һ����
/// 4.��������ӵ��������������;�������ͬһ����ʱ������ӣ�����ͬǩ���������ͣ�
/// 5.�������;���ȫ�ֶ�����
class TypeSystem {
  /// @class TypeData type_system.h
  /// @brief �洢ĳ���������µĲ�ͬ��������Ϣ
  class TypeData {
   public:
    /// @brief ���һ������
    /// @param[in] type_to_add ������ӵ�������
    /// @return ������ӽ����������ͼ�AddTypeResult����
    /// @details
    /// 1.���ĳ���������µ�һ�����ͣ������Ҫ�ӵ���ָ��ת��Ϊlist���Զ�����
    /// 2.ͬ��������ÿ�����ʹ�����������һ�����ͣ�����
    ///   TypeSystem::TypeData::IsSameKind�ж����������Ƿ�����ͬһ���࣬
    ///   StructOrBasicType�г���BasicType��PointerType����ͬһ�������⣬����
    ///   ���͸��Գ�һ����
    /// 3.ֻ�к���ǩ����ͬʱ�������ظ���Ӻ������ͣ�����ӵĺ������ͻḲ������
    ///   �ĺ�������
    AddTypeResult AddType(
        const std::shared_ptr<const TypeInterface>& type_to_add);
    /// @brief ��������ƫ�û�ȡ����
    /// @param[in] type_prefer ������ƫ��
    /// @return ǰ�벿��Ϊ��ȡ����������ͷ���ָ�룬��벿��Ϊ��ȡ���
    /// @details
    /// 1.������ƫ��ʱʹ��StructOrBasicType::kNotSpecified
    /// 2.���ж�������ʱ�����ָ��Ҫ��ȡ�������򷵻�����
    ///   StructOrBasicType::kBasic/kPointer��������ͣ������ڸô���������򷵻�
    ///   GetTypeResult::kSeveralSameLevelMatches
    /// 3.ָ�����Ͳ�����ʱ����GetTypeResult::kSeveralSameLevelMatches
    std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
        StructOrBasicType type_prefer) const;
    /// @brief ��ѯ�����Ƿ�Ϊ��
    /// @return ���������Ƿ�Ϊ��
    /// @retval true ��������
    /// @retval false ����������
    bool Empty() const {
      return std::get_if<std::monostate>(&type_data_) != nullptr;
    }

    /// @brief ������������Ƿ�����ͬһ����
    /// @param[in] type1 ��һ������
    /// @param[in] type2 ����һ������
    /// @return �������������Ƿ�����ͬһ����
    /// @retval true ��������������ͬһ����
    /// @retval false ���������Ͳ�����ͬһ����
    /// @details
    /// StructOrBasicType::kBasic��StructOrBasicType::kPointer����ͬһ����
    /// �����������͸��Գ�һ����
    static bool IsSameKind(StructOrBasicType type1, StructOrBasicType type2);
    /// @brief ����Ѵ��ں�������/�������������ͬ����������/����Ĵ������
    /// @param[in] function_type_exist ���Ѵ��ڵĺ�������
    /// @param[in] function_type_to_add ������ӵĺ�������
    /// @retval AddTypeResult::kTypeAlreadyIn �Ѵ�����ͬ�ĺ�������
    /// @retval AddTypeResult::kOverrideFunction �Ѵ��ڲ�����ͬ�ĺ�������
    /// @attention �ú����������������ֽ��
    static AddTypeResult CheckFunctionDefineAddResult(
        const FunctionType& function_type_exist,
        const FunctionType& function_type_to_add) {
      if (function_type_exist.IsSameSignature(function_type_to_add)) {
        return AddTypeResult::kTypeAlreadyIn;
      } else {
        return AddTypeResult::kOverrideFunction;
      }
    }

   private:
    /// @brief �洢ĳ�������µ�ȫ������
    /// @details
    /// ��ʼ����ʱΪ�գ��������ָ��������ӵ�����ʹ��ֱ�Ӵ洢��list�洢
    /// ��StructOrBasicType::kBasic/kPointer���͵�ָ��ŵ���ǰ��Ӷ�
    /// ��������ƫ��ʱ���ٲ��ң��������ȫ���洢��ָ��
    /// StructOrBasicType::kBasic/kPointerֻ��������һ�֣���ֹ����
    std::variant<
        std::monostate, std::shared_ptr<const TypeInterface>,
        std::unique_ptr<std::list<std::shared_ptr<const TypeInterface>>>>
        type_data_;
  };
  /// @brief �洢��������������������ӳ��
  using TypeNodeContainerType = std::unordered_map<std::string, TypeData>;

 public:
  /// @brief ָ���������Ͷ�Ӧ�������ĵ�����
  using TypeNodeContainerIter = TypeNodeContainerType::const_iterator;
  /// @brief ������ϵͳ���������
  /// @param[in] type_name ��������
  /// @param[in] type_pointer ��������ͷ���ָ��
  /// @return ǰ�벿��Ϊָ�����λ�õĵ���������벿��Ϊ��ӽ��
  /// @note ������͵Ĺ����TypeData::AddType��ע��
  /// @ref TypeData::AddType
  template <class TypeName>
  std::pair<TypeNodeContainerType::const_iterator, AddTypeResult> DefineType(
      TypeName&& type_name,
      const std::shared_ptr<const TypeInterface>& type_pointer);
  /// @brief ������������
  /// @param[in] function_type ������������ͷ���
  /// @return ǰ�벿��Ϊָ�����λ�õĵ���������벿��Ϊ��ӽ��
  std::pair<TypeNodeContainerType::const_iterator, AddTypeResult>
  AnnounceFunctionType(
      const std::shared_ptr<const FunctionType>& function_type) {
    assert(function_type->GetType() == StructOrBasicType::kFunction);
    return DefineType(function_type->GetFunctionName(), function_type);
  }
  /// @brief ���������������͵�ѡ�������ȡ����
  /// @param[in] type_name ��������
  /// @param[in] type_prefer ������ѡ������
  /// @return ǰ�벿��Ϊָ���ȡ����������ͷ���ָ�룬��벿��Ϊ��ȡ���
  /// @note ������͵Ĺ����TypeData::AddType��ע��
  /// @ref TypeData::AddType
  std::pair<std::shared_ptr<const TypeInterface>, GetTypeResult> GetType(
      const std::string& type_name, StructOrBasicType type_prefer);

 private:
  /// @brief ��ȡȫ������������������ӳ��
  /// @return ���ش洢��������������ӳ������������
  TypeNodeContainerType& GetTypeNameToNode() { return type_name_to_node_; }
  /// @brief ��������������������ӳ��
  TypeNodeContainerType type_name_to_node_;
};

template <class TypeName>
inline std::pair<TypeSystem::TypeNodeContainerType::const_iterator,
                 AddTypeResult>
TypeSystem::DefineType(
    TypeName&& type_name,
    const std::shared_ptr<const TypeInterface>& type_pointer) {
  assert(type_pointer->GetType() != StructOrBasicType::kEnd);
  assert(type_pointer->GetType() != StructOrBasicType::kNotSpecified);
  auto [iter, inserted] = GetTypeNameToNode().emplace(
      std::forward<TypeName>(type_name), TypeData());
  return std::make_pair(iter, iter->second.AddType(type_pointer));
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

}  // namespace c_parser_frontend::type_system
#endif  /// !CPARSERFRONTEND_PARSE_CLASSES_H_