/// @file process_functions_classes.h
/// @brief �����û�����ķ��ս����ʽ�����װ��Լ��������
/// @details
/// Ϊ�˱���д���ӽű���ͨ���麯�������������û�����Ĺ�Լ�������Դﵽ����
/// ��ͬ����ʽ���ò�ͬ��Լ������Ŀ��
/// ÿ�����ս����ʽ�嶼�ᶨ���Ӧ��Ψһ��װ��Լ�������࣬Generator�ڹ�������ʱ
/// ÿ���඼ʵ����Ψһ�Ķ�����Щ�����ڱ���/��������ʱһ�����л�
/// ��Լʱ�Ȼ�ȡ��Ӧ����ʽ�Ķ���Ȼ�����Reduct�麯����Լ
#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAXCONFIG_PROCESS_FUNCTIONS_CLASSES_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAXCONFIG_PROCESS_FUNCTIONS_CLASSES_H_
#include "process_function_interface.h"
#include "reduct_type_register.h"
#include <list>
#include "Logger/logger.h"

namespace frontend::generator::syntax_generator {
// ����ĺ꽫�������ļ����û�����ķ��ս����ʽת��Ϊ��װ��Լ��������
// �������η�����syntax_generate.h��NONTERMINAL_PRODUCTION_SYMBOL_MODIFY��
// NONTERMINAL_PRODUCTION_SYMBOL_MODIFY_STR
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_
#include "Config/ProductionConfig/production_config-inc.h"
#undef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_
}  // namespace frontend::generator::syntax_generator

#endif  /// !GENERATOR_SYNTAXGENERATOR_SYNTAXCONFIG_PROCESSFUNCTIONS_H_