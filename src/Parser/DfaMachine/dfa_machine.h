#ifndef PARSER_DFAMACHINE_DFAMACHINE_H_
#define PARSER_DFAMACHINE_DFAMACHINE_H_

#include <fstream>

#include "Common/common.h"
#include "Generator/DfaGenerator/dfa_generator.h"
#include "boost/archive/binary_iarchive.hpp"

namespace frontend::parser {
// 线程全局变量，存储当前解析到的行数
// 从0开始计数
extern thread_local size_t line_;
// 线程全局变量，存储当前解析到的列数
// 从0开始计数
extern thread_local size_t column_;
}  // namespace frontend::parser

namespace frontend::parser::dfamachine {

class DfaMachine {
  using DfaGenerator = frontend::generator::dfa_generator::DfaGenerator;
  using DfaConfigType = DfaGenerator ::DfaConfigType;
  using WordAttachedData = DfaGenerator::WordAttachedData;
  using TransformArrayId = DfaGenerator::TransformArrayId;

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
    // 获取到的单词
    std::string symbol_;
  };
  // 设置输入文件
  bool SetInputFile(const std::string filename);
  // 获取下一个词
  // 如果遇到了文件结尾则返回tail_node_id = WordAttachedData::InvalidId()
  WordInfo GetNextWord();
  // 获取当前行数
  static size_t GetLine() { return line_; }
  // 设置当前行数
  static void SetLine(size_t line) { line_ = line; }
  // 获取当前列数
  static size_t GetColumn() { return column_; }
  // 设置当前列数
  static void SetColumn(size_t column) { column_ = column; }

  // 重置状态
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
  // 放回一个单词
  void PutbackWord(WordInfo&& word_info) {
    putback_words_.emplace_back(std::move(word_info));
  }
  // 加载配置
  void LoadConfig() {
    std::ifstream config_file(frontend::common::kDfaConfigFileName);
    boost::archive::binary_iarchive iarchive(config_file);
    iarchive >> *this;
  }

 private:
  // 允许序列化类访问成员
  friend class boost::serialization::access;

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar >> dfa_config_;
    ar >> root_index_;
    ar >> file_end_saved_data_;
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  // 起始DFA分析表ID
  TransformArrayId root_index_;
  // DFA配置
  DfaConfigType dfa_config_;
  // 遇到文件尾时返回的数据
  WordAttachedData file_end_saved_data_;
  // 当前输入文件
  FILE* file_;
  // 保存放回单词的数组
  std::vector<WordInfo> putback_words_;
};

}  // namespace frontend::parser::dfamachine
#endif  // !PARSER_DFAMACHINE_DFAMACHINE_H_