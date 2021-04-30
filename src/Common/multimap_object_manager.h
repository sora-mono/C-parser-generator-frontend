#include <assert.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "Common/object_manager.h"

#ifndef COMMON_MULTIMAP_OBJECT_MANAGER_H_
#define COMMON_MULTIMAP_OBJECT_MANAGER_H_

namespace frontend::common {

template <class T>
class MultimapObjectManager {
 public:
  using ObjectId = size_t;
  using Iterator = ObjectManager<T>::Iterator;

  MultimapObjectManager() {}
  MultimapObjectManager(const MultimapObjectManager&) = delete;
  MultimapObjectManager(MultimapObjectManager&&) = delete;
  ~MultimapObjectManager() {}
  //��ȡָ����Ч�ڵ��ָ��
  T& GetNode(ObjectId handler);
  //ͨ��һ��handler��ѯ���õײ�ڵ������handler
  const std::unordered_set<ObjectId>& GetHandlersReferringSameNode(
      ObjectId handler);
  bool IsSame(ObjectId handler1, ObjectId handler2);

  //ϵͳ����ѡ�����λ�÷��ýڵ�
  template <class... Args>
  ObjectId EmplaceNode(Args&&... args);
  //��ɾ���ڵ㲻�ͷ�ָ��ָ����ڴ棬���ع����ָ��
  T* RemovePointer(ObjectId handler);
  //ɾ���ڵ㲢�ͷŽڵ��ڴ�
  bool RemoveNode(ObjectId handler);
  //��handler_src�ϲ���handler_dst��
  //�ϲ��ɹ���ɾ��handler_src��Ӧ�Ľڵ㣨���߲�ͬǰ���£�
  bool MergeNodes(ObjectId handler_dst, ObjectId handler_src,
                  std::function<bool(T&, T&)> merge_function =
                      ObjectManager<T>::DefaultMergeFunction2);
  template <class Manager>
  bool MergeNodesWithManager(
      ObjectId handler_dst, ObjectId handler_src, Manager& manager,
      const std::function<bool(T&, T&, Manager&)>& merge_function =
          ObjectManager<T>::DefaultMergeFunction3);
  //���������������
  void Swap(MultimapObjectManager& manager_other);
  //���ýڵ�����ϲ���־
  bool SetNodeMergeAllowed(ObjectId handler);
  //���ýڵ��ֹ�ϲ���־
  bool SetNodeMergeRefused(ObjectId handler);
  //�������нڵ�����ϲ��������δ����ڵ�����������
  void SetAllMergeAllowed() { node_manager_.SetAllNodesMergeAllowed(); }
  //�������нڵ��ֹ�ϲ�
  void SetAllNodesMergeRefused() { node_manager_.SetAllNodesMergeRefused(); }
  //����handler��Ӧ�ڵ�ɺϲ�״̬
  bool CanMerge(ObjectId handler);

  //��ʱ��д���Ż��ͷź�Ŀռ�̫��
  // void remap_optimization();	//��ӳ�����еĽڵ㣬����node_manager����ռ�

  //������ͷ����нڵ�
  void Clear();
  //��������ͷ����нڵ�
  void ClearNoRelease();
  //���ó�Ա������shrink_to_fit
  void ShrinkToFit();

  //���ؽڵ������������ɾ�����Ա���index�Ľڵ㣩
  size_t Size() { return node_manager_.Size(); }
  //����ʵ�ʽڵ����
  size_t ItemSize() { return node_manager_.ItemSize(); }

  //���л�������
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);

  Iterator Begin() { return node_manager_.Begin; }
  Iterator End() { return node_manager_.End(); }

  T* operator[](ObjectId handler);

 private:
  using InsideIndex = ObjectManager<T>::ObjectId;
  //��handlerָ���ʵ�ʽڵ���ӳ��Ϊinside_indexָ��Ľڵ㣬
  //���Խ���δӳ���handler
  bool RemapHandler(ObjectId handler, InsideIndex inside_index);
  ObjectId CreateHandler() { return next_handler_index_++; }
  //���Ӹ�handler��inside_index�ڵ�����ü�¼���������Ӻ����ü���
  void AddReference(ObjectId handler, InsideIndex inside_index);
  //�Ƴ���handler��inside_index�ڵ�����ü�¼�������Ƴ������ü�����
  //���ü���Ϊ0ʱ���Զ�ɾ���ڵ�
  size_t RemoveReference(ObjectId handler, InsideIndex inside_index);
  //�����Ƴ��ڲ��ڵ�ǰ�Ĳ��裬��ɾ�����о�����ײ�ID������
  bool PreRemoveInsideNode(InsideIndex inside_index);
  //��ȡ�����Ӧ�ĵײ�ڵ�
  InsideIndex GetInsideIndex(ObjectId handler);

  //��һ��handler���ֵ��ֻ������
  size_t next_handler_index_ = 0;
  ObjectManager<T> node_manager_;
  //����ָ��ͬһ���ײ�ڵ��handler
  std::vector<std::unordered_set<ObjectId>> referred_handlers_;
  //�Եײ�handler����һ���װ��ʹ����֧�ֶ��handlerָ��ͬһ������
  std::unordered_map<ObjectId, InsideIndex> handler_to_index_;
};

template <class T>
inline typename MultimapObjectManager<T>::InsideIndex
MultimapObjectManager<T>::GetInsideIndex(ObjectId handler) {
  auto iter = handler_to_index_.find(handler);
  assert(iter != handler_to_index_.end());
  return iter->second;
}

template <class T>
inline T& MultimapObjectManager<T>::GetNode(ObjectId handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  return node_manager_.GetNode(inside_index);
}

template <class T>
inline const std::unordered_set<typename MultimapObjectManager<T>::ObjectId>&
MultimapObjectManager<T>::GetHandlersReferringSameNode(ObjectId handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  return referred_handlers_[inside_index];
}

template <class T>
inline bool MultimapObjectManager<T>::IsSame(ObjectId handler1, ObjectId handler2) {
  InsideIndex inside_index1 = GetInsideIndex(handler1);
  InsideIndex inside_index2 = GetInsideIndex(handler2);
  return inside_index1 == inside_index2;
}

template <class T>
template <class... Args>
inline MultimapObjectManager<T>::ObjectId MultimapObjectManager<T>::EmplaceNode(
    Args&&... args) {
  ObjectId handler = CreateHandler();
  InsideIndex inside_index =
      node_manager_.EmplaceNode(std::forward<Args>(args)...);
  AddReference(handler, inside_index);
  handler_to_index_[handler] = inside_index;
  return handler;
}

template <class T>
inline bool MultimapObjectManager<T>::SetNodeMergeAllowed(ObjectId handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  return node_manager_.SetNodeMergeAllowed(inside_index);
}

template <class T>
inline bool MultimapObjectManager<T>::SetNodeMergeRefused(ObjectId handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  return node_manager_.SetNodeMergeRefused(inside_index);
}

template <class T>
inline bool MultimapObjectManager<T>::CanMerge(ObjectId handler) {
  InsideIndex index = GetInsideIndex(handler);
  return node_manager_.CanMerge(index);
}

template <class T>
inline bool MultimapObjectManager<T>::MergeNodes(
    ObjectId handler_dst, ObjectId handler_src,
    std::function<bool(T&, T&)> merge_function) {
  InsideIndex inside_index_dst = GetInsideIndex(handler_dst);
  InsideIndex inside_index_src = GetInsideIndex(handler_src);
  if (!node_manager_.MergeNodes(inside_index_dst, inside_index_src,
                                merge_function)) {
    return false;
  }
  RemapHandler(handler_src, inside_index_dst);
  return true;
}

template <class T>
template <class Manager>
inline bool MultimapObjectManager<T>::MergeNodesWithManager(
    typename MultimapObjectManager<T>::ObjectId handler_dst,
    typename MultimapObjectManager<T>::ObjectId handler_src, Manager& manager,
    const std::function<bool(T&, T&, Manager&)>& merge_function) {
  InsideIndex inside_index_dst = GetInsideIndex(handler_dst);
  InsideIndex inside_index_src = GetInsideIndex(handler_src);
  if (!node_manager_.MergeNodesWithManager(inside_index_dst, inside_index_src,
                                           manager, merge_function)) {
    return false;
  }
  RemapHandler(handler_src, inside_index_dst);
  return true;
}

template <class T>
inline bool MultimapObjectManager<T>::PreRemoveInsideNode(
    InsideIndex inside_index) {
  assert(inside_index < node_manager_.Size());
  const auto& handlers_referring_same_inside_id =
      GetHandlersReferringSameNode(inside_index);
  if (handlers_referring_same_inside_id.size() != 0) {
    for (ObjectId id : handlers_referring_same_inside_id) {
      auto iter = handler_to_index_.find(id);
      handler_to_index_.erase(iter);
    }
    referred_handlers_[inside_index].clear();
  }
  return true;
}

template <class T>
inline T* MultimapObjectManager<T>::RemovePointer(ObjectId handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  PreRemoveInsideNode(inside_index);
  return node_manager_.RemovePointer(inside_index);
}

template <class T>
inline bool MultimapObjectManager<T>::RemoveNode(ObjectId handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  PreRemoveInsideNode(inside_index);
  node_manager_.RemoveNode(inside_index);
  return true;
}

template <class T>
inline void MultimapObjectManager<T>::Swap(
    MultimapObjectManager& manager_other) {
  std::swap(next_handler_index_, manager_other.next_handler_index_);
  node_manager_.Swap(manager_other.node_manager_);
  referred_handlers_.Swap(manager_other.referred_handlers_);
  handler_to_index_.Swap(manager_other.handler_to_index_);
}

template <class T>
inline bool MultimapObjectManager<T>::RemapHandler(ObjectId handler,
                                                   InsideIndex inside_index) {
  assert(inside_index < node_manager_.Size());
  InsideIndex inside_index_old = GetInsideIndex(handler);
  AddReference(handler, inside_index);
  handler_to_index_[handler] = inside_index;
  return true;
}

template <class T>
inline void MultimapObjectManager<T>::AddReference(ObjectId handler,
                                                   InsideIndex inside_index) {
  assert(inside_index < node_manager_.Size());
  referred_handlers_[inside_index].insert(handler);
}

template <class T>
inline size_t MultimapObjectManager<T>::RemoveReference(
    ObjectId handler, InsideIndex inside_index) {
  assert(inside_index < node_manager_.Size());
  auto& ref_uset = referred_handlers_[inside_index];
  auto iter = ref_uset.find(handler);
  if (iter != ref_uset.end()) {
    if (ref_uset.size() == 1) {
      node_manager_.RemoveNode(inside_index);
    }
    ref_uset.erase(iter);
  }
  return ref_uset.size();
}

template <class T>
inline void MultimapObjectManager<T>::Clear() {
  next_handler_index_ = 0;
  node_manager_.Clear();
  referred_handlers_.clear();
  handler_to_index_.clear();
}

template <class T>
inline void MultimapObjectManager<T>::ClearNoRelease() {
  next_handler_index_ = 0;
  node_manager_.ClearNoRelease();
  referred_handlers_.clear();
  handler_to_index_.clear();
}

template <class T>
inline void MultimapObjectManager<T>::ShrinkToFit() {
  node_manager_.ShrinkToFit();
  referred_handlers_.shrink_to_fit();
}

template <class T>
inline T* MultimapObjectManager<T>::operator[](ObjectId handler) {
  return GetNode(handler);
}

}  // namespace frontend::common
#endif  // !COMMON_COMMON_MULTIMAP_NODE_MANAGER