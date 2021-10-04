// ���ļ����û�����Ĳ���ʽת��Ϊ��װ��Լ��������
#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAXCONFIG_PROCESS_FUNCTIONS_CLASSES_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAXCONFIG_PROCESS_FUNCTIONS_CLASSES_H_

#include "Config/ProductionConfig/user_defined_functions.h"
#include "process_function_interface.h"

namespace frontend::generator::syntaxgenerator {
// ����ĺ꽫�������ļ����û�����Ĳ���ʽת��Ϊ�����Լ��������
// �������η�����syntax_generate.h
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_
#include "Config/ProductionConfig/production_config-inc.h"
#undef GENERATOR_LEXICALGENERATOR_PROCESS_FUNCTIONS_CLASSES_
// ���������־
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_END
}  // namespace frontend::generator::syntaxgenerator

// ��boost::serialization��ע���װ�û����庯�����������л�������
#include <boost/serialization/export.hpp>
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER
#include "Config/ProductionConfig/production_config-inc.h"
#undef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER

// ���������־
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_END

#endif  // !GENERATOR_SYNTAXGENERATOR_SYNTAXCONFIG_PROCESSFUNCTIONS_H_