#include "line_and_column.h"

namespace frontend::parser {
/// @brief �߳�ȫ�ֱ������洢��ǰ������������
/// @note ��0��ʼ����
static thread_local size_t line_ = 0;
/// @brief �߳�ȫ�ֱ������洢��ǰ������������
/// @note ��0��ʼ����
static thread_local size_t column_ = 0;

size_t GetLine() { return line_; }
size_t GetColumn() { return column_; }
void SetLine(size_t line) { line_ = line; }
void SetColumn(size_t column) { column_ = column; }
}  // namespace frontend::parser