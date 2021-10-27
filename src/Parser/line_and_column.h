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

size_t GetLine();
size_t GetColumn();
void SetLine(size_t line);
void SetColumn(size_t column);

}  // namespace frontend::parser

#endif  // !PARSER_LINE_AND_COLUMN_H_