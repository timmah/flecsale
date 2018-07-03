/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Define the main types for the hydro solver.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <flecsale/eqns/euler_eqns.h>
#include <flecsale/eqns/flux.h>

#include "../common/utils.h"
#include "../common/types.h"

namespace apps {
namespace hydro {

using eqns_t = typename flecsale::eqns::euler_eqns_t<real_t, mesh_t::num_dimensions>;

using flux_data_t = eqns_t::flux_data_t;

} // namespace hydro
} // namespace apps
