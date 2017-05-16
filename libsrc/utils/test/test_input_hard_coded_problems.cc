// test_input_hard_coded_problems.cc
// T. M. Kelley
// May 10, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#include "test_input_hard_coded_problems.h"

namespace aletest{

template <class T> using registry = spec_t::registry<T>;

spec_t
make_mock_box_2d(){
  constexpr uint32_t dim(2u);
  using string_t = spec_t::string_t;
  using real_t = spec_t::real_t;
  using vec2r_t = std::array<real_t,dim>;
  using vec2s_t = std::array<real_t,dim>;
  using ics_f_t = spec_t::ics_function_t<dim>;
  spec_t hc;

  registry<real_t> &real_t_reg = hc.get_registry<real_t>();
  real_t_reg["final_time"] = 0.2;
  real_t_reg["CFL"] = 0.5;
  real_t_reg["gas_constant"] = 1.4;
  real_t_reg["specific_heat"] = 1.0;

  registry<size_t> &size_t_reg = hc.get_registry<size_t>();
  size_t_reg["max_steps"] = 1e6;
  size_t_reg["output_freq"] = 7;

  registry<string_t> &string_t_reg = hc.get_registry<string_t>();
  string_t_reg["prefix"] = "mock_box_2d";
  string_t_reg["suffix"] = "dat";
  string_t_reg["mesh_type"] = "box";
  string_t_reg["eos_type"] = "ideal_gas";
  string_t_reg["file"] = "shouldn't come to this";

  registry<vec2r_t> &vec2r_t_reg = hc.get_registry<vec2r_t>();
  vec2r_t_reg["xmin"] = {-0.5,-0.5};
  vec2r_t_reg["xmax"] = {0.5,0.5};

  registry<vec2s_t> &vec2s_t_reg = hc.get_registry<vec2s_t>();
  vec2s_t_reg["dimensions"] = {10,10};

  return hc;
}

} // aletest::


// End of file
