/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simulation configuration for sample applications.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "flecsale/eos/eos_base.h"
#include "flecsale/eos/ideal_gas.h"
#include "flecsale/mesh/burton/burton.h"
#include "flecsale/mesh/factory.h"
#include "input_types.h"

#include <memory>
#include <tuple>

namespace apps::hydro{

struct SimConfig{
  // types and class constants
  using input_traits = apps::hydro::input_traits;
  static constexpr auto dim = input_traits::dim;

  using real_t = input_traits::real_t;
  using string_t = input_traits::string_t;
  using eos_t = flecsale::eos::eos_base_t<real_t>;
  using eos_ptr_t = std::unique_ptr<eos_t>;
  using mesh_t = flecsale::mesh::burton::burton_mesh_t<dim>;
  using mesh_ptr_t = std::unique_ptr<mesh_t>;
  using ics_function_t = input_traits::ics_function_t;
  using bcs_function_t = input_traits::bcs_function_t;
  using bcs_list_t = input_traits::bcs_list_t;

  explicit SimConfig(apps::hydro::input_engine &inputs) : m_inputs(inputs) {}

  /**\brief Construct an EOS object wrapped in unique_ptr.
   *
   * Currently implemented only for ideal_gas
   */
  eos_ptr_t get_eos() {
    using ideal_t = flecsale::eos::ideal_gas_t<real_t>;
    string_t eos_type(m_inputs.get_value<string_t>("eos_type"));
    if ("ideal_gas" == eos_type) {
      real_t const g = m_inputs.get_value<real_t>("gas_constant");
      real_t const cv = m_inputs.get_value<real_t>("specific_heat");
      eos_ptr_t peos = std::make_unique<ideal_t>(g, cv);
      return std::move(peos);
    }
    raise_implemented_error("Unknown eos type \"" + eos_type + "\"");
  } // get_eos

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
      input_traits::bcs_ptr_t bc_object(make_boundary_condition<dim>(bc_type));
      bcs.push_back( std::make_pair(bc_object, bc_func));
    }
    return std::move(bcs);
  }

  /**\brief Construct a box mesh, or read a file, or just quit */
  mesh_t make_mesh(real_t /* sim_time */) {
    string_t const mesh_type(m_inputs.get_value<string_t>("mesh_type"));
    if ("box" == mesh_type) {
      using arrayr_t = input_traits::arr_d_r_t;
      using arrays_t = input_traits::arr_d_s_t;
      arrayr_t const xmin = m_inputs.get_value<arrayr_t>("xmin");
      arrayr_t const xmax = m_inputs.get_value<arrayr_t>("xmax");
      arrays_t const dimensions = m_inputs.get_value<arrays_t>("dimensions");
      return flecsale::mesh::box<mesh_t>(dimensions, xmin, xmax);
    }
    else if ("read" == mesh_type) {
      string_t const mesh_file(m_inputs.get_value<string_t>("file"));
      mesh_t m;
      flecsale::mesh::read_mesh(mesh_file, m);
      return m;
    }
    raise_implemented_error("Unknown mesh type \"" + mesh_type + "\"");
  } // get_mesh

private:
  apps::hydro::input_engine &m_inputs;
}; // SimConfig

} // apps::hydro::

// End of file
