/// @file common.h
/// @brief �洢�����Ķ��������

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#include <climits>

namespace frontend::common {
/// @brief �﷨�����������ļ���
constexpr const char* kSyntaxConfigFileName = "syntax_config.conf";
/// @brief �ʷ������������ļ���
constexpr const char* kDfaConfigFileName = "dfa_config.conf";
/// @brief char����ȡֵ����Ŀ
constexpr size_t kCharNum = CHAR_MAX - CHAR_MIN + 1;

/// @brief �����������ͣ����ϣ��ҽ��
enum class OperatorAssociatityType { kLeftToRight, kRightToLeft };

}  // namespace frontend::common

#endif  /// !COMMON_COMMON_H_