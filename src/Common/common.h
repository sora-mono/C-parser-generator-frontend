#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#include <climits>

namespace frontend::common {
// �﷨�����������ļ���
constexpr const char* kSyntaxConfigFileName = "syntax_config.conf";
// �ʷ������������ļ���
constexpr const char* kDfaConfigFileName = "dfa_config.conf";
// char����ȡֵ����Ŀ
constexpr size_t kCharNum = CHAR_MAX - CHAR_MIN + 1;
// ����ʽ�����й������ĸ�˰
// �ؼ��ֶ�����
// �����ս����ʽ������
// �����������
// ��ͨ����ʽ��
constexpr size_t kFunctionPartSize = 4;

// �����������ͣ����ϣ��ҽ��
enum class OperatorAssociatityType { kLeftToRight, kRightToLeft };

}  // namespace frontend::common

#endif  // !COMMON_COMMON_H_