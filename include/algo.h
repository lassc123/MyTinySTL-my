#ifndef MYTINYSTL_ALGO_H_
#define MYTINYSTL_ALGO_H_

// 这个头文件包含了mystl的一系列算法

#include <cstddef>
#include <ctime>

#include "algobase.h"
#include "functional.h"
#include "heap_algo.h"
#include "iterator.h"
#include "memory.h"

namespace mystl {
/********************************************************************************/
// all_of
// 检查[first,last)内的全部元素是否都满足一元操作unary_pred为true的情况，满足则返回true，否则返回false
/********************************************************************************/
template <class InputIter, class UnaryPredicate>
bool all_of(InputIter first, InputIter last, UnaryPredicate unary_pred) {
  for (; first != last; ++first) {
    if (!unary_pred(*first)) {
      return false;
    }
  }
  return true;
}

/********************************************************************************/
// any_of
// 检查[first,last)内是否存在某一元素满足一元操作unary_pred为true的情况，存在则返回true，否则返回false
/*********************************************************************************/
template <class InputIter, class UnaryPredicate>
bool any_of(InputIter first, InputIter last, UnaryPredicate unary_pred) {
  for (; first != last; ++first) {
    if (unary_pred(*first)) {
      return true;
    }
  }
  return false;
}
/*****************************************************************************************/
// none_of
// 检查[first, last)内是否全部元素都不满足一元操作 unary_pred 为
// true的情况，满足则返回 true
/*****************************************************************************************/
template <class InputIter, class UnaryPredicate>
bool none_of(InputIter first, InputIter last, UnaryPredicate unary_pred) {
  for (; first != last; ++first) {
    if (unary_pred(*first)) {
      return false;
    }
  }
  return true;
}

/*****************************************************************************************/
// count对[first,
// last)区间内的元素与给定值进行比较，缺省使用operator==，返回元素相等的个数
/*****************************************************************************************/
template <class InputIter, class T>
size_t count(InputIter first, InputIter last, const T &value) {
  size_t n = 0;
  for (; first != last; ++first) {
    if (*first == value) {
      ++n;
    }
  }
  return n;
}
/*****************************************************************************************/
// count_if对[first, last)区间内的每个元素都进行一元 unary_pred
// 操作，返回结果为true的个数
/*****************************************************************************************/
template <class InputIter, class UnaryPredicate>
size_t count_if(InputIter first, InputIter last, UnaryPredicate unary_pred) {
  size_t n = 0;
  for (; first != last; ++first) {
    if (unary_pred(*first)) {
      ++n;
    }
  }
  return n;
}
/*****************************************************************************************/
// find
// 在[first, last)区间内找到等于 value 的元素，返回指向该元素的迭代器
/*****************************************************************************************/
template <class InputIter, class T>
InputIter find(InputIter first, InputIter last, const T &value) {
  for (; first != last; ++first) {
    if (*first == value) {
      return first;
    }
  }
  return last;
}
/*****************************************************************************************/
// find_if
// 在[first, last)区间内找到第一个令一元操作 unary_pred 为 true
// 的元素并返回指向该元素的迭代器
/*****************************************************************************************/
template <class InputIter, class UnaryPredicate>
InputIter find_if(InputIter first, InputIter last, UnaryPredicate unary_pred) {
  for (; first != last; ++first) {
    if (unary_pred(*first)) {
      return first;
    }
  }
  return last;
}
/*****************************************************************************************/
// find_if_not
// 在[first, last)区间内找到第一个令一元操作 unary_pred 为 false
// 的元素并返回指向该元素的迭代器
/*****************************************************************************************/
template <class InputIter, class UnaryPredicate>
InputIter find_if_not(InputIter first, InputIter last,
                      UnaryPredicate unary_pred) {
  for (; first != last; ++first) {
    if (!unary_pred(*first)) {
      return first;
    }
  }
  return last;
}
/*****************************************************************************************/
// search
// 在[first1, last1)中查找[first2, last2)的首次出现点
/*****************************************************************************************/
template <class ForwardIter1, class ForwardIter2>
ForwardIter1 search(ForwardIter1 first1, ForwardIter1 last1,
                    ForwardIter2 first2, ForwardIter2 last2) {
  auto d1 = mystl::distance(first1, last1);
  auto d2 = mystl::distance(first2, last2);
  if (d1 < d2)
    return last1;
  auto curr1 = first1;
  auto curr2 = first2;
  while (curr2 != last2) {
    if (*curr1 == *curr2) {
      ++curr1;
      ++curr2;
    } else {
      if (d1 == d2) {
        return last1;
      } else {
        curr1 = ++first1;
        curr2 = first2;
        --d1;
      }
    }
  }
  return first1;
}
// 重载版本使用函数对象 comp 代替比较操作
template <class ForwardIter1, class ForwardIter2, class Compared>
ForwardIter1 search(ForwardIter1 first1, ForwardIter1 last1,
                    ForwardIter2 first2, ForwardIter2 last2, Compared comp) {
  auto d1 = mystl::distance(first1, last1);
  auto d2 = mystl::distance(first2, last2);
  if (d1 < d2)
    return last1;
  auto curr1 = first1;
  auto curr2 = first2;
  while (curr2 != last2) {
    if (comp(*curr1, *curr2)) {
      ++curr1;
      ++curr2;
    } else {
      if (d1 == d2) {
        return last1;
      } else {
        curr1 = ++first1;
        curr2 = first2;
        --d1;
      }
    }
  }
  return first1;
}
/*****************************************************************************************/
// search_n
// 在[first, last)中查找连续 n 个 value
// 所形成的子序列，返回一个迭代器指向该子序列的起始处
/*****************************************************************************************/
template <class ForwardIter, class Size, class T>
ForwardIter search_n(ForwardIter first, ForwardIter last, Size n,
                     const T &value) {
  if (n <= 0) {
    return first;
  } else {
    first = mystl::find(first, last, value);
    while (first != last) {
      auto m = n - 1;
      auto cur = first;
      ++cur;
      while (cur != last && m != 0 && *cur == value) {
        ++cur;
        --m;
      }
      if (m == 0) {
        return first;
      } else {
        first = mystl::find(cur, last, value);
      }
    }
  }
  return last;
}
// 重载版本使用函数对象 comp 代替比较操作
template <class ForwardIter, class Size, class T, class Compared>
ForwardIter search_n(ForwardIter first, ForwardIter last, Size n,
                     const T &value, Compared comp) {
  if (n <= 0) {
    return first;
  } else {
    first = mystl::find(first, last, value, comp);
    while (first != last) {
      auto m = n - 1;
      auto cur = first;
      ++cur;
      while (cur != last && m != 0 && comp(*cur, value)) {
        ++cur;
        --m;
      }
      if (m == 0) {
        return first;
      } else {
        first = mystl::find(cur, last, value, comp);
      }
    }
  }
  return last;
}
/*****************************************************************************************/
// find_end
// 在[first1, last1)区间中查找[first2,
// last2)最后一次出现的地方，若不存在返回last1
/*****************************************************************************************/
// find_end_dispatch 的 forward_iterator_tag 版本
template <class ForwardIter1, class ForwardIter2>
ForwardIter1 find_end_dispatch(ForwardIter1 first1, ForwardIter1 last1,
                               ForwardIter2 first2, ForwardIter2 last2,
                               mystl::forward_iterator_tag,
                               mystl::forward_iterator_tag) {
  if (first2 == last2) {
    return last1;
  } else {
    auto result = last1;
    while (true) {
      // 利用 search 查找某个子序列的首次出现点，找不到则返回 last1
      auto new_result = mystl::search(first1, last1, first2, last2);
      if (new_result == last1) {
        return result;
      } else {
        result = new_result;
        first1 = new_result;
        ++first1;
      }
    }
  }
}
// find_end_dispatch 的 bidirectional_iterator_tag 版本
template <class BidirectionalIter1, class BidirectionalIter2>
BidirectionalIter1
find_end_dispatch(BidirectionalIter1 first1, BidirectionalIter1 last1,
                  BidirectionalIter2 first2, BidirectionalIter2 last2,
                  mystl::bidirectional_iterator_tag,
                  mystl::bidirectional_iterator_tag) {
  typedef mystl::reverse_iterator<BidirectionalIter1> reviter1;
  typedef mystl::reverse_iterator<BidirectionalIter2> reviter2;
  reviter1 rlast1(first1);
  reviter2 rlast2(first2);
  reviter1 rresult =
      mystl::search(reviter1(last1), rlast1, reviter2(last2), rlast2);
  if (rresult == rlast1) {
    return last1;
  } else {
    auto result = rresult.base();
    mystl::advance(result, -mystl::distance(first2, last2));
    return result;
  }
}

template <class ForwardIter1, class ForwardIter2>
ForwardIter1 find_end(ForwardIter1 first1, ForwardIter1 last1,
                      ForwardIter2 first2, ForwardIter2 last2) {
  return mystl::find_end_dispatch(
      first1, last1, first2, last2,
      typename iterator_traits<ForwardIter1>::iterator_category(),
      typename iterator_traits<ForwardIter2>::iterator_category());
}

// 重载版本使用函数对象 comp 代替比较操作
// find_end_dispatch 的 forward_iterator_tag 版本
template <class ForwardIter1, class ForwardIter2, class Compared>
ForwardIter1 find_end_dispatch(ForwardIter1 first1, ForwardIter1 last1,
                               ForwardIter2 first2, ForwardIter2 last2,
                               mystl::forward_iterator_tag,
                               mystl::forward_iterator_tag, Compared comp) {
  if (first2 == last2) {
    return last1;
  } else {
    auto result = last1;
    while (true) {
      // 利用 search 查找某个子序列的首次出现点，找不到则返回 last1
      auto new_result = mystl::search(first1, last1, first2, last2, comp);
      if (new_result == last1) {
        return result;
      } else {
        result = new_result;
        first1 = new_result;
        ++first1;
      }
    }
  }
}
// find_end_dispatch 的 bidirectional_iterator_tag 版本
template <class BidirectionalIter1, class BidirectionalIter2, class Compared>
BidirectionalIter1
find_end_dispatch(BidirectionalIter1 first1, BidirectionalIter1 last1,
                  BidirectionalIter2 first2, BidirectionalIter2 last2,
                  mystl::bidirectional_iterator_tag,
                  mystl::bidirectional_iterator_tag, Compared comp) {
  typedef mystl::reverse_iterator<BidirectionalIter1> reviter1;
  typedef mystl::reverse_iterator<BidirectionalIter2> reviter2;
  reviter1 rlast1(first1);
  reviter2 rlast2(first2);
  reviter1 rresult =
      mystl::search(reviter1(last1), rlast1, reviter2(last2), rlast2, comp);
  if (rresult == rlast1) {
    return last1;
  } else {
    auto result = rresult.base();
    mystl::advance(result, -mystl::distance(first2, last2));
    return result;
  }
}
template <class ForwardIter1, class ForwardIter2, class Compared>
ForwardIter1 find_end(ForwardIter1 first1, ForwardIter1 last1,
                      ForwardIter2 first2, ForwardIter2 last2, Compared comp) {
  return mystl::find_end_dispatch(
      first1, last1, first2, last2,
      typename iterator_traits<ForwardIter1>::iterator_category(),
      typename iterator_traits<ForwardIter2>::iterator_category(), comp);
}

/*****************************************************************************************/
// find_first_of
// 在[first1, last1)中查找[first2,
// last2)中的某些元素，返回指向第一次出现的元素的迭代器
/*****************************************************************************************/
template <class InputIter, class ForwardIter>
InputIter find_first_of(InputIter first1, InputIter last1, ForwardIter first2,
                        ForwardIter last2) {
  for (; first1 != last1; ++first1) {
    for (auto iter = first2; iter != last2; ++iter) {
      if (*first1 == *iter) {
        return first1;
      }
    }
  }
  return last1;
}
// 重载版本使用函数对象 comp 代替比较操作
template <class InputIter, class ForwardIter, class Compared>
InputIter find_first_of(InputIter first1, InputIter last1, ForwardIter first2,
                        ForwardIter last2, Compared comp) {
  for (; first1 != last1; ++first1) {
    for (auto iter = first2; iter != last2; ++iter) {
      if (comp(*first1, *iter)) {
        return first1;
      }
    }
  }
  return last1;
}
/*****************************************************************************************/
// for_each
// 使用一个函数对象 f 对[first, last)区间内的每个元素执行一个 operator()
// 操作，但不能改变元素内容 f() 可返回一个值，但该值会被忽略
/*****************************************************************************************/
template <class InputIter, class Function>
Function for_each(InputIter first, InputIter last, Function f) {
  for (; first != last; ++first) {
    f(*first);
  }
  return f;
}

/*****************************************************************************************/
// adjacent_find
// 找出第一对匹配的相邻元素，缺省使用 operator==
// 比较，如果找到返回一个迭代器，指向这对元素的第一个元素
/*****************************************************************************************/
template <class ForwardIter>
ForwardIter adjacent_find(ForwardIter first, ForwardIter last) {
  if (first == last)
    return last;
  auto next = first;
  while (++next != last) {
    if (*first == *next)
      return first;
    first = next;
  }
  return last;
}
// 重载版本使用函数对象 comp 代替比较操作
template <class ForwardIter, class Compared>
ForwardIter adjacent_find(ForwardIter first, ForwardIter last, Compared comp) {
  if (first == last)
    return last;
  auto next = first;
  while (++next != last) {
    if (comp(*first, *next))
      return first;
    first = next;
  }
  return last;
}
/*****************************************************************************************/
// lower_bound
// 在[first, last)中查找第一个不小于 value
// 的元素，并返回指向它的迭代器，若没有则返回 last
/*****************************************************************************************/
// lbound_dispatch 的 forward_iterator_tag 版本
template <class ForwardIter, class T>
ForwardIter lbound_dispatch(ForwardIter first, ForwardIter last, const T &value,
                            forward_iterator_tag) {
  auto len = mystl::distance(first, last);
  auto half = len;
  ForwardIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first;
    mystl::advance(middle, half);
    if (*middle < value) {
      first = ++middle;
      len = len - half - 1;
    } else {
      len = half;
    }
  }
  return first;
}
// lbound_dispatch 的 random_access_iterator_tag 版本
template <class RandomIter, class T>
RandomIter lbound_dispatch(RandomIter first, RandomIter last, const T &value,
                           random_access_iterator_tag) {
  auto len = last - first;
  auto half = len;
  RandomIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first + half;
    if (*middle < value) {
      first = middle + 1;
      len = len - half - 1;
    } else {
      len = half;
    }
  }
  return first;
}
template <class ForwardIter, class T>
ForwardIter lower_bound(ForwardIter first, ForwardIter last, const T &value) {
  return mystl::lbound_dispatch(first, last, value, iterator_category(first));
}
// 重载版本使用函数对象 comp 代替比较操作
// lbound_dispatch 的 forward_iterator_tag 版本
template <class ForwardIter, class T, class Compared>
ForwardIter lbound_dispatch(ForwardIter first, ForwardIter last, const T &value,
                            forward_iterator_tag, Compared comp) {
  auto len = mystl::distance(first, last);
  auto half = len;
  ForwardIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first;
    mystl::advance(middle, half);
    if (comp(*middle, value)) {
      first = middle;
      ++first;
      len = len - half - 1;
    } else {
      len = half;
    }
  }
  return first;
}
// lbound_dispatch 的 random_access_iterator_tag 版本
template <class RandomIter, class T, class Compared>
RandomIter lbound_dispatch(RandomIter first, RandomIter last, const T &value,
                           random_access_iterator_tag, Compared comp) {
  auto len = last - first;
  auto half = len;
  RandomIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first + half;
    if (comp(*middle, value)) {
      first = middle + 1;
      len = len - half - 1;
    } else {
      len = half;
    }
  }
  return first;
}
template <class ForwardIter, class T, class Compared>
ForwardIter lower_bound(ForwardIter first, ForwardIter last, const T &value,
                        Compared comp) {
  return mystl::lbound_dispatch(first, last, value, iterator_category(first),
                                comp);
}
/*****************************************************************************************/
// upper_bound
// 在[first, last)中查找第一个大于value
// 的元素，并返回指向它的迭代器，若没有则返回 last
/*****************************************************************************************/
// ubound_dispatch 的 forward_iterator_tag 版本
template <class ForwardIter, class T>
ForwardIter ubound_dispatch(ForwardIter first, ForwardIter last, const T &value,
                            forward_iterator_tag) {
  auto len = mystl::distance(first, last);
  auto half = len;
  ForwardIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first;
    mystl::advance(middle, half);
    if (value < *middle) {
      len = half;
    } else {
      first = middle;
      ++first;
      len = len - half - 1;
    }
  }
  return first;
}
// ubound_dispatch 的 random_access_iterator_tag 版本
template <class RandomIter, class T>
RandomIter ubound_dispatch(RandomIter first, RandomIter last, const T &value,
                           random_access_iterator_tag) {
  auto len = last - first;
  auto half = len;
  RandomIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first + half;
    if (value < *middle) {
      len = half;
    } else {
      first = middle + 1;
      len = len - half - 1;
    }
  }
  return first;
}
template <class ForwardIter, class T>
ForwardIter upper_bound(ForwardIter first, ForwardIter last, const T &value) {
  return mystl::ubound_dispatch(first, last, value,
                                mystl::iterator_category(first));
}

// 重载版本使用函数对象 comp 代替比较操作
// ubound_dispatch 的 forward_iterator_tag 版本
template <class ForwardIter, class T, class Compared>
ForwardIter ubound_dispatch(ForwardIter first, ForwardIter last, const T &value,
                            forward_iterator_tag, Compared comp) {
  auto len = mystl::distance(first, last);
  auto half = len;
  ForwardIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first;
    mystl::advance(middle, half);
    if (comp(value, *middle)) {
      len = half;
    } else {
      first = middle;
      ++first;
      len = len - half - 1;
    }
  }
  return first;
}
// ubound_dispatch 的 random_access_iterator_tag 版本
template <class RandomIter, class T, class Compared>
RandomIter ubound_dispatch(RandomIter first, RandomIter last, const T &value,
                           random_access_iterator_tag, Compared comp) {
  auto len = last - first;
  auto half = len;
  RandomIter middle;
  while (len > 0) {
    half = len >> 1;
    middle = first + half;
    if (comp(value, *middle)) {
      len = half;
    } else {
      first = middle + 1;
      len = len - half - 1;
    }
  }
  return first;
}

template <class ForwardIter, class T, class Compared>
ForwardIter upper_bound(ForwardIter first, ForwardIter last, const T &value,
                        Compared comp) {
  return mystl::ubound_dispatch(first, last, value,
                                mystl::iterator_category(first), comp);
}

/*****************************************************************************************/
// binary_search
// 二分查找，如果在[first, last)内有等同于 value 的元素，返回 true，否则返回
// false
/*****************************************************************************************/
template <class ForwardIter, class T>
bool binary_search(ForwardIter first, ForwardIter last, const T &value) {
  auto i = mystl::lower_bound(first, last, value);
  return i != last && !(value < *i);
}
// 重载版本使用函数对象 comp 代替比较操作
template <class ForwardIter, class T, class Compared>
bool binary_search(ForwardIter first, ForwardIter last, const T &value,
                   Compared comp) {
  auto i = mystl::lower_bound(first, last, value, comp);
  return i != last && !comp(value, *i);
}
/*****************************************************************************************/
// equal_range
// 查找[first,last)区间中与 value
// 相等的元素所形成的区间，返回一对迭代器指向区间首尾
// 第一个迭代器指向第一个不小于 value 的元素，第二个迭代器指向第一个大于 value
// 的元素
/*****************************************************************************************/
// erange_dispatch 的 forward_iterator_tag 版本
template <class ForwardIter, class T>
mystl::pair<ForwardIter, ForwardIter>
erange_dispatch(ForwardIter first, ForwardIter last, const T &value,
                forward_iterator_tag) {
  auto left = mystl::lower_bound(first, last, value);
  auto right = mystl::upper_bound(first, last, value);
  return {left, right};
}
// erange_dispatch 的 random_access_iterator_tag 版本
template <class RandomIter, class T>
mystl::pair<RandomIter, RandomIter>
erange_dispatch(RandomIter first, RandomIter last, const T &value,
                random_access_iterator_tag) {
  auto left = mystl::lower_bound(first, last, value);
  auto right = mystl::upper_bound(first, last, value);
  return {left, right};
}

template <class ForwardIter, class T>
mystl::pair<ForwardIter, ForwardIter>
equal_range(ForwardIter first, ForwardIter last, const T &value) {
  return mystl::erange_dispatch(first, last, value, iterator_category(first));
}
/*****************************************************************************************/
// generate
// 将函数对象 gen 的运算结果对[first, last)内的每个元素赋值
/*****************************************************************************************/
template <class ForwardIter, class Generator>
void generate(ForwardIter first, ForwardIter last, Generator gen) {
  for (; first != last; ++first) {
    *first = gen();
  }
}

/*****************************************************************************************/
// generate_n
// 用函数对象 gen 连续对 n 个元素赋值
/*****************************************************************************************/
template <class ForwardIter, class Generator>
void generate_n(ForwardIter first, size_t n, Generator gen) {
  for (; n > 0; --n, ++first) {
    *first = gen();
  }
}
/*****************************************************************************************/
// includes
// 判断序列一S1 是否包含序列二S2
/*****************************************************************************************/
template <class InputIter1, class InputIter2>
bool includes(InputIter1 first1, InputIter1 last1, InputIter2 first2,
              InputIter2 last2) {
  while (first1 != last1 && first2 != last2) {
    if (*first2 < *first1)
      return false;
    else if (*first1 < *first2)
      ++first1;
    else {
      ++first1;
      ++first2;
    }
  }
  return first2 == last2;
}
// 重载版本使用函数对象 comp 代替比较操作
template <class InputIter1, class InputIter2, class Compared>
bool includes(InputIter1 first1, InputIter1 last1, InputIter2 first2,
              InputIter2 last2, Compared comp) {
  while (first1 != last1 && first2 != last2) {
    if (comp(*first2, *first1))
      return false;
    else if (comp(*first1, *first2))
      ++first1;
    else {
      ++first1;
      ++first2;
    }
  }
  return first2 == last2;
}

/*****************************************************************************************/
// is_heap
// 检查[first, last)内的元素是否为一个堆，如果是，则返回 true
/*****************************************************************************************/
template <class RandomIter> bool is_heap(RandomIter first, RandomIter last) {
  auto n = mystl::distance(first, last);
  auto parent = 0;
  for (auto child = 1; child < n; ++child) {
    if (first[parent] < first[child]) {
      return false;
    }
    if ((child & 1) == 0) {
      ++parent;
    }
  }
  return true;
}
// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class Compared>
bool is_heap(RandomIter first, RandomIter last, Compared comp) {
  auto n = mystl::distance(first, last);
  auto parent = 0;
  for (auto child = 1; child < n; ++child) {
    if (comp(first[parent], first[child])) {
      return false;
    }
    if ((child & 1) == 0) {
      ++parent;
    }
  }
  return true;
}

} // namespace mystl
#endif // !MYTINYSTL_ALGO_H_