#include "Common/object_manager.h"

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_
namespace frontend::generator::lexicalgenerator {
class ProcessFunctionInterface;
}

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
enum class AssociatityType { kLeftAssociate, kRightAssociate };
// �ڵ����ͣ��ս���ţ�����������ս���ţ��ļ�β�ڵ�
// Ϊ��֧��ClassfiyProductionNodes�������Զ������ֵ
// ����Զ������ֵ���������ֵ������С��sizeof(ProductionNodeType)
enum class ProductionNodeType {
  kTerminalNode,
  kOperatorNode,
  kNonTerminalNode,
  kEndNode
};
// ��װ�û��Զ��庯�������ݵ�����ѷ������ID
using ProcessFunctionClassId = frontend::common::ObjectManager<
    frontend::generator::lexicalgenerator::ProcessFunctionInterface>::ObjectId;
}  // namespace frontend::common

#ifdef _DEBUG

#define USE_INIT_FUNCTION
#define USE_SHIFT_FUNCTION
#define USE_REDUCT_FUNCTION
#define USE_USER_DEFINED_FILE
#define USE_AMBIGUOUS_GRAMMAR

#endif  // _DEBUG

#endif  // !COMMON_COMMON_H_