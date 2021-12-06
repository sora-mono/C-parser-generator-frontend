/// @file unordered_struct_manager.h
/// @brief ����Ԫ�ؽṹ��ϣ�洢����
/// @note �����֧�ֲ�������ȫ����գ���֧��ɾ����������
#ifndef COMMON_UNORDERED_STRUCT_MANAGER_H_
#define COMMON_UNORDERED_STRUCT_MANAGER_H_

#include <unordered_set>
#include <vector>

#include "Common/id_wrapper.h"

namespace frontend::common {

/// TODO ����ʹ��std::stringʱ���ػ�
/// @brief ����stlδ�ṩhash�����Ľṹ�Ĺ�ϣ�洢,ͨ��IDһһ��ʶ����
/// @class UnorderedStructManager unordered_struct_manager.h
/// @tparam StructType ��Ҫ����Ķ�������
/// @tparam Hasher ����ϣStructType�ĺ����İ�װ��
template <class StructType, class Hasher = std::hash<StructType>>
class UnorderedStructManager {
 public:
  enum class WrapperLabel { kObjectHashType };
  /// @brief ��ʶ�����ID
  using ObjectId = ObjectManager<StructType>::ObjectId;
  /// @brief ����hash��õ�������
  using ObjectHashType =
      ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kObjectHashType>;

  UnorderedStructManager() {}
  ~UnorderedStructManager() {}

  /// @brief ����ָ�����Ķ��������
  /// @param[in] object_id ��Ҫ��ȡ�Ķ����ID
  /// @return ����object_id��Ӧ�Ķ��������
  /// @attention ��α����Ӧ���ڵĶ���
  StructType& GetObject(ObjectId object_id) {
    return const_cast<StructType&>(
        static_cast<const UnorderedStructManager&>(*this).GetObject(
            object_id));
  }
  /// @brief ����ָ�����Ķ����const����
  /// @param[in] object_id ��Ҫ��ȡ�Ķ����ID
  /// @return ����object_id��Ӧ�Ķ����const����
  /// @attention ��α����Ӧ���ڵĶ���
  const StructType& GetObject(ObjectId object_id) const {
    return *id_to_object_[object_id];
  }
  /// @brief ����һ��������Ķ���
  /// @param[in,out] args ������������Ķ���Ĳ���
  /// @return ǰ�벿��Ϊ����ID����벿��Ϊ�Ƿ�ִ���˲������
  /// @note ����ṩconst��ֵ������Ḵ��һ�ݶ���洢��
  /// ����ṩ��ֵ��������ƶ�����ö���
  /// �������������Ѵ�����᷵���Ѵ��ڵĶ����ID
  /// @attention ���಻�������Ĳ���������������Ǹ��ݴ������������¶���
  template <class... Args>
  std::pair<ObjectId, bool> EmplaceObject(Args&&... args);
  /// @brief ͨ��ָ������ָ���ȡ�����ID
  /// @param[in] object_pointer ��ָ�򣨴洢��node_manager_�еģ������ָ��
  /// @return ���ض����ID
  /// @retval ObjectId::InvalidId() �����󲻴���
  ObjectId GetObjectIdFromObjectPointer(const StructType* object_pointer) const;
  /// @brief ���ݶ����ȡ��ID
  /// @param[in] object ��Ҫ��ȡID�Ķ���
  /// @return ��ȡ���Ķ���ID
  /// @retval ObjectId::InvalidId() ���ö��󲻴���
  ObjectId GetObjectIdFromObject(const StructType& object) const;
  /// @brief ��ʼ������
  /// @note ��������д��ڶ�����ȫ���ͷ�
  void StructManagerInit() {
    node_manager_.clear();
    id_to_object_.clear();
  }
  /// @brief ���ݶ���ID��ȡ��������
  /// @param[in] object_id ��Ҫ��ȡ���õĶ���ID
  /// @return ���ػ�ȡ���Ķ�������
  /// @note ����ͬGetObject
  /// @attention ��α����Ӧ���ڵĶ���
  StructType& operator[](ObjectId object_id) {
    return GetObject(object_id);
  }
  /// @brief ���ݶ���ID��ȡ����const����
  /// @param[in] object_id ��Ҫ��ȡ���õĶ���ID
  /// @return ���ػ�ȡ���Ķ���const����
  /// @note ����ͬGetObject
  /// @attention ��α����Ӧ���ڵĶ���
  const StructType& operator[](ObjectId object_id) const {
    return GetObject(object_id);
  }

 private:
  /// @brief �洢���й���Ķ���
  std::unordered_set<StructType, Hasher> node_manager_;
  /// @brief �洢����Ķ���ĵ�ַ��ID��ӳ��
  std::unordered_map<const StructType*, ObjectId> object_pointer_to_id_;
  /// @brief �洢ID��ָ�����Ķ���ĵ�����ӳ��
  std::vector<typename std::unordered_set<StructType, Hasher>::iterator>
      id_to_object_;
};

template <class StructType, class Hasher>
template <class... Args>
inline std::pair<typename UnorderedStructManager<StructType, Hasher>::ObjectId,
                 bool>
UnorderedStructManager<StructType, Hasher>::EmplaceObject(Args&&... args) {
  auto [iter, inserted] = node_manager_.emplace(std::forward<Args>(args)...);
  if (!inserted) [[unlikely]] {
    return std::make_pair(GetObjectIdFromObjectPointer(&*iter), false);
  } else {
    ObjectId object_id(id_to_object_.size());
    id_to_object_.emplace_back(std::move(iter));
    object_pointer_to_id_.emplace(&*iter, object_id);
    return std::make_pair(object_id, true);
  }
}

template <class StructType, class Hasher>
UnorderedStructManager<StructType, Hasher>::ObjectId
UnorderedStructManager<StructType, Hasher>::GetObjectIdFromObject(
    const StructType& object) const {
  auto iter = node_manager_.find(object);
  if (iter == node_manager_.end()) {
    return ObjectId::InvalidId();
  } else {
    return GetObjectIdFromObjectPointer(&*iter);
  }
}
template <class StructType, class Hasher>
inline UnorderedStructManager<StructType, Hasher>::ObjectId
UnorderedStructManager<StructType, Hasher>::GetObjectIdFromObjectPointer(
    const StructType* object_pointer) const {
  auto iter = object_pointer_to_id_.find(object_pointer);
  if (iter != object_pointer_to_id_.end()) {
    return iter->second;
  } else {
    return ObjectId::InvalidId();
  }
}

}  // namespace frontend::common
#endif  /// !COMMON_UNORDERED_STRUCT_MANAGER