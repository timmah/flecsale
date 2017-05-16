// inputs.h
// May 08, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#pragma once

#include "flecsale/utils/detail/inputs_non_static_impl.h"
#include "flecsale/utils/input_source.h"
#include "flecsale/common/types.h"

#include <algorithm>  // all_of
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace ale{
namespace utils {

template <class input_user> struct input_traits {};

template <class input_user>
class inputs_t {
public:
  using traits_t = input_traits<input_user>;
  using type_tuple = typename traits_t::types;
  using real_t = ale::common::real_t;
  using mesh_t = typename traits_t::mesh_t;

  using vector_t = typename mesh_t::vector_t;
  using string_t = std::string;
  using str_cr_t = string_t const &;
  using ics_return_t = std::tuple<real_t,vector_t,real_t>;
  using ics_function_t =
    std::function< ics_return_t(const vector_t & x, const real_t & t) >;

  using target_set_t = std::set<string_t>;
  using failed_set_t = std::set<string_t>;
  using deq_bool = std::deque<bool>;
  template <class reg_t> using registry = std::map<string_t,reg_t>;

  /**\brief Used only for type-switching in explicit specializations. */
  template <typename T> struct type_t{};

  static constexpr size_t tsize = std::tuple_size<type_tuple>::value;

protected:
  /**\brief A class for registering targets, holding data, and recording
   * failures associated with a given type. */
  template <class T>
  struct input_registry{
    using key_t = std::string;
    using data_t = std::map<key_t,T>;
    using target_set_t = std::set<key_t>;
    using failed_set_t = std::set<key_t>;

    registry<T> & get_data_registry(){ return m_reg;}

    target_set_t &get_target_set(){return m_targets;}

    failed_set_t &get_failed_set(){return m_failures;}

    registry<T> m_reg;
    target_set_t m_targets;
    failed_set_t m_failures;

    // singleton for each type
    input_registry(){}
    input_registry(input_registry &) = delete;
    input_registry(input_registry&&) = delete;

    static input_registry& instance(){
      static input_registry m;
      return m;
    }
  }; // registry

public:


  /**\brief invoke resolve_inputs for each type in the Tuple.
   *
   * Because this works only at the type level (there is no Tuple value to
   * drive type deduction) we need to take the extra step of enumerating the
   * types in the tuple. That's what the std::index_sequence does here.
   */

  /* This is not working yet. See notes in inputs_non_static_impl.h */
  // apply_f_by_tuple(resolve_inputs_t);


  template <typename Tuple>
  auto resolve_by_tuple(){
    constexpr size_t tsize(std::tuple_size<Tuple>::value);
    return resolve_by_tuple_impl<Tuple>(std::make_index_sequence<tsize>{});
  } // resolve_by_tuple

  /*This is a general pattern, needs a bit more work to be lifted out. */
  /**\brief Helper class to grab the type of the Nth element of a tuple */
  template <typename T> struct param_list {
    template <size_t N> using type = typename std::tuple_element<N, T>::type;
  };

  /**\brief Apply resolve_inputs to each type in a tuple. */
  template <typename Tuple, std::size_t... Is>
  auto resolve_by_tuple_impl(std::index_sequence<Is...>) {
    using parm_list = param_list<Tuple>;
    auto l = {resolve_inputs<typename parm_list::template type<Is> >()...};
    return l;
  } // resolve_by_tuple_impl

  /**\\brief For each target, look through input sources and attempt
   * to resolve each target.
   *
   * \return whether all inputs were successfully resolved.
   *
   * Always tries to find something in an external input file, then looks
   * in the hard-coded source. This makes the hard-coded source the
   * default.
   **/
  bool resolve_inputs(){
    deq_bool resolutions (resolve_by_tuple<type_tuple>());
    bool const all_resolved = std::all_of(resolutions.begin(),resolutions.end(),
      [](bool b){return b;} );
    return all_resolved;
  } // resolve_inputs

  /**\brief Register a Lua input source. This class will delete by default.
   *
   * \param lua_source Pointer to lua_source_t
   */
  void register_lua_source( lua_source_t * lua_source){
    m_lua_source.reset(lua_source);
    return;
  }

  /**\brief Register a hard coded input source. This class will delete
   * the source.
   *
   * \param hard_coded_source the hard_coded_source object
   */
  void register_hard_coded_source(hard_coded_source_t * hard_coded_source){
    m_hard_coded_source.reset(hard_coded_source);
  }

  /* Idea: derived class should call register_target for each thing it expects
   * from the input. */
  template <class T> // TODO add validators, defaults, etc.
  void register_target(str_cr_t name){
    target_set_t & target_set = get_target_set<T>();
    // TODO could have some error checking here: what if something gets
    // registered more than once, for example?
    target_set.insert(name);
    return;
  } // register_target

  template <class T> // TODO add validators, defaults, etc.
  void register_targets(target_set_t names){
    for(auto & n : names){
      register_target<T>(n);
    }
    return;
  } // register_target


  /**\brief Get the value from the input process. */
  template <class T>
  T const & get_value(str_cr_t target_name){
    registry<T> & reg(this->get_registry<T>());
    typename registry<T>::iterator pv(reg.find(target_name));
    if(pv == reg.end()){
      // TODO figure out what to do if lookup fails
      throw std::domain_error("get_value: invalid key");
    }
    return *pv;
  } // get_value

  template <class T>
  registry<T> & get_registry(){
    return input_registry<T>::instance().get_data_registry();
  }

  template <class T>
  target_set_t & get_target_set(){
    return input_registry<T>::instance().get_target_set();
  }

  template <class T>
  target_set_t & get_failed_target_set(){
    return input_registry<T>::instance().get_failed_set();
  }

protected:
  /**\brief Resolve the targets for one particular type.
   *
   * \return true if all targets for T resolved. */
  template <class T>
  bool resolve_inputs(){
    target_set_t & targets(get_target_set<T>());
    registry<T> & registry(get_registry<T>());
    failed_set_t &failures(get_failed_target_set<T>());
    bool missed_any(false);
    for(auto target : targets){
      bool found_target(false);
      T & tval(registry[target]);
      // try to find in the Lua source, if there is one
      if(m_lua_source){
        found_target = m_lua_source->get_value(target,tval);
      }
      if(found_target){
        break;
      }
      // not there? no Lua file? Default to hard coded source
      if(m_hard_coded_source){
        found_target = m_hard_coded_source->get_value(target,tval);
      }
      if(!found_target){
        missed_any = true;
        failures.insert(target);
      }
    } // for(target : targets)
    return !missed_any;
  } // resolve_inputes

private:
  // state
  lua_source_ptr_t m_lua_source;
  hard_coded_source_ptr_t m_hard_coded_source;
}; // class inputs_t



} // common::
} // apps::

// End of file
