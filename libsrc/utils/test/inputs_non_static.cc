// inputs_non_static.cc
// T. M. Kelley
// May 08, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#include "inputs_non_static.h"
#include "cinchtest.h"
#include "test_input_hard_coded_problems.h"

using namespace ale::utils;

TEST(inputs_non_static,header_compiles){
  ASSERT_TRUE(true);
}

struct Mesh{
  using vector_t = std::array<double,2>;
  using vec2s_t = std::array<size_t,2>;
};

struct user_t;

namespace ale{
namespace utils{
template <> struct input_traits<user_t>{
  using mesh_t = Mesh;
  using types = std::tuple<ale::common::real_t,
                           std::string,
                           mesh_t::vector_t,
                           mesh_t::vec2s_t,
                           size_t>;
}; // input_traits
} // common::
} // ale::

struct user_t : public inputs_t<user_t> {
public:
  using mesh_t = Mesh;

  using base_t = inputs_t<user_t>;

  /**\brief relay the base class's state for testing */
  template <class T> base_t::registry<T> &get_registry() {
    return base_t::get_registry<T>();
  } // get_registry

  /**\brief relay the base class's state for testing */
  template <class T> base_t::target_set_t &get_target_set() {
    return base_t::get_target_set<T>();
  } // get_registry

};  // struct user_t

using string_t = user_t::string_t;

TEST(inputs_non_static,instantiate){
  using test_inputs_t = inputs_t<user_t>;
  test_inputs_t t;
  ASSERT_TRUE(true);
} // TEST(inputs_non_static,instantiate){

using ics_r_t = user_t::ics_return_t;

using real_t = user_t::real_t;

ics_r_t ICS_Func(user_t::vector_t,real_t){
  return {real_t(),user_t::vector_t(),real_t()};
}

TEST(inputs_non_static,register_target){
  using test_inputs_t = inputs_t<user_t>;
  using targs_t = test_inputs_t::target_set_t;
  using ics_f_t = user_t::ics_function_t;
  test_inputs_t t;
  t.register_target<real_t>("foo");
  {
    targs_t & real_targets(t.get_target_set<real_t>());
    EXPECT_EQ(1ul,real_targets.count("foo"));
  }
  t.register_target<string_t>("gnu");
  {
    targs_t & string_targets(t.get_target_set<string_t>());
    EXPECT_EQ(1ul,string_targets.count("gnu"));
    targs_t exp_targs = {"gnu"};
    EXPECT_EQ(exp_targs,string_targets);
  }
  t.register_target<string_t>("flu");
  t.register_target<string_t>("zoo");
  {
    targs_t & string_targets(t.get_target_set<string_t>());
    targs_t exp_targs = {"gnu","flu","zoo"};
    EXPECT_EQ(exp_targs,string_targets);
  }
  t.register_target<ics_f_t>("ics");
  {
    targs_t & ics_targets(t.get_target_set<ics_f_t>());
    targs_t exp_targs = {"ics"};
    EXPECT_EQ(exp_targs,ics_targets);
  }
  t.register_target<ics_f_t>("jcs");
  {
    targs_t & ics_targets(t.get_target_set<ics_f_t>());
    targs_t exp_targs = {"jcs","ics"};
    EXPECT_EQ(exp_targs,ics_targets);
  }
  t.register_target<string_t>("moo");
  {
    targs_t & string_targets(t.get_target_set<string_t>());
    targs_t exp_targs = {"gnu","flu","moo","zoo"};
    EXPECT_EQ(exp_targs,string_targets);
  }
  t.register_target<size_t>("n_ns");
  {
    targs_t & size_targets(t.get_target_set<size_t>());
    targs_t exp_targs = {"n_ns"};
    EXPECT_EQ(exp_targs,size_targets);
  }
  t.register_target<std::array<real_t,2>>("xmin");
  {
    targs_t & array_2_targets(t.get_target_set<std::array<real_t,2>>());
    targs_t exp_targs = {"xmin"};
    EXPECT_EQ(exp_targs,array_2_targets);
  }
} // TEST(inputs_non_static,instantiate){

TEST(inputs_non_static,resolve_inputs_from_hc){
  using test_inputs_t = inputs_t<user_t>;
  using targs_t = test_inputs_t::target_set_t;
  using ics_f_t = user_t::ics_function_t;
  using targets_t = test_inputs_t::target_set_t;
  using vec2r_t = std::array<real_t,2>;
  using vec2s_t = std::array<size_t,2>;

  test_inputs_t t;
  targets_t real_t_targets = {
    "CFL","final_time","gas_constant","specific_heat"
  };
  targets_t size_t_targets = {"output_freq","max_steps"};
  targets_t string_t_targets = {
    "prefix","suffix","mesh_type","eos_type","file"
  };
  targets_t vec2r_targets = {"xmin","xmax"};
  targets_t vec2s_targets = {"dimensions"};
  targets_t ics_targets = {"ics_func"};

  t.register_targets<real_t>(real_t_targets);
  t.register_targets<size_t>(size_t_targets);
  t.register_targets<string_t>(string_t_targets);
  t.register_targets<vec2r_t>(vec2r_targets);
  t.register_targets<vec2s_t>(vec2s_targets);
  t.register_targets<lua_result_t*>(ics_targets);

  // string_t fname("mock_box_2d.lua");
  // lua_source_t * pls = new lua_source_t(fname);
  // t.register_lua_source(pls);

  hard_coded_source_t *phcs =
      new hard_coded_source_t(aletest::make_mock_box_2d());
  t.register_hard_coded_source(phcs);

  bool all_resolved = t.resolve_inputs();
  EXPECT_TRUE(all_resolved);
}

// End of file
