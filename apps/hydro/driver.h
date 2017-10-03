/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief This is the main driver for the hydro solver.
///////////////////////////////////////////////////////////////////////////////

#pragma once

// hydro includes
#include "../common/exceptions.h"
#include "../common/parse_arguments.h"
#include "sim_config.h"
#include "input_types.h"
#include "ristra/init_value.h"
#include "ristra/input_source.h"
#include "types.h"

// user includes
#include <flecsale/mesh/mesh_utils.h>
#include <flecsale/utils/time_utils.h>
#include <flecsale/io/catalyst/adaptor.h>


// system includes
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace apps {
namespace hydro {

///////////////////////////////////////////////////////////////////////////////
//! \brief A sample test of the hydro solver
///////////////////////////////////////////////////////////////////////////////
template< typename inputs_t, typename base_problem>
int driver(int argc, char** argv)
{
  using real_t = typename inputs_t::real_t;
  using string_t = typename inputs_t::string_t;

  // set exceptions
  enable_exceptions();

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
  auto args = parse_arguments(argc, argv, long_options, short_options);

  // process the simple ones
  if ( args.count("h") ) {
    print_usage();
    return 0;
  }
  else if ( args.count("?") ) {
    print_usage();
    return 1;
  }

  apps::hydro::input_engine inputs;
  // configure inputs
  // real_t targets
  init_value<real_t> iv_CFL("CFL");
  init_value<real_t> iv_final_time("final_time");
  init_value<real_t> iv_gas_constant("gas_constant");
  init_value<real_t> iv_specific_heat("specific_heat");
  // size_t targets
  init_value<size_t> iv_output_freq("output_freq");
  init_value<size_t> iv_max_steps("max_steps");
  // string targets
  init_value<typename inputs_t::string_t> iv_prefix("prefix");
  init_value<typename inputs_t::string_t> iv_suffix("suffix");
  init_value<typename inputs_t::string_t> iv_mesh_type("mesh_type");
  init_value<typename inputs_t::string_t> iv_eos_type("eos_type");
  init_value<typename inputs_t::string_t> iv_file("file");
  // array<real_t,dim> targets
  init_value<apps::hydro::input_traits::arr_d_r_t> iv_xmin("xmin");
  init_value<apps::hydro::input_traits::arr_d_r_t> iv_xmax("xmax");
  // array<size_t,dim> targets
  init_value<apps::hydro::input_traits::arr_d_s_t> iv_dimensions("dimensions");
  // initial conditions functions
  init_value<SimConfig::ics_function_t> iv_ics_func("ics_func");

  // register inputs sources
  base_problem bp;
  auto phcs(bp());
  inputs.register_hard_coded_source(phcs.release());
  // get the input filename
  auto input_file_name = args.count("f") ? args.at("f") : std::string();
  // if not empty, attempt to use Lua file
  if ( !input_file_name.empty() ) {
    std::cout << "Using input file \"" << input_file_name << "\"."
              << std::endl;
    // inputs_t::load( input_file_name );
    ristra::lua_source_ptr_t plua(ristra::mk_lua(input_file_name));
    inputs.register_lua_source(plua.release());
  }
  bool all_resolved = inputs.resolve_inputs();
  if(!all_resolved){
    // there must be a better way...
    raise_runtime_error("Failed to resolve required inputs!");
  }
  // With input resolution complete, now make configuration data available
  real_t const CFL         = iv_CFL.get();
  size_t const output_freq = iv_output_freq.get();
  string_t const prefix    = iv_prefix.get();
  string_t const suffix    = iv_suffix.get();
  size_t const max_steps   = iv_max_steps.get();
  real_t const final_time  = iv_final_time.get();

  apps::hydro::SimConfig simcfg(inputs);

  // get the catalyst arguments
  auto catalyst_args =
    args.count("c") ? args.at("c") : std::string();
  // split them up into a list
  auto catalyst_scripts = utils::split( catalyst_args, {';'} );

  if ( !catalyst_args.empty() ) {
    std::cout << "Using catalyst args \"" << catalyst_args << "\"."
              << std::endl;
  }

  //===========================================================================
  // Mesh Setup
  //===========================================================================
  // make the mesh
  SimConfig::mesh_t mesh = simcfg.make_mesh( /* solution time */ 0.0 );

  // this is the mesh object
  bool mesh_ok = mesh.is_valid(false);
  Insist(mesh_ok,"mesh not ok");

  cout << mesh;

  //===========================================================================
  // Some typedefs
  //===========================================================================

  using mesh_t = decltype(mesh);
  using vector_t = typename mesh_t::vector_t;

  // get machine zero
  constexpr auto epsilon = std::numeric_limits<real_t>::epsilon();
  const auto machine_zero = std::sqrt(epsilon);

  // the maximum number of retries
  constexpr int max_retries = 5;

  //===========================================================================
  // Field Creation
  //===========================================================================

  // start the timer
  auto tstart = utils::get_wall_time();

  // type aliases
  using eqns_t = eqns_t<mesh_t::num_dimensions>;
  using flux_data_t = flux_data_t<mesh_t::num_dimensions>;

  // create some field data.  Fields are registered as struct of arrays.
  // this allows us to access the data in different patterns.
  flecsi_register_data(mesh, hydro,  density,   real_t, dense, 2, cells);
  flecsi_register_data(mesh, hydro, pressure,   real_t, dense, 1, cells);
  flecsi_register_data(mesh, hydro, velocity, vector_t, dense, 2, cells);

  flecsi_register_data(mesh, hydro, internal_energy, real_t, dense, 2, cells);
  flecsi_register_data(mesh, hydro,     temperature, real_t, dense, 1, cells);
  flecsi_register_data(mesh, hydro,     sound_speed, real_t, dense, 1, cells);

  // set these variables as persistent for plotting
  flecsi_get_accessor(mesh, hydro,  density,   real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro, pressure,   real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro, velocity, vector_t, dense, 0).attributes().set(persistent);

  flecsi_get_accessor(mesh, hydro, internal_energy, real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro,     temperature, real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro,     sound_speed, real_t, dense, 0).attributes().set(persistent);

  // compute the fluxes.  here I am regestering a struct as the stored data
  // type since I will only ever be accesissing all the data at once.
  flecsi_register_data(mesh, hydro, flux, flux_data_t, dense, 1, faces);

  // register the time step and set a cfl
  flecsi_register_data( mesh, hydro, time_step, real_t, global, 1 );
  flecsi_register_data( mesh, hydro, cfl, real_t, global, 1 );
  *flecsi_get_accessor( mesh, hydro, cfl, real_t, global, 0) = CFL;

  // Register the total energy
  flecsi_register_data( mesh, hydro, sum_total_energy, real_t, global, 1 );

  //===========================================================================
  // Initial conditions
  //===========================================================================

  // now call the main task to set the ics.  Here we set primitive/physical
  // quanties
  SimConfig::ics_function_t ics(
    inputs.get_value<SimConfig::ics_function_t>("ics_func"));
  flecsi_execute_task( initial_conditions_task, loc, single, mesh, ics);

  #ifdef HAVE_CATALYST
    auto insitu = io::catalyst::adaptor_t(catalyst_scripts);
    std::cout << "Catalyst on!" << std::endl;
  #endif

  // Update the EOS
  flecsi_execute_task(
    update_state_from_pressure_task, loc, single, mesh, simcfg.get_eos().get()
  );

  //===========================================================================
  // Pre-processing
  //===========================================================================


  // now output the solution
  if (output_freq > 0){
    output(mesh, prefix, suffix, 1);
  }

  //===========================================================================
  // Residual Evaluation
  //===========================================================================

  // get the current time
  auto soln_time = mesh.time();
  auto time_cnt  = mesh.time_step_counter();

  // get an accessor for the time step
  auto time_step = flecsi_get_accessor( mesh, hydro, time_step, real_t, global, 0 );

  // a counter for this session
  size_t num_steps = 0;

  for ( size_t num_retries = 0;
    (num_steps < max_steps && soln_time < final_time);
    ++num_steps
  ) {

    if (num_retries == 0)
      flecsi_execute_task( save_solution_task, loc, single, mesh );

    // compute the time step
    flecsi_execute_task( evaluate_time_step_task, loc, single, mesh );

    // access the computed time step and make sure its not too large
    *time_step = std::min( *time_step, final_time - soln_time );

    //-------------------------------------------------------------------------
    // try a timestep

    // compute the fluxes
    flecsi_execute_task( evaluate_fluxes_task, loc, single, mesh );

    // reset the time stepping mode
    auto mode = mode_t::normal;

    // wrap the stage update in a do loop, so it can be re-executed
    do {

      // output the time step
      cout << std::string(80, '=') << endl;
      auto ss = cout.precision();
      cout.setf( std::ios::scientific );
      cout.precision(6);
      cout << "|  " << "Step:" << std::setw(10) << time_cnt+1
           << "  |  Time:" << std::setw(17) << soln_time + (*time_step)
           << "  |  Step Size:" << std::setw(17) << *time_step
           << "  |" << std::endl;
      cout.unsetf( std::ios::scientific );
      cout.precision(ss);

      // Loop over each cell, scattering the fluxes to the cell
      auto err =
        flecsi_execute_task(
          apply_update_task, loc, single, mesh, machine_zero, true
        );
      auto update_flag = err.get();


      // dump the current errored solution to a file
      if ( update_flag != solution_error_t::ok && output_freq > 0)
        output(mesh, prefix+"-error", suffix, 1);

      // if we got an unphysical solution, half the time step and try again
      if ( update_flag == solution_error_t::unphysical ) {
        // Print a message that we are retrying
        std::cout << "Unphysical solution detected, halfing timestep..." << std::endl;
        // half time step
        *time_step *= 0.5;
        // indicate that we are retrying
        mode = mode_t::retry;
      }

      // if there was variance, retry the solution again
      else if (update_flag == solution_error_t::variance) {
        // Print a message that we are retrying
        std::cout << "Variance in solution detected, retrying..." << std::endl;
        // roll the counter back
        --num_steps;
        // set the flag to restart
        mode = mode_t::restart;
      }

      // if there is no error, just break
      else {
        mode = mode_t::normal;
        break;
      }


      // if we are retrying or restarting, restore the original solution
      if (mode==mode_t::retry || mode==mode_t::restart) {
        // restore the initial solution
        flecsi_execute_task( restore_solution_task, loc, single, mesh );
        // don't retry forever
        if ( ++num_retries > max_retries ) {
          // Print a message we are exiting
          std::cout << "Too many retries, exiting..." << std::endl;
          // flag that we want to quit
          mode = mode_t::quit;
        }
      }


    } while ( mode==mode_t::retry );

    // end timestep
    //-------------------------------------------------------------------------

    // if a restart is detected, restart the whole iteration loop
    if (mode==mode_t::restart) continue;

    // Update derived solution quantities
    flecsi_execute_task(
      update_state_from_energy_task, loc, single, mesh, simcfg.get_eos().get()
    );

    // now we can quit after the solution has been reset to the previous step's
    if (mode==mode_t::quit) break;

    #ifdef HAVE_CATALYST
    if (!catalyst_scripts.empty()) {
      auto vtk_grid = mesh::to_vtk( mesh );
      insitu.process(
        vtk_grid, soln_time, num_steps, (num_steps==max_steps-1)
      );
    }
    #endif

    // update time
    soln_time = mesh.increment_time( *time_step );
    time_cnt = mesh.increment_time_step_counter();

    // now output the solution
    output(mesh, prefix, suffix, output_freq);

    // reset the number of retrys if we eventually made it through a time step
    num_retries  = 0;

  }

  //===========================================================================
  // Post-process
  //===========================================================================

  // now output the solution
  if ( (output_freq > 0) && (time_cnt % output_freq != 0) )
    output(mesh, prefix, suffix, 1);

  cout << "Final solution time is "
       << std::scientific << std::setprecision(2) << soln_time
       << " after " << num_steps << " steps." << std::endl;


  auto tdelta = utils::get_wall_time() - tstart;
  std::cout << "Elapsed wall time is " << std::setprecision(4) << std::fixed
            << tdelta << "s." << std::endl;


  // now output the checksums
  mesh::checksum(mesh);


  // success if you reached here
  return 0;

}

} // namespace
} // namespace
