#ifndef MYTINYSTL_LIST_H
#define MYTINYSTL_LIST_H

// 这个头文件包含了一个模板类
// list : 双向链表

// notes:
//
// 异常保证
// mystl::list<T>
// 满足基本异常保证，部分函数无异常保证，并对以下等函数做强异常安全保证
// *emplace_front
// *emplace_back
// *emplace
// *push_front
// *push_back
// *insert
#include <initializer_list>
#include <type_traits>

#include "algobase.h"
#include "allocator.h"
#include "construct.h"
#include "exceptdef.h"
#include "functional.h"
#include "iterator.h"
#include "memory.h"
#include "util.h"

namespace mystl {
template <class T> struct list_node_base;
template <class T> struct list_node;

template <class T> struct node_traits {
  typedef list_node_base<T> *base_ptr;
  typedef list_node<T> *node_ptr;
};

// list的节点结构
template <class T> struct list_node_base {
  typedef typename node_traits<T>::base_ptr base_ptr;
  typedef typename node_traits<T>::node_ptr node_ptr;

  base_ptr prev; // 前一节点
  base_ptr next; // 后一节点

  list_node_base() = default;

  node_ptr as_node() { return static_cast<node_ptr>(self()); }

  void unlink() { prev = next = self(); }

  base_ptr self() { return static_cast<base_ptr>(&*this); }
};

template <class T> struct list_node : public list_node_base<T> {
  typedef typename node_traits<T>::base_ptr base_ptr;
  typedef typename node_traits<T>::node_ptr node_ptr;

  T value; // 作用域

  list_node() = default;
  list_node(const T &v) : value(v) {}
  list_node(T &&v) : value(mystl::move(v)) {}

  base_ptr as_base() { return static_cast<base_ptr>(self()); }

  node_ptr self() { return static_cast<node_ptr>(&*this); }
};

// list的迭代器设计
template <class T>
struct list_iterator
    : public mystl::iterator<mystl::bidirectional_iterator_tag, T> {
  typedef T value_type;
  typedef T *pointer;
  typedef T &reference;
  typedef typename node_traits<T>::base_ptr base_ptr;
  typedef typename node_traits<T>::node_ptr node_ptr;
  typedef list_iterator<T> self;

  base_ptr node_; // 指向当前节点

  // 构造函数
  list_iterator() = default;
  list_iterator(base_ptr x) : node_(x) {}
  list_iterator(node_ptr x) : node_(x->as_base()) {}
  list_iterator(const list_iterator &rhs) : node_(rhs.node_) {}

  // 重载操作符
  reference operator*() const { return node_->as_node()->value; }
  pointer operator->() const { return &(operator*()); }

  self &operator++() {
    MYSTL_DEBUG(node_ != nullptr);
    node_ = node_->next;
    return *this;
  }

  self operator++(int) {
    self tmp = *this;
    ++*this;
    return tmp;
  }

  self &operator--() {
    MYSTL_DEBUG(node_ != nullptr);
    node_ = node_->prev;
    return *this;
  }

  self operator--(int) {
    self tmp = *this;
    --*this;
    return tmp;
  }

  // 重载比较操作符
  bool operator==(const self &rhs) const { return node_ == rhs.node_; }
  bool operator!=(const self &rhs) const { return node_ != rhs.node_; }
};

template <class T>
struct list_const_iterator : public iterator<bidirectional_iterator_tag, T> {
  typedef T value_type;
  typedef const T *pointer;
  typedef const T &reference;
  typedef typename node_traits<T>::base_ptr base_ptr;
  typedef typename node_traits<T>::node_ptr node_ptr;
  typedef list_const_iterator<T> self;

  base_ptr node_;

  list_const_iterator() = default;
  list_const_iterator(base_ptr x) : node_(x) {}
  list_const_iterator(node_ptr x) : node_(x->as_base()) {}
  list_const_iterator(const list_iterator<T> &rhs) : node_(rhs.node_) {}
  list_const_iterator(const list_const_iterator &rhs) : node_(rhs.node_) {}

  reference operator*() const { return node_->as_node()->value; }
  pointer operator->() const { return &(operator*()); }

  self &operator++() {
    MYSTL_DEBUG(node_ != nullptr);
    node_ = node_->next;
    return *this;
  }
  self operator++(int) {
    self tmp = *this;
    ++*this;
    return tmp;
  }
  self &operator--() {
    MYSTL_DEBUG(node_ != nullptr);
    node_ = node_->prev;
    return *this;
  }
  self operator--(int) {
    self tmp = *this;
    --*this;
    return tmp;
  }

  // 重载比较操作符
  bool operator==(const self &rhs) const { return node_ == rhs.node_; }
  bool operator!=(const self &rhs) const { return node_ != rhs.node_; }
};

// 模板类:list
// 模板参数T代表数据类型
template <class T> class list {
public:
  // list的嵌套型别定义
  typedef mystl::allocator<T> allocator_type;
  typedef mystl::allocator<T> data_allocator;
  typedef mystl::allocator<list_node_base<T>> base_allocator;
  typedef mystl::allocator<list_node<T>> node_allocator;

  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;

  typedef list_iterator<T> iterator;
  typedef list_const_iterator<T> const_iterator;
  typedef mystl::reverse_iterator<iterator> reverse_iterator;
  typedef mystl::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef typename node_traits<T>::base_ptr base_ptr;
  typedef typename node_traits<T>::node_ptr node_ptr;

  allocator_type get_allocator() { return node_allocator(); }

private:
  base_ptr node_;  // 指向尾节点(哨兵节点)
  size_type size_; // 大小

public:
  // 构造，复制，移动，析构函数
  list() { fill_init(0, value_type()); }

  explicit list(size_type n) { fill_init(n, value_type()); }

  list(size_type n, const T &value) { fill_init(n, value); }

  template <class Iter,
            typename std::enable_if<mystl::is_input_iterator<Iter>::value,
                                    int>::type = 1>
  list(Iter first, Iter last) {
    copy_init(first, last);
  }

  list(std::initializer_list<T> ilist) {
    copy_init(ilist.begin(), ilist.end());
  }

  list(const list &rhs) { copy_init(rhs.cbegin(), rhs.cend()); }

  list(list &&rhs) noexcept : node_(rhs.node_), size_(rhs.size_) {
    rhs.node_ = nullptr;
    rhs.size_ = 0;
  }

  list &operator=(const list &rhs) {
    if (this != &rhs) {
      assign(rhs.begin(), rhs.end());
    }
    return *this;
  }

  list &operator=(list &&rhs) {
    clear();
    splice(end(), rhs);
    return *this;
  }

  list &operator=(std::initializer_list<T> ilist) {
    list tmp(ilist.begin(), ilist.end());
    swap(tmp);
    return *this;
  }

  ~list() {
    if (node_) {
      clear();
      base_allocator::deallocate(node_);
      node_ = nullptr;
      size_ = 0;
    }
  }

public:
  // 迭代器相关操作
  iterator begin() noexcept { return node_->next; }
  const_iterator begin() const noexcept { return node_->next; }
  iterator end() noexcept { return node_; }
  const_iterator end() const noexcept { return node_; }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }

  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }

  // 容量相关操作
  bool empty() const noexcept { return node_->next == node_; }

  size_type size() const noexcept { return size_; }

  size_type max_size() const noexcept { return static_cast<size_type>(-1); }

  // 访问元素相关操作
  reference front() {
    MYSTL_DEBUG(!empty());
    return *begin();
  }

  const_reference front() const {
    MYSTL_DEBUG(!empty());
    return *begin();
  }

  reference back() {
    MYSTL_DEBUG(!empty());
    return *(--end());
  }

  const_reference back() const {
    MYSTL_DEBUG(!empty());
    return *(--end());
  }

  // 调整容器相关操作

  // assign

  void assign(size_type n, const value_type &value) { fill_assign(n, value); }

  template <class Iter, typename std::enable_if<
                            mystl::is_input_iterator<Iter>::value, int> = 0>
  void assign(Iter first, Iter last) {
    copy_assign(first, last);
  }

  void assign(std::initializer_list<T> ilist) {
    copy_assign(ilist.begin(), ilist.end());
  }

  // emplace_front / emplace_back / emplace

  template <class... Args> void emplace_front(Args &&...args) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(mystl::forward<Args>(args)...);
    link_nodes_at_front(link_node->as_base(), link_node->as_base());
    ++size_;
  }

  template <class... Args> void emplace_back(Args &&...args) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(mystl::forward<Args>(args)...);
    link_nodes_at_back(link_node->as_base(), link_node->as_base());
    ++size_;
  }

  template <class... Args>
  iterator emplace(const_iterator pos, Args &&...args) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(mystl::forward<Args>(args)...);
    link_nodes(pos.node_, link_node->as_base(), link_node->as_base());
    ++size_;
    return iterator(link_node);
  }

  // insert

  iterator insert(const_iterator pos, const value_type &value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(value);
    ++size_;
    return link_iter_node(pos, link_node->as_base());
  }

  iterator insert(const_iterator pos, value_type &&value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(mystl::move(value));
    ++size_;
    return link_iter_node(pos, link_node->as_base());
  }

  iterator insert(const_iterator pos, size_type n, const value_type &value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    return fill_insert(pos, n, value);
  }

  template <class Iter,
            typename std::enable_if<mystl::is_input_iterator<Iter>::value,
                                    int>::type = 0>
  iterator insert(const_iterator pos, Iter first, Iter last) {
    size_type n = mystl::distance(first, last);
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    return copy_insert(pos, n, first);
  }

  // push_front / push_back

  void push_front(const value_type &value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(value);
    link_nodes_at_front(link_node->as_base(), link_node->as_base());
    ++size_;
  }

  void push_front(value_type &&value) { emplace_front(mystl::move(value)); }

  void push_back(const value_type &value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(value);
    link_nodes_at_back(link_node->as_base(), link_node->as_base());
    ++size_;
  }

  void push_back(value_type &&value) { emplace_back(mystl::move(value)); }

  // pop_front / pop_back

  void pop_front() {
    MYSTL_DEBUG(!empty());
    auto n = node_->next;
    unlink_nodes(n, n);
    destroy_node(n->as_node());
    --size_;
  }

  void pop_back() {
    MYSTL_DEBUG(!empty());
    auto n = node_->prev;
    unlink_nodes(n, n);
    destroy_node(n->as_node());
    --size_;
  }

  // erase / clear
  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);

  void clear();

  // resize
  void resize(size_type new_size) { resize(new_size, value_type()); }
  void resize(size_type new_size, const value_type &value);

  void swap(list &rhs) {
    mystl::swap(node_, rhs.node_);
    mystl::swap(size_, rhs.size_);
  }

  // list 相关操作

  void splice(const_iterator pos, list &other);
  void splice(const_iterator pos, list &other, const_iterator it);
  void splice(const_iterator pos, list &other, const_iterator first,
              const_iterator last);

  void remove(const value_type &value) {
    remove_if([&](const value_type &v) { return v == value; });
  }
  template <class UnaryPredicate> void remove_if(UnaryPredicate pred);

  void unique() { unique(mystl::equal_to<T>()); }
  template <class BinaryPredicate> void unique(BinaryPredicate pred);

  void merge(list &x) { merge(x, mystl::less<T>()); }
  template <class Compared> void merge(list &x, Compared comp);

  void sort() { list_sort(begin(), end(), size(), mystl::less<T>()); }
  template <class Compared> void sort(Compared comp) {
    list_sort(begin(), end(), size(), comp);
  }

  void reverse();

private:
  // helper functions

  // create/destroy node
  template <class... Args> node_ptr create_node(Args &&...args);
  void destroy_node(node_ptr p);

  // initialize
  void fill_init(size_type n, const value_type &value);
  template <class Iter> void copy_init(Iter first, Iter last);

  // link/unlink
  iterator link_iter_node(const_iterator pos, base_ptr link_node);
  void link_nodes(base_ptr p, base_ptr first, base_ptr last);
  void link_nodes_at_front(base_ptr first, base_ptr last);
  void link_nodes_at_back(base_ptr first, base_ptr last);
  void unlink_nodes(base_ptr first, base_ptr last);

  // assign
  void fill_assign(size_type n, const value_type &value);
  template <class Iter> void copy_assign(Iter first, Iter last);

  // insert
  iterator fill_insert(const_iterator pos, size_type n,
                       const value_type &value);
  template <class Iter>
  iterator copy_insert(const_iterator pos, size_type n, Iter first);

  // sort
  template <class Compared>
  iterator list_sort(iterator first, iterator last, size_type n, Compared comp);
};
/*****************************************************************************************/

// 删除pos处的元素
template <class T>
typename list<T>::iterator list<T>::erase(const_iterator pos) {
  MYSTL_DEBUG(pos != cend());
  auto n = pos.node_;
  auto next = n->next;
  unlink_nodes(n, n);
  destroy(n->as_node());
  --size_;
  return iterator(next);
}

// 删除[first,last)内的元素
template <class T>
typename list<T>::iterator list<T>::erase(const_iterator first,
                                          const_iterator last) {
  if (first != last) {
    unlink_nodes(first.node_, last.node_->prev);
    while (first != last) {
      auto cur = first.node_;
      ++first;
      destroy_node(cur->as_node());
      --size_;
    }
  }
  return iterator(last.node_);
}

// 清空list
template <class T> void list<T>::clear() {
  if (size_ != 0) {
    auto cur = node_->next;
    for (base_ptr next = cur->next; cur != node_;
         cur = next, next = cur->next) {
      destroy_node(cur->as_node());
    }
    node_->unlink();
    size_ = 0;
  }
}

// 重置容器大小
template <class T>
void list<T>::resize(size_type new_size, const value_type &value) {
  auto i = begin();
  size_type len = 0;
  while (i != end() && len < new_size) {
    ++i;
    ++len;
  }
  if (len == new_size) {
    erase(i, node_);
  } else {
    insert(node_, new_size - len, value);
  }
}

// 将 list x 接合于pos之前
template <class T> void list<T>::splice(const_iterator pos, list &x) {
  MYSTL_DEBUG(this != &x);
  if (!x.empty()) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - x.size_,
                          "list<T>'s size too big");

    auto f = x.node_->next;
    auto l = x.node_->prev;

    x.unlink_nodes(f, l);
    link_nodes(pos.node_, f, l);

    size_ += x.size_;
    x.size_ = 0;
  }
}

// 将 it 所指的节点结合于pos之前
template <class T>
void list<T>::splice(const_iterator pos, list &x, const_iterator it) {
  if (pos.node_ != it.node_ && pos.node_ != it.node_->next) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");

    auto f = it.node_;
    x.unlink_nodes(f, f);
    link_nodes(pos.node_, f, f);

    ++size_;
    --size_;
  }
}

// 将list x 的[first,last)内的节点结合于pos之前
template <class T>
void list<T>::splice(const_iterator pos, list &x, const_iterator first,
                     const_iterator last) {
  if (first != last && this != &x) {
    size_type n = mystl::distance(first, last);
    THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
    auto f = first.node_;
    auto l = last.node_->prev;

    x.unlink_nodes(f, l);
    link_nodes(pos.node_, f, l);

    size_ += n;
    x.size_ -= n;
  }
}

// 将令一元操作pred为true的所有元素移除
template <class T>
template <class UnaryPredicate>
void list<T>::remove_if(UnaryPredicate pred) {
  auto f = begin();
  auto l = end();
  for (auto next = f; f != l; f = next) {
    ++next;
    if (pred(*f)) {
      erase(f);
    }
  }
}

// 移除list中满足pred为true的重复元素
template <class T>
template <class BinaryPredicate>
void list<T>::unique(BinaryPredicate pred) {
  auto i = begin();
  auto e = end();
  auto j = i;
  ++j;
  while (j != e) {
    if (pred(*i, *j)) {
      erase(j);
    } else {
      i = j;
    }
    j = i;
    ++j;
  }
}

// 与另一个 list 合并,按照comp为true的顺序
template <class T>
template <class Compared>
void list<T>::merge(list &x, Compared comp) {
  if (this != &x) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - x.size_,
                          "list<T>'s size too big");

    auto first1 = begin();
    auto last1 = end();
    auto first2 = x.begin();
    auto last2 = x.end();

    while (first1 != last1 && first2 != last2) {
      if (comp(*first2, *first1)) {
        // 使comp为true的一段区间
        auto next = first2;
        ++next;
        for (; next != last2 && comp(*next, *first1); ++next) {
          ;
        }
        auto first = first2.node_;
        auto last = next.node_->prev;
        first2 = next;

        // link node
        x.unlink_nodes(first, last);
        link_nodes(first1.node_, first, last);
        ++first1;
      } else {
        ++first1;
      }
    }
    // 连接剩余部分
    if (first2 != last2) {
      auto first = first2.node_;
      auto last = last2.node_->prev;
      x.unlink_nodes(first, last);
      link_nodes(last1.node_, first, last);
    }
    size_ += x.size_;
    x.size_ = 0;
  }
}

// 将list反转
template <class T> void list<T>::reverse() {
  if (size_ <= 1) {
    return;
  }
  auto i = begin();
  auto e = end();
  while (i.node_ != e.node_) {
    mystl::swap(i.node_->prev, i.node_->next);
    i.node_ = i.node_->prev;
  }
  mystl::swap(e.node_->prev, e.node_->next);
}
/*****************************************************************************************/
// helper function

// 创建节点
template <class T>
template <class... Args>
typename list<T>::node_ptr list<T>::create_node(Args &&...args) {
  node_ptr p = node_allocator::allocate(1);
  try {
    data_allocator::construct(mystl::address_of(p->value),
                              mystl::forward<Args>(args)...);
    p->prev = nullptr;
    p->next = nullptr;
  } catch (...) {
    node_allocator::deallocate(p);
    throw;
  }
  return p;
}

// 销毁节点
template <class T> void list<T>::destroy_node(node_ptr p) {
  data_allocator::destroy(mystl::address_of(p->value));
  node_allocator::deallocate(p);
}

// 用n个元素初始化容器
template <class T>
void list<T>::fill_init(size_type n, const value_type &value) {
  node_ = base_allocator::allocate(1);
  node_->unlink();
  size_ = n;
  try {
    for (; n > 0; --n) {
      auto node = create_node(value);
      link_nodes_at_back(node->as_base(), node->as_base());
    }
  } catch (...) {
    clear();
    base_allocator::deallocate(node_);
    node_ = nullptr;
    throw;
  }
}

// 以[first,last)初始化容器
template <class T>
template <class Iter>
void list<T>::copy_init(Iter first, Iter last) {
  node_ = base_allocator::allocate(1);
  node_->unlink();
  size_type n = mystl::distance(first, last);
  size_ = n;
  try {
    for (; n > 0; --n, ++first) {
      auto node = create_node(*first);
      link_nodes_at_back(node->as_base(), node->as_base());
    }
  } catch (...) {
    clear();
    base_allocator::deallocate(node_);
    node_ = nullptr;
    throw;
  }
}

// 在pos处连接一个节点
template <class T>
typename list<T>::iterator list<T>::link_iter_node(const_iterator pos,
                                                   base_ptr link_node) {
  if (pos == node_->next) {
    link_nodes_at_front(link_node, link_node);
  } else if (pos == node_) {
    link_nodes_at_back(link_node, link_node);
  } else {
    link_nodes(pos.node_, link_node, link_node);
  }
  return iterator(link_node);
}

// 在pos处连接[first,last)的节点
template <class T>
void list<T>::link_nodes(base_ptr pos, base_ptr first, base_ptr last) {
  pos->prev->next = first;
  first->prev = pos->prev;
  pos->prev = last;
  last->next = pos;
}

// 在头部连接[first,last)的节点
template <class T>
void list<T>::link_nodes_at_front(base_ptr first, base_ptr last) {
  first->prev = node_;
  last->next = node_->next;
  last->next->prev = last;
  node_->next = first;
}

// 在尾部连接[first,last)的节点
template <class T>
void list<T>::link_nodes_at_back(base_ptr first, base_ptr last) {
  last->next = node_;
  first->prev = node_->prev;
  first->prev->next = first;
  node_->prev = last;
}

// 容器与[first,last)节点断开连接
template <class T> void list<T>::unlink_nodes(base_ptr first, base_ptr last) {
  first->prev->next = last->next;
  last->next->prev = first->prev;
}

// 用n个元素为容器赋值
template <class T>
void list<T>::fill_assign(size_type n, const value_type &value) {
  auto i = begin();
  auto e = end();
  for (; n > 0 && i != e; --n, ++i) {
    *i = value;
  }
  if (n > 0) {
    insert(e, n, value);
  } else {
    erase(i, e);
  }
}

// 复制[first,last)为容器赋值
template <class T>
template <class Iter>
void list<T>::copy_assign(Iter first, Iter last) {
  auto f1 = begin();
  auto l1 = end();
  for (; f1 != l1 && first != last; ++f1, ++first) {
    *f1 = *first;
  }
  if (first == last) {
    erase(f1, l1);
  } else {
    insert(l1, first, last);
  }
}

// 在pos处插入n个元素
template <class T>
typename list<T>::iterator list<T>::fill_insert(const_iterator pos, size_type n,
                                                const value_type &value) {
  iterator r(pos.node_);
  if (n != 0) {
    const auto add_size = n;
    auto node = create_node(value);
    node->prev = nullptr;
    r = iterator(node);
    iterator end = r;
    try {
      // 前面已经创建了一个节点，还需n - 1个
      for (--n; n > 0; --n, ++end) {
        auto next = create_node(value);
        end.node_->next = next->as_base(); // link node
        next->prev = end.node_;
      }
      size_ += add_size;
    } catch (...) {
      auto enode = end.node_;
      while (true) {
        auto prev = enode->prev;
        destroy_node(enode->as_node());
        if (prev == nullptr) {
          break;
        }
        enode = prev;
      }
      throw;
    }
    link_nodes(pos.node_, r.node_, end.node_);
  }
  return r;
}

// 在pos处插入[first,last)的元素
template <class T>
template <class Iter>
typename list<T>::iterator list<T>::copy_insert(const_iterator pos, size_type n,
                                                Iter first) {
  iterator r(pos.node_);
  if (n != 0) {
    const auto add_size = n;
    auto node = create_node(*first);
    node->prev = nullptr;
    r = iterator(node);
    iterator end = r;
    try {
      for (--n, ++first; n > 0; --n, ++first, ++end) {
        auto next = create_node(*first);
        end.node_->next = next->as_base(); // link node
        next->prev = end.node_;
      }
      size_ += add_size;
    } catch (...) {
      auto enode = end.node_;
      while (true) {
        auto prev = enode->prev;
        destroy_node(enode->as_node());
        if (prev == nullptr) {
          break;
        }
        enode = prev;
      }
      throw;
    }
    link_nodes(pos.node_, r.node_, end.node_);
  }
  return r;
}

// 对list进行归并排序,并返回一个迭代器指向区间最小元素的位置
template <class T>
template <class Compared>
typename list<T>::iterator list<T>::list_sort(iterator first1, iterator last2,
                                              size_type n, Compared comp) {
  if (n < 2) {
    return first1;
  }
  if (n == 2) {
    if (comp(*--last2, *first1)) {
      auto ln = last2.node_;
      unlink_nodes(ln, ln);
      link_nodes(first1.node_, ln, ln);
      return last2;
    }
    return first1;
  }

  auto n2 = n / 2;
  auto last1 = first1;
  mystl::advance(last1, n2);
  auto result = first1 = list_sort(first1, last1, n2, comp); // 前半段的最小位置
  auto first2 = last1 =
      list_sort(last1, last2, n - n2, comp); // 后半段的最小位置

  // 把较小的一段区间移到前面
  if (comp(*first2, *first1)) {
    auto m = first2;
    ++m;
    for (; m != last2 && comp(*m, *first1); ++m) {
      ;
    }
    auto f = first2.node_;
    auto l = m.node_->prev;
    result = first2;
    last1 = first2 = m;
    unlink_nodes(f, l);
    m = first1;
    ++m;
    link_nodes(first1.node_, f, l);
    first1 = m;
  } else {
    ++first1;
  }

  // 合并两段有序区间
  while (first1 != last1 && first2 != last2) {
    if (comp(*first2, *first1)) {
      auto m = first2;
      ++m;
      for (; m != last2 && comp(*m, *first1); ++m) {
        ;
      }
      auto f = first2.node_;
      auto l = m.node_->prev;
      if (last1 == first2) {
        last1 = m;
      }
      first2 = m;
      unlink_nodes(f, l);
      m = first1;
      ++m;
      link_nodes(first1.node_, f, l);
      first1 = m;
    } else {
      ++first1;
    }
  }
  return result;
}

// 重载比较操作符
template <class T> bool operator==(const list<T> &lhs, const list<T> &rhs) {
  auto first1 = lhs.begin();
  auto last1 = lhs.end();
  auto first2 = rhs.begin();
  auto last2 = rhs.end();
  for (; first1 != last1 && first2 != last2 && *first1 == *first2;
       ++first1, ++first2) {
    ;
  }
  return first1 == last1 && first2 == last2;
}

template <class T> bool operator<(const list<T> &lhs, const list<T> &rhs) {
  return mystl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <class T> bool operator!=(const list<T> &lhs, const list<T> &rhs) {
  return !(lhs == rhs);
}

template <class T> bool operator>(const list<T> &lhs, const list<T> &rhs) {
  return rhs < lhs;
}

template <class T> bool operator<=(const list<T> &lhs, const list<T> &rhs) {
  return !(rhs < lhs);
}

template <class T> bool operator>=(const list<T> &lhs, const list<T> &rhs) {
  return !(lhs < rhs);
}

// 重载mystl的swap
template <class T> void swap(list<T> &lhs, list<T> &rhs) { lhs.swap(rhs); }
} // namespace mystl
#endif // !MYTINYSTL_LIST_H