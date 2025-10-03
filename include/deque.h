#ifndef MYTINYSTL_DUQUE_H
#define MYTINYSTL_DUQUE_H

// 这个头文件包含了一个模板类 deque
// deque: 双端队列

// notes:
//
// 异常保证:
// mystl::deque<T>
// 满足基本的异常保证，部分函数无异常保证，并对以下等函数做强异常保证:
// * emplace_front
// * emplace_back
// * emplace
// * push_front
// * push_back
// * insert

#include <cstddef>
#include <initializer_list>

#include "allocator.h"
#include "exceptdef.h"
#include "iterator.h"
#include "memory.h"
#include "util.h"

namespace mystl {

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

// deque map 初始化的大小
#ifndef DEQUE_MAP_INIT_SIZE
#define DEQUE_MAP_INIT_SIZE 8
#endif

template <class T> struct deque_buf_size {
  static constexpr size_t value = sizeof(T) < 256 ? 4096 / sizeof(T) : 16;
};

// deque 的迭代器设计
template <class T, class Ref, class Ptr>
struct deque_iterator : public iterator<random_access_iterator_tag, T> {
  typedef deque_iterator<T, T &, T *> iterator;
  typedef deque_iterator<T, const T &, const T *> const_iterator;
  typedef deque_iterator self;

  typedef T value_type;
  typedef Ptr pointer;
  typedef Ref reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T *value_pointer;
  typedef T **map_pointer;

  static const size_type buffer_size = deque_buf_size<T>::value;

  // 迭代器所含成员数据
  value_pointer cur;   // 指向所在缓冲区的当前元素
  value_pointer first; // 指向所在缓冲区的头部
  value_pointer last;  // 指向所在缓存区的尾部
  map_pointer node;    // 缓冲区所在节点

  // 构造，复制，析构函数
  deque_iterator() noexcept
      : cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}

  deque_iterator(value_pointer v, map_pointer n)
      : cur(v), first(*n), last(*n + buffer_size), node(n) {}

  deque_iterator(const iterator &rhs)
      : cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}

  deque_iterator(iterator &&rhs) noexcept
      : cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {
    rhs.cur = nullptr;
    rhs.first = nullptr;
    rhs.last = nullptr;
    rhs.node = nullptr;
  }

  deque_iterator(const const_iterator &rhs)
      : cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}

  self &operator=(const iterator &rhs) {
    if (this != &rhs) {
      cur = rhs.cur;
      first = rhs.first;
      last = rhs.last;
      node = rhs.node;
    }
    return *this;
  }

  // 转到另一个缓冲区
  void set_node(map_pointer new_node) {
    node = new_node;
    first = *new_node;
    last = *new_node + buffer_size;
  }

  // 重载运算符
  reference operator*() const { return *cur; }
  pointer operator->() const { return cur; }

  difference_type operator-(const self &x) const {
    return static_cast<difference_type>(buffer_size) * (node - x.node) +
           (cur - first) - (x.cur - x.first);
  }

  self &operator++() {
    ++cur;
    if (cur == last) {
      // 如果到达缓冲区的尾
      set_node(node + 1);
      cur = first;
    }
    return *this;
  }
  self operator++(int) {
    self tmp = *this;
    ++*this;
    return tmp;
  }

  self &operator--() {
    if (cur == first) {
      // 如果到达缓冲区的头
      set_node(node - 1);
      cur = last;
    }
    --cur;
    return *this;
  }
  self operator--(int) {
    self tmp = *this;
    --*this;
    return tmp;
  }

  self &operator+=(difference_type n) {
    const auto offset = n + (cur - first);
    if (offset >= 0 && offset < static_cast<difference_type>(buffer_size)) {
      // 仍在当前缓冲区
      cur += n;
    } else {
      // 要跳到其他缓冲区
      const auto node_offset =
          offset > 0
              ? offset / static_cast<difference_type>(buffer_size)
              : -static_cast<difference_type>((-offset - 1) / buffer_size) - 1;
      set_node(node + node_offset);
      cur = first +
            (offset - node_offset * static_cast<difference_type>(buffer_size));
    }
    return *this;
  }

  self operator+(difference_type n) const {
    self tmp = *this;
    return tmp += n;
  }
  self &operator-=(difference_type n) { return *this += -n; }
  self operator-(difference_type n) const {
    self tmp = *this;
    return tmp -= n;
  }

  reference operator[](difference_type n) const { return *(*this + n); }
  // 重载比较操作符
  bool operator==(const self &rhs) const { return cur == rhs.cur; }
  bool operator<(const self &rhs) const {
    return node == rhs.node ? (cur < rhs.rhs) : (node < rhs.node);
  }
  bool operator!=(const self &rhs) const { return !(*this == rhs); }
  bool operator>(const self &rhs) const { return rhs < *this; }
  bool operator<=(const self &rhs) const { return !(rhs < *this); }
  bool operator>=(const self &rhs) const { return !(*this < rhs); }
};

// 模板类 deque
// 模板参数代表数据类型
template <class T> class deque {
public:
  // deque 的型别定义
  typedef mystl::allocator<T> allocator_type;
  typedef mystl::allocator<T> data_allocator;
  typedef mystl::allocator<T *> map_allocator;

  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;
  typedef pointer *map_pointer;
  typedef const_pointer *const_map_pointer;

  typedef deque_iterator<T, T &, T *> iterator;
  typedef deque_iterator<T, const T &, const T *> const_iterator;
  typedef mystl::reverse_iterator<iterator> reverse_iterator;
  typedef mystl::reverse_iterator<const_iterator> const_reverse_iterator;

  allocator_type get_allocator() { return allocator_type(); }

  static const size_type buffer_size = deque_buf_size<T>::value;

private:
  // 用以下四个数据来表示一个 deque
  iterator begin_; // 指向第一个节点
  iterator end_;   // 指向最后一个节点
  map_pointer
      map_; // 指向一块map，map中的每一个元素都是一个指针，指向一个缓冲区
  size_type map_size_; // map 内指针的数目

  // todo
};
} // namespace mystl

#endif // !MYTINYSTL_DUQUE_H