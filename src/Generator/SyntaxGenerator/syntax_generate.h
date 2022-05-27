/// @file syntax_generate.h
/// @brief �����ʾ����ʽ�ĺ��Լ��������ɹ���
/// @details
/// 1.�����������ޣ��޷����ļ��д洢��Լ�ú�����ֻ�����߾ȹ�����ÿ����Լ����
/// ��װ�����У�ÿ����װ��ʵ����һ�����󣬶�����ͨ���麯����װ��Լ������
/// �����麯����������ʱ������ȷ�Ĺ�Լ�����������Ķ����ͨ��boost-serialization
/// ���л����Ӷ��������ļ��б����Լ����
/// 2.�����������ޣ�����ͨ����ȡ�����ļ�������Ȼ�����������������£�
///   1)����Generator���������ļ�����������������Ҫ׷�ӵ�Դ����
///   2)�ٴα���
///   3)����Generator��������
/// 3)�ⲽ����û�з���ȥ���������û�����Ĺ�Լ������α��浽�����ļ������⣬��
/// ��������ʱ��Щ������ֻ�Ǻ����������ͨ���꽫�������󶨵����庯������ô�û�
/// ��Ҫ�������ļ���д���麯��������һ�黹�Ƕ���EXPORT(func,"func")�����ĺ꣬
/// �˷���Чʱ�����ά�����㣬����Ҫ��������Generator������Ӱ��ʹ�����顣
/// 3.����2��ԭ������ѡ��ͨ�����������ļ�����������ʽ�������Ϣ���������ʽ��
/// ֻ�趨������һ�Σ�����һ�Σ�����һ�Σ����Զ�����Ҫ���ļ���������ش���
/// 4.���ļ��ڶ�������ʾ����ʽ�ĺ꣬��Щ������ڱ�ʾ����ʽ��������Ҫ���ļ���
/// ת��Ϊ��Ӧ���룬����ÿ����Ҫ���ɴ�����ļ��ж��ֶ��������ʽ
/// 5.���ļ������ҽ���Config/ProductionConfig/production_config-inc.h������
/// production_config-inc.h��Ҫ�ҽ���process_functions_classes.h
/// process_functions_classes_register.h��config_construct.cpp�����ļ�������
/// ���������ļ������ɲ�ͬ����
/// 6.���ļ���Ӧʹ�ñ�׼ͷ�ļ��������������ػ��ĵĺ걣��
/// 7.�û�����ͷ�ļ�����user_defined_functions.h����ӣ�
/// ��ֹ��production_config-inc.h������û������ͷ�ļ�
/// 8.�ؼ��ִʷ��������ȼ������������������ʷ��������ȼ�������ͨ����

// �ⲿ�������û��������ʽ�õĺ��ԭ��
// ͬʱ��ԭ��Щ��Ϊ��ʵ�֣���ֹ��ͬ�ļ��Ժ���ػ���ͻ������#undef������
// ǿ�ҽ�����ú�ʱʹ����򵥵��﷨����������
// ʹ���ַ���ʱֱ��ʹ��˫�������ͣ�����ʹ�������֣�
// �Ӷ����������ʱ���ʽ���������������
// ��ǰʵ����ʹ��3���������ȼ���0~2����ͨ�������ȼ�Ϊ0����������ȼ�Ϊ1��
// �ؼ������ȼ�Ϊ2����һ�����ʶ�Ӧ����������Ϊ���ȼ��ߵ�����

/// @brief ���η��ս�ڵ����ʽ��
/// @param[in] production_symbol ������ʽ��
/// @param[in] production_body_seq �����ֲ���ʽ����ַ�
/// @return ������İ�װReduct�����õ�����
/// @note
/// ����
/// NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(example1,2)
/// չ��Ϊ example1_2_
#define NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,   \
                                             production_body_seq) \
  production_symbol##_##production_body_seq##_
/// @brief ���η��ս�ڵ����ʽ��
/// @param[in] production_symbol ������ʽ��
/// @param[in] production_body_seq �����ֲ���ʽ����ַ�
/// @return ���ذ�װReduct�����õ��������ַ�����ʽ
/// @note
/// ����
/// NONTERMINAL_PRODUCTION_SYMBOL_MODIFY_STR(example1,2)
/// չ��Ϊ "example1_2_"
#define NONTERMINAL_PRODUCTION_SYMBOL_MODIFY_STR(production_symbol,   \
                                                 production_body_seq) \
#production_symbol##"_"## #production_body_seq##"_"

#ifdef GENERATOR_DEFINE_KEY_WORD
#undef GENERATOR_DEFINE_KEY_WORD
#endif  // !GENERATOR_DEFINE_KEY_WORD
/// @brief ����ؼ���
/// @param[in] key_word ��������Ĺؼ����ַ���
/// @details
/// ����GENERATOR_DEFINE_KEY_WORD("example_key_word")
/// @note ֧��������ʽ
/// �ؼ��ִʷ��������ȼ��������������ͨ����
#define GENERATOR_DEFINE_KEY_WORD(key_word)

#ifdef GENERATOR_DEFINE_BINARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_OPERATOR
#endif  // !GENERATOR_DEFINE_BINARY_OPERATOR
/// @brief ����˫Ŀ�����
/// @param[in] operator_symbol ��������ַ���
/// @param[in] binary_operator_associatity
/// ��˫Ŀ���������ԣ�ö��OperatorAssociatityType��
/// @param[in] binary_operator_priority ��˫Ŀ��������ȼ�
/// @details
/// ����GENERATOR_DEFINE_BINARY_OPERATOR("example_symbol",
///                                      OperatorAssociatityType::kLeftToRight,
///                                      2)
/// @note ��������ȼ���ֵԽ�����ȼ�Խ��
/// @attention ��ͬoperator_symbol�������ֻ�ܶ���һ��
#define GENERATOR_DEFINE_BINARY_OPERATOR( \
    operator_symbol, binary_operator_associatity, binary_operator_priority)

#ifdef GENERATOR_DEFINE_UNARY_OPERATOR
#undef GENERATOR_DEFINE_UNARY_OPERATOR
#endif  // !GENERATOR_DEFINE_UNARY_OPERATOR
/// @brief ������൥Ŀ�����
/// @param[in] operator_symbol ��������ַ���
/// @param[in] unary_operator_associatity
/// ����൥Ŀ���������ԣ�ö��OperatorAssociatityType��
/// @param[in] unary_operator_priority ����൥Ŀ��������ȼ�
/// @details
/// ����GENERATOR_DEFINE_UNARY_OPERATOR("example_symbol",
///                                      OperatorAssociatityType::kLeftToRight,
///                                      2)
/// @note ��������ȼ���ֵԽ�����ȼ�Խ��
/// @attention ��ͬoperator_symbol�������ֻ�ܶ���һ��
#define GENERATOR_DEFINE_UNARY_OPERATOR( \
    operator_symbol, unary_operator_associatity, unary_operator_priority)

#ifdef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#endif  // !GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
/// @brief ����ͬʱ����˫Ŀ����൥Ŀ����������
/// @param[in] operator_symbol ��˫����������ַ���
/// @param[in] binary_operator_associatity
/// ��˫Ŀ���������ԣ�ö��OperatorAssociatityType��
/// @param[in] binary_operator_priority ��˫Ŀ��������ȼ�
/// @param[in] unary_operator_associatity
/// ����൥Ŀ���������ԣ�ö��OperatorAssociatityType��
/// @param[in] unary_operator_priority ����൥Ŀ��������ȼ�
/// @details
/// ����GENERATOR_DEFINE_BINARY_UNARY_OPERATOR("example_symbol",
///                                      OperatorAssociatityType::kLeftToRight,
///                                      2)
/// @attention ��ͬoperator_symbol�������ֻ�ܶ���һ��
/// ������ͨ�����GENERATOR_DEFINE_UNARY_OPERATOR��
/// GENERATOR_DEFINE_BINARY_OPERATOR������ͬʱ֧��˫Ŀ����൥Ŀ������������
/// ����ʹ�øú�һ�����������������
#define GENERATOR_DEFINE_BINARY_UNARY_OPERATOR(                             \
    operator_symbol, binary_operator_associatity, binary_operator_priority, \
    unary_operator_associatity, unary_operator_priority)

#ifdef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_TERMINAL_PRODUCTION
/// @brief �����ս����ʽ
/// @param[in] production_symbol ���ս�ڵ����ַ���
/// @param[in] production_body ���ս�ڵ����ַ�����������ʽ��ʽ��
/// @details
/// ����GENERATOR_DEFINE_TERMINAL_PRODUCTION("example_symbol",
///                                          "[a-zA-Z_][a-zA-Z_0-9]*")
#define GENERATOR_DEFINE_TERMINAL_PRODUCTION(production_symbol, production_body)

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
/// @brief ������ս����ʽ
/// @param[in] production_symbol �����ս����ʽ��
/// @param[in] reduct_function_name ����Լ���ս����ʽ�ĺ�����
/// @param[in] production_body_seq �����ֲ�ͬ����ʽ����ַ�
/// @param[in] ... ������ʽ��
/// @details
/// ����ʽ�岿��ʹ�ó�ʼ���б���д��
/// ����{"production_body1","production_body2"}
/// ÿһ����ͬ�Ĳ���ʽ���¸�����ʽ���ű��벻ͬ����ʹ��[a-zA-Z0-9_]+
/// �ñ���������ֲ�ͬ�Ĳ���ʽ�岢Ϊ�乹����Ӧ�İ�װ����
/// ����GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(example_symbol,
///                ExampleFunctionName, 0, false, {"Id", "=", "Assignable"})
#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION( \
    production_symbol, reduct_function_name, production_body_seq, ...)

#ifdef GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT
#undef GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT
#endif  // GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT
/// @brief ���÷��ս����ʽ���Կչ�Լ
/// @param[in] production_symbol �������ÿ��Կչ�Լ�ķ��ս����ʽ��
/// @note
/// δ���ô˷����ķ��ս�ڵ�Ĭ�ϲ����Կչ�Լ
/// ����ʱ���豣֤�����ķ��ս����ʽ�Ѿ����壬
/// ��Ҫ���Ѷ�����ս����ʽ�����Ĳ���ʽ
/// ����GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT(example_symbol)
#define GENERATOR_SET_NONTERMINAL_PRODUCTION_COULD_EMPTY_REDUCT( \
    production_symbol)

#ifdef GENERATOR_DEFINE_ROOT_PRODUCTION
#undef GENERATOR_DEFINE_ROOT_PRODUCTION
#endif  // GENERATOR_DEFINE_BINARY_OPERATOR
/// @brief ���ø�����ʽ
/// @param[in] production_symbol �������õĸ����ս����ʽ��
/// @note
/// ����ʱ�����Ѷ���÷��ս�ڵ㣬��Ҫ���Ѷ�����ս����ʽ�����Ĳ���ʽ
/// ����GENERATOR_DEFINE_ROOT_PRODUCTION(example_symbol)
#define GENERATOR_DEFINE_ROOT_PRODUCTION(production_symbol)

// �ⲿ�ֺ��������Ҫ����ʽ��Ϣ���ļ��ػ�

// ��config_construct.cpp��ת��Ϊ��SyntaxGenerator��Ա�����ĵ��ô���
// �����ConfigConstruct�����У���SyntaxGenerator�������ʽ��Ϣ
#ifdef GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_

// ��ֹ�ظ�����
#ifndef GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_END
// ����ú���������������ʹ��IntelliSense��������ͷ�ļ�
// �ں��������ڰ���ͷ�ļ��ᵼ�´�����ִ���
#define SHIELD_HEADERS_FOR_INTELLISENSE

#ifdef GENERATOR_DEFINE_KEY_WORD
#undef GENERATOR_DEFINE_KEY_WORD
#endif  // GENERATOR_DEFINE_KEY_WORD
#define GENERATOR_DEFINE_KEY_WORD(key_word) AddKeyWord(key_word);

#ifdef GENERATOR_DEFINE_BINARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_OPERATOR
#endif  // GENERATOR_DEFINE_BINARY_OPERATOR
#define GENERATOR_DEFINE_BINARY_OPERATOR(                                   \
    operator_symbol, binary_operator_associatity, binary_operator_priority) \
  AddBinaryOperator(operator_symbol, binary_operator_associatity,           \
                    OperatorPriority(binary_operator_priority));

#ifdef GENERATOR_DEFINE_UNARY_OPERATOR
#undef GENERATOR_DEFINE_UNARY_OPERATOR
#endif  // GENERATOR_DEFINE_UNARY_OPERATOR
#define GENERATOR_DEFINE_UNARY_OPERATOR(                                  \
    operator_symbol, unary_operator_associatity, unary_operator_priority) \
  AddLeftUnaryOperator(operator_symbol, unary_operator_associatity,       \
                       OperatorPriority(unary_operator_priority));

#ifdef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#undef GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#endif  // !GENERATOR_DEFINE_BINARY_UNARY_OPERATOR
#define GENERATOR_DEFINE_BINARY_UNARY_OPERATOR(                             \
    operator_symbol, binary_operator_associatity, binary_operator_priority, \
    unary_operator_associatity, unary_operator_priority)                    \
  AddBinaryLeftUnaryOperator(operator_symbol, binary_operator_associatity,  \
                             OperatorPriority(binary_operator_priority),    \
                             unary_operator_associatity,                    \
                             OperatorPriority(unary_operator_priority));

#ifdef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_TERMINAL_PRODUCTION
#define GENERATOR_DEFINE_TERMINAL_PRODUCTION(production_symbol, \
                                             production_body)   \
  AddTerminalProduction(production_symbol, production_body);

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                   \
    production_symbol, reduct_function, production_body_seq, ...)  \
  AddNonTerminalProduction<NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(   \
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

#else
#error ������config_construct.cpp�������production_config-inc.h���ظ�����
#endif
#endif

// ��process_function_classes.h��ת��Ϊ��װ��Լ��������
// �������η�������NONTERMINAL_PRODUCTION_SYMBOL_MODIFY��
// NONTERMINAL_PRODUCTION_SYMBOL_MODIFY_STR
#ifdef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_
// ��ֹ���ظ�����
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_END

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION

/// ����ú���������������ʹ��IntelliSense��������ͷ�ļ�
/// �������ռ��ڰ�����Щͷ�ļ��ᵼ�¸�����ֵĴ���
#define SHIELD_HEADERS_FOR_INTELLISENSE

#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                              \
    production_symbol, reduct_function, production_body_seq, ...)             \
  class NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,               \
                                             production_body_seq)             \
      : public ProcessFunctionInterface {                                     \
   public:                                                                    \
    virtual ProcessFunctionInterface::UserData Reduct(                        \
        std::vector<ProcessFunctionInterface::WordDataToUser>&& word_data)    \
        const override {                                                      \
      return reduct_function(std::move(word_data));                           \
    }                                                                         \
                                                                              \
   private:                                                                   \
    friend class boost::serialization::access;                                \
    template <class Archive>                                                  \
    void serialize(Archive& ar, const unsigned int version) {                 \
      ar& boost::serialization::base_object<ProcessFunctionInterface>(*this); \
    }                                                                         \
  };
#endif
#endif

// ��process_functions_classes_register.h��ת��Ϊע��
// process_function_classes.h��������ĺ꣬���boost-serializationʹ��
#ifdef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER
/// ��ֹ�ظ�����
#ifndef GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_REGISTER_END

/// ����ú���������������ʹ��IntelliSense��������ͷ�ļ�
#define SHIELD_HEADERS_FOR_INTELLISENSE

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  /// GENERATOR_DEFINE_NONTERMINAL_PRODUCTION

#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                     \
    production_symbol, reduct_function, production_body_seq, ...)    \
  BOOST_CLASS_EXPORT_GUID(                                           \
      frontend::generator::syntax_generator::                        \
          NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,    \
                                               production_body_seq), \
      "frontend::generator::"                                        \
      "syntax_generator::" NONTERMINAL_PRODUCTION_SYMBOL_MODIFY_STR( \
          production_symbol, production_body_seq))
#else
#errror ������process_functions_classes_register.h�ļ�������� \
    "production_config-inc.h"���ظ�����
#endif
#endif

/// Ϊ��ʹ��Intellisense��������������Ҫ��ͷ�ļ�
/// �˴���ͷ�ļ��ڴ������ɹ����лᱻ����
/// �û�����user_defined_functions.h������Լ������ͷ�ļ�
/// ��ÿ���������ļ����ļ��ж�Ӧ����SHIELD_HEADERS_FOR_INTELLISENSE����������Щ
/// �ļ����������ָ�����ֱ���
#ifndef SHIELD_HEADERS_FOR_INTELLISENSE
#include "Common/common.h"
#include "Config/ProductionConfig/user_defined_functions.h"
#include "Generator/export_types.h"
/// �ṩ���������Ե�ö��
using OperatorAssociatityType =
    frontend::generator::syntax_generator::OperatorAssociatityType;
#endif  /// !SHIELD_HEADERS_FOR_INTELLIGENCE