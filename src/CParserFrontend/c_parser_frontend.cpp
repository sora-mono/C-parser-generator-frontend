#include "c_parser_frontend.h"

// ��ȡ������Ϣ
// ��������ڸ������Ƶĺ����򷵻ؿ�ָ��
namespace c_parser_frontend {
inline FunctionDefine* CParserFrontend::GetFunction(
    const std::string& function_name) {
  auto iter = functions_.find(function_name);
  if (iter == functions_.end()) [[unlikely]] {
    return nullptr;
  } else {
    return &*iter->second;
  }
}
}  // namespace c_parser_frontend