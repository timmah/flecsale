// application_init.cc
// T. M. Kelley
// Jun 28, 2018
// (c) Copyright 2018 LANSLLC, all rights reserved

#include "application_init.h"
#include "ristra/initialization/init_value.h"

// Hmmm, does this thing have to live in a .cc file?
flecsi_register_global_object(0, config, apps::common::Sim_Config);

namespace apps::common{
  Sim_Config & get_teh_config(){
    auto p_sim_config =
      flecsi_get_global_object(0, config, apps::common::Sim_Config);
    Sim_Config & sim_config(*p_sim_config);
    return sim_config;
  } // get_teh_config
} // apps::common::

// End of file
