#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
#include <any>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include "Common/common.h"
namespace frontend::generator::syntaxgenerator {
// �ӿ��࣬�����û����庯�����Ӹ�������
class ProcessFunctionInterface {
 public:
  using UserData = std::any;
  // �սᵥ�ʵ�����
  struct TerminalWordData {
    // ����
    std::string word;
    // ������������
    size_t line;
  };
  // ���ս�ڵ������
  struct NonTerminalWordData {
    NonTerminalWordData() = default;
    NonTerminalWordData(UserData&& user_data)
        : user_data_(std::move(user_data)) {}
    NonTerminalWordData(NonTerminalWordData&&) = default;
    NonTerminalWordData(const NonTerminalWordData&) = default;
    NonTerminalWordData& operator=(const NonTerminalWordData&) = default;
    UserData user_data_;
  };
  // �洢����ʽ�е����ڵ���Я������Ϣ
  struct WordDataToUser {
    // ���з��ս�ڵ���ս�ڵ����ֿ���
    frontend::common::ProductionNodeType node_type;
    // �õ��ʵ�����
    boost::variant<TerminalWordData, NonTerminalWordData> word_data_to_user_;
    TerminalWordData& GetTerminalWordData() {
      return boost::get<TerminalWordData>(word_data_to_user_);
    }
    NonTerminalWordData& GetNonTerminalWordData() {
      return boost::get<NonTerminalWordData>(word_data_to_user_);
    }
    template <class T>
    void SetWordData(T&& word_data) {
      word_data_to_user_ = word_data;
    }
  };

  virtual ~ProcessFunctionInterface();

  // �������Խ����ջʱ��Խ��
  // ���ص�ֵ��Ϊ����ò���ʽ��Լ�õ��ķ��ս������һ�β����Լ���ݵĲ���
  // �չ�Լ�ڵ��node_type��ProductionNodeType::kEndNode
  virtual UserData Reduct(std::vector<WordDataToUser>&& user_data_) = 0;
};
}  // namespace frontend::generator::syntaxgenerator
#endif  // !GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTION_INTERFACE_H_
