#ifndef MYTINYSTL_VECTOR_H
#define MYTINYSTL_VECTOR_H

// 这个头文件包含一个模板类 vector
// vector: 向量

// notes:
//
// 异常保证：
// mystl::vector<T>
// 满足基本异常保证，部分函数无异常保证，并对以下函数做强异常安全保证： emplace
// emplace_back
// push_back
// 当std::is_nothrow_move_assignable<T>::value ==
// true时，以下函数也满足强异常保证： reserve resize insert

#include <initializer_list>
#include <iterator>

#include "algo.h"
#include "algobase.h"
#include "exceptdef.h"
#include "iterator.h"
#include "memory.h"
#include "uninitialized.h"
#include "util.h"

namespace mystl {
#ifdef max
#pragma message("undefine macro max")
#undef max
#endif // max

#ifdef min
#pragma message("undefine macro min")
#undef min
#endif // min

// 模板类:vector
// 模板参数 T 代表类型
template <class T> class vector {
  static_assert(!std::is_same<bool, T>::value,
                "vector<bool> is abandoned in mystl");

public:
  // vector的嵌套型别定义
  typedef mystl::allocator<T> allocator_type;
  typedef mystl::allocator<T> data_allocator;

  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;

  typedef value_type *iterator;
  typedef const value_type *const_iterator;
  typedef mystl::reverse_iterator<iterator> reverse_iterator;
  typedef mystl::reverse_iterator<const_iterator> const_reverse_iterator;

  allocator_type get_allocator() { return data_allocator(); }

private:
  iterator begin_; // 表示目前所使用空间的头部
  iterator end_;   // 表示目前所使用空间的尾部
  iterator cap_;   // 表示目前储存空间的尾部

public:
  // 构造，复制，移动，析构函数
  vector() noexcept { try_init(); }

  explicit vector(size_type n) { fill_init(n, value_type()); }

  vector(size_type n, const value_type &value) { fill_init(n, value); }

  template <class Iter,
            typename std::enable_if<mystl::is_input_iterator<Iter>::value,
                                    int>::type = 0>
  vector(Iter first, Iter last) {
    MYSTL_DEBUG(!(last < first));
    range_init(first, last);
  }

  vector(const vector &rhs) { range_init(rhs.begin_, rhs.end_); }

  vector(vector &&rhs) noexcept
      : begin_(rhs.begin_), end_(rhs.end_), cap_(rhs.cap_) {
    rhs.begin_ = nullptr;
    rhs.end_ = nullptr;
    rhs.cap_ = nullptr;
  }

  vector(std::initializer_list<value_type> ilist) {
    range_init(ilist.begin(), ilist.end());
  }

  vector &operator=(const vector &rhs);
  vector &operator=(vector &&rhs) noexcept;

  vector &operator=(std::initializer_list<value_type> ilist) {
    vector tmp(ilist.begin(), ilist.end());
    swap(tmp);
    return *this;
  }

  ~vector() {
    destroy_and_recover(begin_, end_, cap_ - begin_);
    begin_ = end_ = cap_ = nullptr;
  }

public:
  // 迭代器相关操作
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
    return reverse_iterator(begin());
  }

  const_iterator cbegin() const noexcept { return begin(); }

  const_iterator cend() const noexcept { return end(); }

  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }

  // 容量相关操作
  bool empty() const noexcept { return begin_ == end_; }
  size_type size() const noexcept {
    return static_cast<size_type>(end_ - begin_);
  }
  size_type max_size() const noexcept {
    return static_cast<size_type>(-1) / sizeof(T);
  }
  size_type capacity() const noexcept {
    return static_cast<size_type>(cap_ - begin_);
  }
  void reserve(size_type n);
  void shrink_to_fit();

  // 訪問元素相关操作
  reference operator[](size_type n) {
    MYSTL_DEBUG(n < size());
    return *(begin_ + n);
  }

  const_reference operator[](size_type n) const {
    MYSTL_DEBUG(n < size());
    return *(begin_ + n);
  }

  reference at(size_type n) {
    THROW_OUT_OF_RANGE_IF(!(n < size()),
                          "vector<T>::at() subscript out of range");
    return (*this)[n];
  }

  const_reference at(size_type n) const {
    THROW_OUT_OF_RANGE_IF(!(n < size()),
                          "vector<T>::at() subscript out of range");
    return (*this)[n];
  }

  reference front() {
    MYSTL_DEBUG(!empty());
    return *begin_;
  }

  const_reference front() const {
    MYSTL_DEBUG(!empty());
    return *begin_;
  }

  reference back() {
    MYSTL_DEBUG(!empty());
    return *(end_ - 1);
  }

  const_reference back() const {
    MYSTL_DEBUG(!empty());
    return *(end_ - 1);
  }

  pointer data() noexcept { return begin_; }

  const_pointer data() const noexcept { return begin_; }

  // 修改容器相关操作

  // assign

  void assign(size_type n, const value_type &value) { fill_assign(n, value); }

  template <class Iter,
            typename std::enable_if<mystl::is_input_iterator<Iter>::value,
                                    int>::type = 0>
  void assign(Iter first, Iter last) {
    MYSTL_DEBUG(!(last < first));
    copy_assign(first, last, iterator_category(first));
  }

  void assign(std::initializer_list<value_type> il) {
    copy_assign(il.begin(), il.end(), mystl::forward_iterator_tag{});
  }

  // emplace / emplace_back

  template <class... Args> iterator emplace(const_iterator pos, Args &&...args);

  template <class... Args> iterator emplace_back(Args &&...args);

  // insert
  iterator insert(const_iterator pos, const value_type &value);
  iterator insert(const_iterator pos, value_type &&value) {
    return emplace(pos, mystl::move(value));
  }

  iterator insert(const_iterator pos, size_type n, const value_type &value) {
    MYSTL_DEBUG(pos >= begin() && pos <= end());
    return fill_insert(const_cast<iterator>(pos), n, value);
  }

  template <class Iter, typename std::enable_if<
                            mystl::is_input_iterator<Iter>::value, int> = 0>
  void insert(const_iterator pos, Iter first, Iter last) {
    MYSTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
    copy_insert(const_cast<iterator>(pos), first, last);
  }

  // erase / clear
  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);
  void clear() { erase(begin(), end()); }

private:
  // helper functions

  // initialize/destory
  void try_init() noexcept;

  void init_space(size_type size, size_type cbap);

  void fill_init(size_type n, const value_type &value);

  template <class Iter> void range_init(Iter first, Iter last);

  void destroy_and_recover(iterator first, iterator last, size_type n);

  // assign
  void fill_assign(size_type n, const value_type &value);

  template <class IIter>
  void copy_assign(IIter first, IIter last, input_iterator_tag);

  template <class FIter>
  void copy_assign(FIter first, FIter last, forward_iterator_tag);
};
/**********************************************************************/

// 复制赋值操作符
template <class T> vector<T> &vector<T>::operator=(const vector &rhs) {
  if (this != &rhs) {
    const auto len = rhs.size();
    if (len > capacity()) {
      vector tmp(rhs.begin_, rhs.end_);
      swap(tmp);
    } else if (size() >= len) {
      auto i = mystl::copy(rhs.begin(), rhs.end(), begin());
      data_allocator::destroy(i, end_);
      end_ = begin_ + len;
    } else {
      mystl::copy(rhs.begin(), rhs.begin() + size(), begin_);
      mystl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);
      cap_ = end_ = begin_ + len;
    }
  }
  return *this;
}

// 移动赋值操作符
template <class T> vector<T> &vector<T>::operator=(vector &&rhs) noexcept {
  destroy_and_recover(begin_, end_, cap_ - begin_);
  begin_ = rhs.begin_;
  end_ = rhs.end_;
  cap_ = rhs.cap_;
  rhs.begin_ = nullptr;
  rhs.end_ = nullptr;
  rhs.cap_ = nullptr;
  return *this;
}

// 在pos位置就地构造元素,避免额外的复制或移动开销
template <class T>
template <class... Args>
typename vector<T>::iterator vector<T>::emplace(const_iterator pos,
                                                Args &&...args) {
  MYSTL_DEBUG(pos >= begin() && pos <= end());
  iterator xpos = const_cast<iterator>(pos);
  const size_type n = xpos - begin_;
  if (end_ != cap_ && xpos == end_) {
    // Todo
  }
}

// 删除pos位置上的元素
template <class T>
typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
  MYSTL_DEBUG(pos >= begin() && pos < end());
  iterator xpos = begin_ + (pos - begin());
  mystl::move(xpos + 1, end_, xpos);
  data_allocator::destroy(end_ - 1);
  --end_;
  return xpos;
}

// 删除[first,last)上的元素
template <class T>
typename vector<T>::iterator vector<T>::erase(const_iterator first,
                                              const_iterator last) {
  MYSTL_DEBUG(first >= begin() && last <= end() && !(last < first));
  const auto n = first - begin();
  iterator r = begin_ + (first - begin());
  data_allocator::destroy(mystl::move(r + (last - first), end_, r), end_);
  end_ = end_ - (last - first);
  return begin_ + n;
}

/**********************************************************************/
// helper function

// try_init函数，若分配失败则忽略，不抛出异常
template <class T> void vector<T>::try_init() noexcept {
  try {
    begin_ = data_allocator::allocate(16);
    end_ = begin_;
    cap_ = begin_ + 16;
  } catch (...) {
    begin_ = nullptr;
    end_ = nullptr;
    cap_ = nullptr;
  }
}

// init_space函数
template <class T> void vector<T>::init_space(size_type size, size_type cap) {
  try {
    begin_ = data_allocator::allocate(cap);
    end_ = begin_ + size;
    cap_ = begin_ + cap;
  } catch (...) {
    begin_ = nullptr;
    end_ = nullptr;
    cap_ = nullptr;
    throw;
  }
}

// fill_init函数
template <class T>
void vector<T>::fill_init(size_type n, const value_type &value) {
  const size_type init_size = mystl::max(static_cast<size_type>(16), n);
  init_sapce(n, init_size);
  mystl::uninitialized_fill_n(begin_, n, value);
}

// range_init函数
template <class T>
template <class Iter>
void vector<T>::range_init(Iter first, Iter last) {
  const size_type len = mystl::distance(first, last);
  const size_type init_size = mystl::max(len, static_cast<size_type>(16));
  init_space(len, init_size);
  mystl::uninitialized_copy(first, last, begin_);
}

// destroy_and_recover函数
template <class T>
void vector<T>::destroy_and_recover(iterator first, iterator last,
                                    size_type n) {
  data_allocator::destroy(first, last);
  data_allocator::deallocate(first, n);
}

// fill_assign函数
template <class T>
void vector<T>::fill_assign(size_type n, const value_type &value) {
  if (n > capacity()) {
    vector tmp(n, value);
    swap(tmp);
  } else if (n > size()) {
    mystl::fill(begin(), end(), value);
    end_ = mystl::uninitialized_fill_n(end_, n - size(), value);
  } else {
    erase(mystl::fill_n(begin_, n, value), end_);
  }
}

// copy_assign函数
template <class T>
template <class IIter>
void vector<T>::copy_assign(IIter first, IIter last, input_iterator_tag) {
  auto cur = begin_;
  for (; first != last && cur != end_; ++first, ++cur)
    ;
  {
    *cur = *first;
  }
  if (first = last) {
    erase(cur, end_);
  } else {
    insert(end_, first, last);
  }
}
} // namespace mystl

#endif //! MYTINYSTL_VECTOR_H