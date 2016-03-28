////////////////////////////////////////////////////////////////////////////////
///
/// Functions to write binary files in vtk format
///
/// \date Friday, May 20 2011
////////////////////////////////////////////////////////////////////////////////

// user includes
#include "vtk.h"

namespace ale {
namespace utils {


////////////////////////////////////////////////////////////////////////////////
//! \brief the type map
////////////////////////////////////////////////////////////////////////////////
const vtk_writer::type_map_t vtk_writer::type_map = 
  { 
    { typeid(float),  "float" },
    { typeid(double), "double" },
    { typeid(int),    "int" },
    { typeid(long),   "long" },
    { typeid(unsigned int),  "unsigned_int" },
    { typeid(unsigned long), "unsigned_long" },
  };

} // namespace
} // namespace
