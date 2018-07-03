// application_init.h
// T. M. Kelley
// Jun 28, 2018
// (c) Copyright 2018 LANSLLC, all rights reserved


#pragma once

#define BURTON_ENABLE_APPLICATION_TLT_INIT // ?? Here? Not really...

namespace flecsi_sp::burton{

/** \brief Application-specific top-level task initialization
 *
 * Note: this function is declared in burton_specialization_init.h*/
void application_tlt_init(int argc, char **argv);

} //

// End of file
