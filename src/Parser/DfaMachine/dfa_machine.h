#ifndef PARSER_DFAMACHINE_DFAMACHINE_H_
#define PARSER_DFAMACHINE_DFAMACHINE_H_

#include "Generator/DfaGenerator/dfa_generator.h"

namespace frontend::parser {
// �߳�ȫ�ֱ������洢��ǰ������������
// ��0��ʼ����
extern thread_local size_t line_;
// �߳�ȫ�ֱ������洢��ǰ������������
// ��0��ʼ����
extern thread_local size_t column_;
}  // namespace frontend::parser

namespace frontend::parser::dfamachine {

class DfaMachine {
  using DfaGenerator = frontend::generator::dfa_generator::DfaGenerator;
  using DfaConfigType = DfaGenerator ::DfaConfigType;
  using WordAttachedData = DfaGenerator::WordAttachedData;
  using TransformArrayId = DfaGenerator::TransformArrayId;

 public:
  DfaMachine() { LoadConfig(); }
  DfaMachine(const DfaMachine&) = delete;
  DfaMachine& operator=(const DfaMachine&) = delete;

  struct WordInfo {
    WordInfo() = default;
    template <class SavedDataType>
    WordInfo(SavedDataType&& saved_data, std::string&& symbol_)
        : word_attached_data_(std::forward<SavedDataType>(saved_data)),
          symbol_(std::move(symbol_)) {}
    WordInfo(WordInfo&& return_data)
        : word_attached_data_(std::move(return_data.word_attached_data_)),
          symbol_(std::move(return_data.symbol_)) {}
    WordInfo& operator=(WordInfo&& return_data) {
      word_attached_data_ = std::move(return_data.word_attached_data_);
      symbol_ = std::move(return_data.symbol_);
      return *this;
    }
    WordAttachedData word_attached_data_;
    // ��ȡ���ĵ���
    std::string symbol_;
  };
  // ���������ļ�
  bool SetInputFile(const std::string filename);
  // ��ȡ��һ����
  // ����������ļ���β�򷵻�tail_node_id = WordAttachedData::InvalidId()
  WordInfo GetNextWord();
  // ��ȡ��ǰ����
  static size_t GetLine() { return line_; }
  // ���õ�ǰ����
  static void SetLine(size_t line) { line_ = line; }
  // ��ȡ��ǰ����
  static size_t GetColumn() { return column_; }
  // ���õ�ǰ����
  static void SetColumn(size_t column) { column_ = column; }

  // ����״̬
  void Reset() {
    file_ = nullptr;
    line_ = 0;
  }
  void SetEndOfFileSavedData(const WordAttachedData& word_attached_data_) {
    file_end_saved_data_ = word_attached_data_;
  }
  const WordAttachedData& GetEndOfFileSavedData() {
    return file_end_saved_data_;
  }
  // �Ż�һ������
  void PutbackWord(WordInfo&& word_info) {
    putback_words_.emplace_back(std::move(word_info));
  }

 private:
  // TODO ���������ù��ܵ�����ȡ�����ŵ�һ������
  // ��������
  void LoadConfig();

  // ��ʼDFA������ID
  TransformArrayId start_id_;
  // DFA����
  DfaConfigType dfa_config_;
  // �����ļ�βʱ���ص�����
  WordAttachedData file_end_saved_data_;
  // ��ǰ�����ļ�
  FILE* file_;
  // ����Żص��ʵ�����
  std::vector<WordInfo> putback_words_;
};

}  // namespace frontend::parser::dfamachine
#endif  // !PARSER_DFAMACHINE_DFAMACHINE_H_