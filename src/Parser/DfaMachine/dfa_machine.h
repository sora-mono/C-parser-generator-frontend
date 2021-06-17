#include "Generator/DfaGenerator/dfa_generator.h"

#ifndef PARSER_DFAMACHINE_DFAMACHINE_H_
#define PARSER_DFAMACHINE_DFAMACHINE_H_

namespace frontend::parser::dfamachine {

class DfaMachine {
  using DfaGenerator = frontend::generator::dfa_generator::DfaGenerator;
  using DfaConfigType = DfaGenerator ::DfaConfigType;
  using SavedData = DfaGenerator::SavedData;
  using TransformArrayId = DfaGenerator::TransformArrayId;

 public:
  DfaMachine() { LoadConfig(); }
  DfaMachine(const DfaMachine&) = delete;
  DfaMachine& operator=(const DfaMachine&) = delete;

  struct ReturnData {
    ReturnData() = default;
    template <class SavedDataType>
    ReturnData(SavedDataType&& saved_data, size_t line, std::string&& symbol_)
        : saved_data_(std::forward<SavedDataType>(saved_data)),
          line_(line),
          symbol_(std::move(symbol_)) {}
    ReturnData(ReturnData&& return_data)
        : saved_data_(std::move(return_data.saved_data_)),
          line_(std::move(return_data.line_)),
          symbol_(std::move(return_data.symbol_)) {}
    ReturnData& operator=(ReturnData&& return_data) {
      saved_data_ = std::move(return_data.saved_data_);
      line_ = std::move(return_data.line_);
      symbol_ = std::move(return_data.symbol_);
      return *this;
    }
    SavedData saved_data_;
    // ����������
    size_t line_;
    // ��ȡ���ĵ���
    std::string symbol_;
  };
  // ���������ļ�
  bool SetInputFile(const std::string filename);
  // ��ȡ��һ����
  // ����������ļ���β�򷵻�tail_node_id = SavedData::InvalidId()
  ReturnData GetNextWord();
  // ��ȡ��ǰ����
  size_t GetLine() { return line_; }
  // ���õ�ǰ����
  void SetLine(size_t line_) { line_ = line_; }
  //����״̬
  void Reset() {
    file_ = nullptr;
    line_ = 0;
  }
  void SetEndOfFileSavedData(const SavedData& saved_data_) {
    file_end_saved_data_ = saved_data_;
  }
  const SavedData& GetEndOfFileSavedData() { return file_end_saved_data_; }

 private:
  // TODO ���������ù��ܵ�����ȡ�����ŵ�һ������
  // ��������
  void LoadConfig();

  // ��ʼDFA������ID
  TransformArrayId start_id_;
  // DFA����
  DfaConfigType dfa_config_;
  // �����ļ�βʱ���ص�����
  SavedData file_end_saved_data_;
  // ��ǰ�����ļ�
  FILE* file_;
  //��ǰ�У���0��ʼ��������ȡ��ÿ���س���+1
  size_t line_ = 0;
};

}  // namespace frontend::parser::dfamachine
#endif  // !PARSER_DFAMACHINE_DFAMACHINE_H_