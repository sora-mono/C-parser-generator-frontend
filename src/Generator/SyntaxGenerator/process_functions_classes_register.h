// ��ͷ�ļ�Ϊ���а�װ�û����庯����ص����ע�����
// ע��������Ϊ��boost::serialization�������л���Щ��
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_H_
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_H_

#include <boost/serialization/export.hpp>

#include "process_function_interface_register.h"
#include "process_functions_classes.h"

// ��boost::serialization��ע���װ�û����庯�����������л�������
// ����ĺ�������process_functions_classes.h�����ɵ���
// ��boost::serialization��ע��Ĵ���
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER
#include "Config/ProductionConfig/production_config-inc.h"
#undef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER

// ���������־
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_END

#endif  // !GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_H_