#include "Generator/DfaGenerator/dfa_generator.h"

#ifndef PARSER_DFAMACHINE_DFAMACHINE_H_
#define PARSER_DFAMACHINE_DFAMACHINE_H_

namespace frontend::parser::dfamachine {

class DfaMachine {
  using DfaConfigType =
      frontend::generator::dfa_generator::DfaGenerator::DfaConfigType;
  using SavedData = frontend::generator::dfa_generator::DfaGenerator::SavedData;
  using TransformArrayId =
      frontend::generator::dfa_generator::DfaGenerator::TransformArrayId;

 public:
  DfaMachine() { LoadConfig(); }
  DfaMachine(const DfaMachine&) = delete;
  DfaMachine& operator=(const DfaMachine&) = delete;

  struct ReturnData {
    SavedData saved_data;
    // ����������
    size_t line;
    // ��ȡ���ĵ���
    std::string symbol;
  };
  // ���������ļ�
  bool SetInputFile(const std::string filename);
  // ��ȡ��һ����
  // ����������ļ���β�򷵻�tail_node_id = SavedData::InvalidId()
  ReturnData GetNextNode();
  // ��ȡ��ǰ����
  size_t GetLine() { return line_; }
  // ���õ�ǰ����
  void SetLine(size_t line) { line_ = line; }
  //����״̬
  void Reset() {
    file_ = nullptr;
    line_ = 0;
  }
  const SavedData& GetSavedDataEndOfFile() { return file_end_saved_data_; }

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