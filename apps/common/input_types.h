/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2018 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Type definitions for input resolution.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ristra/initialization/init_value.h"
#include "ristra/initialization/input_engine.h"
#include "ristra/math/array.h"
#include "types.h"
#include <string>

namespace apps::common{

/* The ristra input_engine is configured by input traits. The one thing trait
 * that must be defined is the 'types' tuple. The input engine will use
 * that tuple to navigate over that set of types. We also define some other
 * useful types here.
 */
struct flecsale_input_traits{
  using mesh_t = apps::common::mesh_t;

  static constexpr size_t dim = mesh_t::num_dimensions;

  using real_t = mesh_t::real_t;
  using string_t = std::string;
  using vector_t = mesh_t::vector_t;
  using arr_d_r_t = ristra::math::array<real_t,dim>;
  using arr_d_s_t = ristra::math::array<size_t,dim>;

  //! The set of types that will be used for input resolution
  using types = std::tuple<real_t,
                           std::string,
                           arr_d_r_t,
                           arr_d_s_t,
                           size_t
                           ,ics_function_t
                           ,bcs_function_t
                           >;

}; // flecsale_input_traits

using input_engine = ristra::input_engine_t<flecsale_input_traits>;

template <typename T> using init_value = ristra::init_value_t<T, input_engine>;

} // apps::common


// End of file
