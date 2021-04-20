#include "Common/NodeManager.h"
#ifndef GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_
#define GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_

namespace generator::lexicalgenerator {
using common::NodeManager;

class LexicalGenerator {
 private:
  struct BaseNode;

  using StringIdType = NodeManager<std::string>::NodeId;
  using NodeIdType = NodeManager<BaseNode>::NodeId;
  //�ڵ����ͣ���Ϊ�ս��������ս����
  enum SymbolType : bool { kTerminalSymbol, kNonTerminalSymbol };
  //�ڵ��ǣ���ͨ�ڵ㡢������ڵ�
  enum SymbolTag : bool { kUsual, kOperator };

  struct BaseNode {
    SymbolTag node_tag;           //�ڵ���
    NodeIdType node_id;           //�ýڵ�ID
    StringIdType node_symbol_id;  //�ýڵ���ŵ�ID
    virtual ~BaseNode() {}
  };
  struct TerminalNode : public BaseNode {
    bool is_opreator;  //�Ƿ�Ϊ�����
  };
  struct NonTerminalNode : public BaseNode {
    //���ʽ����
    std::vector<std::vector<NodeIdType>> body;
  };
};
}  // namespace generator::lexicalgenerator
#endif  // !GENERATOR_LEXICALGENERATOR_LEXICALGENERATOR_H_
