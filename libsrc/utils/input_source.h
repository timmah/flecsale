// input_source.h
// T. M. Kelley
// May 08, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#pragma once

#include "lua_utils.h"
#include "flecsale/common/types.h"
#include <map>
#include <memory>
#include <string>

namespace ale{
namespace utils{

/* TODO: Still not sure how multiple sources will compose. Chain of
 * responsility? If so, how to control precedence?
 */

/**\brief Interface for a source of input information. Goal is to permit
 * a container of sources. */
template <class input_source_impl>
class input_source{
public:
  using string_t = ale::common::string_t;
  using str_cr_t = string_t const &;
  using real_t = ale::common::real_t;
  template <uint32_t d> using vec = std::array<real_t,d>;
  template <uint32_t d> using ics_return_t =
    std::tuple<real_t,vec<d>,real_t>;

  template <uint32_t d> using ics_function_t =
    std::function< ics_return_t<d>(const vec<d> & x, const real_t & t) >;

protected:
  // This might be worth a base class:
  template <class T>
  struct data_registry{
    using registry = std::map<string_t,T>;
    data_registry(){}
    data_registry(data_registry &) = delete;
    data_registry(data_registry &&) = delete;
    static data_registry & instance(){static data_registry m; return m;}
    registry & get_registry(){return m_reg;}
    registry m_reg;
  }; // data_registry

  // Is this really doing anything useful?
  /**\brief get value corr. to key
   * \return true if successfully found key.
   * \param[in] k: the key to look for
   * \param[out] t: reference to which value should be written */
  template <class T> bool get_value(str_cr_t k, T & t){
    return static_cast<input_source_impl>(*this).get_value(k,t);
  }

}; // input_source

class hard_coded_source_t : public input_source<hard_coded_source_t>{
public:
  template <typename reg_t> using registry = std::map<string_t,reg_t>;
  template <class T> struct type_t{};

  template <class T>
  bool get_value(str_cr_t k, T &t){
    auto & reg(get_registry<T>());
    auto it = reg.find(k);
    bool found(false);
    if(reg.end() != it){
      t = it->second;
      found = true;
    }
    return found;
  } // get_value

  template <class T> void set_registry(registry<T> &reg) {
    auto &this_reg(get_registry<T>());
    for (auto &v : reg) {
      this_reg[v.first] = v.second;
    }
  } // set_registry

  template <class T> registry<T> & get_registry(){
    return data_registry<T>::instance().get_registry();
  }

}; // class hard_coded_source_t

using hard_coded_source_ptr_t = std::unique_ptr<hard_coded_source_t>;

inline
hard_coded_source_ptr_t mk_hard_coded_source(){
  return std::make_unique<hard_coded_source_t>();
} // mk_hard_coded_source


/**\brief Use a Lua file as input source.
 *
 Right now the Lua file structure is hard-coded here. Eventually we
 want to get the structure from some sort of description.

 The structure is maintained in severla maps. Table map lists the entity
 in which each key is stored, whether it's a table or other data item.

 m_tables maintains the pointers to the actual tables.

 The base state of the lua file is a different type, so it's stored on its
 own. This offers a bit of complication in load_table.

 Finally, the lua tables act as namespaces, so there can be overlaps in the
 Lua names. We deal with this by optionally setting a key in the lua_keys
 map. If a C++ key is a key in the lua_keys map, then the corresponding value
 is used to query Lua. If not, then the C++ key is also the Lua key.
 */
class lua_source_t : public input_source<lua_source_t>{
public:
#ifdef HAVE_LUA
  using table_map_t = std::map<string_t,string_t>;
  using lua_keys_t = std::map<string_t,string_t>;
  using tables_t = std::map<string_t,lua_result_t>;

  explicit lua_source_t(str_cr_t filename) :
    m_lua_state(),
    m_filename(filename)
  {
    /* TODO Want this configuration to come from elsewhere. Maybe a traits
     * class, maybe generated from a description file. Not hard coded here. */
    m_lua_state.loadfile(filename);
    // where are tables located?
    m_table_map["hydro"] = "base_state";
    m_table_map["eos"] = "hydro";
    m_table_map["mesh"] = "hydro";
    load_table("hydro");
    load_table("mesh");
    load_table("eos");
    // where are values located?
    m_table_map["prefix"] = "hydro";
    m_table_map["suffix"] = "hydro";
    m_lua_key["suffix"] = "postfix";
    m_table_map["output_freq"] = "hydro";
    m_table_map["CFL"] = "hydro";
    m_table_map["final_time"] = "hydro";
    m_table_map["max_steps"] = "hydro";

    m_table_map["eos_type"] = "eos";
    m_lua_key["eos_type"] = "type";
    m_table_map["gas_constant"] = "eos";
    m_table_map["specific_heat"] = "eos";

    m_table_map["mesh_type"] = "mesh";
    m_lua_key["mesh_type"] = "type";
    m_table_map["dimensions"] = "mesh";
    m_table_map["xmin"] = "mesh";
    m_table_map["xmax"] = "mesh";
    m_table_map["file"] = "mesh";
    return;
  } // ctor

  template <typename T>
  struct value_getter{
    bool get_value(str_cr_t k, T &t,lua_source_t & ls){
      // check if there is a Lua key distinct from k
      str_cr_t l_key(1 == ls.m_lua_key.count(k) ? ls.m_lua_key[k] : k);
      // now recover the table to look in
      str_cr_t from_name = ls.m_table_map.at(k);
      auto & from_table = ls.m_tables.at(from_name);
      // use lua key in table
      bool found = true;
      auto lua_val = from_table[l_key];
      if(lua_val.empty()){
        found = false;
        throw std::runtime_error(
          "\'" + from_table[l_key].name() +
          "\' does not exist in the lua state you are accessing.");
      }
      t = lua_val.as<T>();
      return found;
    }
  }; // struct value_getter

  // specialization for just betting back lua_result_t below...

  template <typename T>
  bool get_value(str_cr_t f, T&t){
    value_getter<T> g;
    return g.get_value(f,t,*this);
  }

private:

  void load_table(str_cr_t load_name){
    str_cr_t from_name = m_table_map[load_name];
    if("base_state" == from_name){
      m_tables.emplace(load_name,lua_try_access(m_lua_state,load_name));
    }
    else{
      lua_result_t & from_state = m_tables.at(from_name);
      m_tables.emplace(load_name,lua_try_access(from_state,load_name));
    }
    // printf("%s:%i Loaded table '%s'\n",__FUNCTION__,__LINE__,load_name.c_str());
    return;
  }

  ale::utils::lua_t m_lua_state;
  string_t m_filename;
  table_map_t m_table_map;
  tables_t m_tables;
  lua_keys_t m_lua_key;

#else  // no lua
  explicit lua_source_t(str_cr_t ) {}

  template <class T>
  bool get_value(str_cr_t /*k*/, T &/*t*/){return false;}
#endif // HAVE_LUA
}; // lua_source_t

template <>
struct lua_source_t::value_getter<lua_result_t *>{
  bool get_value(str_cr_t k, lua_result_t *t, lua_source_t &ls) {
    // check if there is a Lua key distinct from k
    str_cr_t l_key(1 == ls.m_lua_key.count(k) ? ls.m_lua_key[k] : k);
    // now recover the table to look in
    str_cr_t from_name = ls.m_table_map.at(k);
    auto &from_table = ls.m_tables.at(from_name);
    // use lua key in table
    bool found = true;
    auto lua_val = from_table[l_key];
    if (lua_val.empty()) {
      found = false;
      throw std::runtime_error(
          "\'" + from_table[l_key].name() +
          "\' does not exist in the lua state you are accessing.");
    }
    *t = lua_val;
    return found;
  }
};


using lua_source_ptr_t = std::unique_ptr<lua_source_t>;

/**\brief */
inline
lua_source_ptr_t mk_lua(lua_source_t::str_cr_t filename){
  return std::make_unique<lua_source_t>(filename);
}

} // utils
} // ale
// End of file
