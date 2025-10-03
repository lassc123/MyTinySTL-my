#ifndef MYTINYSTL_DEQUE_H_
#define MYTINYSTL_DEQUE_H_

// 这个头文件包含了一个模板类 deque
// deque: 双端队列

// notes:
//
// 异常保证：
// mystl::deque<T>
// 满足基本异常保证，部分函数无异常保证，并对以下等函数做强异常安全保证：
//   * emplace_front
//   * emplace_back
//   * emplace
//   * push_front
//   * push_back
//   * insert

#include <initializer_list>

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
  value_pointer last;  // 指向所在缓冲区的尾部 (one-past-the-end)
  map_pointer node;    // 缓冲区所在节点

  // 构造、复制、移动函数
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
    last = first + buffer_size;
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
    if (cur == last) { // 如果到达缓冲区的尾
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
    if (cur == first) { // 如果到达缓冲区的头
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
    if (offset >= 0 &&
        offset < static_cast<difference_type>(buffer_size)) { // 仍在当前缓冲区
      cur += n;
    } else { // 要跳到其他的缓冲区
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
    return node == rhs.node ? (cur < rhs.cur) : (node < rhs.node);
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
  // 用以下四个数据来表现一个 deque
  iterator begin_; // 指向第一个元素
  iterator end_;   // 指向最后一个元素的下一个位置
  map_pointer
      map_; // 指向一块 map，map 中的每个元素都是一个指针，指向一个缓冲区
  size_type map_size_; // map 的大小

public:
  // 构造、复制、移动、析构函数

  deque() { fill_init(0, value_type()); }

  explicit deque(size_type n) { fill_init(n, value_type()); }

  deque(size_type n, const value_type &value) { fill_init(n, value); }

  template <class IIter,
            typename std::enable_if<mystl::is_input_iterator<IIter>::value,
                                    int>::type = 0>
  deque(IIter first, IIter last) {
    copy_init(first, last, iterator_category(first));
  }

  deque(std::initializer_list<value_type> ilist) {
    copy_init(ilist.begin(), ilist.end(), mystl::forward_iterator_tag());
  }

  deque(const deque &rhs) {
    copy_init(rhs.begin(), rhs.end(), mystl::forward_iterator_tag());
  }

  deque(deque &&rhs) noexcept
      : begin_(mystl::move(rhs.begin_)), end_(mystl::move(rhs.end_)),
        map_(rhs.map_), map_size_(rhs.map_size_) {
    rhs.map_ = nullptr;
    rhs.map_size_ = 0;
  }

  deque &operator=(const deque &rhs);
  deque &operator=(deque &&rhs) noexcept;

  deque &operator=(std::initializer_list<value_type> ilist) {
    deque tmp(ilist);
    swap(tmp);
    return *this;
  }

  ~deque() {
    if (map_ != nullptr) {
      destroy_elements_and_buffers();
      map_allocator::deallocate(map_, map_size_);
    }
  }

public:
  // 迭代器操作

  iterator begin() noexcept { return begin_; }
  const_iterator begin() const noexcept { return begin_; }
  iterator end() noexcept { return end_; }
  const_iterator end() const noexcept { return end_; }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }

  // 容量相关操作
  bool empty() const noexcept { return begin() == end(); }
  size_type size() const noexcept { return end_ - begin_; }
  size_type max_size() const noexcept { return static_cast<size_type>(-1); }
  void resize(size_type new_size) { resize(new_size, value_type()); }
  void resize(size_type new_size, const value_type &value);
  void shrink_to_fit() noexcept;

  // erase / clear

  iterator erase(iterator position);
  iterator erase(iterator first, iterator last);
  void clear();

  // swap
  void swap(deque &rhs) noexcept;

private:
  // cleanup
  void destroy_elements_and_buffers();
};
/************************************************************/
template <class T> deque<T> &deque<T>::operator=(const deque &rhs) {
  if (this != &rhs) {
    const auto len = size();
    if (len >= rhs.size()) {
      erase(mystl::copy(rhs.begin_, rhs.end_, begin_), end_);
    } else {
      iterator mid = rhs.begin() + static_cast<difference_type>(len);
      mystl::copy(rhs.begin_, mid, begin_);
      insert(end_, mid, rhs.end_);
    }
  }
  return *this;
}

// 交换两个swap
template <class T> void deque<T>::swap(deque &rhs) noexcept {
  if (this != &rhs) {
    mystl::swap(begin_, rhs.begin_);
    mystl::swap(end_, rhs.end_);
    mystl::swap(map_, rhs.map_);
    mystl::swap(map_size_, rhs.map_size_);
  }
}

/*****************************************************************************************/
// helper function

// destroy_elements_and_buffers 函数
template <class T> void deque<T>::destroy_elements_and_buffers() {
  if (!empty()) {
    // 析构所有元素
    clear();
  }
  // 释放所有缓冲区
  destroy_buffer(begin_.node, end_.node);
}
} // namespace mystl

#endif // !MYTINYSTL_DUQUE_H