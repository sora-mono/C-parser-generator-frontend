/// @file process_functions_classes_register.h
/// @brief ��boost-serializationע��process_functions_classes.h�ж����������
/// @details
/// boost���л������л�������ǰ������ע�ᣬ��������쳣
/// ���ļ��ж���ע��process_functions_classes.h��������ĺ�
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_H_
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_H_

#include <boost/serialization/export.hpp>

#include "process_function_interface_register.h"
#include "process_functions_classes.h"

/// ����ĺ�����process_functions_classes.h�е�����boost::serialization��ע��
/// �Ĵ���
#define GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER
#include "Config/ProductionConfig/production_config-inc.h"
#undef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER

#endif  /// !GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_H_