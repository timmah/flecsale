/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simulation configuration for sample applications.
////////////////////////////////////////////////////////////////////////////////

#pragma once

// common includes
#include "input_types.h"
#include "types.h"

// flecsale/flecsi includes
#include "flecsale/eos/eos_base.h"
#include "flecsale/eos/ideal_gas.h"
// #include "flecsi-sp/burton/burton_mesh_factory.h"

// standard includes
#include <memory>
#include <tuple>

namespace apps::common{

struct Sim_Config{
  // types and class constants
private:
  using input_traits = apps::common::flecsale_input_traits;
  static constexpr auto dim = input_traits::dim;
  using real_t = input_traits::real_t;
  using string_t = input_traits::string_t;
  using mesh_t = apps::common::mesh_t;
  using bcs_function_t = apps::common::bcs_function_t;

  // Public types
public:
  using eos_t = apps::common::eos_t;
  using eos_ptr_t = std::unique_ptr<eos_t>;
  using bcs_list_t = apps::common::bcs_list_t;
  using mesh_ptr_t = std::shared_ptr<mesh_t>;
  using input_engine_t = apps::common::input_engine;
  using input_engine_ptr_t = std::shared_ptr<input_engine_t>;
  using ics_function_t = apps::common::ics_function_t;

  // Public interface

  //! Ctor
  explicit Sim_Config() {
    // Need to ensure the engine is configured, then run it to resolve inputs.
    // Also need to register initial values?

  }

  /**\brief Get an initial conditions function */
  ics_function_t get_initial_conditions_function() const { return m_ics; }

  /**\brief Set the initial conditions function */
  void set_initial_conditions_function(ics_function_t ics) { m_ics = ics; }

  string_t get_eos_type() const { return m_eos_type; }

  string_t get_file_prefix() const { return m_prefix; }

  string_t get_file_suffix() const { return m_suffix; }

  string_t get_file() const { return m_file; }

  size_t get_output_freq() const { return m_output_freq; }

  size_t get_max_steps() const { return m_max_steps; }

  real_t get_final_time() const { return m_final_time; }

  real_t get_CFL() const { return m_CFL; }

  void set_eos_type(string_t const& e_t) { m_eos_type = e_t; }

  void set_gas_constant(real_t const v) { m_gas_constant = v; }

  void set_specific_heat(real_t const v) { m_specific_heat = v; }

  void set_file_prefix(string_t const &p) { m_prefix = p; }

  void set_file_suffix(string_t const &p) { m_suffix = p; }

  void set_output_freq(size_t const f) { m_output_freq = f; }

  void set_max_steps(size_t const m) { m_max_steps = m; }

  void set_final_time( real_t const t) { m_final_time = t; }

  void set_CFL( real_t const cfl) { m_CFL = cfl; }

  void set_file( string_t const &file) { m_file = file; }

  /**\brief Construct an EOS object wrapped in unique_ptr.
   *
   * Currently implemented only for ideal_gas
   */
  eos_ptr_t get_eos() {
    using ideal_t = flecsale::eos::ideal_gas_t<real_t>;
    if ("ideal_gas" == m_eos_type) {
      eos_ptr_t peos =
          std::make_unique<ideal_t>(m_gas_constant, m_specific_heat);
      return std::move(peos);
    }
    // std::set<string_t> known_eoses{"ideal_gas"};
    // OneOf(eos_type, known_eoses);
    return nullptr;
  } // get_eos

  /*!\brief Get the list of boundary conditions functions */
  template <typename bcs_func_t>
  bcs_list_t get_bcs(
    std::vector<init_value<string_t>> &iv_bcs_types,
    std::vector<init_value<bcs_func_t>> &iv_bcs_funcs
    ){
    Require(iv_bcs_funcs.size() == dim,"size of vector != dim");
    Require(iv_bcs_types.size() == dim,"size of vector != dim");
    bcs_list_t bcs;
    for(auto i = 0; i < dim; ++i){
      string_t bc_type = iv_bcs_types[i].get();
      bcs_func_t bc_func = iv_bcs_funcs[i].get();
      bcs_ptr_t bc_object(make_boundary_condition(bc_type));
      bcs.push_back( std::make_pair(bc_object, bc_func));
    }
    return std::move(bcs);
  }

  // /**\brief Construct a box mesh, or read a file, or just quit */
  // mesh_ptr_t make_mesh(real_t /* sim_time */) {
  //   string_t const mesh_type(m_inputs->get_value<string_t>("mesh_type"));
  //   // std::set<string_t> mesh_options{"box","read"};
  //   // OneOf(mesh_type, mesh_options);
  //   if ("box" == mesh_type) {
  //     using arrayr_t = input_traits::arr_d_r_t;
  //     using arrays_t = input_traits::arr_d_s_t;
  //     arrayr_t const xmin = m_inputs->get_value<arrayr_t>("xmin");
  //     arrayr_t const xmax = m_inputs->get_value<arrayr_t>("xmax");
  //     arrays_t const dimensions = m_inputs->get_value<arrays_t>("dimensions");
  //     return flecs_sp::burton::ptr_box<mesh_t>(dimensions, xmin, xmax);
  //   }
  //   else if ("read" == mesh_type) {
  //     string_t const mesh_file(m_inputs->get_value<string_t>("file"));
  //     auto m = std::make_shared<mesh_t>();
  //     flecsale::mesh::read_mesh(mesh_file, *m);
  //     return m;
  //   }
  // } // get_mesh

private:
  ics_function_t m_ics;

  string_t m_eos_type;
  string_t m_prefix;
  string_t m_suffix;
  string_t m_file;

  real_t m_gas_constant;
  real_t m_specific_heat;
  real_t m_final_time;
  real_t m_CFL;

  size_t m_output_freq;
  size_t m_max_steps;
}; // Sim_Config

} // apps::common::

// End of file
