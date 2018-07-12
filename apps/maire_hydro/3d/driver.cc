/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simple tests related to solving full hydro solutions.
///////////////////////////////////////////////////////////////////////////////

#include "../../common/application_init.h"
#include "../driver.h"
#include "base_problem_3d.h"

namespace flecsi_sp::burton {

void application_tlt_init(int argc, char **argv){
  application_tlt_init_templ<apps::hydro::base_problem_3d>(argc, argv);
  return;
}

} // flecsi_sp::burton

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
