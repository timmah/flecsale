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
  using ics_return_t   = std::tuple<real_t,arr_d_r_t,real_t>;
  using ics_function_t =
    std::function<ics_return_t(arr_d_r_t const &, real_t const & t)>;

  using types = std::tuple<real_t,
                           std::string,
                           arr_d_r_t,
                           arr_d_s_t,
                           size_t
                           ,ics_function_t
                           >;

  struct Lua_ICS_Func_Wrapper{
  // interface
    ics_return_t
    operator()(
      arr_d_r_t const & x, real_t const t)
    {
      real_t d,p;
      arr_d_r_t v;
      // why not just return the tuple?
      std::tie(d, v, p) =
        lua_func_(x, t).template as<real_t, arr_d_r_t, real_t>();
      return std::make_tuple( d, std::move(v), p);
    }

    explicit Lua_ICS_Func_Wrapper(ristra::lua_result_t &u)
        : lua_func_(u) {}

  // state:
    /* We interpret the lua_result as a function. Would prefer to verify
       that is is a function. */
    ristra::lua_result_t &lua_func_;
  }; // struct Lua_ICS_Func_Wrapper

}; // maire_input_traits

} // flecsale

// End of file
