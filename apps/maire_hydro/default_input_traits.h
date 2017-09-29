// default_input_traits.h
// Sep 25, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved


#pragma once

#include "ristra/lua_access.h"
// #include "types.h"
#include <functional>
#include <string>
#include <tuple>

namespace apps::hydro{

inline std::string mk_bc_func_name(size_t i){
  return "bc_function_" + std::to_string(i+1);
}
inline std::string mk_bc_type_name(size_t i){
  return "bc_type_" + std::to_string(i+1);
}

template <size_t ndim>
struct maire_input_traits{
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

  //! the bcs function type
  //! \{
  using bcs_t = apps::hydro::boundary_condition_t<dim>;
  using bcs_ptr_t = std::shared_ptr< bcs_t >;
  using bcs_function_t =
    std::function< bool(const vector_t & x, const real_t & t) >;
  using bcs_pair = std::pair< bcs_ptr_t, bcs_function_t >;
  using bcs_list_t = std::vector< bcs_pair>;
  //! \}

  using types = std::tuple<real_t,
                           std::string,
                           arr_d_r_t,
                           arr_d_s_t,
                           size_t
                           ,ics_function_t
                           ,bcs_function_t
                           >;

}; // maire_input_traits

} // flecsale

// End of file
