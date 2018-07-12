/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simple tests related to solving full hydro solutions.
///////////////////////////////////////////////////////////////////////////////

// hydro includes
#include "../driver.h"
#include "base_problem_3d.h"
#include "inputs.h"

namespace flecsi_sp {
namespace burton {

void application_tlt_init(int argc, char **argv){
  application_tlt_init_templ<apps::hydro::base_problem_3d>(argc, argv);
  return;
}
} // burton
} // flecsi_sp

namespace flecsi {
namespace execution {

///////////////////////////////////////////////////////////////////////////////
//! \brief A sample test of the hydro solver
///////////////////////////////////////////////////////////////////////////////
void driver(int argc, char** argv)
{
  apps::hydro::driver( argc, argv );
}

} // namespace
} // namespace
