/// @file config_construct.cpp
/// @brief ������ӹ�������������Χ��Ϣ�ĺ���ConfigConstruct
/// @details
/// ���ļ����û�ͨ���궨��Ĳ���ʽת��ΪC++���������ƹ����﷨���������õĹ���
/// �������ļ�process_functions_classes.h�ж����װ�û�����Ĺ�Լ��������
/// ��Щ������AddNonTerminalNode
#include "process_functions_classes.h"
#include "syntax_generator.h"
#include "Common/common.h"
#include "Config/ProductionConfig/user_defined_functions.h"

namespace frontend::generator::syntax_generator {
/// @brief ��ӹ�������������Χ��Ϣ
/// @details
/// �ú���ͨ���꽫Config/ProductionConfig/production_config-inc.h���Ժ����ʽ
/// ����Ĳ���ʽת��ΪAddKeyWord��AddTerminalNode��AddBinaryOperatorNode��
/// AddLeftUnaryOperatorNode��AddBinaryUnaryOperatorNode��AddNonTerminalNode
/// �Ⱥ�������ӹ��������������Χ��Ϣ
void SyntaxGenerator::ConfigConstruct() {
  // ����ĺ꽫�������ļ����û�����Ĳ���ʽת��Ϊ����ʽ��������
  // ��AddTerminalNode��AddNonTerminalNode��
#define GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_
#include "Config/ProductionConfig/production_config-inc.h"
#undef GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_
}
}  // namespace frontend::generator::syntax_generator