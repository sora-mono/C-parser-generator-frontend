#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#include <any>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

namespace frontend::generator::syntax_generator {
// �ӿ��࣬�����û����庯�����Ӹ�������
class ProcessFunctionInterface {
 public:
  using UserData = std::any;
  // �սᵥ�ʵ�����
  struct TerminalWordData {
    // ����
    std::string word;
  };
  // ���ս�ڵ������
  struct NonTerminalWordData {
    NonTerminalWordData() = default;
    NonTerminalWordData(UserData&& user_data)
        : user_returned_data(std::move(user_data)) {}
    UserData user_returned_data;
  };
  // �洢����ʽ�е����ڵ���Я������Ϣ
  class WordDataToUser {
   public:
    const TerminalWordData& GetTerminalWordData() const {
      return boost::get<TerminalWordData>(word_data_to_user);
    }
    TerminalWordData& GetTerminalWordData() {
      return const_cast<TerminalWordData&>(
          const_cast<const WordDataToUser&>(*this).GetTerminalWordData());
    }
    const NonTerminalWordData& GetNonTerminalWordData() const {
      return boost::get<NonTerminalWordData>(word_data_to_user);
    }
    NonTerminalWordData& GetNonTerminalWordData() {
      return const_cast<NonTerminalWordData&>(
          const_cast<const WordDataToUser&>(*this).GetNonTerminalWordData());
    }

    template <class BasicObjectType>
    void SetWordDataToUser(BasicObjectType&& word_data) {
      word_data_to_user = std::forward<BasicObjectType>(word_data);
    }

   private:
    // �õ��ʵ�����
    // �չ�Լ�ڵ�洢NonTerminalWordData
    // ����NonTerminalWordData::user_data_Ϊ��
    boost::variant<TerminalWordData, NonTerminalWordData> word_data_to_user;
  };

  virtual ~ProcessFunctionInterface() {}

  // ����Ϊ����ʽ����˳��
  // ���ص�ֵ��Ϊ����ò���ʽ��Լ�õ��ķ��ս������һ�β����Լ���ݵĲ���
  // �չ�Լ�ڵ��node_type��ProductionNodeType::kEndNode
  virtual UserData Reduct(std::vector<WordDataToUser>&& word_data) const = 0;

 private:
  // �������л������
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive&& ar, const unsigned int version) {}
};

// �ڲ�ʵ���ø��ڵ�Ĺ�Լ����
class RootReductClass : public ProcessFunctionInterface {
  virtual UserData Reduct(
      std::vector<WordDataToUser>&& word_data) const override {
    return UserData();
  }

 private:
  // �������л������
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive&& ar, const unsigned int version) {
    ar& boost::serialization::base_object<ProcessFunctionInterface>(*this);
  }
};
}  // namespace frontend::generator::syntax_generator

#endif  // !GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
