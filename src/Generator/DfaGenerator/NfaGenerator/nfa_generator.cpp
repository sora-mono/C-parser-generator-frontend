#include "nfa_generator.h"

#include <algorithm>
#include <format>
#include <queue>
#include <sstream>

#include "Generator/DfaGenerator/NfaGenerator/nfa_generator.h"
#include "nfa_generator.h"

namespace frontend::generator::dfa_generator::nfa_generator {

NfaGenerator::NfaNodeId NfaGenerator::NfaNode::GetForwardNodesId(
    char c_transfer) {
  auto iter = nodes_forward.find(c_transfer);
  if (iter == nodes_forward.end()) {
    return NfaNodeId::InvalidId();
  } else {
    return iter->second;
  }
}

inline void NfaGenerator::NfaNode::SetConditionTransfer(
    char c_condition, NfaGenerator::NfaNodeId node_id) {
  nodes_forward[c_condition] = node_id;
}

inline void NfaGenerator::NfaNode::AddNoconditionTransfer(NfaNodeId node_id) {
  conditionless_transfer_nodes_id.insert(node_id);
}

inline void NfaGenerator::NfaNode::RemoveConditionalTransfer(char c_treasfer) {
  auto iter = nodes_forward.find(c_treasfer);
  if (iter != nodes_forward.end()) {
    nodes_forward.erase(iter);
  }
}

inline void NfaGenerator::NfaNode::RemoveConditionlessTransfer(
    NfaNodeId node_id) {
  auto iter = conditionless_transfer_nodes_id.find(node_id);
  if (iter != conditionless_transfer_nodes_id.end()) {
    conditionless_transfer_nodes_id.erase(iter);
  }
}

std::pair<std::unordered_set<typename NfaGenerator::NfaNodeId>,
          typename NfaGenerator::TailNodeData>
NfaGenerator::Closure(NfaNodeId production_node_id) {
  std::unordered_set<NfaNodeId> result_set;
  TailNodeData word_attached_data(NotTailNodeTag);
  std::queue<NfaNodeId> q;
  for (auto x : node_manager_.GetIdsReferringSameObject(production_node_id)) {
    q.push(x);
  }
  // ��յ�Ч�ڵ㼯�ϣ�������������нڵ㶼�ڼ����ڣ�����
  result_set.clear();
  while (!q.empty()) {
    NfaNodeId id_now = q.front();
    q.pop();
    if (result_set.find(id_now) != result_set.end()) {
      continue;
    }
    auto [result_iter, inserted] = result_set.insert(id_now);
    assert(inserted);
    const auto& nfa_node = GetNfaNode(id_now);
    auto iter = tail_nodes_.find(&nfa_node);
    // �ж��Ƿ�Ϊβ�ڵ�
    if (iter != tail_nodes_.end()) {
      TailNodeData& tail_node_data_new = iter->second;
      WordPriority priority_old = word_attached_data.second;
      WordPriority priority_new = tail_node_data_new.second;
      if (word_attached_data == NotTailNodeTag) {
        // ��ǰ��β�ڵ��¼
        word_attached_data = tail_node_data_new;
      } else if (priority_new > priority_old) {
        // ��ǰ��¼���ȼ�������ǰ�����ȼ�
        word_attached_data = tail_node_data_new;
      } else if (priority_new == priority_old &&
                 tail_node_data_new.first != word_attached_data.first) {
        // ����β�ڵ������ȼ���ͬ����Ӧβ�ڵ㲻ͬ
        // ���������Ϣ
        std::cerr
            << std::format(
                   "NfaGenerator "
                   "Error:"
                   "��������������ʽ����ͬ�������¾��ɻ�ȡ���ʣ��ҵ������ȼ�"
                   "��ͬ���������壬�����ս�ڵ㶨��/��������岿��")
            << std::endl;
        assert(false);
        exit(-1);
      }
    }
    for (auto x : nfa_node.GetUnconditionTransferNodesIds()) {
      q.push(x);
    }
  }
  return std::make_pair(std::move(result_set), std::move(word_attached_data));
}

std::pair<std::unordered_set<typename NfaGenerator::NfaNodeId>,
          typename NfaGenerator::TailNodeData>
NfaGenerator::Goto(NfaNodeId id_src, char c_transform) {
  NfaNode& pointer_node = GetNfaNode(id_src);
  NfaNodeId production_node_id = pointer_node.GetForwardNodesId(c_transform);
  if (!production_node_id.IsValid()) {
    return std::make_pair(std::unordered_set<NfaNodeId>(), NotTailNodeTag);
  }
  return Closure(production_node_id);
}

void NfaGenerator::NfaInit() {
  head_node_id_ = NfaNodeId::InvalidId();
  tail_nodes_.clear();
  node_manager_.MultimapObjectManagerInit();
  head_node_id_ = node_manager_.EmplaceObject();  // ���ͷ���
}

bool NfaGenerator::NfaNode::MergeNodesWithManager(NfaNode& node_src) {
  if (&node_src == this) {  // ��ͬ�ڵ�ϲ���ֱ�ӷ���true
    return true;
  }
  if (nodes_forward.size() != 0 && node_src.nodes_forward.size() != 0) {
    bool CanBeSourceInMerge = true;
    for (auto& p : node_src.nodes_forward) {
      auto iter = nodes_forward.find(p.first);
      if (iter != nodes_forward.end() && iter->second != p.second) {
        CanBeSourceInMerge = false;
        break;
      }
    }
    if (!CanBeSourceInMerge) {
      return false;
    }
  }
  nodes_forward.merge(node_src.nodes_forward);
  conditionless_transfer_nodes_id.merge(
      node_src.conditionless_transfer_nodes_id);
  return true;
}

inline const NfaGenerator::TailNodeData NfaGenerator::GetTailNodeData(
    NfaNode* pointer) {
  auto iter = tail_nodes_.find(pointer);
  if (iter == tail_nodes_.end()) {
    return NotTailNodeTag;
  }
  return iter->second;
}

inline const NfaGenerator::TailNodeData NfaGenerator::GetTailNodeData(
    NfaNodeId production_node_id) {
  return GetTailNodeData(&GetNfaNode(production_node_id));
}
// TODO �����ĳ�printf�Ⱥ���
std::pair<NfaGenerator::NfaNodeId, NfaGenerator::NfaNodeId>
NfaGenerator::RegexConstruct(std::istream& in, const TailNodeData& tag,
                             const bool add_to_NFA_head,
                             const bool return_when_right_bracket) {
  NfaNodeId head_id = node_manager_.EmplaceObject();
  NfaNodeId tail_id = head_id;
  NfaNodeId pre_tail_id = head_id;
  char c_now;
  in >> c_now;
  while (c_now != '\0' && in) {
    NfaNodeId temp_head_id = NfaNodeId::InvalidId(),
              temp_tail_id = NfaNodeId::InvalidId();
    switch (c_now) {
      case '[': {
        std::tie(temp_head_id, temp_tail_id) = CreateSwitchTree(in);
        if (!(temp_head_id.IsValid() && temp_tail_id.IsValid())) {
          throw std::invalid_argument("�Ƿ�����");
        }
        GetNfaNode(tail_id).AddNoconditionTransfer(temp_head_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
      } break;
      case ']':
        throw std::runtime_error(
            "regex_construct������Ӧ�ô���]�ַ���Ӧ����create_switch_tree����");
        break;
      case '(': {
        std::tie(temp_head_id, temp_tail_id) =
            RegexConstruct(in, NotTailNodeTag, false, true);
        if (!(temp_head_id.IsValid() && temp_tail_id.IsValid())) {
          throw std::invalid_argument("�Ƿ�����");
        }
        GetNfaNode(tail_id).AddNoconditionTransfer(temp_head_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
      } break;
      case ')':
        in >> c_now;
        if (!in) {
          c_now = '\0';
          break;
        }
        switch (c_now) {
          case '*':
            temp_tail_id = node_manager_.EmplaceObject();
            GetNfaNode(tail_id).AddNoconditionTransfer(temp_tail_id);
            GetNfaNode(temp_tail_id).AddNoconditionTransfer(head_id);
            GetNfaNode(head_id).AddNoconditionTransfer(temp_tail_id);
            pre_tail_id = tail_id;
            tail_id = temp_tail_id;
            break;
          case '+':
            temp_tail_id = node_manager_.EmplaceObject();
            GetNfaNode(temp_tail_id).AddNoconditionTransfer(head_id);
            GetNfaNode(tail_id).AddNoconditionTransfer(head_id);
            pre_tail_id = tail_id;
            tail_id = temp_tail_id;
            break;
          case '?':
            temp_tail_id = node_manager_.EmplaceObject();
            GetNfaNode(tail_id).AddNoconditionTransfer(temp_tail_id);
            GetNfaNode(head_id).AddNoconditionTransfer(temp_tail_id);
            pre_tail_id = tail_id;
            tail_id = temp_tail_id;
            break;
          default:
            in.putback(c_now);
            break;
        }
        if (return_when_right_bracket) {
          c_now = '\0';
        }
        break;
      case '+':  // ���Ե����ַ���Ч
        temp_tail_id = node_manager_.EmplaceObject();
        GetNfaNode(tail_id).AddNoconditionTransfer(temp_tail_id);
        GetNfaNode(pre_tail_id).AddNoconditionTransfer(temp_tail_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
        break;
      case '*':  // ���Ե����ַ���Ч
        temp_tail_id = node_manager_.EmplaceObject();
        GetNfaNode(tail_id).AddNoconditionTransfer(temp_tail_id);
        GetNfaNode(pre_tail_id).AddNoconditionTransfer(temp_tail_id);
        GetNfaNode(temp_tail_id).AddNoconditionTransfer(pre_tail_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
        break;
      case '?':  // ���Ե����ַ���Ч
        temp_tail_id = node_manager_.EmplaceObject();
        GetNfaNode(tail_id).AddNoconditionTransfer(temp_tail_id);
        GetNfaNode(pre_tail_id).AddNoconditionTransfer(temp_tail_id);
        pre_tail_id = tail_id;
        tail_id = pre_tail_id;
        break;
      case '\\':  // ���Ե����ַ���Ч
        in >> c_now;
        if (!in || c_now == '\0') {
          throw std::invalid_argument("�Ƿ�����");
        }
        temp_tail_id = node_manager_.EmplaceObject();
        GetNfaNode(tail_id).SetConditionTransfer(c_now, temp_tail_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
        break;
      case '.':  // ���Ե����ַ���Ч
        temp_tail_id = node_manager_.EmplaceObject();
        for (char transform_char = CHAR_MIN; transform_char != CHAR_MAX;
             ++transform_char) {
          GetNfaNode(tail_id).SetConditionTransfer(transform_char,
                                                   temp_tail_id);
        }
        GetNfaNode(tail_id).SetConditionTransfer(CHAR_MAX, temp_tail_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
        break;
      default:
        temp_tail_id = node_manager_.EmplaceObject();
        GetNfaNode(tail_id).SetConditionTransfer(c_now, temp_tail_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
        break;
    }
    if (c_now != '\0') {
      in >> c_now;
    }
  }
  if (head_id != tail_id) {
    if (add_to_NFA_head) {
      GetNfaNode(head_node_id_).AddNoconditionTransfer(head_id);
      AddTailNode(tail_id, tag);
    }
  } else {
    node_manager_.RemoveObject(head_id);
    head_id = tail_id = NfaNodeId::InvalidId();
  }
  return std::make_pair(head_id, tail_id);
}

std::pair<NfaGenerator::NfaNodeId, NfaGenerator::NfaNodeId>
NfaGenerator::WordConstruct(const std::string& str,
                            TailNodeData&& word_attached_data) {
  assert(str.size() != 0);
  NfaNodeId head_id = node_manager_.EmplaceObject();
  NfaNodeId tail_id = head_id;
  for (auto c : str) {
    NfaNodeId temp_id = node_manager_.EmplaceObject();
    GetNfaNode(tail_id).SetConditionTransfer(c, temp_id);
    tail_id = temp_id;
  }
  GetNfaNode(head_node_id_).AddNoconditionTransfer(head_id);
  AddTailNode(tail_id, std::move(word_attached_data));
  return std::make_pair(head_id, tail_id);
}

void NfaGenerator::MergeOptimization() {
  node_manager_.SetAllObjectsCanBeSourceInMerge();
  std::queue<NfaNodeId> q;
  q.push(GetHeadNfaNodeId());
  while (!q.empty()) {
    NfaNodeId id_now = q.front();
    NfaNode& node_now = GetNfaNode(id_now);
    q.pop();
    if (!node_manager_.CanBeSourceInMerge(id_now)) {
      continue;
    }
    // ���ÿһ���뵱ǰ����ڵ�ȼ۵Ľڵ㣨���ǵ�ǰ�ڵ����������ת�Ƶ��Ľڵ㣩
    // �����ǰ�ڵ�ת�Ʊ��е����ڵȼ۽ڵ��д�������Դӵ�ǰ�ڵ�ת�Ʊ���ɾ��
    for (auto equal_node_id :
         GetNfaNode(id_now).GetUnconditionTransferNodesIds()) {
      if (equal_node_id == id_now) [[unlikely]] {
        // ����ת�Ƶ��Լ������
        continue;
      }
      auto& equal_node = GetNfaNode(equal_node_id);
      // ����������ת�Ʊ�
      const auto& equal_node_unconditional_transfer_node_ids =
          equal_node.GetUnconditionTransferNodesIds();
      for (auto iter = node_now.GetUnconditionTransferNodesIds().begin();
           iter != node_now.GetUnconditionTransferNodesIds().end();) {
        if (equal_node_unconditional_transfer_node_ids.find(*iter) !=
            equal_node_unconditional_transfer_node_ids.end()) {
          // ��ǰ�ڵ�ת�Ʊ��е����ڵȼ۽ڵ��д���
          // �Ƴ�����
          if (*iter != equal_node_id) [[likely]] {
            // ������ڵȼ۽ڵ���������Ի��ڵ������Ƴ�
            // �����ʧȥָ��ȼ۽ڵ�ļ�¼�����������������ڴ�й©��
            iter = node_now.GetUnconditionTransferNodesIds().erase(iter);
            // continue��ֹ����ǰ��iter
            continue;
          }
        }
        ++iter;
      }
      // ��������ת�Ʊ�
      const auto& equal_node_conditional_transfers =
          equal_node.GetConditionalTransfers();
      for (auto iter = node_now.GetConditionalTransfers().begin();
           iter != node_now.GetConditionalTransfers().end();) {
        auto equal_node_iter =
            equal_node_conditional_transfers.find(iter->first);
        if (equal_node_iter != equal_node_conditional_transfers.end() &&
            iter->second == equal_node_iter->second) {
          // ��ǰ�ڵ�ת�Ʊ��е����ڵȼ۽ڵ��д���
          // �Ƴ�����
          iter = node_now.GetConditionalTransfers().erase(iter);
          // continue��ֹ����ǰ��iter
          continue;
        }
        ++iter;
      }
      // ѹ���Ч�ڵ�ȴ�����
      q.push(equal_node_id);
      // ѹ�뵱ǰ�ڵ�����п�������ת�Ƶ��Ľڵ�ȴ�����
      for (const auto& conditional_transfer :
           node_now.GetConditionalTransfers()) {
        q.push(conditional_transfer.second);
      }
    }
    if (node_now.GetConditionalTransfers().empty() &&
        node_now.GetUnconditionTransferNodesIds().size() == 1) [[unlikely]] {
      // ֻʣһ��������ת��·�����ýڵ������������ת�Ƶ��Ľڵ�ϲ�
      bool result = node_manager_.MergeObjectsWithManager<NfaGenerator>(
          *node_now.GetUnconditionTransferNodesIds().begin(), id_now, *this,
          MergeNfaNodes);
      assert(result);
    } else {
      // û��ִ���κβ����������ڴӸýڵ㿪ʼ�ĺϲ�����
      // ���øýڵ��ںϲ�ʱ������ΪԴ�ڵ�
      node_manager_.SetObjectCanNotBeSourceInMerge(id_now);
    }
  }
}

inline bool NfaGenerator::RemoveTailNode(NfaNode* pointer) {
  if (pointer == nullptr) {
    return false;
  }
  tail_nodes_.erase(pointer);
  return true;
}

bool NfaGenerator::AddTailNode(NfaNode* pointer, const TailNodeData& tag) {
  assert(pointer != nullptr);
  tail_nodes_.insert(std::make_pair(pointer, tag));
  return false;
}

std::pair<NfaGenerator::NfaNodeId, NfaGenerator::NfaNodeId>
NfaGenerator::CreateSwitchTree(std::istream& in) {
  NfaNodeId head_id = node_manager_.EmplaceObject();
  NfaNodeId tail_id = node_manager_.EmplaceObject();
  char character_now, character_pre;
  in >> character_now;
  character_pre = character_now;
  NfaNode& head_node = GetNfaNode(head_id);
  while (in && character_now != ']') {
    switch (character_now) {
      case '-':
        // ������һ�˵��ַ�
        in >> character_now;
        for (char bigger_character = std::max(character_now, character_pre),
                  smaller_character = std::min(character_pre, character_now);
             smaller_character != character_now; smaller_character++) {
          head_node.SetConditionTransfer(smaller_character, tail_id);
        }
        head_node.SetConditionTransfer(character_now, tail_id);
        break;
      case '\\':
        in >> character_now;
        if (!in || character_now == '\0') {
          throw std::invalid_argument("�Ƿ�����");
        }
        head_node.SetConditionTransfer(character_now, tail_id);
        break;
      default:
        head_node.SetConditionTransfer(character_now, tail_id);
        break;
    }
    character_pre = character_now;
    in >> character_now;
  }
  if (character_now != ']' || !in) {
    throw std::invalid_argument("�Ƿ�����");
  }
  if (head_node.GetConditionalTransfers().empty()) {
    node_manager_.RemoveObject(head_id);
    throw std::invalid_argument("[]��Ϊ��");
  }

  in >> character_now;
  if (!in) {
    return std::make_pair(head_id, tail_id);
  }
  switch (character_now) {
    case '*':
      GetNfaNode(head_id).AddNoconditionTransfer(tail_id);
      GetNfaNode(tail_id).AddNoconditionTransfer(head_id);
      break;
    case '+':
      GetNfaNode(tail_id).AddNoconditionTransfer(head_id);
      break;
    case '?':
      GetNfaNode(head_id).AddNoconditionTransfer(tail_id);
      break;
    default:
      in.putback(character_now);
      break;
  }
  return std::make_pair(head_id, tail_id);
}

bool NfaGenerator::MergeNfaNodes(NfaGenerator::NfaNode& node_dst,
                                 NfaGenerator::NfaNode& node_src,
                                 NfaGenerator& nfa_generator) {
  const NfaGenerator::TailNodeData dst_tag =
      nfa_generator.GetTailNodeData(&node_dst);
  const NfaGenerator::TailNodeData src_tag =
      nfa_generator.GetTailNodeData(&node_src);
  if (dst_tag != NfaGenerator::NotTailNodeTag &&
      src_tag != NfaGenerator::NotTailNodeTag &&
      dst_tag.second == src_tag.second && dst_tag.first != src_tag.first) {
    throw std::runtime_error("����β�ڵ������ͬ���ȼ��Ҳ���Ӧͬһ���ڵ�");
  }
  bool result = node_dst.MergeNodesWithManager(node_src);
  if (result) {
    if (src_tag != NfaGenerator::NotTailNodeTag) {
      nfa_generator.RemoveTailNode(&node_src);
      nfa_generator.AddTailNode(&node_dst, src_tag);
    }
    return true;
  } else {
    return false;
  }
}

const NfaGenerator::TailNodeData NfaGenerator::NotTailNodeTag =
    NfaGenerator::TailNodeData(WordAttachedData(), WordPriority::InvalidId());

// ������һ�������Ƿ�Ϊ��Լ�ж�ʹ����൥Ŀ��������ȼ�����˫Ŀ��������ȼ�
// ���ػ�ȡ���Ľ�����ͺ����ȼ�

std::pair<frontend::common::OperatorAssociatityType, size_t>
NfaGenerator::WordAttachedData::GetAssociatityTypeAndPriority(
    bool is_last_operate_reduct) const {
  assert(node_type == frontend::common::ProductionNodeType::kOperatorNode);
  if (binary_operator_priority != -1) {
    if (unary_operator_priority != -1) {
      // �������������
      if (is_last_operate_reduct) {
        // �ϴβ���Ϊ��Լ��Ӧʹ����൥Ŀ���������
        return std::make_pair(unary_operator_associate_type,
                              unary_operator_priority);
      } else {
        // �ϴβ���Ϊ���룬Ӧʹ��˫Ŀ���������
        return std::make_pair(binary_operator_associate_type,
                              binary_operator_priority);
      }
    } else {
      // ������˫Ŀ��������壬ֱ�ӷ���
      return std::make_pair(binary_operator_associate_type,
                            binary_operator_priority);
    }
  } else {
    // �����ڵ�Ŀ��������壬ֱ�ӷ���
    return std::make_pair(unary_operator_associate_type,
                          unary_operator_priority);
  }
}

}  // namespace frontend::generator::dfa_generator::nfa_generator