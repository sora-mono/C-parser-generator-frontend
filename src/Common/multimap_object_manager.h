#include <assert.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "Common/id_wrapper.h"
#include "Common/object_manager.h"

#ifndef COMMON_MULTIMAP_OBJECT_MANAGER_H_
#define COMMON_MULTIMAP_OBJECT_MANAGER_H_

namespace frontend::common {

template <class T>
class MultimapObjectManager {
 private:
  using InsideId = ObjectManager<T>::ObjectId;

 public:
  enum class WrapperLabel { kObjectId };
  using ObjectId =
      ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kObjectId>;
  using Iterator = ObjectManager<T>::Iterator;

  MultimapObjectManager() {}
  MultimapObjectManager(const MultimapObjectManager&) = delete;
  MultimapObjectManager(MultimapObjectManager&&) = delete;
  ~MultimapObjectManager() {}
  //��ȡָ����Ч���������
  T& GetObject(ObjectId id);
  //ͨ��һ��id��ѯ������ͬ�ײ���������id
  const std::unordered_set<ObjectId>& GetIdsReferringSameObject(
      ObjectId object_id) {
    InsideId inside_id = GetInsideId(object_id);
    return GetIdsReferringSameObject(inside_id);
  }

  bool IsSame(ObjectId id1, ObjectId id2);

  //ϵͳ����ѡ�����λ�÷��ö���
  template <class... Args>
  ObjectId EmplaceObject(Args&&... args);
  //��ɾ�������ͷ�ָ��ָ����ڴ棬���ع����ָ��
  T* RemoveObjectNoDelete(ObjectId id);
  //ɾ�������ͷŶ����ڴ�
  bool RemoveObject(ObjectId id);
  //��id_src�ϲ���id_dst��
  //�ϲ��ɹ���ɾ��id_src��Ӧ�Ķ��󣨶��߲�ͬǰ���£�
  bool MergeObjects(ObjectId id_dst, ObjectId id_src,
                    std::function<bool(T&, T&)> merge_function =
                        ObjectManager<T>::DefaultMergeFunction2);
  template <class Manager>
  bool MergeObjectsWithManager(
      ObjectId id_dst, ObjectId id_src, Manager& manager,
      const std::function<bool(T&, T&, Manager&)>& merge_function =
          ObjectManager<T>::DefaultMergeFunction3);
  //���������������
  void Swap(MultimapObjectManager& manager_other);
  //���ö�������ϲ���־
  bool SetObjectMergeAllowed(ObjectId id);
  //���ö����ֹ�ϲ���־
  bool SetObjectMergeRefused(ObjectId id);
  //�������ж�������ϲ��������δ�����������������
  void SetAllObjectsMergeAllowed() {
    node_manager_.SetAllObjectsMergeAllowed();
  }
  //�������ж����ֹ�ϲ�
  void SetAllObjectsMergeRefused() {
    node_manager_.SetAllObjectsMergeRefused();
  }
  //����id��Ӧ����ɺϲ�״̬
  bool CanMerge(ObjectId id);

  //��ʱ��д���Ż��ͷź�Ŀռ�̫��
  // void remap_optimization();	//��ӳ�����еĶ�������node_manager����ռ�

  //������ͷ����ж���
  void Clear();
  //��������ͷ����ж���
  void ClearNoRelease();
  //���ó�Ա������shrink_to_fit
  void ShrinkToFit();

  //���ض��������������ɾ�����Ա���index�Ķ���
  size_t Size() { return node_manager_.Size(); }
  //����ʵ�ʶ������
  size_t ItemSize() { return node_manager_.ItemSize(); }

  //���л�������
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);

  Iterator Begin() { return node_manager_.Begin(); }
  Iterator End() { return node_manager_.End(); }

  T& operator[](ObjectId id);

 private:
  //ͨ��һ��id��ѯ������ͬ�ײ���������id
  const std::unordered_set<ObjectId>& GetIdsReferringSameObject(
      InsideId inside_id) {
    return objectids_referring_same_object[inside_id];
  }
  //��idָ���ʵ�ʶ�����ӳ��Ϊinside_idָ��Ķ���
  //���Խ���δӳ���id
  bool RemapId(ObjectId id, InsideId inside_id);
  //����һ�������ڷ����ID
  ObjectId CreateId();
  //����һ����id��inside_id��������ü�¼���������Ӻ����ü���
  size_t AddReference(ObjectId id, InsideId inside_id);
  //�Ƴ���id��inside_id��������ü�¼�������Ƴ������ü���
  //���ü���Ϊ0ʱ���Զ�ɾ������
  size_t RemoveReference(ObjectId id, InsideId inside_id);
  //�����Ƴ��ڲ�����ǰ�Ĳ��裬��ɾ�����о�����ײ�ID������
  bool PreRemoveInsideObject(InsideId inside_id);
  //��ȡ�����Ӧ�ĵײ����ID
  InsideId GetInsideId(ObjectId id);

  //��һ��id���ֵ��ֻ������
  ObjectId next_id_index_ = ObjectId(0);
  ObjectManager<T> node_manager_;
  //����ָ��ͬһ���ײ�����id
  std::vector<std::unordered_set<ObjectId>> objectids_referring_same_object;
  //�Եײ�id����һ���װ��ʹ����֧�ֶ��idָ��ͬһ������
  std::unordered_map<ObjectId, InsideId> id_to_index_;
};

template <class T>
inline typename MultimapObjectManager<T>::InsideId
MultimapObjectManager<T>::GetInsideId(ObjectId id) {
  auto iter = id_to_index_.find(id);
  assert(iter != id_to_index_.end());
  return iter->second;
}

template <class T>
inline T& MultimapObjectManager<T>::GetObject(ObjectId id) {
  InsideId inside_id = GetInsideId(id);
  return node_manager_.GetObject(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::IsSame(ObjectId id1,
                                             ObjectId id2) {
  InsideId inside_id1 = GetInsideId(id1);
  InsideId inside_id2 = GetInsideId(id2);
  return inside_id1 == inside_id2;
}

template <class T>
template <class... Args>
inline MultimapObjectManager<T>::ObjectId
MultimapObjectManager<T>::EmplaceObject(Args&&... args) {
  ObjectId id = CreateId();
  InsideId inside_id = node_manager_.EmplaceObject(std::forward<Args>(args)...);
  AddReference(id, inside_id);
  id_to_index_[id] = inside_id;
  return id;
}

template <class T>
inline bool MultimapObjectManager<T>::SetObjectMergeAllowed(ObjectId id) {
  InsideId inside_id = GetInsideId(id);
  return node_manager_.SetObjectMergeAllowed(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::SetObjectMergeRefused(ObjectId id) {
  InsideId inside_id = GetInsideId(id);
  return node_manager_.SetObjectMergeRefused(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::CanMerge(ObjectId id) {
  InsideId index = GetInsideId(id);
  return node_manager_.CanMerge(index);
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
    for (ObjectId id : ids_referring_same_inside_id) {
      auto iter = id_to_index_.find(id);
      id_to_index_.erase(iter);
    }
    objectids_referring_same_object[inside_id].clear();
  }
  return true;
}

template <class T>
inline T* MultimapObjectManager<T>::RemoveObjectNoDelete(ObjectId id) {
  InsideId inside_id = GetInsideId(id);
  PreRemoveInsideObject(inside_id);
  return node_manager_.RemovePointer(inside_id);
}

template <class T>
inline bool MultimapObjectManager<T>::RemoveObject(ObjectId id) {
  InsideId inside_id = GetInsideId(id);
  PreRemoveInsideObject(inside_id);
  node_manager_.RemoveObject(inside_id);
  return true;
}

template <class T>
inline void MultimapObjectManager<T>::Swap(
    MultimapObjectManager& manager_other) {
  std::swap(next_id_index_, manager_other.next_id_index_);
  node_manager_.Swap(manager_other.node_manager_);
  objectids_referring_same_object.Swap(manager_other.objectids_referring_same_object);
  id_to_index_.Swap(manager_other.id_to_index_);
}

template <class T>
inline bool MultimapObjectManager<T>::RemapId(ObjectId id,
                                                   InsideId inside_id) {
  assert(inside_id < node_manager_.Size());
  InsideId inside_id_old = GetInsideId(id);
  AddReference(id, inside_id);
  id_to_index_[id] = inside_id;
  return true;
}

template <class T>
inline MultimapObjectManager<T>::ObjectId
MultimapObjectManager<T>::CreateId() {
  ObjectId return_id = next_id_index_;
  next_id_index_ = ObjectId(return_id + 1);
  return return_id;
}

template <class T>
inline size_t MultimapObjectManager<T>::AddReference(ObjectId id,
                                                     InsideId inside_id) {
  assert(inside_id < node_manager_.Size());
  objectids_referring_same_object[inside_id].insert(id);
  return objectids_referring_same_object[inside_id].size();
}

template <class T>
inline size_t MultimapObjectManager<T>::RemoveReference(ObjectId id,
                                                        InsideId inside_id) {
  assert(inside_id < node_manager_.Size());
  auto& ref_uset = objectids_referring_same_object[inside_id];
  auto iter = ref_uset.find(id);
  if (iter != ref_uset.end()) {
    if (ref_uset.size() == 1) {
      node_manager_.RemoveObject(inside_id);
    }
    ref_uset.erase(iter);
  }
  return ref_uset.size();
}

template <class T>
inline void MultimapObjectManager<T>::Clear() {
  next_id_index_ = ObjectId(0);
  node_manager_.Clear();
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
inline T& MultimapObjectManager<T>::operator[](ObjectId id) {
  return GetObject(id);
}

}  // namespace frontend::common
#endif  // !COMMON_COMMON_MULTIMAP_NODE_MANAGER