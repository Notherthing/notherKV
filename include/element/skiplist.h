#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

#include <atomic>
#include <assert.h>

#include "common/config.h"
#include "util/random.h"

namespace notherkv {

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
class Skiplist {
 private:
  struct Node;  // 对于跳表节点声明

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

 private:
  /**
   * @brief 判断两个Key值是否相等
   * @return true 相同  false 不相同
   */
  bool Equal(const _KeyType& a, const _KeyType& b) const;
  // 指定高度建立一个node
  Node* NewNode(const _KeyType& key, int32_t height);
  // 得到一个随机高度
  int32_t RandomHeight();
  // 得到当前最大高度
  int32_t GetMaxHeight();
  // 找到一个大于等于Key的最近节点
  Node* FindGreaterOrEqual(const _KeyType& key, Node** prev)


 private:
  _KeyComparator cmp_;
  _Allocator arena_;
  std::atomic<int32_t> cur_height_;
  Random rnd_;
  Node* head_ = nullptr;
};

template <typename _KeyType, typename _KeyComparator, typename _Allocator>
struct Skiplist<_KeyType, _KeyComparator, _Allocator>::Node {
  explicit Node(const _KeyType& key);

  Node* Next(int n);

  void SetNext(int n, Node* node);

  Node* NoBarrier_Next(int n);

  void NoBarrier_SetNext(int n, Node* node);

 public:
  const _KeyType key_;

 private:
  // Array of length equal to the node height.  next_[0] is lowest level link.
  std::atomic<Node*> next_[1];
};

}  // namespace notherkv
#endif