/// @file dfa_parser.h
/// @brief DFA������
#ifndef PARSER_DFAPARSER_DFAPARSER_H_
#define PARSER_DFAPARSER_DFAPARSER_H_

#include <fstream>
#include <iostream>
#include <list>

#include "Common/common.h"
#include "Generator/export_types.h"
#include "Parser/line_and_column.h"
#include "boost/archive/binary_iarchive.hpp"

namespace frontend::parser::dfa_parser {

/// @class DfaParser dfa_parser.h
/// @brief DFA������
class DfaParser {
  using DfaConfigType = frontend::generator::dfa_generator::DfaConfigType;
  using WordAttachedData = frontend::generator::dfa_generator::WordAttachedData;
  using TransformArrayId = frontend::generator::dfa_generator::TransformArrayId;

 public:
  DfaParser() {}
  DfaParser(const DfaParser&) = delete;
  ~DfaParser() {
    if (file_ != nullptr) {
      fclose(file_);
    }
  }
  DfaParser& operator=(const DfaParser&) = delete;

  /// @class WordInfo dfa_parser.h
  /// @brief ��������
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
    /// @brief ���ʵĸ������ݣ���ӵ���ʱ�洢��
    WordAttachedData word_attached_data_;
    /// @brief ��ȡ���ĵ���
    std::string symbol_;
  };

  /// @brief ���������ļ�
  /// @param[in] filename �������ļ���
  /// @return ���ش��ļ��Ƿ�ɹ�
  /// @retval true �ɹ����ļ�
  /// @retval false ���ļ�ʧ��
  /// @note �Զ���ȡ�����ļ��ĵ�һ���ַ���character_now
  bool SetInputFile(const std::string filename);
  /// @brief ��ȡ��һ������
  /// @return ���ػ�ȡ���ĵ�������
  /// @retval WordInfo(GetEndOfFileSavedData,std::string())
  /// �ﵽ�ļ�β��δ��ȡ������
  /// @note
  /// �����ȡ����ʱ�ﵽ�ļ�β�򷵻ػ�ȡ���ĵ��ʺ͸�������
  WordInfo GetNextWord();

  /// @brief ����״̬
  void Reset() {
    file_ = nullptr;
    SetLine(0);
    SetColumn(0);
  }
  /// @brief ���ôﵽ�ļ�β��δ��ȡ���κε���ʱ���صĵ�������
  /// @param[in] word_attached_data ����������
  void SetEndOfFileSavedData(const WordAttachedData& word_attached_data) {
    file_end_saved_data_ = word_attached_data;
  }
  /// @brief ��ȡ�ﵽ�ļ�β��δ��ȡ���κε���ʱ���صĵ�������
  /// @return ���شﵽ�ļ�β��δ��ȡ���κε���ʱ���صĵ�������
  const WordAttachedData& GetEndOfFileSavedData() const {
    return file_end_saved_data_;
  }
  /// @brief ��������
  /// @note �����ļ���Ϊfrontend::common::kDfaConfigFileName
  void LoadConfig() {
    std::ifstream config_file(frontend::common::kDfaConfigFileName,
                              std::ios_base::binary);
    boost::archive::binary_iarchive iarchive(config_file);
    iarchive >> *this;
  }
  /// @brief ���õ�ǰ�������ַ�
  /// @param[in] character_now ����ǰ�������ַ�
  void SetCharacterNow(char character_now) { character_now_ = character_now; }
  /// @brief ��ȡ��ǰ�������ַ�
  /// @return ���ص�ǰ�������ַ�
  char GetCharacterNow() const { return character_now_; }

 private:
  /// @brief �������л�����ʳ�Ա
  friend class boost::serialization::access;

  /// @brief boost-serialization����DFA���õĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar >> dfa_config_;
    ar >> root_transform_array_id_;
    ar >> file_end_saved_data_;
  }
  /// �����л���Ϊ��������أ�Parser���������ã�������
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  /// @brief ��ʼDFA������ID
  TransformArrayId root_transform_array_id_;
  /// @brief DFA����
  DfaConfigType dfa_config_;
  /// @brief �����ļ�β��δ��ȡ������ʱ���ص�����
  WordAttachedData file_end_saved_data_;
  /// @brief ��ǰ�����ļ�
  FILE* file_ = nullptr;
  /// @brief ��ǰ�������ַ�
  char character_now_;
};

}  // namespace frontend::parser::dfa_parser
#endif  /// !PARSER_DFAMACHINE_DFAMACHINE_H_