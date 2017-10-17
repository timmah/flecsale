/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simple tests related to solving full hydro solutions.
///////////////////////////////////////////////////////////////////////////////

// hydro incdludes
#include "../common/exceptions.h"
#include "../common/parse_arguments.h"
#include "sim_config.h"
#include "input_types.h"
#include "ristra/init_value.h"
#include "ristra/input_source.h"
#include "types.h"

// user includes
#include <flecsale/eos/ideal_gas.h>
#include <flecsale/mesh/mesh_utils.h>
#include <flecsale/utils/time_utils.h>

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
//! \tparam base_problem: function that returns a pointer to ristra::hard_coded_source
///////////////////////////////////////////////////////////////////////////////

template< typename inputs_t, typename base_problem>
int driver(int argc, char** argv)
{
  using real_t = typename input_traits::real_t;
  using string_t = typename input_traits::string_t;
  // using ::init_value;
  using mesh_t = SimConfig::mesh_t;
  using vector_t = typename mesh_t::vector_t;
  // type aliases
  using matrix_t = matrix_t< mesh_t::num_dimensions >;
  using flux_data_t = flux_data_t< mesh_t::num_dimensions >;
  constexpr auto num_dimensions(mesh_t::num_dimensions);
  using ics_function_t = apps::hydro::input_traits::ics_function_t;
  using bcs_function_t = apps::hydro::input_traits::bcs_function_t;

  // set exceptions
  enable_exceptions();

  //===========================================================================
  // Parse arguments
  //===========================================================================

  // the usage stagement
  auto print_usage = [&argv]() {
    std::cout << "Usage: " << argv[0]
              << " [--file INPUT_FILE]"
              << " [--help]"
              << std::endl << std::endl;
    std::cout << "\t--file INPUT_FILE:\t Override the input file "
              << "with INPUT_FILE." << std::endl;
    std::cout << "\t--help:\t Print a help message." << std::endl;
  };

  // Define the options
  struct option long_options[] =
    {
      {"help",       no_argument, 0, 'h'},
      {"file", required_argument, 0, 'f'},
      {0, 0, 0, 0}
    };
  const char * short_options = "hf:";

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

  // ---------------------------------------------------------------------------
  // initialization values
  // ---------------------------------------------------------------------------
  inputs_t inputs;
  // 1. Configure input targets:
  // real_t targets

  init_value<real_t> iv_CFL_acoustic("CFL_acoustic");
  init_value<real_t> iv_CFL_volume("CFL_volume");
  init_value<real_t> iv_CFL_growth("CFL_growth");
  init_value<real_t> iv_final_time("final_time");
  init_value<real_t> iv_initial_time_step("initial_time_step");
  init_value<real_t> iv_gas_constant("gas_constant");
  init_value<real_t> iv_specific_heat("specific_heat");
  // size_t targets
  init_value<size_t> iv_output_freq("output_freq");
  init_value<size_t> iv_max_steps("max_steps");
  // string targets
  init_value<string_t> iv_prefix("prefix");
  init_value<string_t> iv_suffix("suffix");
  init_value<string_t> iv_mesh_type("mesh_type");
  init_value<string_t> iv_eos_type("eos_type");
  // init_value<string_t> iv_file("file");
  // array<real_t,dim> targets
  init_value<apps::hydro::input_traits::arr_d_r_t> iv_xmin("xmin");
  init_value<apps::hydro::input_traits::arr_d_r_t> iv_xmax("xmax");
  // array<size_t,dim> targets
  init_value<apps::hydro::input_traits::arr_d_s_t> iv_dimensions("dimensions");
  // initial conditions functions
  init_value<ics_function_t> iv_ics_func("ics_func");

  std::vector<init_value<bcs_function_t>> iv_bcs_funcs;
  std::vector<init_value<string_t>> iv_bcs_types;
  for(size_t i = 0; i < num_dimensions; ++i){
    string_t func_name_str(mk_bc_func_name(i));
    iv_bcs_funcs.emplace_back( init_value<bcs_function_t>( func_name_str));
    string_t type_name_str(mk_bc_type_name(i));
    iv_bcs_types.emplace_back( init_value<string_t>( type_name_str));
  }

  // 2. Register inputs sources
  // default problem
  base_problem bp;
  auto phcs(bp());
  inputs.register_hard_coded_source(phcs.release());
  // get the input file, if any
  auto input_file_name = args.count("f") ? args.at("f") : std::string();
  // use input file to override defaults
  if ( !input_file_name.empty() ) {
    std::cout << "Using input file \"" << input_file_name << "\"."
              << std::endl;
    ristra::lua_source_ptr_t plua(ristra::mk_lua(input_file_name));
    // convey structure of CFL and bcs tables to lua_source
    plua->register_table("initial_time_step","hydro");
    plua->register_table("CFL","hydro");
    plua->register_value("CFL_acoustic","CFL","acoustic");
    plua->register_value("CFL_volume","CFL","volume");
    plua->register_value("CFL_growth","CFL","growth");
    for(size_t i = 0; i < num_dimensions; ++i){
      string_t table_name("bcs" + std::to_string(i+1));
      plua->register_table(table_name,"hydro");
      plua->register_value(mk_bc_type_name(i),table_name,"type");
      plua->register_value(mk_bc_func_name(i),table_name,"func");
    }
    // install Lua handler
    inputs.register_lua_source(plua.release());
  }
  // 3. Resolve initialization values
  bool all_resolved = inputs.resolve_inputs();
  if(!all_resolved){
    // there must be a better way...
    raise_runtime_error("Failed to resolve required inputs!");
  }
  // 4. Transfer to local values, applying validators
  time_constants_t const CFL = {iv_CFL_acoustic.get(), iv_CFL_volume.get(),
                                iv_CFL_growth.get()};
  real_t const initial_time_step = iv_initial_time_step.get();
  size_t const output_freq = iv_output_freq.get();
  string_t const prefix    = iv_prefix.get();
  string_t const suffix    = iv_suffix.get();
  size_t const max_steps   = iv_max_steps.get();
  real_t const final_time  = iv_final_time.get();

  Insist(false,"bcs.size() != 0");
  // need ics and bcs
  apps::hydro::SimConfig simcfg(inputs);
  input_traits::bcs_list_t bcs(
      simcfg.get_bcs<bcs_function_t>(iv_bcs_types, iv_bcs_funcs));
  //===========================================================================
  // Mesh Setup
  //===========================================================================

  // make the mesh
  auto pmesh = simcfg.make_mesh( /* solution time */ 0.0 );
  auto &mesh(*pmesh);
  // this is the mesh object
  bool mesh_ok = mesh.is_valid(false);
  Insist(mesh_ok,"mesh not ok");

  cout << mesh;

  //===========================================================================
  // Some typedefs
  //===========================================================================


  // get machine zero
  constexpr auto epsilon = std::numeric_limits<real_t>::epsilon();
  const auto machine_zero = std::sqrt(epsilon);

  // the maximum number of retries
  constexpr int max_retries = 5;

  // the time stepping stage coefficients
  std::vector<real_t> stages = {0.5, 1.0};

  //===========================================================================
  // Field Creation
  //===========================================================================

  // start the timer
  auto tstart = flecsale::utils::get_wall_time();

  // create some field data.  Fields are registered as struct of arrays.
  // this allows us to access the data in different patterns.
  flecsi_register_data(mesh, hydro, cell_volume,     real_t, dense, 1, cells);
  flecsi_register_data(mesh, hydro, cell_mass,       real_t, dense, 1, cells);
  flecsi_register_data(mesh, hydro, cell_pressure,   real_t, dense, 1, cells);
  flecsi_register_data(mesh, hydro, cell_velocity, vector_t, dense, 2, cells);

  flecsi_register_data(mesh, hydro, cell_density,         real_t, dense, 2, cells);
  flecsi_register_data(mesh, hydro, cell_internal_energy, real_t, dense, 2, cells);
  flecsi_register_data(mesh, hydro, cell_temperature,     real_t, dense, 1, cells);
  flecsi_register_data(mesh, hydro, cell_sound_speed,     real_t, dense, 1, cells);

  // node state
  flecsi_register_data(mesh, hydro, node_coordinates, vector_t, dense, 1, vertices);
  flecsi_register_data(mesh, hydro, node_velocity, vector_t, dense, 1, vertices);

  // solver state
  flecsi_register_data(mesh, hydro, cell_residual, flux_data_t, dense, 1, cells);

  flecsi_register_data(mesh, hydro, corner_normal, vector_t, dense, 1, corners);
  flecsi_register_data(mesh, hydro, corner_force, vector_t, dense, 1, corners);

  // register the time step and set a cfl
  flecsi_register_data( mesh, hydro, time_step, real_t, global, 1 );
  flecsi_register_data( mesh, hydro, cfl, time_constants_t, global, 1 );

  *flecsi_get_accessor( mesh, hydro, time_step, real_t, global, 0 ) = initial_time_step;
  *flecsi_get_accessor( mesh, hydro, cfl, time_constants_t, global, 0 ) = CFL;

  // Register the total energy
  flecsi_register_data( mesh, hydro, sum_total_energy, real_t, global, 1 );

  // set the persistent variables, i.e. the ones that will be plotted
  flecsi_get_accessor(mesh, hydro, cell_mass,       real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro, cell_pressure,   real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro, cell_velocity, vector_t, dense, 0).attributes().set(persistent);

  flecsi_get_accessor(mesh, hydro, cell_density,         real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro, cell_internal_energy, real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro, cell_temperature,     real_t, dense, 0).attributes().set(persistent);
  flecsi_get_accessor(mesh, hydro, cell_sound_speed,     real_t, dense, 0).attributes().set(persistent);

  flecsi_get_accessor(mesh, hydro, node_velocity, vector_t, dense, 0).attributes().set(persistent);
  //===========================================================================
  // Boundary Conditions
  //===========================================================================

  // get the current time
  auto soln_time = mesh.time();
  auto time_cnt  = mesh.time_step_counter();

  // the boundary mapper
  boundary_map_t< mesh_t::num_dimensions > boundaries;

  // install each boundary
  for ( const input_traits::bcs_pair & bc_pair : bcs )
  {
    input_traits::bcs_t * bc_type = bc_pair.first.get();
    auto bc_function = bc_pair.second;
    auto bc_key = mesh.install_boundary(
      [=](auto f)
      {
        if ( f->is_boundary() ) {
          vector_t fx( f->midpoint());
          return bc_function(fx, soln_time);
        }
        return false;
      }
    );
    boundaries.emplace( bc_key, bc_type );
  }
  //===========================================================================
  // Initial conditions
  //===========================================================================

  // now call the main task to set the ics.  Here we set primitive/physical
  // quanties
  ics_function_t ics(iv_ics_func.get());
  flecsi_execute_task( initial_conditions_task, loc, single, mesh, ics );

  // Update the EOS
  flecsi_execute_task(
    update_state_from_pressure_task, loc, single, mesh, simcfg.get_eos().get()
  );


  //===========================================================================
  // Pre-processing
  //===========================================================================

  // now output the solution
  if ( output_freq > 0 )
    output(mesh, prefix, suffix, 1);


  //===========================================================================
  // Residual Evaluation
  //===========================================================================

  // get the time step accessor
  auto time_step = flecsi_get_accessor( mesh, hydro, time_step, real_t, global, 0 );

  // a counter for this session
  size_t num_steps = 0;

  for (
    size_t num_retries = 0;
    (num_steps < max_steps && soln_time < final_time);
    ++num_steps
  ) {

    //--------------------------------------------------------------------------
    // Begin Time step
    //--------------------------------------------------------------------------

    // Save solution at n=0
    if (num_retries == 0) {
      flecsi_execute_task( save_coordinates_task, loc, single, mesh );
      flecsi_execute_task( save_solution_task, loc, single, mesh );
    }

    // keep the old time step
    real_t time_step_old = time_step;

    //--------------------------------------------------------------------------
    // Predictor step : Evaluate Forces at n=0
    //--------------------------------------------------------------------------

    // estimate the nodal velocity at n=0
    flecsi_execute_task( estimate_nodal_state_task, loc, single, mesh );

    // compute the nodal velocity at n=0
    flecsi_execute_task(
      evaluate_nodal_state_task, loc, single, mesh, boundaries
    );

    // compute the fluxes
    flecsi_execute_task( evaluate_residual_task, loc, single, mesh );

    //--------------------------------------------------------------------------
    // Time step evaluation
    //--------------------------------------------------------------------------

    // compute the time step
    std::string limit_string;
    flecsi_execute_task( evaluate_time_step_task, loc, single, mesh, limit_string );

    // access the computed time step and make sure its not too large
    *time_step = std::min( *time_step, final_time - soln_time );

    cout << std::string(60, '=') << endl;
    auto ss = cout.precision();
    cout.setf( std::ios::scientific );
    cout.precision(6);
    cout << "| " << std::setw(8) << "Step:"
         << " | " << std::setw(13) << "Time:"
         << " | " << std::setw(13) << "Step Size:"
         << " | " << std::setw(13) << "Limit:"
         << " |" << std::endl;
    cout << "| " << std::setw(8) << time_cnt+1
         << " | " << std::setw(13) << soln_time + (*time_step)
         << " | " << std::setw(13) << *time_step
         << " | " << std::setw(13) << limit_string
         << " |" << std::endl;
    cout.unsetf( std::ios::scientific );
    cout.precision(ss);

    //--------------------------------------------------------------------------
    // Multistage
    //--------------------------------------------------------------------------

    // a stage counter
    int istage = 0;

    // reset the time stepping mode
    auto mode = mode_t::normal;

    do {

      //------------------------------------------------------------------------
      // Move to n^stage

      // move the mesh to n+1/2
      flecsi_execute_task( move_mesh_task, loc, single, mesh, stages[istage] );

      // update solution to n+1/2
      auto err = flecsi_execute_task(
        apply_update_task, loc, single, mesh, stages[istage], machine_zero, (istage==0)
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
        // reset time step
        *time_step = time_step_old;
        // roll the counter back
        --num_steps;
        // set the flag to restart
        mode = mode_t::restart;
      }

      // otherewise, set the mode to none
      else {
        mode = mode_t::normal;
      }

      // if we are retrying or restarting, restore the original solution
      if (mode == mode_t::restart || mode == mode_t::retry) {
        // restore the initial solution
        flecsi_execute_task( restore_coordinates_task, loc, single, mesh );
        flecsi_execute_task( restore_solution_task, loc, single, mesh );
        mesh.update_geometry();
        // don't retry forever
        if ( ++num_retries > max_retries ) {
          // Print a message we are exiting
          std::cout << "Too many retries, exiting..." << std::endl;
          // flag that we want to quit
          mode = mode_t::quit;
        }
      }

      // Update derived solution quantities
      flecsi_execute_task(
        update_state_from_energy_task, loc, single, mesh, simcfg.get_eos().get()
      );

      // compute the current nodal velocity
      flecsi_execute_task(
        evaluate_nodal_state_task, loc, single, mesh, boundaries
      );

      // if we are retrying, then restart the loop since all the state has been
      // reset
      if (mode == mode_t::retry) {
        istage = 0;
        continue;
      }

      // now we can quit once all the state has been reset
      else if (
        mode == mode_t::quit ||
        mode == mode_t::restart ||
        ++istage==stages.size()
      )
        break;

      //------------------------------------------------------------------------
      // Corrector : Evaluate Forces at n^stage

      // compute the fluxes
      flecsi_execute_task( evaluate_residual_task, loc, single, mesh );

      //------------------------------------------------------------------------
      // Move to n+1

      // restore the solution to n=0
      flecsi_execute_task( restore_coordinates_task, loc, single, mesh );
      flecsi_execute_task( restore_solution_task, loc, single, mesh );

    } while(true); // do

    //--------------------------------------------------------------------------
    // End Time step
    //--------------------------------------------------------------------------

    // now we can quit once all the state has been reset
    if (mode == mode_t::quit) break;
    // now we can restart once all the state has been reset
    else if (mode == mode_t::restart) continue;

    // update time
    soln_time = mesh.increment_time( *time_step );
    time_cnt = mesh.increment_time_step_counter();

    // now output the solution
    output(
      mesh, prefix, suffix, output_freq
    );

    // if we got through a whole cycle, reset the retry counter
    num_retries = 0;

  }


  //===========================================================================
  // Post-process
  //===========================================================================

  // now output the solution
  if ( (output_freq > 0) && (time_cnt % output_freq != 0) )
    output(mesh, prefix, suffix, 1);

  cout << "Final solution time is "
       << std::scientific << std::setprecision(6) << soln_time
       << " after " << num_steps << " steps." << std::endl;


  auto tdelta = flecsale::utils::get_wall_time() - tstart;
  std::cout << "Elapsed wall time is " << std::setprecision(4) << std::fixed
            << tdelta << "s." << std::endl;

  // now output the checksums
  flecsale::mesh::checksum(mesh);

  // success
  return 0;

}

} // namespace
} // namespace
