#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

namespace frontend::common {

// char����ȡֵ����Ŀ
constexpr size_t kCharNum = CHAR_MAX - CHAR_MIN + 1;
// ����ʽ�����й������ĸ�˰
// �ؼ��ֶ�����
// �����ս����ʽ������
// �����������
// ��ͨ����ʽ��
constexpr size_t kFunctionPartSize = 4;
// �����������ͣ����ϣ��ҽ��
enum class OperatorAssociatityType { kLeftToRight, kRightToRight };
// �ڵ����ͣ��ս���ţ�����������ս���ţ��ļ�β�ڵ�
// Ϊ��֧��ClassfiyProductionNodes�������Զ������ֵ
// ����Զ������ֵ���������ֵ������С��sizeof(ProductionNodeType)
enum class ProductionNodeType {
  kTerminalNode,
  kOperatorNode,
  kNonTerminalNode,
  kEndNode
};

}  // namespace frontend::common

#ifdef _DEBUG

#define USE_USER_DEFINED_FILE
#define USE_AMBIGUOUS_GRAMMAR

#endif  // _DEBUG

#endif  // !COMMON_COMMON_H_