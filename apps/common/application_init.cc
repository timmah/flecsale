// application_init.cc
// T. M. Kelley
// Jun 28, 2018
// (c) Copyright 2018 LANSLLC, all rights reserved

#include "application_init.h"
#include <flecsi/execution/execution.h>

namespace flecsi_sp::burton{

struct Sim_Config{
  explicit Sim_Config(std::string const &s):m_string(s) {}

  std::string hoopla() const { return m_string; }

  std::string m_string;
};

// enum struct global_object_types{
//   config
// };

flecsi_register_global_object(0, config, Sim_Config);

void application_tlt_init(int argc, char ** argv){
  flecsi_initialize_global_object(0, config, Sim_Config, "sim config!");

  auto sim_config = flecsi_get_global_object(0,config,Sim_Config);

  std::string s = sim_config->hoopla();
  std::cout << "Sim_Config: \"" << s << "\"\n";
} // application_tlt_init

} //



// End of file
