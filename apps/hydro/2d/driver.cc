/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simple tests related to solving full hydro solutions.
///////////////////////////////////////////////////////////////////////////////

// hydro includes
// tasks.h has to be first!
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
void driver(int argc, char** argv)
{
  apps::hydro::driver<apps::hydro::input_engine>( argc, argv );
}

} // namespace
} // namespace
