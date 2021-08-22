#ifndef CPARSERFRONTEND_PARSE_FUNCTIONS_H_
#define CPARSERFRONTEND_PARSE_FUNCTIONS_H_

#include <any>
#include <format>

#include "Generator/SyntaxGenerator/process_function_interface.h"
#include "c_parser_frontend.h"
#include "operator_node.h"
#include "type_system.h"
namespace c_parser_frontend::parse_functions {
using WordDataToUser = frontend::generator::syntaxgenerator::
    ProcessFunctionInterface::WordDataToUser;
using c_parser_frontend::operator_node::BasicTypeInitializeOperatorNode;
using c_parser_frontend::operator_node::InitializeType;
using c_parser_frontend::type_system::BuiltInType;
using c_parser_frontend::type_system::CommonlyUsedTypeGenerator;
using c_parser_frontend::type_system::ConstTag;
using c_parser_frontend::type_system::SignTag;
// ������ǰ��
c_parser_frontend::CParserFrontend parser_front_end;

// ����ʽ��Լʱʹ�õĺ���

// SingleConstexprValue->Char
// ����std::shared_ptr<BasicTypeInitializeOperatorNode>
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
std::any SingleConstexprValueChar(std::vector<WordDataToUser>&& word_data);
// SingleConstexprValue->Num
// ����std::shared_ptr<BasicTypeInitializeOperatorNode>
// ���ԣ�InitializeType::kBasic��BuiltInType::kChar��SignTag::kSigned
// TODO ��ȷ��ȡ������ֵ��Ӧ������
std::any SingleConstexprValueNum(std::vector<WordDataToUser>&& word_data);
// SingleConstexprValue->Str
std::any SingleConstexprValueString(std::vector<WordDataToUser>&& word_data);

}  // namespace c_parser_frontend::parse_functions

#endif