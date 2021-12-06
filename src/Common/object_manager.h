/// @file object_manager.h
/// @brief �������ͨ������ID��ʶ����ID�������һһӳ��Ĺ�ϵ
/// @details
/// �������Ƴ���Ϊ��ͨ�����ͱ�ʶ���󣬴Ӷ�����ʹ��unordered�������洢��ʶ�����Ч��
/// @attention
/// Ϊ��֧�����л���ʹ����boost::serialization������ʱ��Ҫ��boost������
#ifndef COMMON_OBJECT_MANAGER_H_
#define COMMON_OBJECT_MANAGER_H_

#include <assert.h>

#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>

#include "Common/id_wrapper.h"

namespace frontend::common {

/// @class ObjectManager object_manager.h
/// @brief ������󣬶�����IDһһ��Ӧ
template <class T>
class ObjectManager {
 public:
  /// @brief ��������ObjectId���͵ķַ���ǩ
  enum class WrapperLabel { kObjectId };
  /// @brief ��ʶ�����ID�������һһ��Ӧ
  using ObjectId =
      ExplicitIdWrapper<size_t, WrapperLabel, WrapperLabel::kObjectId>;

  ObjectManager() {}
  ObjectManager(const ObjectManager&) = delete;
  ObjectManager(ObjectManager&&) = delete;
  ~ObjectManager();

  /// @class ObjectManager::Iterator object_manager.h
  /// @brief �������ĵ�����
  class Iterator {
   public:
    Iterator() : manager_pointer_(nullptr), id_(ObjectId::InvalidId()) {}
    Iterator(ObjectManager<T>* manager_pointer, ObjectId id)
        : manager_pointer_(manager_pointer), id_(id) {}
    Iterator(const Iterator& iter)
        : manager_pointer_(iter.manager_pointer_), id_(iter.id_) {}

    /// @brief ��ȡ�õ�������Ч��ID
    /// @return ���ظõ�������Ч��ID
    /// @note ����õ�����Ϊ��β�򷵻�ObjectId::InvalidId()
    ObjectId GetId() const { return id_; }
    /// @brief ���øõ������󶨵�������
    /// @param[in] manager_pointer ���õ������󶨵�������
    /// @attention ����������nullptr
    void SetManagerPointer(ObjectManager<T>* manager_pointer) {
      assert(manager_pointer);
      manager_pointer_ = manager_pointer;
    }
    /// @brief ���øõ�������Ч��ID
    /// @param[in] id ��Ҫ���õ�ID
    /// @attention ����ʹ����ЧID�������Ѿ����ð󶨵�������
    void SetId(ObjectId id);

    /// @brief ����ƶ�������ֱ����һ����Ч�����β��ʹ��ǰ׺++����
    /// @return �����ƶ���ĵ����������ã�*this��
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// ������Գ�β���øú���
    Iterator& operator++();
    /// @brief ����ƶ�������ֱ����һ����Ч�����β��ʹ�ú�׺++����
    /// @return �����ƶ�ǰ�ĵ�����
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// ������Գ�β���øú���
    Iterator operator++(int);
    /// @brief ��ǰ�ƶ�������ֱ����һ����Ч����ʹ��ǰ׺--����
    /// @return �����ƶ���ĵ����������ã�*this��
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// �������ָ���һ����Ч����ĵ��������øú���
    Iterator& operator--();
    /// @brief ��ǰ�ƶ�������ֱ����һ����Ч����ʹ�ú�׺--����
    /// @return �����ƶ�ǰ�ĵ�����
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// �������ָ���һ����Ч����ĵ��������øú���
    Iterator operator--(int);
    /// @brief �Ե�����������
    /// @return ���ص�����ָ��Ķ��������
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    T& operator*() const;
    /// @brief �Ե�����������
    /// @return ���ص�����ָ��Ķ����ָ��
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    T* operator->() const;
    /// @brief �ж������������Ƿ���ͬ������ͬ��������ָ����ͬλ�ã�
    /// @param[in] iter �����жϵ���һ��������
    /// @return �����������Ƿ���ͬ
    /// @retval true ��������������ͬ
    /// @retval false ��������������ͬ
    /// @note ������*this
    bool operator==(const Iterator& iter) const {
      return manager_pointer_ == iter.manager_pointer_ && id_ == iter.id_;
    }
    /// @brief �ж������������Ƿ�ͬ���벻ͬ�����󶨻�ָ��ͬλ�ã�
    /// @param[in] iter �����жϵ���һ��������
    /// @return �����������Ƿ�ͬ
    /// @retval true ��������������ͬ
    /// @retval false ��������������ͬ
    /// @note ������*this
    bool operator!=(const Iterator& iter) const {
      return !this->operator==(iter);
    }

   private:
    /// @brief �������󶨵�������
    ObjectManager<T>* manager_pointer_;
    /// @brief ������ָ��Ķ���λ��
    ObjectId id_;
  };
  /// @class ObjectManager::ConstIterator object_manager.h
  /// @brief ��������const������
  class ConstIterator {
   public:
    ConstIterator() : manager_pointer_(nullptr), id_(ObjectId::InvalidId()) {}
    ConstIterator(const ObjectManager<T>* manager_pointer, ObjectId id)
        : manager_pointer_(manager_pointer), id_(id) {}
    ConstIterator(const Iterator& iterator)
        : manager_pointer_(iterator.manager_pointer), id_(iterator.id_) {}
    ConstIterator(const ConstIterator& iter)
        : manager_pointer_(iter.manager_pointer_), id_(iter.id_) {}

    /// @brief ��ȡ�õ�������Ч��ID
    /// @return ���ظõ�������Ч��ID
    /// @note ����õ�����Ϊ��β�򷵻�ObjectId::InvalidId()
    ObjectId GetId() const { return id_; }
    /// @brief ���øõ������󶨵�������
    /// @param[in] manager_pointer ���õ������󶨵�������
    /// @attention ����������nullptr
    void SetManagerPointer(const ObjectManager<T>* manager_pointer) {
      manager_pointer_ = manager_pointer;
    }
    /// @brief ���øõ�������Ч��ID
    /// @param[in] id ��Ҫ���õ�ID
    /// @attention ����ʹ����ЧID�������Ѿ����ð󶨵�������
    void SetId(ObjectId id) {
      assert(manager_pointer_ != nullptr &&
             id < manager_pointer_->nodes_.size());
      id_ = id;
    }

    /// @brief ����ƶ�������ֱ����һ����Ч�����β��ʹ��ǰ׺++����
    /// @return �����ƶ���ĵ����������ã�*this��
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// ������Գ�β���øú���
    ConstIterator& operator++();
    /// @brief ����ƶ�������ֱ����һ����Ч�����β��ʹ�ú�׺++����
    /// @return �����ƶ�ǰ�ĵ�����
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// ������Գ�β���øú���
    ConstIterator operator++(int);
    /// @brief ��ǰ�ƶ�������ֱ����һ����Ч����ʹ��ǰ׺--����
    /// @return �����ƶ���ĵ����������ã�*this��
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// �������ָ���һ����Ч����ĵ��������øú���
    ConstIterator& operator--();
    /// @brief ��ǰ�ƶ�������ֱ����һ����Ч����ʹ�ú�׺--����
    /// @return �����ƶ�ǰ�ĵ�����
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    /// �������ָ���һ����Ч����ĵ��������øú���
    ConstIterator operator--(int);
    /// @brief �Ե�����������
    /// @return ���ص�����ָ��Ķ����const����
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    const T& operator*() const { return manager_pointer_->GetObject(id_); }
    /// @brief �Ե�����������
    /// @return ���ص�����ָ��Ķ����constָ��
    /// @attention ���øú���ǰ�����Ѿ����õ�ЧID�Ͱ󶨵�������
    const T* operator->() const { return &manager_pointer_->GetObject(id_); }
    /// @brief �ж������������Ƿ���ͬ������ͬ��������ָ����ͬλ�ã�
    /// @param[in] iter �����жϵ���һ��������
    /// @return �����������Ƿ���ͬ
    /// @retval true ��������������ͬ
    /// @retval false ��������������ͬ
    /// @note ������*this
    bool operator==(const ConstIterator& iter) const {
      return manager_pointer_ == iter.manager_pointer_ && id_ == iter.id_;
    }
    /// @brief �ж������������Ƿ�ͬ���벻ͬ�����󶨻�ָ��ͬλ�ã�
    /// @param[in] iter �����жϵ���һ��������
    /// @return �����������Ƿ�ͬ
    /// @retval true ��������������ͬ
    /// @retval false ��������������ͬ
    /// @note ������*this
    bool operator!=(const ConstIterator& iter) const {
      return !this->operator==(iter);
    }

   private:
    /// @brief �������󶨵�������
    const ObjectManager<T>* manager_pointer_;
    /// @brief ������ָ��Ķ���λ��
    ObjectId id_;
  };

  /// @brief Ĭ�������ϲ���������ĺ���
  /// @param[in] node_dst ���ϲ����Ķ��󣬸ö����ڳɹ��ϲ���ᱻ����
  /// @param[in] node_src �����ϲ��Ķ��󣬸ö����ڳɹ��ϲ���ᱻɾ��
  /// @return ���غϲ��Ƿ�ɹ�
  /// @retval true ���ϲ��ɹ�
  /// @retval false ���ϲ�ʧ��
  static bool DefaultMergeFunction2(T& node_dst, T& node_src) {
    return node_dst.MergeNode(node_src);
  }
  /// @brief Ĭ�������ڹ����������ºϲ���������ĺ���
  /// @param[in] node_dst ���ϲ����Ķ��󣬸ö����ڳɹ��ϲ���ᱻ����
  /// @param[in] node_src �����ϲ��Ķ��󣬸ö����ڳɹ��ϲ���ᱻɾ��
  /// @param[in,out] manager ���ϲ�����������ƺϲ�����
  /// @return ���غϲ��Ƿ�ɹ�
  /// @retval true ���ϲ��ɹ�
  /// @retval false ���ϲ�ʧ��
  template <class Manager>
  static bool DefaultMergeFunction3(T& node_dst, T& node_src,
                                    Manager& manager) {
    return manager.MergeNodes(node_dst, node_src);
  }
  /// @brief ��ȡ�����const����
  /// @param[in] id ��Ҫ��ȡconst���õĶ����ID
  /// @return ���ػ�ȡ���Ķ����const����
  /// @attention id����ָ����Ч�ĵĶ���
  const T& GetObject(ObjectId id) const;
  /// @brief ��ȡ���������
  /// @param[in] id ��Ҫ��ȡ���õĶ����ID
  /// @return ���ػ�ȡ���Ķ��������
  /// @attention id����ָ����Ч�ĵĶ���
  T& GetObject(ObjectId id);
  /// @brief ��ȡ�����const����
  /// @param[in] id ��Ҫ��ȡconst���õĶ����ID
  /// @return ���ػ�ȡ���Ķ����const����
  /// @note ��GetObject��Ч
  /// @attention id����ָ����Ч�ĵĶ���
  const T& operator[](ObjectId id) const;
  /// @brief ��ȡ���������
  /// @param[in] id ��Ҫ��ȡ���õĶ����ID
  /// @return ���ػ�ȡ���Ķ��������
  /// @note ��GetObject��Ч
  /// @attention id����ָ����Ч�ĵĶ���
  T& operator[](ObjectId id);

  /// @brief ϵͳ����ѡ�����λ�÷��ö���
  /// @param[in,out] args �������캯��ʹ�õĲ���
  /// @return ����Ψһ��Ӧ�ڶ����ID
  template <class ObjectType = T, class... Args>
  ObjectId EmplaceObject(Args&&... args);

  /// @brief ��ɾ���ڵ��¼�����ͷŽڵ��ڴ�
  /// @param[in] id ��Ҫɾ���Ķ����ID
  /// @return ����ָ������ָ��
  /// @attention ��α���ָ����Ч�Ķ���
  [[nodiscard]] T* RemoveObjectNoDelete(ObjectId id);
  /// @brief ɾ���ڵ㲢�ͷŽڵ��ڴ棨Ĭ��ɾ�����壩
  /// @param[in] id ��Ҫɾ���Ķ����ID
  /// @return һ������true
  /// @attention ��α���ָ����Ч�Ķ���
  bool RemoveObject(ObjectId id);

  /// @brief �ϲ��������󣬺ϲ��ɹ���ɾ��id_src����
  /// @param[in] id_dst ��Ҫ�ϲ����Ķ��󣨺ϲ�������
  /// @param[in] id_src �������ϲ��Ķ��󣨳ɹ��ϲ���ɾ����
  /// @param[in] merge_function
  /// �����ƺϲ����̵ĺ�������һ������Ϊid_dst��Ӧ�Ķ���
  /// �ڶ�������Ϊid_src��Ӧ�Ķ���
  /// @return �����Ƿ�ϲ��ɹ�
  /// @retval false
  /// ���ϲ�ͬһ�������id_src��Ӧ�Ľڵ㲻����ϲ���merge_function����false
  /// @retval true ���ϲ��ɹ�
  /// @attention �ϲ�ʧ���򲻻�ɾ��Դ����id_dst��id_src����ָ���Ѵ��ڵĶ���
  bool MergeObjects(ObjectId id_dst, ObjectId id_src,
                    const std::function<bool(T&, T&)>& merge_function =
                        DefaultMergeFunction2);
  /// @brief �ϲ��������󣬺ϲ��ɹ���ɾ��id_src�ڵ�
  /// @param[in] id_dst ��Ҫ�ϲ����Ķ��󣨺ϲ�������
  /// @param[in] id_src �������ϲ��Ķ��󣨳ɹ��ϲ���ɾ����
  /// @param[in] manager ���ϲ����������ڵ���merge_functionʱʹ��
  /// @param[in] merge_function
  /// �����ƺϲ����̵ĺ�������һ������Ϊid_dst��Ӧ�Ķ���
  /// �ڶ�������Ϊid_src��Ӧ�Ķ��󣬵���������Ϊ�����manager
  /// @return �����Ƿ�ϲ��ɹ�
  /// @retval false
  /// ���ϲ�ͬһ�������id_src��Ӧ�Ľڵ㲻����ϲ���merge_function����false
  /// @retval true ���ϲ��ɹ�
  /// @attention �ϲ�ʧ���򲻻�ɾ��Դ����id_dst��id_src����ָ���Ѵ��ڵĶ���
  template <class Manager>
  bool MergeObjectsWithManager(ObjectId id_dst, ObjectId id_src,
                               Manager& manager,
                               const std::function<bool(T&, T&, Manager&)>&
                                   merge_function = DefaultMergeFunction3);
  /// @brief ��ѯID��Ӧ�Ľڵ��Ƿ�ɺϲ�
  /// @param[in] id ������ѯ�Ľڵ�ID
  /// @return �����ڵ��Ƿ�ɺϲ�
  /// @retval true �����Ժϲ�
  /// @retval false �������Ժϲ�
  /// @attention ����Ĳ�������ָ���Ѵ��ڵĶ���
  bool CanBeSourceInMerge(ObjectId id) const {
    assert(id < nodes_can_be_source_in_merge.size() && nodes_[id] != nullptr);
    return nodes_can_be_source_in_merge[id];
  }
  /// @brief ���ø��������ںϲ�ʱ������ΪԴ���󣨺ϲ��ɹ���Դ�����ͷţ�
  /// @param[in] id �������õĶ���ID
  /// @return һ������true
  /// @attention ����Ĳ�������ָ���Ѵ��ڵĶ���
  bool SetObjectCanBeSourceInMerge(ObjectId id);
  /// ���ø��������ںϲ�ʱ������ΪԴ���󣨺ϲ��ɹ���Դ�����ͷţ�
  /// @param[in] id �������õĶ���ID
  /// @return һ������true
  /// @attention ����Ĳ�������ָ���Ѵ��ڵĶ���
  bool SetObjectCanNotBeSourceInMerge(ObjectId id);
  /// @brief �������ж����ںϲ�ʱ��������ΪԴ���󣨺ϲ��ɹ���Դ�����ͷţ�
  void SetAllObjectsCanBeSourceInMerge();
  /// @brief �������ж����ںϲ�ʱ����������ΪԴ���󣨺ϲ��ɹ���Դ�����ͷţ�
  void SetAllObjectsCanNotBeSourceInMerge();

  /// @brief �ж�����ID�Ƿ�ָ��ͬһ����
  /// @param[in] id1 ��Ҫ�жϵ�һ��ID
  /// @param[in] id2 ��Ҫ�жϵ���һ��ID
  /// @return ��������ID�Ƿ�ָ����ͬ����
  /// @retval true ������IDָ����ͬ����
  /// @retval false ������IDָ��ͬ����
  /// @note ����������ͬID
  bool IsSame(ObjectId id1, ObjectId id2) const { return id1 == id2; }

  /// @brief ������������������
  /// @param[in,out] manager_other ��Ҫ��������һ������
  /// @note ������this������ִ�������Ľ�������
  /// @attention ���������ָ��
  void Swap(ObjectManager* manager_other);

  /// @brief ��ȡ�����ܴ洢�Ķ������
  /// @return ���������ܴ洢�Ķ������
  size_t Capacity() const { return nodes_.size(); }
  /// @brief ��ȡ����ʵ�ʴ洢����Ч��������
  /// @return ����������ʵ�ʴ洢����Ч�������
  size_t Size() const;

  /// @brief ��ʼ������������д��ڽڵ���ȫ���ͷ�
  void ObjectManagerInit();
  /// @brief ������нڵ��¼�����ͷ�
  void ClearNoRelease();
  /// @brief ���ó�Ա������shrink_to_fit����ʡ�ռ�
  void ShrinkToFit();

  /// @brief ��ȡָ��β�ĵ�����
  /// @return ����ָ��β�ĵ�����
  Iterator End() { return Iterator(this, ObjectId(nodes_.size())); }
  /// @brief ��ȡָ���һ����Ч�ڵ�ĵ�����
  /// @return ����ָ���һ����Ч�ڵ�ĵ�����
  /// @attention �����������Ч�ڵ��򷵻�End()
  Iterator Begin();
  /// @brief ��ȡָ��β��const������
  /// @return ����ָ��β��const������
  ConstIterator ConstEnd() const {
    return ConstIterator(this, ObjectId(nodes_.size()));
  }
  /// @brief ��ȡָ���һ����Ч�ڵ��const������
  /// @return ����ָ���һ����Ч�ڵ��const������
  /// @attention �����������Ч�ڵ��򷵻�ConstEnd()
  ConstIterator ConstBegin() const;

 private:
  /// @brief ������Ԫ������boost-serialization������ڲ���Ա
  friend class boost::serialization::access;

  /// @brief ���л�����
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& nodes_;
    ar& removed_ids_;
    ar& nodes_can_be_source_in_merge;
  }

  /// @brief ��ָ��λ�÷��ö���
  /// @tparam ObjectType �����õĶ������ͣ�Ĭ��ʹ����������ʱָ���Ķ�������
  /// ����ʹ�ø����͵�����������
  /// @param[in] id ��Ҫ���ö����λ�ã�����Ϊ���ڵ���nodes_.size()��ֵ
  /// @param[in] args ������Ĺ��캯������
  /// @return ����id
  template <class ObjectType = T, class... Args>
  ObjectId EmplaceObjectIndex(ObjectId id, Args&&... args);
  /// @brief ��ָ��λ�÷���ָ��
  /// @param[in] id �����ö����λ��
  /// @param[in] pointer �������ָ��
  /// @return ����id
  /// @attention ���������ָ��
  ObjectId EmplacePointerIndex(ObjectId id, T* pointer);

  /// @brief ��ȡ��ǰ��ѿ���ID
  /// @return ���ص�ǰ��ѿ���ID
  /// @note ����ѡ��removed_ids_�е�ID������������򷵻�nodes_.size()
  ObjectId GetBestEmptyIndex();
  /// @brief ������Ƴ���ID
  /// @param[in] id �����Ƴ��Ķ����Ӧ��ID
  void AddRemovedIndex(ObjectId id) { removed_ids_.push_back(id); }

  /// @brief ���ָ��ڵ��ָ�룬�ڵ㲻��������Ϊnullptr
  std::vector<T*> nodes_;
  /// @brief ������б�ɾ���ڵ��Ӧ��ID�����ܴ���nodes_.size()��
  std::vector<ObjectId> removed_ids_;
  /// @brief �洢�ڵ��ںϲ�ʱ�Ƿ�������ΪԴ�ڵ�
  std::vector<bool> nodes_can_be_source_in_merge;
};

template <class T>
ObjectManager<T>::~ObjectManager() {
  for (T* ptr : nodes_) {
    delete ptr;
  }
}

template <class T>
inline const T& ObjectManager<T>::GetObject(ObjectId id) const {
  assert(id < nodes_.size() && nodes_[id] != nullptr);
  return *nodes_[id];
}
template <class T>
inline T& frontend::common::ObjectManager<T>::GetObject(ObjectId id) {
  return const_cast<T&>(
      static_cast<const ObjectManager<T>&>(*this).GetObject(id));
}

template <class T>
inline bool ObjectManager<T>::RemoveObject(ObjectId id) {
  T* removed_pointer = RemoveObjectNoDelete(id);
  delete removed_pointer;
  return true;
}

template <class T>
inline T* ObjectManager<T>::RemoveObjectNoDelete(ObjectId id) {
  assert(id < nodes_.size());
  T* temp_pointer = nodes_[id];
  nodes_[id] = nullptr;
  assert(temp_pointer != nullptr);
  AddRemovedIndex(id);
  nodes_can_be_source_in_merge[id] = false;
  while (nodes_.size() > 0 && nodes_.back() == nullptr) {
    nodes_.pop_back();  /// ����ĩβ��ЧID
    nodes_can_be_source_in_merge.pop_back();
  }
  return temp_pointer;
}

template <class T>
template <class ObjectType, class... Args>
inline ObjectManager<T>::ObjectId ObjectManager<T>::EmplaceObject(
    Args&&... args) {
  ObjectId id = GetBestEmptyIndex();
  T* object_pointer = new ObjectType(std::forward<Args>(args)...);
  ObjectId result = EmplacePointerIndex(id, object_pointer);
  assert(result.IsValid());
  return result;
}

template <class T>
template <class ObjectType, class... Args>
inline ObjectManager<T>::ObjectId ObjectManager<T>::EmplaceObjectIndex(
    ObjectId id, Args&&... args) {
  T* pointer = new ObjectType(std::forward<Args>(args)...);
  ObjectId result = EmplacePointerIndex(id, pointer);
  assert(result.IsValid());
  return result;
}

template <class T>
inline ObjectManager<T>::ObjectId ObjectManager<T>::EmplacePointerIndex(
    ObjectId id, T* pointer) {
  assert(pointer != nullptr);
  size_t size_old = nodes_.size();
  if (id >= size_old) {
    nodes_.resize(id + 1, nullptr);
    nodes_can_be_source_in_merge.resize(id + 1, false);
    for (size_t i = size_old; i < id; i++) {
      removed_ids_.push_back(ObjectId(i));
    }
  }
  /// �����Ը������зǿ��Ҳ�ͬ��ָ��
  assert(!(nodes_[id] != nullptr && nodes_[id] != pointer));
  nodes_[id] = pointer;
  return id;
}

template <class T>
inline void ObjectManager<T>::Swap(ObjectManager* manager_other) {
  nodes_.swap(manager_other->nodes_);
  removed_ids_.swap(manager_other->removed_ids_);
  nodes_can_be_source_in_merge.swap(
      manager_other->nodes_can_be_source_in_merge);
}

template <class T>
inline bool ObjectManager<T>::SetObjectCanBeSourceInMerge(ObjectId id) {
  assert(id < nodes_.size() && nodes_[id] != nullptr);
  nodes_can_be_source_in_merge[id] = true;
  return true;
}

template <class T>
inline bool ObjectManager<T>::SetObjectCanNotBeSourceInMerge(ObjectId id) {
  assert(id < nodes_.size() && nodes_[id] != nullptr);
  nodes_can_be_source_in_merge[id] = false;
  return true;
}

template <class T>
inline void ObjectManager<T>::SetAllObjectsCanBeSourceInMerge() {
  for (size_t id = 0; id < nodes_.size(); id++) {
    if (nodes_[id] != nullptr) {
      nodes_can_be_source_in_merge[id] = true;
    }
  }
}

template <class T>
inline void ObjectManager<T>::SetAllObjectsCanNotBeSourceInMerge() {
  std::vector<bool> vec_temp(nodes_can_be_source_in_merge.size(), false);
  nodes_can_be_source_in_merge.Swap(vec_temp);
}

template <class T>
inline bool ObjectManager<T>::MergeObjects(
    ObjectId id_dst, ObjectId id_src,
    const std::function<bool(T&, T&)>& merge_function) {
  // ����Ƿ�ϲ�ͬһ���ڵ�
  if (id_dst == id_src) [[unlikely]] {
    return false;
  }
  T& object_dst = GetObject(id_dst);
  T& object_src = GetObject(id_src);
  if (!CanBeSourceInMerge(id_src)) {
    /// ����Դ�ڵ㲻������Ϊ�ϲ�ʱ��Դ�ڵ�
    return false;
  }
  if (!merge_function(object_dst, object_src)) {
    /// �ϲ�ʧ��
    return false;
  }
  RemoveObject(id_src);
  return true;
}

template <class T>
template <class Manager>
bool ObjectManager<T>::MergeObjectsWithManager(
    typename ObjectManager<T>::ObjectId id_dst,
    typename ObjectManager<T>::ObjectId id_src, Manager& manager,
    const std::function<bool(T&, T&, Manager&)>& merge_function) {
  T& object_dst = GetObject(id_dst);
  T& object_src = GetObject(id_src);
  if (!CanBeSourceInMerge(id_src)) {
    /// ����Դ�ڵ㲻������Ϊ�ϲ�ʱ��Դ�ڵ�
    return false;
  }
  if (!merge_function(object_dst, object_src, manager)) {
    /// �ϲ�ʧ��
    return false;
  }
  RemoveObject(id_src);
  return true;
}

template <class T>
inline void ObjectManager<T>::ObjectManagerInit() {
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
    nodes_can_be_source_in_merge.pop_back();
  }
  nodes_.shrink_to_fit();
  removed_ids_.shrink_to_fit();
}

template <class T>
inline size_t ObjectManager<T>::Size() const {
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
  ObjectId id = ObjectId::InvalidId();
  if (removed_ids_.size() != 0) {
    while (!id.IsValid() && removed_ids_.size() != 0)  /// ������ЧID
    {
      if (removed_ids_.back() < nodes_.size()) {
        id = removed_ids_.back();
      }
      removed_ids_.pop_back();
    }
  }
  if (!id.IsValid())  /// ����Ч��ɾ��ID
  {
    id = ObjectId(nodes_.size());
  }
  return id;
}

template <class T>
inline const T& ObjectManager<T>::operator[](ObjectId id) const {
  assert(id < nodes_.size());
  return GetObject(id);
}
template <class T>
inline T& ObjectManager<T>::operator[](ObjectId id) {
  return const_cast<T&>(
      static_cast<const ObjectManager<T>&>(*this).operator[](id));
}

template <class T>
inline void ObjectManager<T>::Iterator::SetId(ObjectId id) {
  assert(manager_pointer_ != nullptr && id < manager_pointer_->nodes_.size());
  id_ = id;
}

template <class T>
inline ObjectManager<T>::Iterator ObjectManager<T>::Begin() {
  ObjectId id(0);
  while (id <= nodes_.size() && nodes_[id] == nullptr) {
    ++id;
  }
  return Iterator(this, id);
}

template <class T>
inline ObjectManager<T>::ConstIterator ObjectManager<T>::ConstBegin() const {
  ObjectId id(0);
  while (id <= nodes_.size() && nodes_[id] == nullptr) {
    ++id;
  }
  return ConstIterator(this, id);
}

template <class T>
inline ObjectManager<T>::Iterator& ObjectManager<T>::Iterator::operator++() {
  assert(*this != manager_pointer_->End());
  auto& nodes = manager_pointer_->nodes_;
  do {
    ++id_;
  } while (id_ < nodes.size() && nodes[id_] == nullptr);
  if (id_ >= nodes.size()) [[likely]] {
    *this = manager_pointer_->End();
  }
  return *this;
}

template <class T>
inline ObjectManager<T>::Iterator ObjectManager<T>::Iterator::operator++(int) {
  assert(*this != manager_pointer_->End());
  ObjectId id_temp = id_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    ++id_temp;
  } while (id_temp < nodes.size() && nodes[id_temp] == nullptr);
  if (id_temp < nodes.size()) [[likely]] {
    std::swap(id_temp, id_);
    return Iterator(manager_pointer_, id_temp);
  } else {
    *this = manager_pointer_->End();
    return manager_pointer_->End();
  }
}

template <class T>
inline ObjectManager<T>::Iterator& ObjectManager<T>::Iterator::operator--() {
  assert(id_.IsValid() && *this != manager_pointer_->End());
  auto& nodes = manager_pointer_->nodes_;
  do {
    --id_;
    assert(id_ != -1);
  } while (nodes[id_] == nullptr);
  return *this;
}

template <class T>
inline ObjectManager<T>::Iterator ObjectManager<T>::Iterator::operator--(int) {
  assert(id_.IsValid() && *this != manager_pointer_->End());
  size_t temp_id = id_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    --temp_id;
    assert(temp_id != -1);
  } while (nodes[temp_id] == nullptr);
  std::swap(temp_id, id_);
  return Iterator(manager_pointer_, temp_id);
}

template <class T>
inline T& ObjectManager<T>::Iterator::operator*() const {
  return manager_pointer_->GetObject(id_);
}

template <class T>
inline T* ObjectManager<T>::Iterator::operator->() const {
  return &manager_pointer_->GetObject(id_);
}

template <class T>
inline ObjectManager<T>::ConstIterator&
ObjectManager<T>::ConstIterator::operator++() {
  assert(*this != manager_pointer_->ConstEnd());
  auto& nodes = manager_pointer_->nodes_;
  do {
    ++id_;
  } while (id_ < nodes.size() && nodes[id_] == nullptr);
  if (id_ >= nodes.size()) [[likely]] {
    *this = manager_pointer_->ConstEnd();
  }
  return *this;
}

template <class T>
inline ObjectManager<T>::ConstIterator
ObjectManager<T>::ConstIterator::operator++(int) {
  assert(*this != manager_pointer_->ConstEnd());
  ObjectId id_temp = id_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    ++id_temp;
  } while (id_temp < nodes.size() && nodes[id_temp] == nullptr);
  if (id_temp < nodes.size()) [[likely]] {
    std::swap(id_temp, id_);
    return ConstIterator(manager_pointer_, id_temp);
  } else {
    *this = manager_pointer_->ConstEnd();
    return manager_pointer_->ConstEnd();
  }
}

template <class T>
inline ObjectManager<T>::ConstIterator&
ObjectManager<T>::ConstIterator::operator--() {
  assert(id_.IsValid() && *this != manager_pointer_->End());
  auto& nodes = manager_pointer_->nodes_;
  do {
    --id_;
    assert(id_ != -1);
  } while (nodes[id_] == nullptr);
  return *this;
}

template <class T>
inline ObjectManager<T>::ConstIterator
ObjectManager<T>::ConstIterator::operator--(int) {
  assert(id_.IsValid() && *this != manager_pointer_->End());
  size_t temp_id = id_;
  auto& nodes = manager_pointer_->nodes_;
  do {
    --temp_id;
    assert(temp_id != -1);
  } while (nodes[temp_id] == nullptr);
  std::swap(temp_id, id_);
  return ConstIterator(manager_pointer_, temp_id);
}
}  // namespace frontend::common
#endif  /// !COMMON_COMMON_NODE_MANAGER