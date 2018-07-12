// application_init.cc
// T. M. Kelley
// Jun 28, 2018
// (c) Copyright 2018 LANSLLC, all rights reserved

#include "application_init.h"
#include "ristra/initialization/init_value.h"

namespace flecsi_sp::burton{

using apps::common::Sim_Config;

// flecsi_register_global_object(0, config, Sim_Config);
//
//template <class base_problem_t>
//void application_tlt_init(int argc, char ** argv){
//  // using apps::common::init_value;
//  using namespace apps::common;
//  // create and initialize input_engine
//  Sim_Config::input_engine_ptr_t sp_ie =
//      std::make_shared<apps::common::input_engine>();
//  // configure inputs
//  // real_t targets
//  init_value<real_t> iv_CFL("CFL");
//  init_value<real_t> iv_final_time("final_time");
//  init_value<real_t> iv_gas_constant("gas_constant");
//  init_value<real_t> iv_specific_heat("specific_heat");
//  // size_t targets
//  init_value<size_t> iv_output_freq("output_freq");
//  init_value<size_t> iv_max_steps("max_steps");
//  // string targets
//  init_value<string_t> iv_prefix("prefix");
//  init_value<string_t> iv_suffix("suffix");
//  init_value<string_t> iv_mesh_type("mesh_type");
//  init_value<string_t> iv_eos_type("eos_type");
//  init_value<string_t> iv_file("file");
//  // array<real_t,dim> targets
//  init_value<flecsale_input_traits::arr_d_r_t> iv_xmin("xmin");
//  init_value<flecsale_input_traits::arr_d_r_t> iv_xmax("xmax");
//  // array<size_t,dim> targets
//  init_value<flecsale_input_traits::arr_d_s_t> iv_dimensions("dimensions");
//  // initial conditions functions
//  init_value<Sim_Config::ics_function_t> iv_ics_func("ics_func");
//
//  // register inputs sources
//  base_problem bp;
//  auto phcs(bp());
//  inputs.register_hard_coded_source(phcs.release());
//  // get the input filename
//  auto input_file_name = args.count("f") ? args.at("f") : std::string();
//  // if not empty, attempt to use Lua file
//  if ( !input_file_name.empty() ) {
//    std::cout << "Using input file \"" << input_file_name << "\"."
//              << std::endl;
//    // inputs_t::load( input_file_name );
//    ristra::lua_source_ptr_t plua(ristra::mk_lua(input_file_name));
//    inputs.register_lua_source(plua.release());
//  }
//  bool all_resolved = inputs.resolve_inputs();
//
//  flecsi_initialize_global_object(0, config, Sim_Config);
//
//  auto sim_config = flecsi_get_global_object(0,config,Sim_Config);
//  sim_config.set_eos_type(iv_eos_type.get());
//  sim_config.set_initial_conditions_function(iv_ics_func.get());
//  sim_config.set_gas_constant(iv_gas_constant.get());
//  sim_config.set_specific_heat(iv_specific_heat.get());
//
//  return;
//} // application_tlt_init

} //



// End of file
