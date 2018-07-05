/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief This is the main driver for the hydro solver.
///////////////////////////////////////////////////////////////////////////////
#pragma once

// common headers
#include "../common/application_init.h"

// hydro includes
#include "tasks.h"
#include "types.h"

// user includes
#include <ristra/utils/time_utils.h>
#include <ristra/io/catalyst/adaptor.h>


// system includes
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace apps {
namespace hydro {

// create some field data.  Fields are registered as struct of arrays.
// this allows us to access the data in different patterns.
flecsi_register_field(
  mesh_t,
  hydro,
  density,
  mesh_t::real_t,
  dense,
  1,
  mesh_t::index_spaces_t::cells
);

flecsi_register_field(
  mesh_t,
  hydro,
  velocity,
  mesh_t::vector_t,
  dense,
  1,
  mesh_t::index_spaces_t::cells
);

flecsi_register_field(
  mesh_t,
  hydro,
  internal_energy,
  mesh_t::real_t,
  dense,
  1,
  mesh_t::index_spaces_t::cells
);

flecsi_register_field(
  mesh_t,
  hydro,
  pressure,
  mesh_t::real_t,
  dense,
  1,
  mesh_t::index_spaces_t::cells
);

flecsi_register_field(
  mesh_t,
  hydro,
  temperature,
  mesh_t::real_t,
  dense,
  1,
  mesh_t::index_spaces_t::cells
);

flecsi_register_field(
  mesh_t,
  hydro,
  sound_speed,
  mesh_t::real_t,
  dense,
  1,
  mesh_t::index_spaces_t::cells
);

// Here I am regestering a struct as the stored data
// type since I will only ever be accesissing all the data at once.
flecsi_register_field(
  mesh_t,
  hydro,
  flux,
  flux_data_t,
  dense,
  1,
  mesh_t::index_spaces_t::faces
);

///////////////////////////////////////////////////////////////////////////////
//! \brief A sample test of the hydro solver
///////////////////////////////////////////////////////////////////////////////
int driver(int argc, char** argv)
{
  using apps::common::Sim_Config;

  // get the context
  auto & context = flecsi::execution::context_t::instance();
  auto rank = context.color();

  auto sim_config = flecsi_get_global_object(0,config,Sim_Config);
  std::string s = sim_config->hoopla();
  std::cout << "Sim_Config: \"" << s << "\"\n";

  //===========================================================================
  // Mesh Setup
  //===========================================================================

  // get the client handle
  auto mesh = flecsi_get_client_handle(mesh_t, meshes, mesh0);

  // cout << mesh;

  //===========================================================================
  // Some typedefs
  //===========================================================================

  using size_t = typename mesh_t::size_t;
  using real_t = typename mesh_t::real_t;
  using vector_t = typename mesh_t::vector_t;

  // get machine zero
  constexpr auto epsilon = std::numeric_limits<real_t>::epsilon();
  const auto machine_zero = std::sqrt(epsilon);

  //===========================================================================
  // Access what we need
  //===========================================================================

  auto d  = flecsi_get_handle(mesh, hydro,  density,   real_t, dense, 0);
  //auto d0 = flecsi_get_handle(mesh, hydro,  density,   real_t, dense, 1);
  auto v  = flecsi_get_handle(mesh, hydro, velocity, vector_t, dense, 0);
  //auto v0 = flecsi_get_handle(mesh, hydro, velocity, vector_t, dense, 1);
  auto e  = flecsi_get_handle(mesh, hydro, internal_energy, real_t, dense, 0);
  //auto e0 = flecsi_get_handle(mesh, hydro, internal_energy, real_t, dense, 1);

  auto p  = flecsi_get_handle(mesh, hydro,        pressure,   real_t, dense, 0);
  auto T  = flecsi_get_handle(mesh, hydro,     temperature, real_t, dense, 0);
  auto a  = flecsi_get_handle(mesh, hydro,     sound_speed, real_t, dense, 0);

  auto F = flecsi_get_handle(mesh, hydro, flux, flux_data_t, dense, 0);

  //===========================================================================
  // Initial conditions
  //===========================================================================

  // the solution time starts at zero
  real_t soln_time{0};
  size_t time_cnt{0};

  // now call the main task to set the ics.  Here we set primitive/physical
  // quanties
  flecsi_execute_task(
    initial_conditions,
    apps::hydro,
    single,
    mesh,
    inputs_t::ics,
    inputs_t::eos,
    soln_time,
    d, v, e, p, T, a
  );

  #ifdef HAVE_CATALYST
    auto insitu = io::catalyst::adaptor_t(catalyst_scripts);
    std::cout << "Catalyst on!" << std::endl;
  #endif

  //===========================================================================
  // Pre-processing
  //===========================================================================

  auto prefix_char = flecsi_sp::utils::to_char_array( inputs_t::prefix );
 	auto postfix_char =  flecsi_sp::utils::to_char_array( "exo" );

  // now output the solution
  auto has_output = (inputs_t::output_freq > 0);
  if (has_output) {
    flecsi_execute_task(
      output,
 			apps::hydro,
 			single,
 			mesh,
 			prefix_char,
 			postfix_char,
			time_cnt,
      soln_time,
 			d, v, e, p, T, a
    );
  }


  // dump connectivity
  auto name = flecsi_sp::utils::to_char_array( inputs_t::prefix+".txt" );
  auto f = flecsi_execute_task(print, apps::hydro, single, mesh, name);
  f.wait();

  // start a clock
  auto tstart = ristra::utils::get_wall_time();

  //===========================================================================
  // Residual Evaluation
  //===========================================================================

  for (
    size_t num_steps = 0;
    (num_steps < inputs_t::max_steps && soln_time < inputs_t::final_time);
    ++num_steps
  ) {

    //-------------------------------------------------------------------------
    // compute the time step

    // we dont need the time step yet
    auto local_future_time_step = flecsi_execute_task(
      evaluate_time_step, apps::hydro, single, mesh, d, v, e, p, T, a,
      inputs_t::CFL, inputs_t::final_time - soln_time
    );

    //-------------------------------------------------------------------------
    // try a timestep

    // compute the fluxes
    flecsi_execute_task( evaluate_fluxes, apps::hydro, single, mesh,
        d, v, e, p, T, a, F );

    // now we need it
    auto time_step =
      flecsi::execution::context_t::instance().reduce_min(local_future_time_step);

    // Loop over each cell, scattering the fluxes to the cell
    flecsi_execute_task(
      apply_update, apps::hydro, single, mesh, inputs_t::eos,
      time_step, F, d, v, e, p, T, a
    );

    //-------------------------------------------------------------------------
    // Post-process

    // update time
    soln_time += time_step;
    time_cnt++;

    // output the time step
    if ( rank == 0 ) {
      std::cout << std::string(80, '=') << std::endl;
      auto ss = std::cout.precision();
      std::cout.setf( std::ios::scientific );
      std::cout.precision(6);
      std::cout << "|  " << "Step:" << std::setw(10) << time_cnt
           << "  |  Time:" << std::setw(17) << soln_time
           << "  |  Step Size:" << std::setw(17) << time_step
           << "  |" << std::endl;
      std::cout.unsetf( std::ios::scientific );
      std::cout.precision(ss);
    }

#ifdef HAVE_CATALYST
    if (!catalyst_scripts.empty()) {
      auto vtk_grid = mesh::to_vtk( mesh );
      insitu.process(
        vtk_grid, soln_time, num_steps, (num_steps==inputs_t::max_steps-1)
      );
    }
#endif


    // now output the solution
    if ( has_output &&
        (time_cnt % inputs_t::output_freq == 0 ||
         num_steps==inputs_t::max_steps-1 ||
         std::abs(soln_time-inputs_t::final_time) < epsilon
        )
      )
    {
      flecsi_execute_task(
        output,
	 			apps::hydro,
 				single,
 				mesh,
	 			prefix_char,
 				postfix_char,
 				time_cnt,
        soln_time,
 				d, v, e, p, T, a
      );
    }


  }

  //===========================================================================
  // Post-process
  //===========================================================================

  auto tdelta = ristra::utils::get_wall_time() - tstart;

  if ( rank == 0 ) {

    std::cout << "Final solution time is "
         << std::scientific << std::setprecision(2) << soln_time
         << " after " << time_cnt << " steps." << std::endl;


    std::cout << "Elapsed wall time is " << std::setprecision(4) << std::fixed
              << tdelta << "s." << std::endl;

  }


  // success if you reached here
  return 0;

}

} // namespace
} // namespace
