// ���ļ���Ŀ����Ϊ��ȥ��ִ��generator���ɱ������������һ��
// ͨ�����ڶ������ʽʱ���ɴ��룬�������ʽ��Ϳ��Ա�������generator��������
// ���ļ���Ҫ�������ļ����������������ļ�����ͬ�ĺ����ɲ�ͬ����

// ����ĺ걣֤�ú궨�����ս���process_functions_classes.h
// user_defined_function_and_data_register.h��config_construct.cpp����
// ����ͬ���ֵĺ��������ļ��ж��岻ͬ����
// ���ļ���Ӧʹ�ñ�׼ͷ�ļ�������Ӧ��ʹ������ĺ걣��
// ��ôдΪ��ʹ�û��ڱ༭������Կ���������
// ���ҿ��Խ�дһ�β���ʽ�����ڶ���ļ��������������
// ��ͷ�ļ�������ʹ�ú�֮ǰ������
// ����򵥵ķ����Ƿ���production_config-inc.h��һ�У�

// �ⲿ�������û��������ʽ�õĺ��ԭ��
// ͬʱ��ԭ��Щ��Ϊ��ʵ�֣���ֹ��ͬ�ļ��Ժ���ػ���ͻ������#undef������
// ǿ�ҽ���ʹ����򵥵��﷨����������
// ʹ���ַ���ʱֱ��ʹ��˫�������ͣ�����ʹ�������֣�
// �Ӷ����������ʱ���ʽ���������������

// ���η��ս�ڵ����ʽ���Ի�ȡ��װReduct�����õ�����
#define NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,   \
                                             production_body_seq) \
  production_symbol##production_body_seq##_

// ����ؼ���
// ����GENERATOR_DEFINE_KEY_WORD("example_key_word")
#ifdef GENERATOR_DEFINE_KEY_WORD
#undef GENERATOR_DEFINE_KEY_WORD
#endif  // !GENERATOR_DEFINE_KEY_WORD
#define GENERATOR_DEFINE_KEY_WORD(key_word)

// ���˫Ŀ�����
// ������������ң���������ţ�����ԣ�ö��OperatorAssociatityType�������ȼ�
// ����GENERATOR_DEFINE_BINARY_OPERATOR("example_symbol",
//                                      OperatorAssociatityType::kLeftToRight,
//                                      2)
#ifdef GENERATOR_DEFINE_BINARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_OPERATOR
#endif  // !GENERATOR_DEFINE_BINARY_OPERATOR
#define GENERATOR_DEFINE_BINARY_OPERATOR( \
    operator_symbol, binary_operator_associatity, binary_operator_priority)
// �����൥Ŀ�����
// ������������ң���������ţ�����ԣ�ö��OperatorAssociatityType�������ȼ�
// ����GENERATOR_DEFINE_UNARY_OPERATOR("example_symbol",
//                                      OperatorAssociatityType::kLeftToRight,
//                                      2)
#ifdef GENERATOR_DEFINE_UNARY_OPERATOR
#undef GENERATOR_DEFINE_UNARY_OPERATOR
#endif  // !GENERATOR_DEFINE_UNARY_OPERATOR
#define GENERATOR_DEFINE_UNARY_OPERATOR( \
    operator_symbol, unary_operator_associatity, unary_operator_priority)
// �����൥Ŀ��˫Ŀ�����
// ������ʹ��GENERATOR_DEFINE_UNARY_OPERATOR��GENERATOR_DEFINE_BINARY_OPERATOR
// �������ͬʱ֧����൥Ŀ��˫Ŀ�������������ʹ�øú�һ������������
// ������������ң���������ţ�����ԣ�ö��OperatorAssociatityType�������ȼ�
// ����GENERATOR_DEFINE_BINARY_UNARY_OPERATOR("example_symbol",
//                                      OperatorAssociatityType::kLeftToRight,
//                                      2)
#ifdef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#endif  // !GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#define GENERATOR_DEFINE_BINARY_UNARY_OPERATOR(                             \
    operator_symbol, binary_operator_associatity, binary_operator_priority, \
    unary_operator_associatity, unary_operator_priority)

// �����ս�ڵ㣬production_bodyΪ������ʽ
// ����GENERATOR_DEFINE_TERMINAL_PRODUCTION(example_symbol,
//                                          "[a-zA-Z_][a-zA-Z_0-9]*")
#ifdef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_TERMINAL_PRODUCTION
#define GENERATOR_DEFINE_TERMINAL_PRODUCTION(production_symbol, production_body)

// ���ս�ڵ㶨�壬����ʽ�岿����ʹ�ó�ʼ���б���д��
// ����{"production_body1","production_body2"}
// ������������������˫����
// production_symbolΪ����ʽ��
// reduct_function�ǹ�Լ�ú���
// production_body_seq�ǲ���ʽ����
// ...����Ϊ����ʽ��
// ÿһ����ͬ�Ĳ���ʽ���¸�����ʽ���ű��벻ͬ����ʹ��[a-zA-Z0-9_]+
// �ñ���������ֲ�ͬ�Ĳ���ʽ�岢Ϊ�乹����Ӧ�İ�װ����
// ����GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(example_symbol,
//                ExampleFunctionName, 0, false, {"Id", "=", "Assignable"})
#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION( \
    production_symbol, reduct_function_name, production_body_seq, ...)

// ���÷��ս�ڵ���Կչ�Լ
// δ���ô˷����ķ��ս�ڵ�Ĭ�ϲ����Կչ�Լ
// �ڵ���ʱӦ��֤�����ķ��ս�ڵ��Ѿ��������壨�����������Ѷ��壩
// ����ǿ�ҽ����ڶ������в���ʽ����ø÷���
// ����GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT(example_symbol)
#ifdef GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT
#undef GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT
#endif
#define GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT( \
    production_symbol)

// ���ø�����ʽ
// ������ʽ���壬��Ҫ�ڸ�������ʽ����ʽ��Ӻ�ʹ��
// �����ò���ʽ��������ȫ���ڵ㶼�ܹ���Ӻ�
// ����GENERATOR_DEFINE_ROOT_PRODUCTION(example_symbol)
#ifdef GENERATOR_DEFINE_ROOT_PRODUCTION
#undef GENERATOR_DEFINE_ROOT_PRODUCTION
#endif  // GENERATOR_DEFINE_BINARY_OPERATOR
#define GENERATOR_DEFINE_ROOT_PRODUCTION(production_symbol)

// �ⲿ�ֶ�������Ҫ����ʽ��Ϣ���ļ��ػ�
#ifdef GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_
// ����ú���������������ʹ��IntelliSense��������ͷ�ļ�
// production_config-inc.h��config_construct.cpp�����ɺ���������
// �ں��������ڰ���ͷ�ļ��ᵼ�´�����ִ���
#define SHIELD_HEADERS_FOR_INTELLISENSE
// ��ֹ�ظ�����
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_END

#ifdef GENERATOR_DEFINE_KEY_WORD
#undef GENERATOR_DEFINE_KEY_WORD
#endif  // GENERATOR_DEFINE_KEY_WORD
#define GENERATOR_DEFINE_KEY_WORD(key_word) AddKeyWord(key_word);

#ifdef GENERATOR_DEFINE_BINARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_OPERATOR
#endif  // GENERATOR_DEFINE_BINARY_OPERATOR
#define GENERATOR_DEFINE_BINARY_OPERATOR(                                   \
    operator_symbol, binary_operator_associatity, binary_operator_priority) \
  AddBinaryOperatorNode(operator_symbol,                                    \
                        frontend::common::binary_operator_associatity,      \
                        OperatorPriority(binary_operator_priority));
#ifdef GENERATOR_DEFINE_UNARY_OPERATOR
#undef GENERATOR_DEFINE_UNARY_OPERATOR
#endif  // GENERATOR_DEFINE_UNARY_OPERATOR
#define GENERATOR_DEFINE_UNARY_OPERATOR(                                  \
    operator_symbol, unary_operator_associatity, unary_operator_priority) \
  AddUnaryOperatorNode(operator_symbol,                                   \
                       frontend::common::unary_operator_associatity,      \
                       OperatorPriority(unary_operator_priority));
#ifdef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#endif  // !GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#define GENERATOR_DEFINE_BINARY_UNARY_OPERATOR(                             \
    operator_symbol, binary_operator_associatity, binary_operator_priority, \
    unary_operator_associatity, unary_operator_priority)                    \
  AddBinaryUnaryOperatorNode(operator_symbol,                               \
                             frontend::common::binary_operator_associatity, \
                             OperatorPriority(binary_operator_priority),    \
                             frontend::common::unary_operator_associatity,  \
                             OperatorPriority(unary_operator_priority))

#ifdef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_TERMINAL_PRODUCTION
#define GENERATOR_DEFINE_TERMINAL_PRODUCTION(production_symbol, \
                                             production_body)   \
  AddTerminalNode(#production_symbol, production_body);

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                   \
    production_symbol, reduct_function, production_body_seq, ...)  \
  AddNonTerminalNode<NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(         \
      production_symbol, production_body_seq)>(#production_symbol, \
                                               __VA_ARGS__);

#ifdef GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT
#undef GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT
#endif
#define GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT( \
    production_symbol)                                           \
  SetNonTerminalNodeCouldEmptyReduct(#production_symbol);

#ifdef GENERATOR_DEFINE_ROOT_PRODUCTION
#undef GENERATOR_DEFINE_ROOT_PRODUCTION
#endif  // GENERATOR_DEFINE_ROOT_PRODUCTION
#define GENERATOR_DEFINE_ROOT_PRODUCTION(production_symbol) \
  SetRootProduction(#production_symbol);

#endif

#elif defined GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_
// ��ֹ���ظ�����
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_END

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION

#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                           \
    production_symbol, reduct_function, production_body_seq, ...)          \
  class NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,            \
                                             production_body_seq)          \
      : public frontend::generator::syntaxgenerator::                      \
            ProcessFunctionInterface {                                     \
   public:                                                                 \
    virtual ProcessFunctionInterface::UserData Reduct(                     \
        std::vector<ProcessFunctionInterface::WordDataToUser>&& word_data) \
        override {                                                         \
      return reduct_function(std::move(word_data));                        \
    }                                                                      \
  };
#endif

#elif defined GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER
// ��ֹ�ظ�����
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_END

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION

#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                              \
    production_symbol, production_body, reduct_function, production_body_seq, \
    ...)                                                                      \
  BOOST_CLASS_EXPORT_GUID(                                                    \
      frontend::generator::syntaxgenerator::                                  \
          NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,             \
                                               production_body_seq),          \
      "frontend::generator::"                                                 \
      "syntaxgenerator::" #NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(              \
          production_symbol, production_body_seq))
#endif

#else

#error ���ļ����ұ������ձ�Generator/SyntaxGenerator�µ�\
process_functions_classes.h \
process_functions_classes_register.h \
config_construct.cpp������

#endif

// Ϊ��ʹ��Intellisense��������������Ҫ��ͷ�ļ�
// �˴���ͷ�ļ���config_construct.cpp�лᱻ����
// �û�����user_defined_functions.h������Լ������ͷ�ļ�
#ifndef SHIELD_HEADERS_FOR_INTELLISENSE
#include "Common/common.h"
#include "Config/ProductionConfig/user_defined_functions.h"
// �ṩ���������Ե�ö��
using OperatorAssociatityType = frontend::common::OperatorAssociatityType;
#endif  // !SHIELD_HEADERS_FOR_INTELLIGENCE