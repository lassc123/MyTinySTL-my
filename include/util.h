#ifndef MYTINYSTL_UTIL_H_
#define MYTINYSTL_UTIL_H_

#include "iterator.h"
#include <cstddef>
#include <type_traits>

namespace mystl {
template <class T>
typename std::remove_reference<T>::type &&move(T &&arg) noexcept {
  return static_cast<typename std::remove_reference<T>::type &&>(arg);
}

template <class T>
T &&forward(typename std::remove_reference<T>::type &arg) noexcept {
  return static_cast<T &&>(arg);
}

template <class T>
T &&forward(typename std::remove_reference<T>::type &&arg) noexcept {
  static_assert(!std::is_lvalue_reference<T>::value,
                "T must be an rvalue reference");
  return static_cast<T &&>(arg);
}

template <class Tp> void swap(Tp &lhs, Tp &rhs) {
  auto tmp(mystl::move(lhs));
  lhs = mystl::move(rhs);
  rhs = mystl::move(tmp);
}

template <class ForwardIter1, class ForwardIter2>
ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1,
                        ForwardIter2 first2) {
  for (; first1 != last1; ++first1, (void)++first2) {
    mystl::swap(*first1, *first2);
  }
  return first2;
}

template <class Tp, size_t N> void swap(Tp (&a)[N], Tp (&b)[N]) {
  mystl::swap_range(a, a + N, b);
}

template <class Ty1, class Ty2> struct pair {
  typedef Ty1 first_type;
  typedef Ty2 second_type;

  first_type first;
  second_type second;

  template <class Other1 = first_type, class Other2 = second_type,
            typename = typename std::enable_if<
                std::is_default_constructible<Other1>::value &&
                    std::is_default_constructible<Other2>::value,
                void>::type>
  constexpr pair() : first(), second() {}

  template <
      class U1 = first_type, class U2 = second_type,
      typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                  std::is_copy_constructible<U2>::value &&
                                  std::is_convertible<const U1 &, Ty1>::value &&
                                  std::is_convertible<const U2 &, Ty2>::value,
                              int>::type = 0>
  constexpr pair(const first_type &x, const second_type &y)
      : first(x), second(y) {}

  template <class U1 = first_type, class U2 = second_type,
            typename std::enable_if<
                std::is_copy_constructible<U1>::value &&
                    std::is_copy_constructible<U2>::value &&
                    (!std::is_convertible<const U1 &, Ty1>::value) &&
                    (!std::is_convertible<const U2 &, Ty2>::value),
                int>::type = 0>
  explicit constexpr pair(const first_type &x, const second_type &y)
      : first(x), second(y) {}

  pair(const pair &rhs) = default;
  pair(pair &&rhs) noexcept(
      std::is_nothrow_move_constructible<first_type>::value &&
      std::is_nothrow_move_constructible<second_type>::value) =
      default; // 增添点1

  template <class Other1, class Other2,
            typename std::enable_if<
                std::is_constructible<first_type, Other1>::value &&
                    std::is_constructible<second_type, Other2>::value &&
                    std::is_convertible<Other1 &&, first_type>::value &&
                    std::is_convertible<Other2 &&, second_type>::value,
                int>::type = 0>
  constexpr pair(Other1 &&x, Other2 &&y)
      : first(mystl::forward<Other1>(x)), second(mystl::forward<Other2>(y)) {}

  template <class Other1, class Other2,
            typename std::enable_if<
                std::is_constructible<first_type, Other1>::value &&
                    std::is_constructible<second_type, Other2>::value &&
                    (!std::is_convertible<Other1 &&, first_type>::value ||
                     !std::is_convertible<Other2 &&, second_type>::value),
                int>::type = 0>
  explicit constexpr pair(Other1 &&x, Other2 &&y)
      : first(mystl::forward<Other1>(x)), second(mystl::forward<Other2>(y)) {}

  template <class Other1, class Other2,
            typename std::enable_if<
                std::is_constructible<first_type, const Other1 &>::value &&
                    std::is_constructible<second_type, const Other2 &>::value &&
                    std::is_convertible<const Other1 &, first_type>::value &&
                    std::is_convertible<const Other2 &, second_type>::value,
                int>::type = 0>
  constexpr pair(const pair<Other1, Other2> &other)
      : first(other.first), second(other.second) {}

  template <class Other1, class Other2,
            typename std::enable_if<
                std::is_constructible<first_type, const Other1 &>::value &&
                    std::is_constructible<second_type, const Other2 &>::value &&
                    (!std::is_convertible<const Other1 &, first_type>::value ||
                     !std::is_convertible<const Other2 &, second_type>::value),
                int>::type = 0>
  explicit constexpr pair(const pair<Other1, Other2> &other)
      : first(other.first), second(other.second) {}

  template <
      class Other1, class Other2,
      typename std::enable_if<
          std::is_constructible<first_type, Other1>::value &&
              std::is_constructible<second_type, Other2>::value &&
              std::is_convertible<Other1 &&, first_type>::value && // 增添点2
              std::is_convertible<Other2 &&, second_type>::value,
          int>::type = 0>
  constexpr pair(pair<Other1, Other2> &&other)
      : first(mystl::forward<Other1>(other.first)),
        second(mystl::forward<Other2>(other.second)) {}

  template <class Other1, class Other2,
            typename std::enable_if<
                std::is_constructible<first_type, Other1>::value &&
                    std::is_constructible<second_type, Other2>::value &&
                    (!std::is_convertible<Other1 &&, first_type>::value ||
                     !std::is_convertible<Other2 &&, second_type>::value),
                int>::type = 0>
  explicit constexpr pair(pair<Other1, Other2> &&other)
      : first(mystl::forward<Other1>(other.first)),
        second(mystl::forward<Other2>(other.second)) {}

  pair &operator=(const pair &rhs) {
    if (this != &rhs) {
      first = rhs.first;
      second = rhs.second;
    }
    return *this;
  }

  pair &operator=(pair &&rhs) noexcept(
      std::is_nothrow_move_assignable<first_type>::value &&
      std::is_nothrow_move_assignable<second_type>::value) {
    if (this != &rhs) {
      first = mystl::move(rhs.first);
      second = mystl::move(rhs.second);
    }
    return *this;
  }

  template <class Other1, class Other2>
  pair &operator=(const pair<Other1, Other2> &other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  template <class Other1, class Other2>
  pair &operator=(pair<Other1, Other2> &&other) noexcept(
      std::is_nothrow_move_assignable<first_type>::value &&
      std::is_nothrow_move_assignable<second_type>::value) {
    first = mystl::move(other.first);
    second = mystl::move(other.second);
    return *this;
  }

  ~pair() = default;

  void
  swap(pair &other) noexcept(std::is_nothrow_swappable<first_type>::value &&
                             std::is_nothrow_swappable<second_type>::value) {
    if (this != &other) {
      mystl::swap(first, other.first);
      mystl::swap(second, other.second);
    }
  }
};

template <class Ty1, class Ty2>
bool operator==(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
  return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class Ty1, class Ty2>
bool operator!=(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
  return !(lhs == rhs);
}

template <class Ty1, class Ty2>
bool operator<(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
  return lhs.first < rhs.first ||
         ((rhs.first == lhs.first) && lhs.second < rhs.second);
}

template <class Ty1, class Ty2>
bool operator>(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
  return rhs < lhs;
}

template <class Ty1, class Ty2>
bool operator<=(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
  return !(rhs < lhs);
}

template <class Ty1, class Ty2>
bool operator>=(const pair<Ty1, Ty2> &lhs, const pair<Ty1, Ty2> &rhs) {
  return !(lhs < rhs);
}

template <class Ty1, class Ty2>
void swap(pair<Ty1, Ty2> &lhs,
          pair<Ty1, Ty2> &rhs) noexcept(std::is_nothrow_swappable<Ty1>::value &&
                                        std::is_nothrow_swappable<Ty2>::value) {
  lhs.swap(rhs);
}

template <class Ty1, class Ty2>
pair<Ty1, Ty2> make_pair(Ty1 &&first, Ty2 &&seconde) {
  return pair<Ty1, Ty2>(mystl::forward<Ty1>(first),
                        mystl::forward<Ty2>(seconde));
}

} // namespace mystl
#endif // !MYTINYSTL_UTIL_H_