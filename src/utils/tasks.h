/*~-------------------------------------------------------------------------~~*
 *     _   ______________     ___    __    ______
 *    / | / / ____/ ____/    /   |  / /   / ____/
 *   /  |/ / / __/ /  ______/ /| | / /   / __/   
 *  / /|  / /_/ / /__/_____/ ___ |/ /___/ /___   
 * /_/ |_/\____/\____/    /_/  |_/_____/_____/   
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
/*!
 *
 * \file tasks.h
 * 
 * \brief Utilities for creating simple tasks from iterators.
 *
 ******************************************************************************/
#pragma once


//! user includes
#include "../utils/zip.h"

namespace ale {
namespace utils {


////////////////////////////////////////////////////////////////////////////////
//! \brief generate a simple tasks from a function and iterator
//!
//!
//! sample usage:
//! 
//!   auto internal_energy = [&](const real_t & a, const real_t & b, auto & c) 
//!     { c = eos.compute_internal_energy(a, b); };
//!   simple_task( internal_energy, d, p, e );
////////////////////////////////////////////////////////////////////////////////


// helper function that actually calls the tasks
template<class Func, class Iterator, std::size_t... I>
void simple_task_impl( Func & func, Iterator &tup, std::index_sequence<I...> )
{
  for ( auto it : tup ) {
     func( get<I>(it)... );
  }
}

// main interface
template<class Func, class... Args>
void simple_task( Func & func, Args&&... args )
{
  constexpr size_t N = sizeof...(Args);
  auto tup = zip( args... );
  simple_task_impl( func, tup, std::make_index_sequence<N>() );
}


} // namespace
} // namespace



/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
