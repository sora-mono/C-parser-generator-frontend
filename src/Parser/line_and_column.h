// ��ͷ�ļ��洢��ǰ��������Դ�ļ�����������
// ������������thread_loacl���ε�ȫ�ֱ���
#ifndef PARSER_LINE_AND_COLUMN_H_
#define PARSER_LINE_AND_COLUMN_H_

namespace frontend::parser {
// �߳�ȫ�ֱ������洢��ǰ������������
// ��0��ʼ����
extern thread_local size_t line_;
// �߳�ȫ�ֱ������洢��ǰ������������
// ��0��ʼ����
extern thread_local size_t column_;

extern inline size_t GetLine();
extern inline size_t GetColumn();
extern inline void SetLine(size_t line);
extern inline void SetColumn(size_t column);

}  // namespace frontend::parser

#endif  // !PARSER_LINE_AND_COLUMN_H_