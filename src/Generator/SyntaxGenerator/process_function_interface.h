/// @file process_function_interface.h
/// @brief ���ļ������װ�û������Լ��������Ļ���
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#include <any>
#include <boost/serialization/base_object.hpp>
#include <cassert>
#include <memory>
#include <vector>

namespace frontend::generator::syntax_generator {
/// @class ProcessFunctionInterface process_function_interface.h
/// @brief ���а�װ�û����庯��������Ӹ�������
/// @details
/// ��������������ҽ���������Reduct�������ú������ع�Լ���û����ص�����
/// ����������һ�ι�Լʱʹ��
class ProcessFunctionInterface {
 public:
  virtual ~ProcessFunctionInterface() {}

  /// @brief �����û�����Ĺ�Լ����
  /// @param[in] word_data ����Լ�Ĳ���ʽ����ÿ������ʽ������
  /// @return �����û���Լ�󷵻ص�����
  /// @details
  /// word_data������˳��Ϊ����ʽ����˳��
  /// @note �չ�Լ�ڵ�洢std::monostate
  virtual std::any Reduct(std::vector<std::any>&& word_data) const = 0;

 private:
  /// @brief �������л������
  friend class boost::serialization::access;

  /// @brief ���л�����ĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  /// ���������������д�ú���
  template <class Archive>
  void serialize(Archive&& ar, const unsigned int version) {}
};

/// @class RootReductClass process_function_interface.h
/// @brief �ڲ�ʵ���ø��ڵ�Ĺ�Լ����
/// @details
/// �﷨��������������ʱ���Զ�����һ���ڲ����ڵ㣬�ø��ڵ�Ϊ���ս����ʽ��
/// ��Ψһ�Ĳ���ʽ��Ϊ�û�����ĸ�����ʽ��
/// �ò���ʽ��֧��ActionType::kAccept����
/// RootReductClass������װ����Ĺ�Լ����
class RootReductClass : public ProcessFunctionInterface {
  /// @brief �����ڲ����ڵ�Ĺ�Լ����
  /// @param[in] word_data ����Լ�Ĳ���ʽ����ÿ������ʽ������
  /// @return ����UserData()
  /// @details
  /// word_data������˳��Ϊ����ʽ����˳��
  /// @note �չ�Լ�ڵ�洢std::monostate
  /// @attention �ڲ����ڵ������ActionType::kAccept���������Լ����
  /// ���øú����ᵼ�´���assert(false)
  virtual std::any Reduct(std::vector<std::any>&& word_data) const override {
    assert(false);
    // ��ֹ����
    return std::any();
  }

 private:
  /// @brief �������л������
  friend class boost::serialization::access;

  /// @brief ���л�����ĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  /// ���������������д�ú���
  template <class Archive>
  void serialize(Archive&& ar, const unsigned int version) {
    ar& boost::serialization::base_object<ProcessFunctionInterface>(*this);
  }
};
}  // namespace frontend::generator::syntax_generator

#endif  /// !GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
