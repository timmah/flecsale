// application_init.h
// T. M. Kelley
// Jun 28, 2018
// (c) Copyright 2018 LANSLLC, all rights reserved

#pragma once

#include <string>

namespace flecsi_sp::burton{

/** \brief Application-specific top-level task initialization
 *
 * Note: this function is declared in burton_specialization_init.h*/
void application_tlt_init(int argc, char **argv);

} //

namespace apps::common{

struct Sim_Config{
explicit Sim_Config(std::string const &s):m_string(s) {}

std::string hoopla() const { return m_string; }

std::string m_string;
};

}

// End of file
