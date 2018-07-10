// application_init.h
// T. M. Kelley
// Jun 28, 2018
// (c) Copyright 2018 LANSLLC, all rights reserved

#pragma once

#include "input_types.h"
#include "parse_arguments.h"
#include "sim_config.h"

#include <flecsi/execution/execution.h>

#include <string>

namespace flecsi_sp::burton{

flecsi_register_global_object(0, config, apps::common::Sim_Config);

/**\brief Setup and call parse_arguments.
 *\return 0 if h called, 1 if ? called, 2 otherwise.
 */
inline
int process_args(int argc, char ** argv, apps::common::args_map_t &arg_map){
  //===========================================================================
  // Parse arguments
  //===========================================================================

  // the usage stagement
  auto print_usage = [&argv]() {
    std::cout << "Usage: " << argv[0]
              << " [--file INPUT_FILE]"
              << " [--catalyst PYTHON_SCRIPT]"
              << " [--help]"
              << std::endl << std::endl;
    std::cout << "\t--file INPUT_FILE:\t Override the input file "
              << "with INPUT_FILE." << std::endl;
    std::cout << "\t--catalyst PYTHON_SCRIPT:\t Load catalyst with "
              << "using PYTHON_SCRIPT." << std::endl;
    std::cout << "\t--help:\t Print a help message." << std::endl;
  };

  // Define the options
  struct option long_options[] =
    {
      {"help",           no_argument, 0, 'h'},
      {"file",     required_argument, 0, 'f'},
      {"catalyst", required_argument, 0, 'c'},
      {0, 0, 0, 0}
    };
  const char * short_options = "hf:c:";
  // parse the arguments
  arg_map =
      apps::common::parse_arguments(argc, argv, long_options, short_options);

  // process the simple ones
  if ( arg_map.count("h") ) {
    print_usage();
    return 0;
  }
  else if ( arg_map.count("?") ) {
    print_usage();
    return 1;
  }
  return 2;
}

/** \brief Application top-level task initialization
 *
 * This function sets a number of initialization values that must be resolved
 * in order for any flecsale app to run. It can resolve those init_values
 * from either a hard coded base problem--thus the base_problem_t template
 * parameter--or from a Lua file provided at runtime.
 *
 *\tparam base_problem_t: struct that can provide a registry when called. Since
 *the base problems vary from app to app, it is used as a parameter.
 *
 * Note: this function is declared in burton_specialization_init.h
 */
template <class base_problem_t>
void application_tlt_init_templ(int argc, char **argv){
  using namespace apps::common;

  // create and initialize input_engine
  Sim_Config::input_engine_ptr_t sp_ie =
      std::make_shared<apps::common::input_engine>();
  Sim_Config::input_engine_t &inputs(*sp_ie);
  // -------------------------------------------------------------------------
  // configure inputs
  // -------------------------------------------------------------------------
  // real_t targets
  init_value<real_t> iv_CFL("CFL");
  init_value<real_t> iv_final_time("final_time");
  init_value<real_t> iv_gas_constant("gas_constant");
  init_value<real_t> iv_specific_heat("specific_heat");
  // size_t targets
  init_value<size_t> iv_output_freq("output_freq");
  init_value<size_t> iv_max_steps("max_steps");
  // string targets
  init_value<string_t> iv_file_prefix("prefix");
  init_value<string_t> iv_file_suffix("suffix");
  // init_value<string_t> iv_mesh_type("mesh_type");
  init_value<string_t> iv_eos_type("eos_type");
  init_value<string_t> iv_file("file");
  // // array<real_t,dim> targets
  // init_value<flecsale_input_traits::arr_d_r_t> iv_xmin("xmin");
  // init_value<flecsale_input_traits::arr_d_r_t> iv_xmax("xmax");
  // // array<size_t,dim> targets
  // init_value<flecsale_input_traits::arr_d_s_t> iv_dimensions("dimensions");
  // initial conditions functions
  init_value<Sim_Config::ics_function_t> iv_ics_func("ics_func");

  // -------------------------------------------------------------------------
  // register inputs sources
  // -------------------------------------------------------------------------
  base_problem_t bp;
  auto phcs(bp());
  inputs.register_hard_coded_source(phcs.release());

  // -------------------------------------------------------------------------
  // process command line arguments, exiting if user requested help
  // -------------------------------------------------------------------------
  args_map_t args;
  int arg_ret = process_args(argc, argv, args);
  if(arg_ret < 2){
    printf("%s:%i Early termination\n",__FUNCTION__,__LINE__);
    return;
  }

  // -------------------------------------------------------------------------
  // get the input filename, if any
  // -------------------------------------------------------------------------
  auto input_file_name = args.count("f") ? args.at("f") : std::string();
  // if not empty, attempt to use Lua file
  if ( !input_file_name.empty() ) {
    std::cout << "Using input file \"" << input_file_name << "\"."
              << std::endl;
    // inputs_t::load( input_file_name );
    ristra::lua_source_ptr_t plua(ristra::mk_lua(input_file_name));
    inputs.register_lua_source(plua.release());
  }
  // -------------------------------------------------------------------------
  // resolve input values
  // -------------------------------------------------------------------------
  bool all_resolved = inputs.resolve_inputs();

  // -------------------------------------------------------------------------
  // transfer to simulation configuration
  // -------------------------------------------------------------------------
  flecsi_initialize_global_object(0, config, Sim_Config);

  auto p_sim_config = flecsi_get_global_object(0,config,Sim_Config);
  Sim_Config &sim_config(*p_sim_config);

  sim_config.set_initial_conditions_function(iv_ics_func.get());

  sim_config.set_CFL(iv_CFL.get());
  sim_config.set_final_time(iv_final_time.get());
  sim_config.set_gas_constant(iv_gas_constant.get());
  sim_config.set_specific_heat(iv_specific_heat.get());

  sim_config.set_output_freq(iv_output_freq.get());
  sim_config.set_max_steps(iv_max_steps.get());

  sim_config.set_file_prefix(iv_file_prefix.get());
  sim_config.set_file_suffix(iv_file_suffix.get());
  // sim_config.set_mesh_type(iv_mesh_type.get());
  sim_config.set_eos_type(iv_eos_type.get());
  sim_config.set_file(iv_file.get());

  return;
} // application_tlt_init_templ

} //

namespace apps::common{

// struct Sim_Config{
// explicit Sim_Config(std::string const &s):m_string(s) {}

// std::string hoopla() const { return m_string; }

// std::string m_string;
// };

}

// End of file
