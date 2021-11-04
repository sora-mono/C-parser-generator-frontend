#include "nfa_generator.h"

#include <format>
#include <queue>

namespace frontend::generator::dfa_generator::nfa_generator {

NfaGenerator::NfaNodeId NfaGenerator::NfaNode::GetForwardNodesId(
    char c_transfer) {
  auto iter = nodes_forward_.find(c_transfer);
  if (iter == nodes_forward_.end()) {
    return NfaNodeId::InvalidId();
  } else {
    return iter->second;
  }
}

inline void NfaGenerator::NfaNode::SetConditionTransfer(
    char c_condition, NfaGenerator::NfaNodeId node_id) {
  auto [iter, inserted] = nodes_forward_.emplace(c_condition, node_id);
  assert(inserted || iter->second == node_id);
}

inline void NfaGenerator::NfaNode::AddNoconditionTransfer(NfaNodeId node_id) {
  auto [iter, inserted] = conditionless_transfer_nodes_id.insert(node_id);
  assert(inserted);
}

inline void NfaGenerator::NfaNode::RemoveConditionalTransfer(char c_treasfer) {
  auto iter = nodes_forward_.find(c_treasfer);
  if (iter != nodes_forward_.end()) {
    nodes_forward_.erase(iter);
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
  if (nodes_forward_.size() != 0 && node_src.nodes_forward_.size() != 0) {
    bool CanBeSourceInMerge = true;
    for (auto& p : node_src.nodes_forward_) {
      auto iter = nodes_forward_.find(p.first);
      if (iter != nodes_forward_.end() && iter->second != p.second) {
        CanBeSourceInMerge = false;
        break;
      }
    }
    if (!CanBeSourceInMerge) {
      return false;
    }
  }
  nodes_forward_.merge(node_src.nodes_forward_);
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
std::pair<NfaGenerator::NfaNodeId, NfaGenerator::NfaNodeId>
NfaGenerator::RegexConstruct(const TailNodeData& tail_node_data,
                             const std::string& raw_regex_string,
                             size_t&& next_character_index,
                             const bool add_to_nfa_head) {
  NfaNodeId head_id = node_manager_.EmplaceObject();
  NfaNodeId tail_id = head_id;
  NfaNodeId pre_tail_id = head_id;
  if (next_character_index >= raw_regex_string.size()) [[unlikely]] {
    return std::make_pair(head_id, tail_id);
  }
  char c_now;
  while (next_character_index < raw_regex_string.size()) {
    c_now = raw_regex_string[next_character_index];
    ++next_character_index;
    switch (c_now) {
      case '[': {
        auto [temp_head_id, temp_tail_id] =
            CreateSwitchTree(raw_regex_string, &next_character_index);
        if (!(temp_head_id.IsValid() && temp_tail_id.IsValid())) [[unlikely]] {
          std::cerr << std::format("�Ƿ����� {:}\n{: >{}}", raw_regex_string,
                                   '^', next_character_index + 9)
                    << std::endl;
          exit(-1);
        }
        GetNfaNode(tail_id).AddNoconditionTransfer(temp_head_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
      } break;
      case ']':
        // regex_construct������Ӧ�ô���]�ַ���Ӧ����create_switch_tree����
        assert(false);
        break;
      case '(': {
        auto [temp_head_id, temp_tail_id] =
            RegexConstruct(NotTailNodeTag, raw_regex_string,
                           std::move(next_character_index), false);
        if (!(temp_head_id.IsValid() && temp_tail_id.IsValid())) [[unlikely]] {
          std::cerr << std::format("�Ƿ����� {:}\n{: >{}}", raw_regex_string,
                                   '^', next_character_index + 9)
                    << std::endl;
          exit(-1);
        }
        GetNfaNode(tail_id).AddNoconditionTransfer(temp_head_id);
        pre_tail_id = tail_id;
        tail_id = temp_tail_id;
      } break;
      case ')':
        return std::make_pair(head_id, tail_id);
        break;
      case '+':  // �Ե����ַ�����Ͻṹ����Ч
        GetNfaNode(tail_id).AddNoconditionTransfer(pre_tail_id);
        break;
      case '*':  // �Ե����ַ�����Ͻṹ����Ч
        GetNfaNode(tail_id).AddNoconditionTransfer(pre_tail_id);
        GetNfaNode(pre_tail_id).AddNoconditionTransfer(tail_id);
        break;
      case '?':  // �Ե����ַ�����Ͻṹ����Ч
        GetNfaNode(pre_tail_id).AddNoconditionTransfer(tail_id);
        break;
      case '\\':  // ���Ե����ַ���Ч
        if (next_character_index >= raw_regex_string.size()) [[unlikely]] {
          std::cerr << std::format("�Ƿ����� {:}\n{: >{}}", raw_regex_string,
                                   '^', next_character_index + 9)
                    << std::endl;
          exit(-1);
        }
        c_now = raw_regex_string[next_character_index];
        ++next_character_index;
        pre_tail_id = tail_id;
        tail_id = node_manager_.EmplaceObject();
        GetNfaNode(pre_tail_id).SetConditionTransfer(c_now, tail_id);
        break;
      case '.':  // ���Ե����ַ���Ч
        pre_tail_id = tail_id;
        tail_id = node_manager_.EmplaceObject();
        for (char transform_char = CHAR_MIN; transform_char != CHAR_MAX;
             ++transform_char) {
          GetNfaNode(pre_tail_id).SetConditionTransfer(transform_char, tail_id);
        }
        GetNfaNode(pre_tail_id).SetConditionTransfer(CHAR_MAX, tail_id);
        break;
      default:
        pre_tail_id = tail_id;
        tail_id = node_manager_.EmplaceObject();
        GetNfaNode(pre_tail_id).SetConditionTransfer(c_now, tail_id);
        break;
    }
  }
  if (head_id != tail_id) [[likely]] {
    if (add_to_nfa_head) {
      GetNfaNode(head_node_id_).AddNoconditionTransfer(head_id);
      AddTailNode(tail_id, tail_node_data);
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
NfaGenerator::CreateSwitchTree(const std::string& raw_regex_string,
                               size_t* next_character_index) {
  NfaNodeId head_id = node_manager_.EmplaceObject();
  NfaNodeId tail_id = node_manager_.EmplaceObject();
  NfaNode& head_node = GetNfaNode(head_id);
  // ��ʼ���ɲ���']'��ֵ�Ϳ��ԣ��Ӷ����Խ���ѭ��
  char character_pre;
  char character_now = '[';
  while (character_now != ']') {
    if (*next_character_index >= raw_regex_string.size()) [[unlikely]] {
      std::cerr << std::format("�Ƿ����� {:}\n{: >{}}", raw_regex_string, '^',
                               *next_character_index + 9)
                << std::endl;
      exit(-1);
    }
    character_pre = character_now;
    character_now = raw_regex_string[*next_character_index];
    ++*next_character_index;
    switch (character_now) {
      case '-': {
        // ������һ�˵��ַ�
        if (*next_character_index >= raw_regex_string.size()) [[unlikely]] {
          std::cerr << std::format("�Ƿ����� {:}\n{: >{}}", raw_regex_string,
                                   '^', *next_character_index + 9)
                    << std::endl;
          exit(-1);
        }
        character_now = raw_regex_string[*next_character_index];
        ++*next_character_index;
        if (character_now == ']') [[unlikely]] {
          // [+-]����������ʽ
          break;
        }
        char bigger_character = std::max(character_now, character_pre);
        for (char smaller_character = std::min(character_pre, character_now);
             smaller_character < bigger_character; smaller_character++) {
          head_node.SetConditionTransfer(smaller_character, tail_id);
        }
        // ��ֹbigger_character == CHAR_MAX���»ػ����޷��˳�����ѭ��
        head_node.SetConditionTransfer(bigger_character, tail_id);
      } break;
      case '\\':
        if (*next_character_index >= raw_regex_string.size()) [[unlikely]] {
          std::cerr << std::format("�Ƿ����� {:}\n{: >{}}", raw_regex_string,
                                   '^', *next_character_index + 9)
                    << std::endl;
          exit(-1);
        }
        character_now = raw_regex_string[*next_character_index];
        ++*next_character_index;
        head_node.SetConditionTransfer(character_now, tail_id);
        break;
      case ']':
        break;
      default:
        head_node.SetConditionTransfer(character_now, tail_id);
        break;
    }
  }
  if (head_node.GetConditionalTransfers().empty()) [[unlikely]] {
    node_manager_.RemoveObject(head_id);
    std::cerr << std::format("�Ƿ����� {:}\n{: >{}}\n[]��Ϊ��",
                             raw_regex_string, '^', *next_character_index + 9)
              << std::endl;
    exit(-1);
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

}  // namespace frontend::generator::dfa_generator::nfa_generator