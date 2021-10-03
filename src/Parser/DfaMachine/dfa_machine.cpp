#include "Parser/DfaMachine/dfa_machine.h"

#include <format>
namespace frontend::parser {
thread_local size_t line_ = 0;
thread_local size_t column_ = 0;
}  // namespace frontend::parser

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

DfaMachine::WordInfo DfaMachine::GetNextWord() {
  // �ж��Ƿ��зŻصĵ���
  if (!putback_words_.empty()) {
    WordInfo return_word = std::move(putback_words_.back());
    putback_words_.pop_back();
    return return_word;
  }
  std::string symbol;
  WordInfo return_data;
  // ��ǰ״̬ת�Ʊ�ID
  TransformArrayId transform_array_id = root_index_;
  while (true) {
    char c = fgetc(file_);
    if (ferror(file_)) {
      std::cerr << std::format("��ȡ�����ļ����������ļ�\n") << std::endl;
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
        // ��������Ϊ0
        SetColumn(0);
        break;
      default:
        break;
    }
    symbol += c;
    SetColumn(GetColumn() + 1);
    TransformArrayId next_array_id = dfa_config_[transform_array_id].first[c];
    if (!next_array_id.IsValid()) {
      break;
    } else {
      transform_array_id = next_array_id;
    }
  }
Return:
  if (feof(file_)) {
    // �����ļ���β
    return WordInfo(GetEndOfFileSavedData(), std::move(symbol));
  } else {
    return WordInfo(dfa_config_[transform_array_id].second, std::move(symbol));
  }
}

}  // namespace frontend::parser::dfamachine