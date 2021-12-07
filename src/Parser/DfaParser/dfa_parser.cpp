#include "Parser/DfaParser/dfa_parser.h"

#include <format>

namespace frontend::parser::dfa_parser {
bool DfaParser::SetInputFile(const std::string filename) {
  FILE* file;
  fopen_s(&file, filename.c_str(), "r");
  if (file == nullptr) {
    return false;
  } else {
    file_ = file;
    SetCharacterNow(fgetc(file_));
    return true;
  }
}

DfaParser::WordInfo DfaParser::GetNextWord() {
  std::string symbol;
  WordInfo return_data;
  // ��ǰ״̬ת�Ʊ�ID
  TransformArrayId transform_array_id = root_transform_array_id_;
  // �����հ��ַ�
  while (std::isspace(GetCharacterNow())) {
    if (GetCharacterNow() == '\n') [[unlikely]] {
      // ����+1
      SetLine(GetLine() + 1);
      // ��������Ϊ0
      SetColumn(0);
    }
    SetCharacterNow(fgetc(file_));
    SetColumn(GetColumn() + 1);
  }
  while (true) {
    // �ж��������
    switch (GetCharacterNow()) {
      case EOF:
        if (feof(file_)) {
          if (!symbol.empty()) {
            // ����Ѿ���ȡ���˵����򷵻ص���Я�������ݶ�����ֱ�ӷ����ļ�β����
            return WordInfo(dfa_config_[transform_array_id].second,
                            std::move(symbol));
          } else {
            // û�л�ȡ�����ʣ�ֱ�ӷ����ļ�β����
            return WordInfo(GetEndOfFileSavedData(), std::string());
          }
        }
        break;
      case '\n':
        // ����+1
        SetLine(GetLine() + 1);
        // ��������Ϊ0
        SetColumn(0);
        SetCharacterNow(fgetc(file_));
        continue;
        break;
      default:
        break;
    }
    TransformArrayId next_array_id =
        dfa_config_[transform_array_id].first[GetCharacterNow()];
    if (!next_array_id.IsValid()) {
      // �޷����뵱ǰ�ַ�
      break;
    } else {
      // ��������
      symbol += GetCharacterNow();
      transform_array_id = next_array_id;
      SetCharacterNow(fgetc(file_));
      SetColumn(GetColumn() + 1);
    }
  }
  std::cout << symbol << std::endl;
  return WordInfo(dfa_config_[transform_array_id].second, std::move(symbol));
}

}  // namespace frontend::parser::dfa_parser