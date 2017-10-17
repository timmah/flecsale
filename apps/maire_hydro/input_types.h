/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Type specializations for maire-hydro simulations
////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "default_input_traits.h"
#include "ristra/init_value.h"
#include "ristra/input_engine.h"

namespace apps::hydro{

// Configure input_engine macros from build system here:
#ifdef FLECSALE_DIMENSION
constexpr uint8_t default_dim = FLECSALE_DIMENSION;
#else
constexpr uint8_t default_dim = 2;
#endif

using input_traits = maire_input_traits<default_dim>;
using input_engine = ristra::input_engine_t<input_traits>;
template <typename T> using init_value = ristra::init_value_t<T,input_engine>;

} // flecsale::


// End of file
