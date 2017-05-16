/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Some utilities for using lua.
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifdef HAVE_LUA

// user includes
#include "utils/string_utils.h"
#include "utils/errors.h"

// use lua
extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

// system libraries
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

namespace ale {
namespace utils {


#define lua_try_access_as(state, key, ...)                                     \
  (!state[key].empty()) ?                                                      \
    state[key].as<__VA_ARGS__>() :                                             \
    throw std::runtime_error(                                                  \
      "\'" + state[key].name() +                                               \
      "\' does not exist in the lua state you are accessing."                  \
    )

#define lua_try_access(state, key)                                             \
  (!state[key].empty()) ?                                                      \
    state[key] :                                                               \
    throw std::runtime_error(                                                  \
      "\'" + state[key].name() +                                               \
      "\' does not exist in the lua state you are accessing."                  \
    )

//! \brief Use a shared pointer to the lua state.
//! Multiple objects may use the same lua state, so we don't want to
//! destroy it unless all objects are destroyed.
using lua_state_ptr_t = std::shared_ptr<lua_State>;

////////////////////////////////////////////////////////////////////////////////
/// \defgroup lua_value lua_value
/// \brief A struct to extract and typecast values from the lua stack.
////////////////////////////////////////////////////////////////////////////////
/// \{

/// \brief The default implementation.
/// \tparam T  The type.
template < typename T, typename Enable = void>
struct lua_value {};

/// \brief The implementation for integral values.
template <typename T>
struct lua_value< T, std::enable_if_t< std::is_integral<T>::value > >
{
  /// \brief Return the value in the lua stack.
  /// \param [in] s  The lua state to query.
  /// \param [in] index  The row to access.  Defaults to the value at the top
  ///                    of the stack.
  /// \return The requested value as a long long.
  static T get(lua_State * s, int index = -1)
  {
    if ( !lua_isnumber(s,index) )
     raise_runtime_error( "Invalid conversion of type \"" <<
      lua_typename(s, lua_type(s, index)) << "\" to int."
    );
    auto i = lua_tointeger(s, index);
    lua_remove(s,index);
    return i;
  }
};


/// \brief The implementation for double.
template <typename T>
struct lua_value<T, std::enable_if_t< std::is_floating_point<T>::value > >
{
  /// \brief Return the value in the lua stack.
  /// \param [in] s  The lua state to query.
  /// \param [in] index  The row to access.  Defaults to the value at the top
  ///                    of the stack.
  /// \return The requested value as a double.
  static T get(lua_State * s, int index = -1)
  {
    if ( !lua_isnumber(s,index) )
     raise_runtime_error( "Invalid conversion of type \"" <<
      lua_typename(s, lua_type(s, index)) << "\" to double."
    );
    auto x = lua_tonumber(s, index);
    lua_remove(s,index);
    return x;
  }
};

/// \brief The implementation for bool.
template <>
struct lua_value<bool>
{
  /// \brief Return the value in the lua stack.
  /// \param [in] s  The lua state to query.
  /// \param [in] index  The row to access.  Defaults to the value at the top
  ///                    of the stack.
  /// \return The requested value as a boolean.
  static bool get(lua_State * s, int index = -1)
  {
    if ( !lua_isboolean(s,index) )
     raise_runtime_error( "Invalid conversion of type \"" <<
      lua_typename(s, lua_type(s, index)) << "\" to bool."
    );
    auto b = lua_toboolean(s, index);
    lua_remove(s, index);
    return b;
  }
};

/// \brief The implementation for std::string.
template <>
struct lua_value<std::string>
{
  /// \brief Return the value in the lua stack.
  /// \param [in] s  The lua state to query.
  /// \param [in] index  The row to access.  Defaults to the value at the top
  ///                    of the stack.
  /// \return The requested value as a string.
  static std::string get(lua_State * s, int index = -1)
  {
    if ( !lua_isstring(s, index) )
     raise_runtime_error( "Invalid conversion of type \"" <<
      lua_typename(s, lua_type(s, index)) << "\" to string."
    );
    auto str = lua_tostring(s, index);
    lua_remove(s, index);
    return str;
  }
};

/// \brief The implementation for vectors.
template<
  typename T,
  typename Allocator,
  template<typename,typename> class Vector
>
struct lua_value< Vector<T,Allocator> >
{

  /// \brief Return the value in the lua stack.
  /// \param [in] s  The lua state to query.
  /// \param [in] index  The row to access.  Defaults to the value at the top
  ///                    of the stack.
  /// \return The requested value as a vector.
  static Vector<T,Allocator> get(lua_State * s, int index = -1)
  {
    // make sure we are accessing a table
    if ( !lua_istable(s, index) )
     raise_runtime_error( "Invalid conversion of type \"" <<
      lua_typename(s, lua_type(s, index)) << "\" to vector."
    );
    // get the size of the table
    auto n = lua_rawlen(s, -1);
    // extract the results
    Vector<T,Allocator> res;
    res.reserve(n);
    for ( int i=1; i<=n; ++i ) {
      lua_rawgeti(s, -1, i);  // push t[i]
      res.emplace_back( lua_value<T>::get(s) );
    }
    // remove it from the stack
    lua_remove(s, index);
    return res;
  }
};

/// \brief The implementation for vectors.
template<
  typename T,
  std::size_t N,
  template <typename,std::size_t> class Array
>
struct lua_value< Array<T,N> >
{

  /// \brief Return the value in the lua stack.
  /// \param [in] s  The lua state to query.
  /// \param [in] index  The row to access.  Defaults to the value at the top
  ///                    of the stack.
  /// \return The requested value as a vector.
  static Array<T,N> get(lua_State * s, int index = -1)
  {
    // make sure we are accessing a table
    // make sure we are accessing a table
    if ( !lua_istable(s, index) )
     raise_runtime_error( "Invalid conversion of type \"" <<
      lua_typename(s, lua_type(s, index)) << "\" to vector."
    );
    // get the size of the table
    auto n = lua_rawlen(s, -1);
    if ( n != N )
      raise_runtime_error(
        "Expecting array of size"<<N<<", stack array is size " << n
      );
    // extract the results
    Array<T,N> res;
    for ( int i=1; i<=std::min(n,N); ++i ) {
      lua_rawgeti(s, -1, i);  // push t[i]
      res[i-1] = lua_value<T>::get(s);
    }
    // remove it from the stack
    lua_remove(s, index);
    return res;
  }
};

/// \}

////////////////////////////////////////////////////////////////////////////////
/// \defgroup lua_push lua_push
/// \brief Functions to push values onto the lua stack.
////////////////////////////////////////////////////////////////////////////////
/// \{

/// \brief Push an integer onto the stack.
/// \param [in] s  The lua state to push a value to.
/// \param [in] i  The integer to push.
inline void lua_push(lua_State * s, int i)
{ lua_pushinteger( s, i ); }

/// \brief Push a long long onto the stack.
/// \param [in] s  The lua state to push a value to.
/// \param [in] i  The long long to push.
inline void lua_push(lua_State * s, long long i)
{ lua_pushinteger( s, i ); }

/// \brief Push a float onto the stack.
/// \param [in] s  The lua state to push a value to.
/// \param [in] x  The float to push.
inline void lua_push(lua_State * s, float x)
{ lua_pushnumber( s, x ); }

/// \brief Push a double onto the stack.
/// \param [in] s  The lua state to push a value to.
/// \param [in] x  The double to push.
inline void lua_push(lua_State * s, double x)
{ lua_pushnumber( s, x ); }

/// \brief Push a boolean onto the stack.
/// \param [in] s  The lua state to push a value to.
/// \param [in] b  The boolean to push.
inline void lua_push(lua_State * s, bool b)
{ lua_pushboolean( s, b ); }

/// \brief Push a character array onto the stack.
/// \param [in] s  The lua state to push a value to.
/// \param [in] str  The character array to push.
inline void lua_push(lua_State * s, const char * str)
{ lua_pushstring( s, str ); }

/// \brief Push a std::string onto the stack.
/// \param [in] s  The lua state to push a value to.
/// \param [in] str  The string to push.
inline void lua_push(lua_State * s, const std::string & str)
{ lua_pushlstring( s, str.c_str(), str.size() ); }
/// \}

////////////////////////////////////////////////////////////////////////////////
/// \brief A base class for several of the implemented objects.
/// This class mainly contains a lua state pointer which all derived
/// classes will use.  It also has some utility member functions.
////////////////////////////////////////////////////////////////////////////////
class lua_base_t {

protected:

  /// \brief The state pointer.
  lua_state_ptr_t state_;

public:

  /// \brief Default constructor.
  lua_base_t()
    : state_( luaL_newstate(), [](lua_State * s) { lua_close(s); } )
  {}

  /// \brief Copy constructor.
  lua_base_t(const lua_state_ptr_t & state)
    : state_(state)
  {}

  /// \brief Return the raw state pointer.
  /// \remark Non-const version.
  auto state() { return state_.get(); }
  /// \brief Return the raw state pointer.
  /// \remark Const version.
  auto state() const { return state_.get(); }

  /// \brief Get the ith row of the stack.
  /// \param [in] i The row of the stack.
  /// \return A string with the type and value at the ith row.
  std::string get_row(int i) const
  {
    auto s = state();
    std::stringstream os;
    auto t = lua_type(s, i);
    switch (t) {
      case LUA_TSTRING:  // strings
        os << "string >> " << lua_tostring(s, i);
        break;
      case LUA_TBOOLEAN:  // booleans
        os << "bool   >> " << (lua_toboolean(s, i) ? "true" : "false");
        break;
      case LUA_TNUMBER:  // numbers
        os << "number >> " << lua_tonumber(s, i);
        break;
      default:  // other values
        os << "other  >> " << lua_typename(s, t);
        break;
    }
    return os.str();
  }

  /// \brief Dump the stack to an output stream/
  /// \param [in,out] os The stream to output to.
  /// \return A reference to the output stream.
  std::ostream& dump_stack(std::ostream& os) const
  {
    auto s = state();
    auto top = lua_gettop(s);
    if ( top ) {
      os << "Row : Type   >> Value" << std::endl;
      for (int i = 1; i <= top; i++) {  /* repeat for each level */
        os << std::setw(3) << i << " : ";
        os << get_row(i);
        os << std::endl;  // put a separator
      }
    }
    else {
      os << "(stack empty)" << std::endl;
    }
    return os;
  }

  /// \brief Print the last row in the stack.
  void print_last_row() const
  {
    auto s = state();
    auto top = lua_gettop(s);
    std::cerr << "Row : Type   >> Value" << std::endl;
    std::cerr << std::setw(3) << top << " : " << get_row(-1) << std::endl;
    //lua_pop(state(), 1);
  }

  /// \brief the output operator.
  /// \param [in,out] os  The output stream.
  /// \param [in] s  The object whose stack to dump.
  /// \return The output stream.
  friend std::ostream& operator<<(std::ostream& os, const lua_base_t & s)
  {
    return s.dump_stack(os);
  }
};

////////////////////////////////////////////////////////////////////////////////
/// \brief A class to keep track of a lua reference.
////////////////////////////////////////////////////////////////////////////////
class lua_ref_t : public lua_base_t {

  /// \brief The reference is stored as a shared_ptr to an int.
  /// A shared pointer is used so that the reference isn't deleted until
  /// all associated objects are destroyed.  Multiple copies of a single
  /// reference may exist.
  std::shared_ptr<int> ref_;

  /// \brief Also store the type id of the object we are referencing
  int type_ = LUA_TNONE;

public:

  /// Delete the default destructor.
  lua_ref_t() = delete;

  /// \brief The main constructor.
  /// References are created in LUA_REGISTRYINDEX table.
  /// \param [in] state  A pointer to a lua state.
  /// \param [in] ref  The lua reference key.
  lua_ref_t ( const lua_state_ptr_t & state, int ref, int type )
    : lua_base_t(state),
      ref_(
        new int{ref},
        [s=state](int * r)
        { luaL_unref(s.get(), LUA_REGISTRYINDEX, *r); }
      ),
      type_(type)
  {}

  /// \brief Constructor to create an empty reference.
  lua_ref_t( const lua_state_ptr_t & state )
    : lua_ref_t(state, LUA_REFNIL, LUA_TNONE)
  {}

  /// \brief Push the refered value onto the stack.
  void push() const
  {
    lua_rawgeti(state(), LUA_REGISTRYINDEX, *ref_);
  }

  /// \brief return true if the pointed reference is null
  bool empty() const
  {
    return (*ref_ == LUA_REFNIL || *ref_ == LUA_NOREF);
  }

}; // class lua_ref_t

/// \brief Create a lua reference to the last value on the stack.
/// \param [in] state A lua state pointer.
/// \return A new lua_ref_t object.
inline lua_ref_t make_lua_ref(const lua_state_ptr_t & state)
{
  auto s = state.get();
  return { state, luaL_ref(s, LUA_REGISTRYINDEX), lua_type(s, -1) };
}


////////////////////////////////////////////////////////////////////////////////
/// \brief This class stores a reference to a lua value.
/// The class is used to convert the refered lua value to a desired type.  It is
/// is the root of the implementation.
////////////////////////////////////////////////////////////////////////////////
class lua_result_t : public lua_base_t {

  /// \brief The name of the object.
  std::string name_;
  /// \brief A reference to the lua value in the LUA_REGISTRYINDEX table.
  std::vector<lua_ref_t> refs_;
  /// \brief The size of the object on the stack
  std::size_t size_ = 0;

  /// \defgroup get_results get_results
  /// \brief These templates are used to extract multiple results as tuples.
  /// \{

  /// \brief Final templated function to end the recursion.
  /// \tparam Tup The tuple type the data gets stored as.
  /// \tparam I The static index of the tuple.
  /// \param [in,out] tup  The tuple to store the lua results in.
  template< typename Tup, int I >
  void get_results( Tup & tup ) const
  {}

  /// \brief Main recursive function to set each value of the tuple.
  /// \tparam Tup The tuple type the data gets stored as.
  /// \tparam I The static index of the tuple.
  /// \tparam Arg1,Args  The requested argument types.
  /// \param [in,out] tup  The tuple to store the lua results in.
  template<
    typename Tup, int I, typename Arg1, typename... Args
  >
  void get_results( Tup & tup ) const
  {
    std::get<I>(tup) = lua_value<Arg1>::get( state() );
    // recursively extract tuple
    get_results<Tup,I+1,Args...>( tup );
  }
  /// /}

  /// \defgroup push_args push_args
  /// \brief These templates are used to push function arguments on the stack.
  /// \{

  /// \brief Final templated function to end the recursion.
  void push_args() const
  {}

  /// \brief The main recursive function.
  /// \tparam Arg,Args  The function argument types.
  /// \param [in]  arg,args  The values of the function arguments.
  template< typename Arg, typename... Args >
  void push_args( Arg&& arg, Args&&... args ) const
  {
    // grow the stack
    check_stack(1);
    // chop off an argument and push it
    lua_push( state(), std::forward<Arg>(arg) );
    // set the remaining arguments
    push_args(std::forward<Args>(args)...);
  }
  /// /}

  /// \brief Check the stack to make sure it can be expanded.
  /// \param [in] extra The desired size to grow the stack by.
  void check_stack(int extra) const
  {
    auto s = state();
    auto ret = lua_checkstack(s, extra);
    if ( !ret ) {
      std::ostringstream ss;
      ss << "Cannot grow stack " << extra << " slots operating on element \""
         << name_ << "\"." << std::endl << "Current stack size is "
         << lua_gettop(s) << ".";
      raise_runtime_error(ss.str());
    }
  }

  void check_nil(const std::string & name) const
  {
    if ( lua_isnil(state(), -1) ) {
      print_last_row();
      raise_runtime_error("\"" << name << "\" does not exist.");
    }
  }

  void check_table(const std::string & name) const
  {
    check_nil(name);
    if ( !lua_istable(state(), -1) ) {
      print_last_row();
      raise_runtime_error("\"" << name << "\" is not a table.");
    }
  }

  /// \brief Check that a result is valid.
  /// \parm [in] name  The name of the object we are checking.
  void check_result(const std::string & name) const
  {
    if (lua_isnil(state(), -1)) {
      print_last_row();
      raise_runtime_error("\"" << name << "\" returned nil.");
    }
  }

  /// \brief Check that a result is a function.
  /// \parm [in] name  The name of the object we are checking.
  void check_function(const std::string & name) const
  {
    check_nil(name);
    if ( !lua_isfunction(state(), -1) ) {
      print_last_row();
      raise_runtime_error("\"" << name << "\" is not a function.");
    }
  }

  /// \brief Push the last value onto the stack.
  /// \remark The stack references are collected in reverse order.
  void push_last() const
  {
    check_stack(1);
    refs_.back().push();
  }

  /// \brief Push all referred values onto the stack.
  /// \remark The stack references are collected in reverse order.
  void push_all() const
  {
    // make sure the stack can handle this
    check_stack(refs_.size());
    // push everything onto the stack in reverse order
    for ( auto && r : refs_ ) std::forward<decltype(r)>(r).push();
  }

public:

  /// \brief No default constructor.
  lua_result_t() = delete;

  /// \brief This is the main constructor with a single reference.
  /// \param [in] state  The lua state pointer.
  /// \param [in] name  The name of the object.
  /// \param [in] ref  A reference to the last value popped off the stack.
  /// \param [in] size  The size of the object on the stack.
  lua_result_t(
    const lua_state_ptr_t & state,
    const std::string & name,
    lua_ref_t && ref,
    std::size_t size
  ) : lua_base_t(state), name_(name), refs_({std::move(ref)}), size_(size)
  {}

  /// \brief This is the main constructor with a multiple references.
  /// \param [in] state  The lua state pointer.
  /// \param [in] name  The name of the object.
  /// \param [in] refs  A references to the last set of values popped off the
  ///                   stack.
  lua_result_t(
    const lua_state_ptr_t & state,
    const std::string & name,
    std::vector<lua_ref_t> && refs
  ) : lua_base_t(state), name_(name), refs_(std::move(refs)),
      size_(refs_.size())
  {}

  /// \brief Return true if all references are nil.
  bool empty() const
  {
    for ( const auto & r : refs_ )
      if (!r.empty()) return false;
    return true;
  }

  /// \brief Return the name of the object.
  const auto & name() const
  {
    return name_;
  }

  /// \brief Return the size of the table.
  std::size_t size() const
  {
    return size_;
  }

  /// \brief Explicit type conversion operators for single values.
  /// \tparam T The type to convert to.
  /// \return The typecast value.
  template< typename T >
  explicit operator T() const {
    if ( refs_.size() != 1 )
      raise_runtime_error( "Expecting 1 result, stack has " << refs_.size() );
    push_last();
    return lua_value<T>::get(state());
  }

  /// \brief Explicit type conversion operators for tuples.
  /// \tparam Args The element types of the tuple.
  /// \return The typecast tuple of values.
  template< typename...Args >
  explicit operator std::tuple<Args...>() const {
    constexpr int N = sizeof...(Args);
    using Tup = std::tuple<Args...>;
    if ( refs_.size() != N )
      raise_runtime_error(
        "Expecting "<<N<<" results, stack has " << refs_.size()
      );
    push_all();
    Tup tup;
    get_results<Tup,0,Args...>(tup);
    return tup;
  }

  /// \brief Explicit type conversion operators for single values.
  /// \tparam T The type to convert to.
  /// \return The typecast value.
  template< typename T >
  T as() const {
    return static_cast<T>(*this);
  }

  /// \brief Explicit type conversion operators for tuples.
  /// \tparam Args The element types of the tuple.
  /// \return The typecast tuple of values.
  template< typename T1, typename T2, typename...Ts >
  auto as() const {
    return static_cast< std::tuple<T1,T2,Ts...> >(*this);
  }

  /// \brief Evaluate a lua function.
  /// \tparam Args The function argument types.
  /// \param [in] args The argument types.
  /// \return A new lua_result_t with a reference to the result of the function
  ///         call.
  template < typename...Args >
  lua_result_t operator()(Args&&...args) const
  {
    auto s = state();
    // get the current stack position ( the function and arguments will get
    // deleted from the stack after the function call )
    auto pos0 = lua_gettop(s);
    // push the function onto the stack
    check_stack(1);
    push_last();
    // now make sure its a function
    check_function(name_);
    // add the arguments
    push_args(std::forward<Args>(args)...);
    // keep track of the arguments
    auto pos1 = lua_gettop(s);
    std::stringstream args_ss;
    args_ss << "(";
    for ( auto p = pos0+1; p<=pos1; ++p ) {
      auto t = lua_type(s, p);
      args_ss << lua_typename(s, t);
      if (p<pos1) args_ss << ",";
    }
    args_ss << ")";
    // call the function
    auto ret = lua_pcall(s, sizeof...(args), LUA_MULTRET, 0);
    if (ret) {
      print_last_row();
      raise_runtime_error("Problem calling \"" << name_ << "\".");
    }
    // make sure the result is non nill
    check_result(name_);
    // figure out how much the stack grew
    pos1 = lua_gettop(s);
    auto num_results = pos1 - pos0;
    assert( num_results >= 0 );
    // because there could be multiple results, get a reference to each one
    std::vector<lua_ref_t> ref_list;
    ref_list.reserve( num_results );
    for (int i=0; i<num_results; ++i)
      ref_list.emplace_back( std::move(make_lua_ref(state_)) );
    // get the result
    return { state_, name_+args_ss.str(), std::move(ref_list) };
  }

  /// \brief Get the lua value in the table given a key.
  /// \param [in] key  The key to search for.
  /// \return A new lua_result_t with a reference to the result of the table
  ///         lookup.
  lua_result_t operator[]( const std::string & key ) const &
  {
    auto s = state();
    auto new_name = name_+".[\""+key+"\"]";
    // push the table onto the stack
    push_last();
    // make sure we are accessing a table
    check_table(name_);
    // push the key onto the stack
    check_stack(1);
    lua_pushlstring(s, key.c_str(), key.size());
    // now get the table value, the key gets pushed from the stack
    lua_rawget(s, -2);
    // get the size of the object
    auto len = lua_rawlen(s, -1);
    // get a reference to the result, and pop the table from the stack
    auto ref = make_lua_ref(state_);
    lua_pop(s, -1);
    // return the global object with a pointer to a location in the stack
    return { state_, new_name, std::move(ref), len };
  }

  /// \brief Get the lua value in the table given an index.
  /// \param [in] n  The index to access.
  /// \return A new lua_result_t with a reference to the result of the table
  ///         lookup.
  lua_result_t operator[]( int n ) const &
  {
    auto s = state();
    auto new_name = name_+"["+std::to_string(n)+"]";
    // push the table onto the stack
    push_last();
    // make sure we are accessing a table
    check_table(name_);
    // now get the table value, the key gets pushed from the stack
    lua_rawgeti(s, -1, n);
    // get the size of the object
    auto len = lua_rawlen(s, -1);
    // get a reference to the result, and pop the table from the stack
    auto ref = make_lua_ref(state_);
    lua_pop(s, -1);
    // return the global object with a pointer to a location in the stack
    return { state_, new_name, std::move(ref), len };
  }

};

////////////////////////////////////////////////////////////////////////////////
/// \brief The top level object for the lua interface.
/// This is the object the user will instantiate.
////////////////////////////////////////////////////////////////////////////////
class lua_t : public lua_base_t {

public:

  /// \brief Main constructor.
  /// \param [in] with_system  If true, load all system libraries.
  ///                          Default is true.
  lua_t(bool with_system = true) : lua_base_t()
  {
    if ( !state_ )
      raise_runtime_error("Cannot initialize lua state.");
    // open all system libraries
    if ( with_system )
      luaL_openlibs(state());
  }

  /// \brief Run a string through the lua interpreter.
  /// \param [in] script  The script to run.
  /// \return The lua error code.
  bool run_string( const std::string & script )
  {
    auto ret = luaL_dostring(state(),script.c_str());
    if ( ret ) {
      print_last_row();
      raise_runtime_error("Cannot load buffer.");
    }
    return ret;
  }

  /// \brief Load a file in the lua interpreter.
  /// \param [in] file  The file to load.
  void loadfile( const std::string & file )
  {
    auto ret = luaL_dofile(state(),file.c_str());
    if ( ret ) {
      print_last_row();
      raise_runtime_error("Cannot load file.");
    }
  }

  /// \brief Access an object in the global table.
  /// \param [in] key  The key to access.
  /// \return A lua_result_t object which points to the value of the table
  ///         lookup.
  lua_result_t operator[]( const std::string & key ) const &
  {
    auto s = state();
    // the function name
    lua_getglobal(s, key.c_str());
    // get the size of the object
    auto len = lua_rawlen(s, -1);
    // return the global object with a pointer to a location in the stack
    return { state_, key, make_lua_ref(state_), len };
  }

  /// \brief Run a string through the lua interpreter.
  /// \param [in] script  The script to run.
  /// \return The lua error code.
  auto operator()( const std::string & script )
  {
    return run_string( script );
  }
};

} // namespace utils
} // namespace ale

#endif // HAVE_LUA

