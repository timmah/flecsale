/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Input type traits.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ristra/lua_access.h"
#include "flecsale/math/vector.h"
#include <functional>
#include <string>
#include <tuple>

namespace apps::hydro{

template <size_t ndim>
struct hydro_input_traits{
// types and constants
  static constexpr size_t dim = ndim;

  using real_t = double;
  using string_t = std::string;
  using vector_t = std::array<real_t,dim>;
  using arr_d_r_t = std::array<real_t,dim>;
  using arr_d_s_t = std::array<size_t,dim>;


  using flecsale_vector_t = flecsale::math::array<real_t,dim>;
  using flecsale_arr_d_r_t = flecsale::math::array<real_t,dim>;
  using flecsale_arr_d_s_t = flecsale::math::array<size_t,dim>;

  using ics_return_t   = std::tuple<real_t,flecsale_arr_d_r_t,real_t>;
  using ics_function_t =
    std::function<ics_return_t(flecsale_arr_d_r_t const &, real_t const & t)>;

  using types = std::tuple<real_t,
                           std::string,
                           arr_d_r_t,
                           arr_d_s_t,
                           size_t
                           ,ics_function_t
                           >;
}; // maire_input_traits

} // flecsale

// End of file
