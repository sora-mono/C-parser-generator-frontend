#include "Generator/DfaGenerator/dfa_generator.h"

#ifndef PARSER_DFAMACHINE_DFAMACHINE_H_
#define PARSER_DFAMACHINE_DFAMACHINE_H_

namespace frontend::parser::dfamachine {

class DfaMachine {
  using DfaConfigType =
      frontend::generator::dfa_generator::DfaGenerator::DfaConfigType;
  using TailNodeId =
      frontend::generator::dfa_generator::DfaGenerator::TailNodeId;
  using TransformArrayId =
      frontend::generator::dfa_generator::DfaGenerator::TransformArrayId;

 public:
  DfaMachine() { LoadConfig(); }
  DfaMachine(const DfaMachine&) = delete;
  DfaMachine& operator=(const DfaMachine&) = delete;

  struct ReturnData {
    TailNodeId tail_node_id;
    size_t line;
    std::string symbol;
  };
  // ���������ļ�
  bool SetInputFile(const std::string filename);
  // ��ȡ��һ����
  // ����������ļ���β�򷵻�tail_node_id = TailNodeId::InvalidId()
  ReturnData GetNextNode();
  //����״̬
  void Reset() {
    file_ = nullptr;
    line_ = 0;
  }

 private:
  // TODO ���������ù��ܵ�����ȡ�����ŵ�һ������
  // ��������
  void LoadConfig();

  // ��ʼDFA������ID
  TransformArrayId start_id_;
  // DFA����
  DfaConfigType dfa_config_;
  // ��ǰ�����ļ�
  FILE* file_;
  //��ǰ�У���0��ʼ��������ȡ��ÿ���س���+1
  size_t line_ = 0;
};

}  // namespace frontend::parser::dfamachine
#endif  // !PARSER_DFAMACHINE_DFAMACHINE_H_