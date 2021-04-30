#include <assert.h>

#include <functional>
#include <stdexcept>
#include <vector>

#ifndef COMMON_OBJECT_MANAGER_H_
#define COMMON_OBJECT_MANAGER_H_

namespace frontend::common {

template <class T>
class ObjectManager {
 public:
  using ObjectId = size_t;

  ObjectManager() {}
  ObjectManager(const ObjectManager&) = delete;
  ObjectManager(ObjectManager&&) = delete;
  ~ObjectManager();

  class Iterator {
   public:
    Iterator() : manager_pointer_(nullptr), index_(-1) {}
    Iterator(ObjectManager<T>* manager_pointer, size_t index)
        : manager_pointer_(manager_pointer), index_(index) {}
    Iterator(const Iterator& iter)
        : manager_pointer_(iter.manager_pointer_), index_(iter.index_) {}

    size_t GetId() { return index_; }
    void SetManagerPointer(const ObjectManager<T>* manager_pointer) {
      manager_pointer_ = manager_pointer;
    }
    void SetIndex(size_t index);

    Iterator& operator++();
    Iterator operator++(int);
    Iterator& operator--();
    Iterator operator--(int);
    T& operator*();
    T* operator->();
    bool operator==(const Iterator& iter) {
      return manager_pointer_ == iter.manager_pointer_ && index_ == iter.index_;
    }
    bool operator!=(const Iterator& iter) { return !this->operator==(iter); }

   private:
    ObjectManager<T>* manager_pointer_;
    size_t index_;
  };

  static bool DefaultMergeFunction2(T& node_dst, T& node_src) {
    return node_dst.MergeNode(node_src);
  }
  template <class Manager>
  static bool DefaultMergeFunction3(T& node_dst, T& node_src,
                                    Manager& manager) {
    return manager.MergeNodes(node_dst, node_src);
  }
  //��ȡ�ڵ�����
  T& GetObject(ObjectId index);
  //�ж�����ID�Ƿ����
  bool IsSame(ObjectId index1, ObjectId index2) { return index1 == index2; }

  //ϵͳ����ѡ�����λ�÷��ö���
  template <class ObjectType = T, class... Args>
  ObjectId EmplaceObject(Args&&... args);

  //��ɾ���ڵ㲻�ͷŽڵ��ڴ棬����ָ������ָ��
  T* RemoveObjectNoDelete(ObjectId index);
  //ɾ���ڵ㲢�ͷŽڵ��ڴ�
  bool RemoveObject(ObjectId index);

  //�ϲ��������󣬺ϲ��ɹ���ɾ��index_src�ڵ�
  bool MergeObjects(ObjectId index_dst, ObjectId index_src,
                    const std::function<bool(T&, T&)>& merge_function =
                        DefaultMergeFunction2);
  //�ϲ��������󣬺ϲ��ɹ���ɾ��index_src�ڵ�
  template <class Manager>
  bool MergeObjectsWithManager(ObjectId index_dst, ObjectId index_src,
                               Manager& manager,
                               const std::function<bool(T&, T&, Manager&)>&
                                   merge_function = DefaultMergeFunction3);
  void Swap(ObjectManager& manager_other);

  bool SetObjectMergeAllowed(ObjectId index);
  bool SetObjectMergeRefused(ObjectId index);
  void SetAllObjectsMergeAllowed();
  void SetAllObjectsMergeRefused();

  bool CanMerge(ObjectId index) {
    return index < nodes_can_merge.size() ? nodes_can_merge[index] : false;
  }
  //������С������δ����ڵ��ָ��
  size_t Size() { return nodes_.size(); }
  //����ʵ�ʳ��еĶ�������
  size_t ItemSize();
  //������ͷ����нڵ�
  void Clear();
  //��������ͷ����нڵ�
  void ClearNoRelease();
  //���ó�Ա������shrink_to_fit
  void ShrinkToFit();

  T* operator[](ObjectId index);
  Iterator End() { return Iterator(this, nodes_.size()); }
  Iterator Begin();

 private:
  friend class Iterator;

  //��ָ��λ�÷��ýڵ�
  template <class ObjectType = T, class... Args>
  ObjectId EmplaceObjectIndex(ObjectId index, Args&&... args);
  //��ָ��λ�÷���ָ��
  ObjectId EmplacePointerIndex(ObjectId index, T* pointer);

  //���л�������
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);
  //��ȡ��ǰ��ѿ���index
  ObjectId GetBestEmptyIndex();
  //������Ƴ���index
  void AddRemovedIndex(ObjectId index) { removed_ids_.push_back(index); }

  std::vector<T*> nodes_;
  //�Ż������飬��ű�ɾ���ڵ��Ӧ��ID
  std::vector<ObjectId> removed_ids_;
  //�洢��Ϣ��ʾ�Ƿ�����ϲ��ڵ�
  std::vector<bool> nodes_can_merge;
};

template <class T>
ObjectManager<T>::~ObjectManager() {
  for (T* ptr : nodes_) {
    delete ptr;
  }
}

template <class T>
inline T& ObjectManager<T>::GetObject(ObjectId index) {
  assert(index < nodes_.size() && nodes_[index] != nullptr);
  return *nodes_[index];
}

template <class T>
inline bool ObjectManager<T>::RemoveObject(ObjectId index) {
  T* removed_pointer = RemoveObjectNoDelete(index);
  delete removed_pointer;
  return true;
}

template <class T>
inline T* ObjectManager<T>::RemoveObjectNoDelete(ObjectId index) {
  assert(index < nodes_.size());
  T* temp_pointer = nodes_[index];
  assert(temp_pointer != nullptr);
  AddRemovedIndex(index);
  nodes_can_merge[index] = false;
  while (nodes_.size() > 0 && nodes_.back() == nullptr) {
    nodes_.pop_back();  //����ĩβ��ЧID
    nodes_can_merge.pop_back();
  }
  return temp_pointer;
}

template <class T>
template <class ObjectType, class... Args>
inline ObjectManager<T>::ObjectId ObjectManager<T>::EmplaceObject(
    Args&&... args) {
  ObjectId index = GetBestEmptyIndex();
  T* object_pointer = new ObjectType(std::forward<Args>(args)...);
  ObjectId result = EmplacePointerIndex(index, object_pointer);
  assert(result != -1);
  return result;
}

template <class T>
template <class ObjectType, class... Args>
inline ObjectManager<T>::ObjectId ObjectManager<T>::EmplaceObjectIndex(
    ObjectId index, Args&&... args) {
  T* pointer = new ObjectType(std::forward<Args>(args)...);
  ObjectId result = EmplacePointerIndex(index, pointer);
  assert(result != -1);
  return result;
}

template <class T>
inline ObjectManager<T>::ObjectId ObjectManager<T>::EmplacePointerIndex(
    ObjectId index, T* pointer) {
  size_t size_old = nodes_.size();
  if (index >= size_old) {
    nodes_.resize(index + 1, nullptr);
    nodes_can_merge.resize(index + 1, false);
    for (size_t i = size_old; i < index; i++) {
      removed_ids_.push_back(i);
    }
  }
  //�����Ը������зǿ��Ҳ�ͬ��ָ��
  assert(!(nodes_[index] != nullptr && nodes_[index] != pointer));
  nodes_[index] = pointer;
  return index;
}

template <class T>
inline void ObjectManager<T>::Swap(ObjectManager& manager_other) {
  nodes_.swap(manager_other.nodes_);
  removed_ids_.swap(manager_other.removed_ids_);
  nodes_can_merge.swap(manager_other.nodes_can_merge);
}

template <class T>
inline bool ObjectManager<T>::SetObjectMergeAllowed(ObjectId index) {
  assert(index < nodes_.size() && nodes_[index] != nullptr);
  nodes_can_merge[index] = true;
  return true;
}

template <class T>
inline bool ObjectManager<T>::SetObjectMergeRefused(ObjectId index) {
  assert(index < nodes_.size() && nodes_[index] != nullptr);
  nodes_can_merge[index] = false;
  return true;
}

template <class T>
inline void ObjectManager<T>::SetAllObjectsMergeAllowed() {
  for (size_t index = 0; index < nodes_.size(); index++) {
    if (nodes_[index] != nullptr) {
      nodes_can_merge[index] = true;
    }
  }
}

template <class T>
inline void ObjectManager<T>::SetAllObjectsMergeRefused() {
  std::vector<bool> vec_temp(nodes_can_merge.size(), false);
  nodes_can_merge.Swap(vec_temp);
}

template <class T>
inline bool ObjectManager<T>::MergeObjects(
    ObjectId index_dst, ObjectId index_src,
    const std::function<bool(T&, T&)>& merge_function) {
  T& object_dst = GetObject(index_dst);
  T& object_src = GetObject(index_src);
  if (!CanMerge(index_dst) || !CanMerge(index_src)) {
    //�����ڵ�������һ�����ɺϲ�
    return false;
  }
  if (!merge_function(object_dst,object_src)) {
    //�ϲ�ʧ��
    return false;
  }
  RemoveObject(index_src);
  return true;
}

template <class T>
template <class Manager>
bool ObjectManager<T>::MergeObjectsWithManager(
    typename ObjectManager<T>::ObjectId index_dst,
    typename ObjectManager<T>::ObjectId index_src, Manager& manager,
    const std::function<bool(T&, T&, Manager&)>& merge_function) {
  T& object_dst = GetObject(index_dst);
  T& object_src = GetObject(index_src);
  if (!CanMerge(index_dst) || !CanMerge(index_src)) {
    //�����ڵ�������һ�����ɺϲ�
    return false;
  }
  if (!merge_function(object_dst,object_src, manager)) {
    //�ϲ�ʧ��
    return false;
  }
  RemoveObject(index_src);
  return true;
}

template <class T>
inline void ObjectManager<T>::Clear() {
  for (auto p : nodes_) {
    delete p;
  }
  nodes_.clear();
  removed_ids_.clear();
}

template <class T>
inline void ObjectManager<T>::ClearNoRelease() {
  nodes_.clear();
  removed_ids_.clear();
}

template <class T>
inline void ObjectManager<T>::ShrinkToFit() {
  while (nodes_.size() > 0 && nodes_.back() == nullptr) {
    nodes_.pop_back();
    nodes_can_merge.pop_back();
  }
  nodes_.shrink_to_fit();
  removed_ids_.shrink_to_fit();
}

template <class T>
inline size_t ObjectManager<T>::ItemSize() {
  size_t count = 0;
  for (auto p : nodes_) {
    if (p != nullptr) {
      ++count;
    }
  }
  return count;
}

template <class T>
inline ObjectManager<T>::ObjectId ObjectManager<T>::GetBestEmptyIndex() {
  ObjectId index = -1;
  if (removed_ids_.size() != 0) {
    while (index == -1 && removed_ids_.size() != 0)  //������ЧID
    {
      if (removed_ids_.back() < nodes_.size()) {
        index = removed_ids_.back();
      }
      removed_ids_.pop_back();
    }
  }
  if (index == -1)  //����Ч��ɾ��ID
  {
    index = nodes_.size();
  }
  return index;
}

template <class T>
inline T* ObjectManager<T>::operator[](size_t index) {
  assert(index < nodes_.size());
  return nodes_[index];
}

template <class T>
inline void ObjectManager<T>::Iterator::SetIndex(size_t index) {
  assert(manager_pointer_ != nullptr &&
         index < manager_pointer_->nodes_.size());
  index_ = index;
}

template <class T>
inline ObjectManager<T>::Iterator ObjectManager<T>::Begin() {
  size_t index = 0;
  while (index <= nodes_.size() && nodes_[index] == nullptr) {
    ++index;
  }
  return Iterator(this, index);
}

template <class T>
inline ObjectManager<T>::Iterator& ObjectManager<T>::Iterator::operator++() {
  size_t index_temp = index_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    ++index_temp;
  } while (index_temp < nodes.size() && nodes[index_temp] == nullptr);
  assert(index_temp < nodes.size());
  index_ = index_temp;
  return *this;
}

template <class T>
inline ObjectManager<T>::Iterator ObjectManager<T>::Iterator::operator++(int) {
  size_t index_temp = index_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    ++index_temp;
  } while (index_temp < nodes.size() && nodes[index_temp] == nullptr);
  assert(index_temp < nodes.size());
  std::swap(index_temp, index_);
  return Iterator(manager_pointer_, index_temp);
}

template <class T>
inline ObjectManager<T>::Iterator& ObjectManager<T>::Iterator::operator--() {
  size_t index_temp = index_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    --index_temp;
  } while (index_temp != -1 && nodes[index_temp] == nullptr);
  assert(index_temp != -1);
  index_ = index_temp;
  return *this;
}

template <class T>
inline ObjectManager<T>::Iterator ObjectManager<T>::Iterator::operator--(int) {
  size_t index_temp = index_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    --index_temp;
  } while (index_temp != -1 && nodes[index_temp] == nullptr);
  assert(index_temp != -1);
  std::swap(index_temp, index_);
  return Iterator(manager_pointer_, index_temp);
}

template <class T>
inline T& ObjectManager<T>::Iterator::operator*() {
  return *manager_pointer_->GetObject(index_);
}

template <class T>
inline T* ObjectManager<T>::Iterator::operator->() {
  return manager_pointer_->GetObject(index_);
}

}  // namespace frontend::common
#endif  // !COMMON_COMMON_NODE_MANAGER