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

DfaMachine::ReturnData DfaMachine::GetNextWord() {
  std::string symbol;
  ReturnData return_data;
  return_data.line_ = line_;
  // ��ǰ״̬ת�Ʊ�ID
  TransformArrayId transform_array_id = start_id_;
  while (true) {
    char c = fgetc(file_);
    if (ferror(file_)) {
      fprintf(stderr, "��ȡ�����ļ����������ļ�\n");
      abort();
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
      break;
    } else {
      transform_array_id = next_id;
    }
  }
Return:
  if (feof(file_)) {
    // �����ļ���β
    return ReturnData(GetEndOfFileSavedData(), GetLine(), std::move(symbol));
  } else {
    return ReturnData(dfa_config_[transform_array_id].second, GetLine(),
                      std::move(symbol));
  }
}

}  // namespace frontend::parser::dfamachine