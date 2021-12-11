/// @file line_and_column.h
/// @brief �洢��ǰ��������Դ�ļ�����������
/// @note ��ʾ�����������ı�������thread_loacl���ε�ȫ�ֱ���
#ifndef PARSER_LINE_AND_COLUMN_H_
#define PARSER_LINE_AND_COLUMN_H_

namespace frontend::parser {

/// @brief ��ȡ��ǰ����
/// @return ���ص�ǰ����
/// @note ��0��ʼ����
extern inline size_t GetLine();
/// @brief ��ȡ��ǰ����
/// @return ���ص�ǰ����
/// @note ��0��ʼ����
extern inline size_t GetColumn();
/// @brief ���õ�ǰ����
/// @param[in] line �������õ�����
/// @note ��0��ʼ����
extern inline void SetLine(size_t line);
/// @brief ���õ�ǰ����
/// @param[in] column �������õ�����
/// @note ��0��ʼ����
extern inline void SetColumn(size_t column);

}  // namespace frontend::parser

#endif  /// !PARSER_LINE_AND_COLUMN_H_