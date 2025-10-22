#ifndef MYTINYSTL_RB_TREE_H_
#define MYTINYSTL_RB_TREE_H_

// 这个头文件包含一个模版类 rb_tree
// rb_tree : 红黑树

#include <initializer_list>

#include <cassert>

#include "exceptdef.h"
#include "functional.h"
#include "iterator.h"
#include "memory.h"
#include "type_traits.h"

namespace mystl {

// rb_tree 节点颜色的类型
typedef bool rb_tree_color_type;

static constexpr rb_tree_color_type rb_tree_red = false;
static constexpr rb_tree_color_type rb_tree_black = true;

// forward declaration

template <class T> struct rb_tree_node_base;
template <class T> struct rb_tree_node;

template <class T> struct rb_tree_iterator;
template <class T> struct rb_tree_const_iterator;

// rb_tree value traits

template <class T, bool> struct rb_tree_value_traits_imp {
  typedef T key_type;
  typedef T mapped_type;
  typedef T value_type;

  template <class Ty> static const key_type &get_key(const Ty &value) {
    return value;
  }

  template <class Ty> static const value_type &get_value(const Ty &value) {
    return value;
  }
};

template <class T> struct rb_tree_value_traits_imp<T, true> {
  typedef typename std::remove_cv<typename T::first_type>::type
      key_type; // std::remove_cv 用于移除类型的 const 和 volatile 限定符。
  typedef typename T::second_value mapped_type;
  typedef T value_type;

  template <class Ty> static const key_type &get_key(const Ty &value) {
    return value.first;
  }
  template <class Ty> static const value_type &get_value(const Ty &value) {
    return value;
  }
};

template <class T> struct rb_tree_value_traits {
  static constexpr bool is_map = mystl::is_pair<T>::value;

  typedef rb_tree_value_traits_imp<T, is_map> value_traits_type;

  typedef typename value_traits_type::key_type key_type;
  typedef typename value_traits_type::mapped_type mapped_type;
  typedef typename value_traits_type::value_type value_type;

  template <class Ty> static const key_type &get_key(const Ty &value) {
    return value_traits_type::get_key(value);
  }

  template <class Ty> static const value_type &get_value(const Ty &value) {
    return value_traits_type::get_value(value);
  }
};

} // namespace mystl

#endif // !MYTINYSTL_RB_TREE_H_