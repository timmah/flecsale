/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Hard-coded defaults for maire-hydro 2D, matches sedov_2d.lua
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ristra/input_source.h"
#include "input_types.h"
#include <memory>

namespace apps::hydro{

template <class T> using reg = ristra::hard_coded_source_t::registry<T>;

namespace detail {
  constexpr input_traits::arr_d_r_t length = {1.2,1.2,1.2};
  constexpr input_traits::arr_d_s_t num_cells = {20,20,20};
}

// This is the Sedov 2D Lua initial condition
inline input_traits::ics_return_t
ics_func(input_traits::vector_t const &x,
         input_traits::real_t const &t) {
  using real_t = input_traits::real_t;
  using arr_d_r_t = input_traits::arr_d_r_t;
  using arr_d_s_t = input_traits::arr_d_s_t;

  constexpr real_t e0 = 0.244816;
  constexpr real_t gamma = 1.4;
  constexpr arr_d_r_t dx = {detail::length[0] / detail::num_cells[0],
                            detail::length[1] / detail::num_cells[1],
                            detail::length[2] / detail::num_cells[2],
                          };
  constexpr real_t delta_r = std::sqrt(
      1.0e-12 + 0.25 * (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2]));
  constexpr real_t delta_vol = dx[0] * dx[1];
  constexpr real_t d = 1.0;
  constexpr real_t p_small_r = (gamma - 1.0) * d * e0 / delta_vol;
  constexpr real_t p_large_r = 1.0e-6;

  real_t sum_sq = 0.0;
  arr_d_r_t v;
  for(size_t i = 0; i < input_traits::dim; ++i){
    v[i] = 0.0;
    sum_sq += x[i] * x[i];
  }
  real_t const r = std::sqrt(sum_sq);
  real_t const p = r < delta_r ? p_small_r : p_large_r;
  return std::make_tuple(d, v, p);
} // ics_func_2d

bool bc_function_1(input_traits::vector_t const &x,
                   input_traits::real_t const &/*t*/){
  if(0.0 == x[0] || detail::length[0] == x[0]) return true;
  return false;
}

bool bc_function_2(input_traits::vector_t const &x,
                   input_traits::real_t const &/*t*/){
  if(0.0 == x[1] || detail::length[1] == x[1]) return true;
  return false;
}

bool bc_function_3(input_traits::vector_t const &x,
                   input_traits::real_t const &/*t*/){
  if(0.0 == x[2] || detail::length[2] == x[2]) return true;
  return false;
}

/**\brief Create a hard-coded problem to provide defaults.
 *
 * \return unique_ptr<ristra::hard_coded_test_problem> */
inline ristra::hard_coded_source_ptr_t base_problem_3d() {
  using input_traits = apps::hydro::input_traits;
  static uint8_t constexpr dim = input_traits::dim;

  using arrayr_t = input_traits::arr_d_r_t;
  using arrays_t = input_traits::arr_d_s_t;
  using string_t = input_traits::string_t;
  using real_t = input_traits::real_t;
  using ics_function_t = input_traits::ics_function_t;
  using bcs_function_t = input_traits::bcs_function_t;

  ristra::hard_coded_source_ptr_t phcs(ristra::mk_hard_coded_source());
  ristra::hard_coded_source_t &hcs(*phcs);
  // register real_t defaults
  reg<real_t> const real_reg = {{"CFL_acoustic", 0.25},
                                {"CFL_volume",0.1},
                                {"CFL_growth",1.01},
                                {"final_time", 1.0},
                                {"initial_time_step",1.0e-5},
                                {"gas_constant", 1.4},
                                {"specific_heat", 1.0}};
  hcs.set_registry<real_t>(real_reg);
  // register size_t defaults
  reg<size_t> const size_reg = {{"output_freq", 20u}, {"max_steps", 20u}};
  hcs.set_registry<size_t>(size_reg);
  // register string_t defaults
  reg<string_t> const string_reg = {{"eos_type", "ideal_gas"},
                                    {"prefix", "sedov_3d"},
                                    {"suffix", "dat"},
                                    {"mesh_type", "box"}
                                   ,{"bc_type_1","symmetry"}
                                   ,{"bc_type_2","symmetry"}
                                   ,{"bc_type_3","symmetry"}
                                  };
  hcs.set_registry<string_t>(string_reg);
  // register array<size_t> defaults
  reg<arrays_t> const arrays_reg = {{"dimensions",detail::num_cells}};
  hcs.set_registry<arrays_t>(arrays_reg);
  // register array<real_t> defaults
  reg<arrayr_t> const arrayr_reg = {{"xmin", {0.0,0.0,0.0}},
                                      {"xmax", detail::length}};
  hcs.set_registry<arrayr_t>(arrayr_reg);
  // register default initial condition, boundary condition functions
  hcs.set_registry<ics_function_t>({{"ics_func", ics_func}});
  hcs.set_registry<bcs_function_t>({{"bc_function_1",bc_function_1}
                                   ,{"bc_function_2",bc_function_2}
                                   ,{"bc_function_3",bc_function_3}
                                  });
  // transfer to caller
  return std::move(phcs);
} // base_problem_2d


} // apps::hydro

// End of file
