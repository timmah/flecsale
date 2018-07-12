// base_problem.h
// Sep 01, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#pragma once

#include "ristra/initialization/input_source.h"
#include "../types.h"

#include <memory>

namespace apps::hydro{

template <class T> using reg = ristra::hard_coded_source_t::registry<T>;

// the grid dimensions
constexpr size_t num_cells_x = 32;
constexpr size_t num_cells_y = 32;
constexpr real_t length_x = 1.2;
constexpr real_t length_y = 1.2;

inline apps::common::ics_return_t
ics_func(apps::common::vector_t const &x,
         apps::common::real_t const &t) {
  apps::common::real_t d, p;
  apps::common::vector_t v;
  bool all_neg(true);
  for(size_t i = 0; i < flecsale_input_traits::dim; ++i){
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
} // ics_func_2d

inline bool bc_function_1(apps::common::vector_t const &x,
                          apps::common::real_t const & /*t*/) {
  return (0.0 == x[0] || length_x == x[0]);
}

inline bool bc_function_2(apps::common::vector_t const &x,
                          apps::common::real_t const & /*t*/) {
  return (0.0 == x[1] || length_y == x[1]);
}

/**\brief Create a hard-coded problem to provide defaults.
 *
 * \return unique_ptr<ristra::hard_coded_test_problem> */
struct base_problem_2d{

  ristra::hard_coded_source_ptr_t
  operator()() {
    using namespace apps::common;
    static uint8_t constexpr dim = flecsale_input_traits::dim;

    ristra::hard_coded_source_ptr_t phcs(ristra::mk_hard_coded_source());
    ristra::hard_coded_source_t &hcs(*phcs);
    // register real_t defaults
    reg<real_t> const real_reg = {{"time_constant_acoustic", 0.25},
                                  {"time_constant_volume",0.1},
                                  {"time_constant_growth",1.01},
                                  {"final_time", 1.0},
                                  {"gas_constant", 1.4},
                                  {"specific_heat", 1.0},
                                  {"initial_time_step",1.0e-5}};
    hcs.set_registry<real_t>(real_reg);
    // register size_t defaults
    reg<size_t> const size_reg = {
      {"output_freq", 20 },
      {"max_steps", 20}};
    hcs.set_registry<size_t>(size_reg);
    // register string_t defaults
    reg<string_t> const string_reg = {{"eos_type", "ideal_gas"},
                                      {"prefix", "sedov_2d"},
                                      {"suffix", "dat"},
                                      // {"mesh_type", "box"},
                                      {"bc_type_1","symmetry"},
                                      {"bc_type_2","symmetry"},
                                      {"file", "zzxyzz"}};
    hcs.set_registry<string_t>(string_reg);
    // // register array<size_t> defaults
    // reg<arrays_t> const arrays_reg = {{"dimensions", {10, 10}}};
    // hcs.set_registry<arrays_t>(arrays_reg);
    // // register array<real_t> defaults
    // reg<arrayr_t> const arrayr_reg = {{"xmin", {-0.5, -0.5}},
    //                                     {"xmax", {0.5, 0.5}}};
    // hcs.set_registry<arrayr_t>(arrayr_reg);
    // register ics_function default
    hcs.set_registry<ics_function_t>({{"ics_func", ics_func}});

    hcs.set_registry<bcs_function_t>({{"bc_function_1",bc_function_1}
                                     ,{"bc_function_2",bc_function_2}
                                    });
    // transfer to caller
    return std::move(phcs);
  } // operator()

}; // base_problem_2d


} // apps::hydro

// End of file
