/// @file multimap_object_manager.h
/// @brief �������֧�ֶ���ĺϲ���������ID��Ӧһ������
/// @details
/// ���ļ�����Ķ����������ObjectManager��������ObjectManagerID�����һһ��Ӧ
/// ���MultimapObjectManagerID���Զ�Ӧһ������
/// @attention ����ʹ����ObjectManager��ʹ�ø�ͷ�ļ�ʱ��Ҫ��boost������
#ifndef COMMON_MULTIMAP_OBJECT_MANAGER_H_
#define COMMON_MULTIMAP_OBJECT_MANAGER_H_

#include <assert.h>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "Common/id_wrapper.h"
#include "Common/object_manager.h"

namespace frontend::common {

/// @class MultimapObjectManager multimap_object_manager.h
/// @brief �������������ID��Ӧһ������
/// @details
/// ��ObjectManager������֧�ֶ���ĺϲ����ܣ����ܹ��Զ�����ָ��ö����IDָ��
/// �µĶ���ID��Ӧ�Ķ���������ϲ����̻�ϲ�ʧ��ʱ��֤ID�����һһ��Ӧ
template <class T>
class MultimapObjectManager {
 private:
  /// @brief ObjectManager��ʹ�õ�ID���ڲ�ID��
  using InsideId = ObjectManager<T>::ObjectId;

 public:
  /// @brief ��������MultimapObjectManagerID��ö��
  enum class WrapperLabel { kObjectId };
  /// @brief MultimapObjectManagerID�����ͣ��ⲿID��
  using ObjectId =
      ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kObjectId>;
  /// @brief �������ĵ�����
  using Iterator = ObjectManager<T>::Iterator;
  /// @brief ��������const������
  using ConstIterator = ObjectManager<T>::ConstIterator;

  MultimapObjectManager() {}
  MultimapObjectManager(const MultimapObjectManager&) = delete;
  MultimapObjectManager(MultimapObjectManager&&) = delete;
  ~MultimapObjectManager() {}

  /// @brief ��ȡָ����Ч���������
  /// @param[in] object_id ��Ҫ��ȡ�Ķ����ID
  /// @return ���ػ�ȡ���Ķ��������
  T& GetObject(ObjectId object_id);
  /// @brief ��ȡָ����Ч�����const����
  /// @param[in] object_id ��Ҫ��ȡ�Ķ����ID
  /// @return ���ػ�ȡ���Ķ����const����
  /// @attention ��ζ�Ӧ��ID�������
  const T& GetObject(ObjectId object_id) const;
  /// @brief ͨ��һ��id��ѯ������ͬ�ײ���������id
  /// @param[in] object_id ��MultimapObjectManagerID
  /// @return ��������ָ��ö����ID
  const std::unordered_set<ObjectId>& GetIdsReferringSameObject(
      ObjectId object_id) const {
    InsideId inside_id = GetInsideId(object_id);
    return GetIdsReferringSameObject(inside_id);
  }

  /// @brief �ж�����ID�Ƿ�ָ��ͬһ����
  /// @param[in] id1 ��һ��Ҫ�жϵ�ID
  /// @param[in] id2 ����һ��Ҫ�жϵ�ID
  /// @note ����ʹ����ͬ���
  bool IsSame(ObjectId id1, ObjectId id2) const;

  /// @brief ���������Զ�ѡ�����λ�÷��ö���
  /// @param[in] args �����󹹽�����Ҫ�����в���
  /// @return ���ش����Ķ����ID
  template <class... Args>
  ObjectId EmplaceObject(Args&&... args);
  /// @details ��ɾ�������¼�����ͷ�ָ��ָ����ڴ�
  /// @param[in] object_id ��Ҫ�ͷŵĶ���ID
  /// @return ����ָ������ָ��
  /// @attention ���뱣֤������ڣ������Ժ��Է���ֵ������ᵼ���ڴ�й©
  [[nodiscard]] T* RemoveObjectNoDelete(ObjectId object_id);
  /// @details ɾ�������ͷŶ����ڴ�
  /// @param[in] object_id ��Ҫɾ���Ķ����ID
  /// @retval true ��һ������true
  /// @attention Ҫɾ���Ķ���������
  bool RemoveObject(ObjectId object_id);
  /// @brief ��id_src��Ӧ�Ľڵ�ϲ���id_dst��Ӧ�Ľڵ�
  /// @param[in] id_dst ���ϲ������Ľڵ��ID
  /// @param[in] id_src ���ϲ���ɾ���Ľڵ��ID
  /// @param[in] merge_function ���������ƺϲ����̵ĺ���
  /// ��һ������Ϊid_dst��Ӧ�Ķ��󣬵ڶ�������Ϊid_src��Ӧ�Ķ���
  /// @return �����Ƿ�ϲ��ɹ�
  /// @retval ��true �ϲ��ɹ�
  /// @retval ��false �ϲ�ͬһ�������merge_function������false
  /// @details
  /// ����ϲ��ɹ���ɾ��id_src��Ӧ�Ľڵ㲢��ӳ��id_srcָ��id_dstָ��Ľڵ�
  /// ����ϲ�ͬһ�������merge_function������false�򲻻�ɾ���ڵ�Ҳ������ӳ��
  /// @attention
  /// �ϲ�ͬһ������ʱ������merge_functionֱ�ӷ���false
  bool MergeObjects(ObjectId id_dst, ObjectId id_src,
                    std::function<bool(T&, T&)> merge_function =
                        ObjectManager<T>::DefaultMergeFunction2);
  /// @brief ��id_src��Ӧ�Ľڵ�ϲ���id_dst��Ӧ�Ľڵ㣬�������������
  /// @param[in] id_dst ���ϲ������Ľڵ��ID
  /// @param[in] id_src ���ϲ���ɾ���Ľڵ��ID
  /// @param[in,out] manager ������ϲ��Ķ����ڵ���merge_functionʱ����
  /// @param[in] merge_function ���������ƺϲ����̵ĺ���
  /// ��һ������Ϊid_dst��Ӧ�Ķ��󣬵ڶ�������Ϊid_src��Ӧ�Ķ���
  /// ����������Ϊ����ϲ����̵Ĺ�����
  /// @return �����Ƿ�ϲ��ɹ�
  /// @retval ��true �ϲ��ɹ�
  /// @retval ��false �ϲ�ͬһ�������merge_function������false
  /// @details
  /// ����ϲ��ɹ���ɾ��id_src��Ӧ�Ľڵ㲢��ӳ��id_srcָ��id_dstָ��Ľڵ�
  /// ����ϲ�ͬһ�������merge_function������false�򲻻�ɾ���ڵ�Ҳ������ӳ��
  /// @attention
  /// �ϲ�ͬһ������ʱ������merge_functionֱ�ӷ���false
  template <class Manager>
  bool MergeObjectsWithManager(
      ObjectId id_dst, ObjectId id_src, Manager& manager,
      const std::function<bool(T&, T&, Manager&)>& merge_function =
          ObjectManager<T>::DefaultMergeFunction3);
  /// @brief ���������������
  /// @param[in,out] manager_other ��ָ����һ����������ָ��
  /// @note ������this�����ǻ�ִ�������Ľ�������
  /// @attention ������ʹ�ÿ�ָ��
  void Swap(MultimapObjectManager* manager_other);
  /// @brief ���ö�������ϲ���־
  /// @param[in] object_id ��Ҫ��������ϲ��Ķ����ID
  /// @attention ��α����Ӧ�Ѵ��ڵĶ���
  bool SetObjectCanBeSourceInMerge(ObjectId object_id);
  /// @brief ���ö����ֹ�ϲ���־
  /// @param[in] object_id ��Ҫ���ý�ֹ�ϲ��Ķ����ID
  /// @attention ��α����Ӧ�Ѵ��ڵĶ���
  bool SetObjectCanNotBeSourceInMerge(ObjectId object_id);
  /// @brief �������ж�������ϲ�
  /// @attention �����δ�������Ŀ�Ѩ����������
  void SetAllObjectsCanBeSourceInMerge() {
    node_manager_.SetAllObjectsCanBeSourceInMerge();
  }
  /// @brief �������ж����ֹ�ϲ�
  /// @note ���ж��󣨰���δ�������Ŀ�Ѩ���ᱻ���ã�
  void SetAllObjectsCanNotBeSourceInMerge() {
    node_manager_.SetAllObjectsCanNotBeSourceInMerge();
  }
  /// @brief ������ζ�Ӧ�����Ƿ�ɺϲ�
  /// @param[in] object_id ����Ҫ��ѯ�Ƿ�ɺϲ��Ķ���
  /// @attention ��α����Ӧ���ڵĶ���
  bool CanBeSourceInMerge(ObjectId object_id) const;

  /// @brief ��ʼ������������д��ڶ�����ȫ���ͷ�
  void MultimapObjectManagerInit();
  /// @brief ������ж��󵫲��ͷ�
  /// @note �������Ա�������ͷų�Առ�õ��ڴ�
  void ClearNoRelease();
  /// @brief ���ó�Ա������shrink_to_fit�����ڴ�ʹ����
  void ShrinkToFit();

  /// @brief ��ȡ�����ܴ洢�Ķ������
  /// @return ���������ܴ洢�Ķ������
  size_t Capacity() const { return node_manager_.Size(); }
  /// @brief ����������ʵ�ʴ洢����Ч�������
  /// @return ����������ʵ�ʴ洢����Ч�������
  size_t ItemSize() const { return node_manager_.ItemSize(); }

  /// @brief ���л�����
  /// @param[in] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);

  /// @brief ��ȡָ��������һ������ĵ�����
  /// @return ����ָ��������һ������ĵ�����
  /// @note �����һ�����󲻴����򷵻�End()
  Iterator Begin() const { return node_manager_.Begin(); }
  /// @brief ����ָ��������β�ĵ�����
  Iterator End() const { return node_manager_.End(); }
  /// @brief ��ȡָ��β��const������
  /// @return ����ָ��β��const������
  ConstIterator ConstEnd() const { return node_manager_.ConstEnd(); }
  /// @brief ��ȡָ���һ����Ч�ڵ��const������
  /// @return ����ָ���һ����Ч�ڵ��const������
  /// @attention �����������Ч�ڵ��򷵻�ConstEnd()
  ConstIterator ConstBegin() const { return node_manager_.ConstBegin(); }

  /// @brief ʹ�ö����ID��ȡ��������
  /// @param[in] object_id ��Ҫ��ȡ���õĶ���ID
  /// @return ���ػ�ȡ���Ķ�������
  /// @attention ��ζ�Ӧ�Ķ���������
  T& operator[](ObjectId object_id);
  /// @brief ���ö����ID��ȡ����const����
  /// @param[in] object_id ��Ҫ��ȡconst���õĶ���ID
  /// @return ���ػ�ȡ���Ķ�������
  /// @attention ��ζ�Ӧ�Ķ���������
  const T& operator[](ObjectId object_id) const;

 private:
  /// @brief ͨ��һ��ID��ѯ����������ͬ�ײ�����ID����������
  /// @param[in] inside_id ��node_manager_ʹ�õ�ID
  /// @return ���ش洢����������ͬ�ײ�����ID�ļ���
  const std::unordered_set<ObjectId>& GetIdsReferringSameObject(
      InsideId inside_id) const {
    return objectids_referring_same_object.at(inside_id);
  }
  /// @brief ��object_idָ��Ķ�����ڲ�ID��ӳ��Ϊinside_idָ��Ķ���
  /// @param[in] object_id ��Ҫ��ӳ����ⲿID
  /// @param[in] inside_id �����ڲ�ID
  /// @note ���Խ���δӳ���object_id
  /// @attention inside_id����ָ����ڵĶ���
  bool RemapId(ObjectId object_id, InsideId inside_id);
  /// @brief ����һ��δ��ʹ�õ��ⲿID
  /// @return �������ɵ��ⲿID
  /// @attention ��������������������洢�������ػ�
  ObjectId CreateId();
  /// @brief ����һ���ⲿID���ڲ���������ü�¼
  /// @param[in] object_id ��ʹ�õ��ⲿID
  /// @param[in] inside_id ��ָ����Ч������ڲ�ID
  /// @return �������Ӻ�inside_idָ��Ķ�����ⲿID���ü���
  /// @attention inside_id����ָ����ڵĶ���
  size_t AddReference(ObjectId object_id, InsideId inside_id);
  /// @brief �Ƴ�һ���ⲿID���ڲ���������ü�¼
  /// @param[in] object_id ��ʹ�õ��ⲿID
  /// @param[out] inside_id ��ָ����Ч������ڲ�ID
  /// @return ����ɾ�����ú��������ü���
  /// @note ������δ���ø��ڲ�������ⲿID
  /// @attention inside_id����ָ����ڵĶ������ü���Ϊ0ʱ���Զ�ɾ���ö���
  size_t RemoveReference(ObjectId object_id, InsideId inside_id);
  /// @brief �����Ƴ��ڲ�����ǰ�Ĳ��裬��ɾ�����о�����ײ�ID������
  /// @param[in] inside_id ��Ҫ�Ƴ��Ķ�����ڲ�ID
  /// @return һ������true
  /// @attention �ú��������Ƴ��ײ����
  bool PreRemoveInsideObject(InsideId inside_id);
  /// @brief ��ȡ�ⲿID��Ӧ���ڲ�ID
  /// @param[in] object_id ���ⲿID
  /// @return �����ⲿID��Ӧ���ڲ�ID
  /// @attention �ⲿID����ָ����ڵĶ���
  InsideId GetInsideId(ObjectId object_id) const;

  /// @brief ��һ��id���ֵ��ֻ������
  ObjectId next_id_index_ = ObjectId(0);
  /// @brief ����������������
  ObjectManager<T> node_manager_;
  /// @brief ���������ڲ�ID��Ӧ������ⲿID
  std::unordered_map<InsideId, std::unordered_set<ObjectId>>
      objectids_referring_same_object;
  /// @brief �洢�ⲿID���ڲ�ID��ӳ��
  std::unordered_map<ObjectId, InsideId> id_to_index_;
};

template <class T>
inline typename MultimapObjectManager<T>::InsideId
MultimapObjectManager<T>::GetInsideId(ObjectId object_id) const {
  auto iter = id_to_index_.find(object_id);
  assert(iter != id_to_index_.end());
  return iter->second;
}

template <class T>
inline T& MultimapObjectManager<T>::GetObject(ObjectId object_id) {
  InsideId inside_id = GetInsideId(object_id);
  return node_manager_.GetObject(inside_id);
}
template <class T>
inline const T& MultimapObjectManager<T>::GetObject(
    ObjectId object_id) const {
  InsideId inside_id = GetInsideId(object_id);
  return node_manager_.GetObject(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::IsSame(ObjectId id1, ObjectId id2) const {
  InsideId inside_id1 = GetInsideId(id1);
  InsideId inside_id2 = GetInsideId(id2);
  return inside_id1 == inside_id2;
}

template <class T>
template <class... Args>
inline MultimapObjectManager<T>::ObjectId
MultimapObjectManager<T>::EmplaceObject(Args&&... args) {
  ObjectId object_id = CreateId();
  InsideId inside_id = node_manager_.EmplaceObject(std::forward<Args>(args)...);
  AddReference(object_id, inside_id);
  id_to_index_[object_id] = inside_id;
  return object_id;
}

template <class T>
inline bool MultimapObjectManager<T>::SetObjectCanBeSourceInMerge(
    ObjectId object_id) {
  InsideId inside_id = GetInsideId(object_id);
  return node_manager_.SetObjectCanBeSourceInMerge(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::SetObjectCanNotBeSourceInMerge(
    ObjectId object_id) {
  InsideId inside_id = GetInsideId(object_id);
  return node_manager_.SetObjectCanNotBeSourceInMerge(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::CanBeSourceInMerge(
    ObjectId object_id) const {
  InsideId index = GetInsideId(object_id);
  return node_manager_.CanBeSourceInMerge(index);
}

template <class T>
inline bool MultimapObjectManager<T>::MergeObjects(
    ObjectId id_dst, ObjectId id_src,
    std::function<bool(T&, T&)> merge_function) {
  InsideId inside_id_dst = GetInsideId(id_dst);
  InsideId inside_id_src = GetInsideId(id_src);
  if (!node_manager_.MergeObjects(inside_id_dst, inside_id_src,
                                  merge_function)) {
    return false;
  }
  RemapId(id_src, inside_id_dst);
  return true;
}

template <class T>
template <class Manager>
inline bool MultimapObjectManager<T>::MergeObjectsWithManager(
    typename MultimapObjectManager<T>::ObjectId id_dst,
    typename MultimapObjectManager<T>::ObjectId id_src, Manager& manager,
    const std::function<bool(T&, T&, Manager&)>& merge_function) {
  InsideId inside_id_dst = GetInsideId(id_dst);
  InsideId inside_id_src = GetInsideId(id_src);
  if (!node_manager_.MergeObjectsWithManager(inside_id_dst, inside_id_src,
                                             manager, merge_function)) {
    return false;
  }
  RemapId(id_src, inside_id_dst);
  return true;
}

template <class T>
inline bool MultimapObjectManager<T>::PreRemoveInsideObject(
    InsideId inside_id) {
  assert(inside_id < node_manager_.Size());
  const auto& ids_referring_same_inside_id =
      GetIdsReferringSameObject(inside_id);
  if (ids_referring_same_inside_id.size() != 0) {
    for (ObjectId object_id : ids_referring_same_inside_id) {
      auto iter = id_to_index_.find(object_id);
      id_to_index_.erase(iter);
    }
    objectids_referring_same_object[inside_id].clear();
  }
  return true;
}

template <class T>
inline T* MultimapObjectManager<T>::RemoveObjectNoDelete(
    ObjectId object_id) {
  InsideId inside_id = GetInsideId(object_id);
  PreRemoveInsideObject(inside_id);
  return node_manager_.RemovePointer(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::RemoveObject(
    ObjectId object_id) {
  InsideId inside_id = GetInsideId(object_id);
  PreRemoveInsideObject(inside_id);
  node_manager_.RemoveObject(inside_id);
  return true;
}

template <class T>
inline void MultimapObjectManager<T>::Swap(
    MultimapObjectManager* manager_other) {
  std::swap(next_id_index_, manager_other->next_id_index_);
  node_manager_.Swap(manager_other->node_manager_);
  objectids_referring_same_object.Swap(
      manager_other->objectids_referring_same_object);
  id_to_index_.Swap(manager_other->id_to_index_);
}

template <class T>
inline bool MultimapObjectManager<T>::RemapId(ObjectId object_id,
                                              InsideId inside_id) {
  assert(inside_id < node_manager_.Size());
  InsideId inside_id_old = GetInsideId(object_id);
  AddReference(object_id, inside_id);
  id_to_index_[object_id] = inside_id;
  return true;
}

template <class T>
inline MultimapObjectManager<T>::ObjectId MultimapObjectManager<T>::CreateId() {
  ObjectId return_id = next_id_index_;
  next_id_index_ = ObjectId(return_id + 1);
  return return_id;
}

template <class T>
inline size_t MultimapObjectManager<T>::AddReference(
    ObjectId object_id, InsideId inside_id) {
  assert(inside_id < node_manager_.Size());
  objectids_referring_same_object[inside_id].insert(object_id);
  return objectids_referring_same_object[inside_id].size();
}

template <class T>
inline size_t MultimapObjectManager<T>::RemoveReference(
    ObjectId object_id, InsideId inside_id) {
  assert(inside_id < node_manager_.Size());
  auto& ref_uset = objectids_referring_same_object[inside_id];
  auto iter = ref_uset.find(object_id);
  if (iter != ref_uset.end()) [[likely]] {
    if (ref_uset.size() == 1) {
      node_manager_.RemoveObject(inside_id);
    }
    ref_uset.erase(iter);
  }
  return ref_uset.size();
}

template <class T>
inline void MultimapObjectManager<T>::MultimapObjectManagerInit() {
  next_id_index_ = ObjectId(0);
  node_manager_.ObjectManagerInit();
  objectids_referring_same_object.clear();
  id_to_index_.clear();
}

template <class T>
inline void MultimapObjectManager<T>::ClearNoRelease() {
  next_id_index_ = ObjectId(0);
  node_manager_.ClearNoRelease();
  objectids_referring_same_object.clear();
  id_to_index_.clear();
}

template <class T>
inline void MultimapObjectManager<T>::ShrinkToFit() {
  node_manager_.ShrinkToFit();
  objectids_referring_same_object.shrink_to_fit();
}

template <class T>
inline T& MultimapObjectManager<T>::operator[](ObjectId object_id) {
  return GetObject(object_id);
}
template <class T>
inline const T& MultimapObjectManager<T>::operator[](
    ObjectId object_id) const {
  return GetObject(object_id);
}

}  // namespace frontend::common
#endif  /// !COMMON_COMMON_MULTIMAP_NODE_MANAGER