// base_problem.h
// Sep 01, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#pragma once

#include "ristra/input_source.h"
#include "../types.h"

#include <memory>

namespace apps::hydro{

template <class T> using reg = ristra::hard_coded_source_t::registry<T>;

inline apps::hydro::input_traits::ics_return_t
ics_func(apps::hydro::input_traits::vector_t const &x,
         apps::hydro::input_traits::real_t const &t) {
  apps::hydro::input_traits::real_t d, p;
  apps::hydro::input_traits::vector_t v;
  bool all_neg(true);
  for(size_t i = 0; i < apps::hydro::input_traits::dim; ++i){
    v[i] = 0.0;
    all_neg = all_neg && x[i] < 0.0;
  }
  if (all_neg) {
    d = 0.125;
    p = 0.1;
  }
  else {
    d = 1.0;
    p = 1.0;
  }
  return std::make_tuple(d, v, p);
} // ics_func_3d

/**\brief Create a hard-coded problem to provide defaults.
 *
 * \return unique_ptr<ristra::hard_coded_test_problem> */
struct base_problem_3d{
  inline
  ristra::hard_coded_source_ptr_t
  operator()() {
    using input_traits = apps::hydro::input_traits;
    static uint8_t constexpr dim = input_traits::dim;

    using arrayr_t = input_traits::arr_d_r_t;
    using arrays_t = input_traits::arr_d_s_t;
    using string_t = input_traits::string_t;
    using ics_function_t = input_traits::ics_function_t;

    ristra::hard_coded_source_ptr_t phcs(ristra::mk_hard_coded_source());
    ristra::hard_coded_source_t &hcs(*phcs);
    // register real_t defaults
    reg<real_t> const real_reg = {{"CFL", 1.0 / 3.0},
                                  {"final_time", 0.2},
                                  {"gas_constant", 1.4},
                                  {"specific_heat", 1.0}};
    hcs.set_registry<real_t>(real_reg);
    // register size_t defaults
    reg<size_t> const size_reg = {{"output_freq", 1lu}, {"max_steps", 1e6}};
    hcs.set_registry<size_t>(size_reg);
    // register string_t defaults
    reg<string_t> const string_reg = {{"eos_type", "ideal_gas"},
                                      {"prefix", "shock_box_3d"},
                                      {"suffix", "dat"},
                                      {"mesh_type", "box"},
                                      {"file", "zzxyzz"}};
    hcs.set_registry<string_t>(string_reg);
    // register array<size_t> defaults
    reg<arrays_t> const arrays_reg = {{"dimensions", {10, 10, 10}}};
    hcs.set_registry<arrays_t>(arrays_reg);
    // register array<real_t> defaults
    reg<arrayr_t> const arrayr_reg = {{"xmin", {-0.5, -0.5, -0.5}},
                                        {"xmax", {0.5, 0.5, 0.5}}};
    hcs.set_registry<arrayr_t>(arrayr_reg);
    // register ics_function default
    hcs.set_registry<ics_function_t>({{"ics_func", ics_func}});
    // transfer to caller
    return std::move(phcs);
  }
}; // base_problem_3d


} // apps::hydro

// End of file
