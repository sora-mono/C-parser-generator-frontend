#ifndef GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT
#define GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT

#include "process_functions_classes.h"
#include "syntax_generator.h"

namespace frontend::generator::syntax_generator {
void SyntaxGenerator::ConfigConstruct() {
  // ����ĺ꽫�������ļ����û�����Ĳ���ʽת��Ϊ����ʽ��������
  // ��AddTerminalNode��AddNonTerminalNode��
#define GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_
#include "Config/ProductionConfig/production_config-inc.h"
#undef GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_
  // ���������־
#define GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_END
}
}  // namespace frontend::generator::syntax_generator

#endif  // !GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT