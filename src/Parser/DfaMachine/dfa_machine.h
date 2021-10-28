#ifndef PARSER_DFAMACHINE_DFAMACHINE_H_
#define PARSER_DFAMACHINE_DFAMACHINE_H_

#include <fstream>
#include <iostream>

#include "Common/common.h"
#include "Generator/export_types.h"
#include "Parser/line_and_column.h"
#include "boost/archive/binary_iarchive.hpp"

namespace frontend::parser::dfamachine {

class DfaMachine {
  using DfaConfigType = frontend::generator::dfa_generator::DfaConfigType;
  using WordAttachedData = frontend::generator::dfa_generator::WordAttachedData;
  using TransformArrayId = frontend::generator::dfa_generator::TransformArrayId;

 public:
  DfaMachine() {}
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
  static size_t GetLine() { return frontend::parser::GetLine(); }
  // ���õ�ǰ����
  static void SetLine(size_t line) { frontend::parser::SetLine(line); }
  // ��ȡ��ǰ����
  static size_t GetColumn() { return frontend::parser::GetColumn(); }
  // ���õ�ǰ����
  static void SetColumn(size_t column) { frontend::parser::SetColumn(column); }

  // ����״̬
  void Reset() {
    file_ = nullptr;
    SetLine(0);
    SetColumn(0);
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
  // ��������
  void LoadConfig() {
    std::ifstream config_file(frontend::common::kDfaConfigFileName,
                              std::ios_base::binary);
    boost::archive::binary_iarchive iarchive(config_file);
    iarchive >> *this;
  }

 private:
  // �������л�����ʳ�Ա
  friend class boost::serialization::access;

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar >> dfa_config_;
    ar >> root_transform_array_id_;
    ar >> file_end_saved_data_;
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  // ��ʼDFA������ID
  TransformArrayId root_transform_array_id_;
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