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

private:
  // helper functions

  // initialize/destory
  void try_init() noexcept;

  void init_space(size_type size, size_type cbap);

  void fill_init(size_type n, const value_type &value);

  template <class Iter> void range_init(Iter first, Iter last);
};
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
} // namespace mystl

#endif //! MYTINYSTL_VECTOR_H