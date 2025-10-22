#include "../include/algo.h"
#include "../include/algobase.h"
#include "../include/allocator.h"
#include "../include/construct.h"
#include "../include/deque.h"
#include "../include/functional.h"
#include "../include/heap_algo.h"
#include "../include/iterator.h"
#include "../include/list.h"
#include "../include/rb_tree.h"
#include "../include/type_traits.h"
#include "../include/uninitialized.h"
#include "../include/util.h"
#include "../include/vector.h"

#include <iostream>
int main(int argc, char **argv) {

  //   mystl::pair<int, double> p1(1, 2.0);
  auto pair1 = mystl::make_pair(10, 3.14);
  std::cout << "pair1.first: " << pair1.first << std::endl;
  std::cout << "pair1.second: " << pair1.second << std::endl;
  mystl::pair<int, double> pair2(pair1);
  std::cout << "hello world!" << std::endl;
  // mystl::rotate_dispatch(ForwardIter first, ForwardIter middle, ForwardIter
  // last, mystl::forward_iterator_tag)
  return 0;
}
