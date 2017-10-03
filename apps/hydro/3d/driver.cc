/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simple tests related to solving full hydro solutions.
///////////////////////////////////////////////////////////////////////////////

// hydro includes
// tasks.h must be first!
#include "tasks.h"

#include "../driver.h"
#include "../input_types.h"
#include "../sim_config.h"
#include "base_problem_3d.h"

namespace flecsi {
namespace execution {

///////////////////////////////////////////////////////////////////////////////
//! \brief A sample test of the hydro solver
///////////////////////////////////////////////////////////////////////////////
void driver(int argc, char** argv)
{
  apps::hydro::driver<apps::hydro::input_engine, apps::hydro::base_problem_3d>(
      argc, argv);
}

} // namespace
} // namespace
