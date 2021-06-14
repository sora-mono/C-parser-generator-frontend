#include "Parser/DfaMachine/dfa_machine.h"

#include "Common/common.h"

namespace frontend::parser::dfamachine {
bool DfaMachine::SetInputFile(const std::string filename) {
  FILE* file;
  fopen_s(&file, filename.c_str(), "r");
  if (file == nullptr) {
    return false;
  } else {
    file_ = file;
    return true;
  }
}

DfaMachine::ReturnData DfaMachine::GetNextNode() {
  std::string symbol;
  ReturnData return_data;
  return_data.line = line_;
  // ��ǰ״̬ת�Ʊ�ID
  TransformArrayId transform_array_id = start_id_;
  while (true) {
    char c = fgetc(file_);
    if (ferror(file_)) {
      fprintf(stderr, "��ȡ�����ļ����������ļ�\n");
      abort();
    }
    if (feof(file_)) {
      break;
    }
    // �ж��������
    switch (c) {
      case EOF:
        if (feof(file_)) {
          // �ļ���β��������
          goto Return;
        }
        break;
      case '\n':
        // ����+1
        SetLine(GetLine() + 1);
        break;
      default:
        break;
    }
    symbol += c;
    TransformArrayId next_id = dfa_config_[transform_array_id].first[c];
    if (!next_id.IsValid()) {
      goto Return;
    } else {
      transform_array_id = next_id;
    }
  }
Return:
  return_data.line = GetLine();
  if (transform_array_id == start_id_) {
    // �����ļ���β
    return_data.saved_data = GetSavedDataEndOfFile();
  } else {
    return_data.symbol = std::move(symbol);
    return_data.saved_data = dfa_config_[transform_array_id].second;
  }
  return return_data;
}

}  // namespace frontend::parser::dfamachine