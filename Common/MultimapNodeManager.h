#pragma once
#include <assert.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "NodeManager.h"
template <class T>
class MultimapNodeManager {
 public:
  using NodeHandler = size_t;

  MultimapNodeManager() {}
  MultimapNodeManager(const MultimapNodeManager&) = delete;
  MultimapNodeManager(MultimapNodeManager&&) = delete;
  ~MultimapNodeManager() {}

  T* GetNode(NodeHandler handler);
  //ͨ��һ��handler��ѯ���õײ�ڵ������handler
  const std::unordered_set<NodeHandler>& GetHandlersReferringSameNode(
      NodeHandler handler);
  bool IsSame(NodeHandler handler1, NodeHandler handler2);

  //ϵͳ����ѡ�����λ�÷��ýڵ�
  template <class... Args>
  NodeHandler EmplaceNode(Args&&... args);
  //��ɾ���ڵ㲻�ͷ�ָ��ָ����ڴ棬���ع����ָ��
  T* RemovePointer(NodeHandler handler);
  //ɾ���ڵ㲢�ͷŽڵ��ڴ�
  bool RemoveNode(NodeHandler handler);
  //��handler_src�ϲ���handler_dst��
  //�ϲ��ɹ���ɾ��handler_src��Ӧ�Ľڵ㣨���߲�ͬǰ���£�
  bool MergeNodes(NodeHandler handler_dst, NodeHandler handler_src,
                  std::function<bool(T&, T&)> merge_function =
                      NodeManager<T>::DefaultMergeFunction2);
  template <class Manager>
  bool MergeNodesWithManager(
      NodeHandler handler_dst, NodeHandler handler_src, Manager& manager,
      const std::function<bool(T&, T&, Manager&)>& merge_function =
          NodeManager<T>::DefaultMergeFunction3);
  //���������������
  void Swap(MultimapNodeManager& manager_other);
  //���ýڵ�����ϲ���־
  bool SetNodeMergeAllowed(NodeHandler handler);
  //���ýڵ��ֹ�ϲ���־
  bool SetNodeMergeRefused(NodeHandler handler);
  //�������нڵ�����ϲ��������δ����ڵ�����������
  void SetAllMergeAllowed() { node_manager_.SetAllNodesMergeAllowed(); }
  //�������нڵ��ֹ�ϲ�
  void SetAllNodesMergeRefused() { node_manager_.SetAllNodesMergeRefused(); }
  //����handler��Ӧ�ڵ�ɺϲ�״̬
  bool CanMerge(NodeHandler handler);

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

 private:
  using InsideIndex = NodeManager<T>::NodeHandler;
  //��handlerָ���ʵ�ʽڵ���ӳ��Ϊinside_indexָ��Ľڵ㣬
  //���Խ���δӳ���handler
  bool RemapHandler(NodeHandler handler, InsideIndex inside_index);
  NodeHandler CreateHandler() { return next_handler_index_++; }
  //���Ӹ�handler��inside_index�ڵ�����ü�¼���������Ӻ����ü���
  size_t AddReference(NodeHandler handler, InsideIndex inside_index);
  //�Ƴ���handler��inside_index�ڵ�����ü�¼�������Ƴ������ü�����
  //���ü���Ϊ0ʱ���Զ�ɾ���ڵ�
  size_t RemoveReference(NodeHandler handler, InsideIndex inside_index);
  //�����Ƴ��ڲ��ڵ�ǰ�Ĳ��裬��ɾ����������
  bool PreRemoveInsideNode(InsideIndex inside_index);
  InsideIndex GetInsideIndex(NodeHandler handler);

  //��һ��handler���ֵ��ֻ������
  size_t next_handler_index_ = 0;
  NodeManager<T> node_manager_;
  //����ָ��ͬһ���ײ�ڵ��handler
  std::vector<std::unordered_set<NodeHandler>> referred_handlers_;
  //�Եײ�handler����һ���װ��ʹ����֧�ֶ��handlerָ��ͬһ������
  std::unordered_map<NodeHandler, InsideIndex> handler_to_index_;
};

template <class T>
inline typename MultimapNodeManager<T>::InsideIndex
MultimapNodeManager<T>::GetInsideIndex(NodeHandler handler) {
  auto iter = handler_to_index_.find(handler);
  if (iter == handler_to_index_.end()) {
    return -1;
  }
  return iter->second;
}

template <class T>
inline T* MultimapNodeManager<T>::GetNode(NodeHandler handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  if (inside_index == -1) {
    return nullptr;
  } else {
    return node_manager_.GetNode(inside_index);
  }
}

template <class T>
inline const std::unordered_set<typename MultimapNodeManager<T>::NodeHandler>&
MultimapNodeManager<T>::GetHandlersReferringSameNode(NodeHandler handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  if (inside_index == -1) {
    throw std::invalid_argument("�����Ч");
  }
  return referred_handlers_[inside_index];
}

template <class T>
inline bool MultimapNodeManager<T>::IsSame(NodeHandler handler1,
                                           NodeHandler handler2) {
  InsideIndex inside_index1 = GetInsideIndex(handler1);
  if (inside_index1 == -1) {
    throw std::runtime_error("Invalid handler1");
  }
  InsideIndex inside_index2 = GetInsideIndex(handler2);
  if (inside_index2 == -1) {
    throw std::runtime_error("Invalid handler2");
  }
  return inside_index1 == inside_index2;
}

template <class T>
template <class... Args>
inline MultimapNodeManager<T>::NodeHandler MultimapNodeManager<T>::EmplaceNode(
    Args&&... args) {
  NodeHandler handler = CreateHandler();
  InsideIndex inside_index =
      node_manager_.EmplaceNode(std::forward<Args>(args)...);
  if (inside_index == -1) {
    return -1;
  }
  if (AddReference(handler, inside_index) == -1) {  //������ü�¼ʧ��
    node_manager_.RemoveNode(inside_index);
    return -1;
  }
  handler_to_index_[handler] = inside_index;

  return handler;
}

template <class T>
inline bool MultimapNodeManager<T>::SetNodeMergeAllowed(NodeHandler handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  if (inside_index == -1) {
    return false;
  }
  return node_manager_.SetNodeMergeAllowed(inside_index);
}

template <class T>
inline bool MultimapNodeManager<T>::SetNodeMergeRefused(NodeHandler handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  if (inside_index == -1) {
    return false;
  }
  return node_manager_.SetNodeMergeRefused(inside_index);
}

template <class T>
inline bool MultimapNodeManager<T>::CanMerge(NodeHandler handler) {
  InsideIndex index = GetInsideIndex(handler);
  if (index == -1) {
    return false;
  }
  return node_manager_.CanMerge(index);
}

template <class T>
inline bool MultimapNodeManager<T>::MergeNodes(
    NodeHandler handler_dst, NodeHandler handler_src,
    std::function<bool(T&, T&)> merge_function) {
  InsideIndex inside_index_dst = GetInsideIndex(handler_dst);
  InsideIndex inside_index_src = GetInsideIndex(handler_src);
  if (inside_index_dst == -1 || inside_index_src == -1) {
    return false;
  }
  if (!node_manager_.MergeNodesWithManager(inside_index_dst, inside_index_src,
                                           merge_function)) {
    return false;
  }
  RemapHandler(handler_src, inside_index_dst);
  return true;
}

template <class T>
template <class Manager>
inline bool MultimapNodeManager<T>::MergeNodesWithManager(
    typename MultimapNodeManager<T>::NodeHandler handler_dst,
    typename MultimapNodeManager<T>::NodeHandler handler_src, Manager& manager,
    const std::function<bool(T&, T&, Manager&)>& merge_function) {
  InsideIndex inside_index_dst = GetInsideIndex(handler_dst);
  InsideIndex inside_index_src = GetInsideIndex(handler_src);
  if (inside_index_dst == -1 || inside_index_src == -1) {
    return false;
  }
  if (!node_manager_.MergeNodesWithManager(inside_index_dst, inside_index_src,
                                           manager, merge_function)) {
    return false;
  }
  RemapHandler(handler_src, inside_index_dst);
  return true;
}

template <class T>
inline bool MultimapNodeManager<T>::PreRemoveInsideNode(
    InsideIndex inside_index) {
  if (inside_index >= node_manager_.Size()) {
    return false;
  }
  if (referred_handlers_[inside_index].size() != 0) {
    for (auto x : referred_handlers_[inside_index]) {
      auto iter = handler_to_index_.find(x);
      handler_to_index_.erase(iter);
    }
    referred_handlers_[inside_index].clear();
  }
  return true;
}

template <class T>
inline T* MultimapNodeManager<T>::RemovePointer(NodeHandler handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  if (inside_index == -1) {
    return nullptr;
  }
  if (!PreRemoveInsideNode(inside_index)) {
    return nullptr;
  }
  return node_manager_.RemovePointer(inside_index);
}

template <class T>
inline bool MultimapNodeManager<T>::RemoveNode(NodeHandler handler) {
  InsideIndex inside_index = GetInsideIndex(handler);
  if (inside_index == -1) {
    return true;
  }
  PreRemoveInsideNode(inside_index);
  node_manager_.RemoveNode(inside_index);
  return true;
}

template <class T>
inline void MultimapNodeManager<T>::Swap(MultimapNodeManager& manager_other) {
  std::swap(next_handler_index_, manager_other.next_handler_index_);
  node_manager_.Swap(manager_other.node_manager_);
  referred_handlers_.Swap(manager_other.referred_handlers_);
  handler_to_index_.Swap(manager_other.handler_to_index_);
}

template <class T>
inline bool MultimapNodeManager<T>::RemapHandler(NodeHandler handler,
                                                 InsideIndex inside_index) {
  if (inside_index >= node_manager_.Size()) {
    return false;
  }
  InsideIndex inside_index_old = GetInsideIndex(handler);
  if (inside_index == inside_index_old) {
    return true;
  }
  if (inside_index_old != -1) {
    size_t result = RemoveReference(handler, inside_index_old);
    assert(result != -1);
  }
  if (AddReference(handler, inside_index) == -1) {
    if (inside_index_old != -1) {
      AddReference(handler, inside_index_old);
    }
    return false;
  }
  handler_to_index_[handler] = inside_index;
  return true;
}

template <class T>
inline size_t MultimapNodeManager<T>::AddReference(NodeHandler handler,
                                                   InsideIndex inside_index) {
  if (inside_index >= node_manager_.Size()) {
    return -1;
  }
  referred_handlers_[inside_index].insert(handler);
  return referred_handlers_[inside_index].size();
}

template <class T>
inline size_t MultimapNodeManager<T>::RemoveReference(
    NodeHandler handler, InsideIndex inside_index) {
  if (inside_index >= node_manager_.Size()) {
    return -1;
  }
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
inline void MultimapNodeManager<T>::Clear() {
  next_handler_index_ = 0;
  node_manager_.Clear();
  referred_handlers_.clear();
  handler_to_index_.clear();
}

template <class T>
inline void MultimapNodeManager<T>::ClearNoRelease() {
  next_handler_index_ = 0;
  node_manager_.ClearNoRelease();
  referred_handlers_.clear();
  handler_to_index_.clear();
}

template <class T>
inline void MultimapNodeManager<T>::ShrinkToFit() {
  node_manager_.ShrinkToFit();
  referred_handlers_.shrink_to_fit();
}