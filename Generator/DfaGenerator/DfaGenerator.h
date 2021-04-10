#pragma once

#include <array>

#include "NfaGenerator/NfaGenerator.h"
#include "common.h"

class DfaGenerator {
  struct DfaNode;
  struct IntermediateDfaNode;
 public:
  using TailNodeTag = NfaGenerator::TailNodeData::first_type;
  using PriorityTag = NfaGenerator::TailNodeData::second_type;
  using TailNodeData = NfaGenerator::TailNodeData;
  using NfaNodeHandler = NfaGenerator::NodeHandler;
  using SetType = std::unordered_set<NfaNodeHandler>;
  using DfaNodeHandler = NodeManager<DfaNode>::NodeHandler;
  using IntermediateNodeHandler = NodeManager<IntermediateDfaNode>::NodeHandler;
  using SetNodeHandler = NodeManager<SetType>::NodeHandler;

  DfaGenerator() : head_node_(-1) {}
  DfaGenerator(const DfaGenerator&) = delete;
  DfaGenerator(DfaGenerator&&) = delete;

  //��ӹؼ���
  bool AddKeyword(const std::string& str, TailNodeTag tail_node_tag,
                  PriorityTag priority_tag);
  //�������
  bool AddRegexpression(const std::string& str, TailNodeTag tail_node_tag,
                        PriorityTag priority_tag);
  //����DFA
  bool DfaConstruct();
  //������д��Config/DFA Config�ļ�����DFA.conf
  bool ConfigConstruct();

 private:
  struct IntermediateDfaNode {
    //�ýڵ��ת���������洢IntermediateDfaNode�ڵ���
    std::array<IntermediateNodeHandler, kchar_num> forward_nodes;
    SetNodeHandler set_handler;  //�ýڵ��Ӧ���Ӽ��հ�
    IntermediateDfaNode(SetNodeHandler handler) : set_handler(handler) {}
    //������������������ת��
    void SetAllUntransable();
  };
  struct DfaNode {
    std::array<DfaNodeHandler, kchar_num> forward_nodes;
  };
  IntermediateDfaNode* GetIntermediateNode(IntermediateNodeHandler handler) {
    return node_manager_intermediate_node_.GetNode(handler);
  }
  DfaNode* GetDfaNode(DfaNodeHandler handler) {
    return node_manager_dfa_node_.GetNode(handler);
  }
 SetType* GetSetNode(SetNodeHandler handler) {
    return node_manager_set_.GetNode(handler);
  }
  //����ת�ƺ������Ӽ����취�ã�
  SetNodeHandler SetGoto(SetNodeHandler set_src, char c_transform);
  // DFAת�ƺ�������С��DFA�ã�
  DfaNodeHandler DfaGoto(DfaNodeHandler dfa_src, char c_transform);
  //����DFA�м�ڵ�����ת��
  bool SetIntermediateNodeTransform(IntermediateNodeHandler node_intermediate_src,
                                    char c_transform,
                                    IntermediateNodeHandler node_intermediate_dst);
  //��������DFA�ڵ�����ת��
  bool SetFinalNodeTransform(DfaNodeHandler node_final_src, char c_transform,
                             DfaNodeHandler node_final_dst);
  //��������Ѵ����򷵻�true���������������벢����false
  //���صĵڶ�������Ϊ��Ӧ�м�ڵ���
  std::pair<bool, DfaNodeHandler> InOrInsert(
      const std::unordered_set<NfaNodeHandler>& uset);
  DfaNodeHandler head_node_;
  NfaGenerator nfa_generator_;
  //�洢DFA�м�ڵ�
  NodeManager<IntermediateDfaNode> node_manager_intermediate_node_;
  //�洢��СDFA�ڵ�
  NodeManager<DfaNode> node_manager_dfa_node_;
  //�洢���ϣ��Ӽ����취�õ��ģ�
  NodeManager<SetType> node_manager_set_;
  //�洢���ϵĹ�ϣ���ڵ�����ӳ��
  std::unordered_multimap<IntergalSetHashType, DfaNodeHandler>
      set_hash_to_intermediate_node_handler_;
};
