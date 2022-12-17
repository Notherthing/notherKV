#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

#include <assert.h>

#include <atomic>
#include <iostream>

#include "common/config.hpp"
#include "util/random.hpp"

namespace notherkv {
struct SkiplistOption {
  static constexpr int32_t kMaxHeight = MaxHeight;
  // 有多少概率被选中, 空间和时间的折中
  static constexpr unsigned int kBranching = Branching;
};

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
class Skiplist {
 private:
  struct Node;  // 对于跳表节点的前向声明

 public:
  Skiplist(_KeyComparator cmp);

  Skiplist(const Skiplist&) = delete;
  Skiplist& operator=(const Skiplist&) = delete;

  /**
   * @brief 用于插入一个key
   * @param key key的值
   */
  void Insert(const _KeyType& key);

  /**
   * @brief 查找跳表中是否已经有key
   * @param key key的值
   * @return true 有相同key的节点  false 无相同key的节点
   */
  bool Contains(const _KeyType& key) const;

  /**
   * @brief 判断两个Key值是否相等
   * @return true 相同  false 不相同
   */
  bool Equal(const _KeyType& a, const _KeyType& b) const;

 private:
  // 指定高度建立一个node
  Node* NewNode(const _KeyType& key, int32_t height);
  // 得到一个随机高度
  int32_t RandomHeight();
  // 得到当前最大高度
  int32_t GetMaxHeight();
  // 找到一个大于等于Key的最近节点
  Node* FindGreaterOrEqual(const _KeyType& key, Node** prev);
  // 找到小于key的最大key
  Node* FindLessThan(const _KeyType& key);
  // 找到最后一个节点的数据
  Node* FindLast();
  // 判断key是否在节点n之后
  bool KeyIsAfterNode(const _KeyType& key, Node* n) {
    return (nullptr != n && cmp_.Compare(n->key, key) < 0);
  }

 private:
  _KeyComparator cmp_;               // Key的比较器
  _Allocator arena_;                 // 内存管理器
  std::atomic<int32_t> cur_height_;  // 当前跳表层数
  Random rnd_;
  Node* head_ = nullptr;
};

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
struct Skiplist<_KeyType, _KeyComparator, _Allocator>::Node {
  explicit Node(const _KeyType& key) : key(k){};

  Node* Next(int n) { return next_[n].load(std::memory_order_acquire); };

  void SetNext(int n, Node* node) {
    next_[n].store(x, std::memory_order_release);
  };

  Node* NoBarrier_Next(int n) {
    return next_[n].load(std::memory_order_relaxed);
  };

  void NoBarrier_SetNext(int n, Node* node) {
    next_[n].store(x, std::memory_order_relaxed);
  };

 public:
  const _KeyType key_;  // 跳表节点的key值
 private:
  // Array of length equal to the node height.  next_[0] is lowest level link.
  std::atomic<Node*> next_[1];
};

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
Skiplist<_KeyType, _KeyComparator, _Allocator>::Skiplist(_KeyComparator cmp)
    : cmp_(cmp), cur_height_(1), head_(NewNode(0, SkiplistOption::kMaxHeight)) {
  for (int i = 0; i < SkiplistOption::kMaxHeight; i++) {
    head_->SetNext(i, nullptr);
  }
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
void Skiplist<_KeyType, _KeyComparator, _Allocator>::Insert(
    const _KeyType& key) {
  // 该对象记录的是要节点要插入位置的前一个对象，本质是链表的插入
  Node* prev[SkiplistOption::kMaxHeight] = {nullptr};
  // 在key的构造过程中，有一个持续递增的序号，因此理论上不会有重复的key
  Node* node = FindGreaterOrEqual(key, prev);
  if (nullptr != node) {
    if (Equal(key, node->key)) {
      std::cerr << "warn: " << key << " has existed" << std::endl;
      return;
    }
  }

  int32_t new_level = RandomHeight();
  int32_t cur_max_level = GetMaxHeight();
  if (new_level > cur_max_level) {
    // 因为skiplist存在多层，而刚开始的时候只是分配kMaxHeight个空间，每一层的next并没有真正使用
    for (int32_t index = cur_max_level; index < new_level; ++index) {
      prev[index] = head_;
    }
    // 更新当前的最大值
    cur_height_.store(new_level, std::memory_order_relaxed);
  }
  Node* new_node = NewNode(key, new_level);
  for (int32_t index = 0; index < new_level; ++index) {
    new_node->NoBarrier_SetNext(index, prev[index]->NoBarrier_Next(index));
    prev[index]->NoBarrier_SetNext(index, new_node);
  }
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
bool Skiplist<_KeyType, _KeyComparator, _Allocator>::Contains(
    const _KeyType& key) const {
  Node* node = FindGreaterOrEqual(key, nullptr);
  return nullptr != node && Equal(key, node->key);
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
bool Skiplist<_KeyType, _KeyComparator, _Allocator>::Equal(
    const _KeyType& a, const _KeyType& b) const {
  return cmp_.Compare(a, b) == 0;
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
typename Skiplist<_KeyType, _KeyComparator, _Allocator>::Node*
Skiplist<_KeyType, _KeyComparator, _Allocator>::NewNode(const _KeyType& key,
                                                        int32_t height) {
  char* node_memory = (char*)arena_.Allocate(
      sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
  // 定位new写法
  return new (node_memory) Node(key);
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
int32_t Skiplist<_KeyType, _KeyComparator, _Allocator>::RandomHeight() {
  int32_t height = 1;
  while (height < SkiplistOption::kMaxHeight &&
         ((rnd_.GetSimpleRandomNum() % SkiplistOption::kBranching) == 0)) {
    height++;
  }
  return height;
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
int32_t Skiplist<_KeyType, _KeyComparator, _Allocator>::GetMaxHeight() {
  return cur_height_.load(std::memory_order_relaxed);
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
typename Skiplist<_KeyType, _KeyComparator, _Allocator>::Node*
Skiplist<_KeyType, _KeyComparator, _Allocator>::FindGreaterOrEqual(
    const _KeyType& key, Node** prev) {
  Node* cur = head_;
  // 当前有效的最高层
  int32_t level = GetMaxHeight() - 1;
  Node* near_bigger_node = nullptr;
  while (true) {
    // 根据跳表原理，他是从最上层开始，向左或者向下遍历
    Node* next = cur->Next(level);
    // 说明key比next要大，直接往后next即可
    if (KeyIsAfterNode(key, next)) {
      cur = next;
    } else {
      if (prev != NULL) {
        prev[level] = cur;
      }
      if (level == 0) {
        return next;
      }
      // 进入下一层
      level--;
    }
  }
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
typename Skiplist<_KeyType, _KeyComparator, _Allocator>::Node*
Skiplist<_KeyType, _KeyComparator, _Allocator>::FindLessThan(
    const _KeyType& key) {
  Node* cur = head_;
  int32_t level = GetMaxHeight() - 1;
  while (true) {
    Node* next = cur->Next(level);
    int32_t cmp = (next == nullptr) ? 1 : comparator_.Compare(next->key, key);
    // 刚好next大于等于0
    if (cmp >= 0) {
      // 因为高度是随机生成的，在这里只有level=0才能确定到底是哪个node
      if (level == 0) {
        return cur;
      } else {
        level--;
      }
    } else {
      cur = next;
    }
  }
}

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
typename Skiplist<_KeyType, _KeyComparator, _Allocator>::Node*
Skiplist<_KeyType, _KeyComparator, _Allocator>::FindLast() {
  Node* cur = head_;
  static constexpr uint32_t kBaseLevel = 0;
  while (true) {
    Node* next = cur->Next(kBaseLevel);
    if (nullptr == next) {
      return cur;
    }
    cur = next;
  }
}
}  // namespace notherkv
#endif