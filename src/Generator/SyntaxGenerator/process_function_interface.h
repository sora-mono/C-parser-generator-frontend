/// @file process_function_interface.h
/// @brief ���ļ������װ�û������Լ��������Ļ���
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#include <any>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <variant>

namespace frontend::generator::syntax_generator {
/// @class ProcessFunctionInterface process_function_interface.h
/// @brief ���а�װ�û����庯��������Ӹ�������
/// @details
/// ��������������ҽ���������Reduct�������ú������ع�Լ���û����ص�����
/// ����������һ�ι�Լʱʹ��
class ProcessFunctionInterface {
 public:
  /// @brief �����û����ص���������
  using UserData = std::any;
  /// @brief �ս����ʽ���ʵ�����
  struct TerminalWordData {
    /// ����
    std::string word;
  };
  /// @brief ���ս����ʽ������
  struct NonTerminalWordData {
    NonTerminalWordData() = default;
    NonTerminalWordData(UserData&& user_data)
        : user_returned_data(std::move(user_data)) {}
    UserData user_returned_data;
  };
  /// @class ProcessFunctionInterface::WordDataToUser
  /// process_function_interface.h
  /// @brief �ṩ���û��Ĺ�Լ�������е�������ʽ������
  /// @note ���಻�洢������������Ϣ���û���ʹ��ʱͨ������ʽ����֪����ͬλ�õ�
  /// �������ս�ڵ��ȡ���ĵ��ʻ��Ƿ��ս�ڵ��Լ�õ�������
  class WordDataToUser {
   public:
    /// @brief ��ȡ�ս����ʽ����const����
    /// @return ���洢������ת��Ϊ�ս����ʽ�������͵�const����
    /// @note ������Ƿ������ȷת��
    const TerminalWordData& GetTerminalWordData() const {
      return std::get<TerminalWordData>(word_data_to_user_);
    }
    /// @brief ��ȡ�ս����ʽ��������
    /// @return ���洢������ת��Ϊ�ս����ʽ�������͵�����
    /// @note ������Ƿ������ȷת��
    TerminalWordData& GetTerminalWordData() {
      return const_cast<TerminalWordData&>(
          const_cast<const WordDataToUser&>(*this).GetTerminalWordData());
    }
    /// @brief ��ȡ���ս����ʽ����const����
    /// @return ���洢������ת��Ϊ���ս����ʽ�������͵�const����
    /// @note ������Ƿ������ȷת��
    const NonTerminalWordData& GetNonTerminalWordData() const {
      return std::get<NonTerminalWordData>(word_data_to_user_);
    }
    /// @brief ��ȡ���ս����ʽ��������
    /// @return ���洢������ת��Ϊ���ս����ʽ�������͵�����
    /// @note ������Ƿ������ȷת��
    NonTerminalWordData& GetNonTerminalWordData() {
      return const_cast<NonTerminalWordData&>(
          const_cast<const WordDataToUser&>(*this).GetNonTerminalWordData());
    }
    /// @brief ��ȡ�Ƿ��в���ʽ����
    /// @return �����Ƿ�Я������ʽ����
    /// @retval true ��Я������ʽ����
    /// @retval false ����Я������ʽ���ݣ���Ӧ����ʽ�չ�Լ��
    bool Empty() const {
      return std::get_if<std::monostate>(&word_data_to_user_) != nullptr;
    }

    /// @brief ���ô洢������
    /// @param[in] word_data ��������
    /// @note ��֧��std::monostate��TerminalWordData��NonTerminalWordData��
    /// const��ֵ���ú���ֵ����
    template <class BasicObjectType>
    void SetWordDataToUser(BasicObjectType&& word_data) {
      word_data_to_user_ = std::forward<BasicObjectType>(word_data);
    }

   private:
    /// @brief �õ��ʵ�����
    /// @note �չ�Լ�ڵ�洢std::monostate
    std::variant<std::monostate, TerminalWordData, NonTerminalWordData>
        word_data_to_user_;
  };

  virtual ~ProcessFunctionInterface() {}

  /// @brief �����û�����Ĺ�Լ����
  /// @param[in] word_data ����Լ�Ĳ���ʽ����ÿ������ʽ������
  /// @return �����û���Լ�󷵻ص�����
  /// @details
  /// word_data������˳��Ϊ����ʽ����˳��
  /// @note �չ�Լ�ڵ�洢std::monostate
  virtual UserData Reduct(std::vector<WordDataToUser>&& word_data) const = 0;

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
  virtual UserData Reduct(
      std::vector<WordDataToUser>&& word_data) const override {
    assert(false);
    // ��ֹ����
    return UserData();
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
