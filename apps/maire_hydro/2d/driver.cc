/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simple tests related to solving full hydro solutions.
///////////////////////////////////////////////////////////////////////////////

// tasks.h must be first
#include "tasks.h"

#include "../SimConfig.h"
#include "../base_problem.h"
#include "../driver_new.h"
#include "../input_types.h"

namespace flecsi {
namespace execution {

///////////////////////////////////////////////////////////////////////////////
//! \brief A sample test of the hydro solver
///////////////////////////////////////////////////////////////////////////////
void driver(int argc, char **argv) {
  using namespace apps::hydro;
  apps::hydro::driver<apps::hydro::input_engine>(argc, argv);
}

} // namespace
} // namespace
