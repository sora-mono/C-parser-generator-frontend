#ifndef PARSER_DFAMACHINE_DFAMACHINE_H_
#define PARSER_DFAMACHINE_DFAMACHINE_H_

#include "Generator/DfaGenerator/dfa_generator.h"

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
    WordInfo(SavedDataType&& saved_data, size_t line, std::string&& symbol_)
        : word_attached_data_(std::forward<SavedDataType>(saved_data)),
          line_(line),
          symbol_(std::move(symbol_)) {}
    WordInfo(WordInfo&& return_data)
        : word_attached_data_(std::move(return_data.word_attached_data_)),
          line_(std::move(return_data.line_)),
          symbol_(std::move(return_data.symbol_)) {}
    WordInfo& operator=(WordInfo&& return_data) {
      word_attached_data_ = std::move(return_data.word_attached_data_);
      line_ = std::move(return_data.line_);
      symbol_ = std::move(return_data.symbol_);
      return *this;
    }
    WordAttachedData word_attached_data_;
    // ����������
    size_t line_;
    // ��ȡ���ĵ���
    std::string symbol_;
  };
  // ���������ļ�
  bool SetInputFile(const std::string filename);
  // ��ȡ��һ����
  // ����������ļ���β�򷵻�tail_node_id = WordAttachedData::InvalidId()
  WordInfo GetNextWord();
  // ��ȡ��ǰ����
  size_t GetLine() { return line_; }
  // ���õ�ǰ����
  void SetLine(size_t line_) { line_ = line_; }
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
  //��ǰ�У���0��ʼ��������ȡ��ÿ���س���+1
  size_t line_ = 0;
  // ����Żص��ʵ�����
  std::vector<WordInfo> putback_words_;
};

}  // namespace frontend::parser::dfamachine
#endif  // !PARSER_DFAMACHINE_DFAMACHINE_H_