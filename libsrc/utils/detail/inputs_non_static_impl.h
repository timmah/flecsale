// inputs_non_static_impl.h
// T. M. Kelley
// May 15, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved


#ifndef INPUTS_NON_STATIC_IMPL_H
#define INPUTS_NON_STATIC_IMPL_H

#include <tuple>

namespace ale{
namespace utils {


// Grrr, this is hard to factor out because it needs to refer to a class
// method, and so needs to be included into the class declaration.
namespace detail{

/*This is a general pattern, needs a bit more work to be lifted out. */
/**\brief Helper class to grab the type of the Nth element of a tuple */
template <typename T> struct param_list {
  template <std::size_t N> using type = typename std::tuple_element<N, T>::type;
};

} // detail::
} // utils::
} // ale::

/* What the what?!?

  In the process of trying to generalize the pattern of invoking a templated
  function on each type of a tuple. This is wonky when there isn't an actual
  tuple value to drive template deduction, thus all the business of a
  std::index_sequence, which gives a list of integers at the constexpr/
  type level.

  That's all working. The next step is to generalize the function being called.
  Right now I haven't figured out a mechanism to store a templated function
  without actually binding a type to the function. Maybe a CRTP with a
  templated derived class?

  The mess below works for void functions, but we'd like to go the next
  step and have it work with returning a list of values, so we could have
  non-void functions for instance.
*/

//#define apply_f_by_tuple_impl(f)                                        \
//template <typename Tuple, std::size_t... Is>                            \
//inline                                                                  \
//auto f##_tuple_impl(std::index_sequence<Is...>) {                       \
//  using parm_list = detail::param_list<Tuple>;                          \
//  return {{(f<typename parm_list::template type<Is> >(),0)...}};        \
//}
//
//#define apply_f_by_tuple(f)                                             \
//                                                                        \
//apply_f_by_tuple_impl(f)                                                \
//                                                                        \
//template <typename Tuple>                                               \
//inline                                                                  \
//auto f##_by_tuple() {                                                   \
//  constexpr size_t tsize(std::tuple_size<Tuple>::value);                \
//  return f##_tuple_impl<Tuple>(std::make_index_sequence<tsize>{});      \
//}
//
#endif // include guard


// End of file
