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
// ���еĺ��еĲ���ʽ��/��������������˫����
// ������ʽ����Ҫ���ճ�ʼ���б��д����д

// ���η��ս�ڵ����ʽ���Ի�ȡ��װReduct�����õ�����
#define NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,   \
                                             production_body_seq) \
  (production_symbol##production_body_seq##_)

// ���¶����GENERATOR_DEFINE_NONTERMINAL_PRODUCTION����˵����
// �ַ������������Ҫд���ַ�����ʽ
// �ؼ��ֶ���
#ifdef GENERATOR_DEFINE_KEY_WORD
#undef GENERATOR_DEFINE_KEY_WORD
#endif  // !GENERATOR_DEFINE_KEY_WORD
#define GENERATOR_DEFINE_KEY_WORD(key_word)

// ��������ţ������(kLeftToRight, kRightAssociate)�����ȼ�
#ifdef GENERATOR_DEFINE_OPERATOR
#undef GENERATOR_DEFINE_OPERATOR
#endif  // !GENERATOR_DEFINE_OPERATOR
#define GENERATOR_DEFINE_OPERATOR(operator_symbol, operator_associatity, \
                                  operator_priority)

// ������ʽ���壬��Ҫ�ڸ�������ʽ����ʽ��Ӻ�ʹ��
// �����ò���ʽ��������ȫ���ڵ㶼�ܹ���Ӻ�
#ifdef GENERATOR_DEFINE_ROOT_PRODUCTION
#undef GENERATOR_DEFINE_ROOT_PRODUCTION
#endif  // GENERATOR_DEFINE_OPERATOR
#define GENERATOR_DEFINE_ROOT_PRODUCTION(production_symbol)

// �ս�ڵ㶨�壬production_bodyΪ������ʽ
#ifdef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_TERMINAL_PRODUCTION
#define GENERATOR_DEFINE_TERMINAL_PRODUCTION(production_symbol, production_body)

// ���ս�ڵ㶨�壬����ʽ�岿����ʹ�ó�ʼ���б���д��
// ����{"production_body1","production_body2"}
// �ĸ����������������˫����
// reduct_function�ǹ�Լ�ú���
// production_body_seq�ǲ���ʽ����
// ÿһ����ͬ�Ĳ���ʽ���¸�����ʽ���ű��벻ͬ����ʹ��[a-zA-Z0-9_]+
// �ñ���������ֲ�ͬ�Ĳ���ʽ�岢Ϊ�乹����Ӧ�İ�װ����
// could_empty_reduct��ʾ�ò���ʽ�Ƿ���Կչ�Լ
// ���й�Լ��ͬһ��production_symbol�Ĳ���ʽ����ֻ�����һ��
#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                              \
    production_symbol, production_body, reduct_function, production_body_seq, \
    could_empty_reduct)

// �ⲿ�ֶ�������Ҫ����ʽ��Ϣ���ļ��ػ�
#ifdef GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_

#ifdef GENERATOR_DEFINE_KEY_WORD
#undef GENERATOR_DEFINE_KEY_WORD
#endif  // GENERATOR_DEFINE_KEY_WORD
#define GENERATOR_DEFINE_KEY_WORD(key_word) AddKeyWord((key_word));

#ifdef GENERATOR_DEFINE_OPERATOR
#undef GENERATOR_DEFINE_OPERATOR
#endif  // GENERATOR_DEFINE_OPERATOR
#define GENERATOR_DEFINE_OPERATOR(operator_symbol, operator_associatity, \
                                  operator_priority)                     \
  AddOperatorNode((operator_symbol), (operator_associatity),             \
                  (OperatorPriority(operator_priority)));

#ifdef GENERATOR_DEFINE_ROOT_PRODUCTION
#undef GENERATOR_DEFINE_ROOT_PRODUCTION
#endif  // GENERATOR_DEFINE_ROOT_PRODUCTION
#define GENERATOR_DEFINE_ROOT_PRODUCTION(production_symbol) \
  SetRootProduction((production_symbol));

#ifdef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_TERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_TERMINAL_PRODUCTION
#define GENERATOR_DEFINE_TERMINAL_PRODUCTION(production_symbol, \
                                             production_body)   \
  AddTerminalNode((production_symbol), (production_body));

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                              \
    production_symbol, production_body, reduct_function, production_body_seq, \
    could_empty_reduct)                                                       \
  AddNonTerminalNode<NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(                    \
      production_symbol, production_body_seq)>(                               \
      (#production_symbol), (production_body), (could_empty_reduct));

#elif defined GENERATOR_SYNTAXGENERATOR_PROCESS_FUNCTIONS_CLASSES_

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION

#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                              \
    production_symbol, production_body, reduct_function, production_body_seq, \
    could_empty_reduct)                                                       \
  class NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,               \
                                             production_body_seq)             \
      : public ProcessFunctionInterface {                                     \
   public:                                                                    \
    virtual ProcessFunctionInterface::UserData Reduct(                        \
        std::vector<ProcessFunctionInterface::WordDataToUser>&& word_data)    \
        override {                                                            \
      return reduct_function(std::move(word_data));                           \
    }                                                                         \
  };

#elif defined GENERATOR_SYNTAXGENERATOR_USER_DEFINED_FUNCTION_AND_DATA_REGISTER_

#ifdef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#undef GENERATOR_DEFINE_NONTERMINAL_PRODUCTION
#endif  // GENERATOR_DEFINE_NONTERMINAL_PRODUCTION

#define GENERATOR_DEFINE_NONTERMINAL_PRODUCTION(                              \
    production_symbol, production_body, reduct_function, production_body_seq, \
    could_empty_reduct)                                                       \
  BOOST_CLASS_EXPORT_GUID(                                                    \
      frontend::generator::syntaxgenerator::                                  \
          NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(production_symbol,             \
                                               production_body_seq),          \
      "frontend::generator::"                                                 \
      "syntaxgenerator:" #NONTERMINAL_PRODUCTION_SYMBOL_MODIFY(               \
          production_symbol, production_body_seq))

#else

#error ���ļ����ұ������ձ�Generator/SyntaxGenerator�µ�\
process_functions_classes.h \
user_defined_function_and_data_register.h \
config_construct.cpp������

#endif  // GENERATOR_SYNTAXGENERATOR_CONFIG_CONSTRUCT_MACRO