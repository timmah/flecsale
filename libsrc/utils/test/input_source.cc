// input_source.cc
// T. M. Kelley
// May 08, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#include "flecsale/utils/input_source.h"
#include "cinchtest.h"
#include <array>

TEST(input_source,compiles){
  EXPECT_TRUE(true);
} // smoke

TEST(hard_coded_source_t,instantiate){
  using namespace ale::utils;
  hard_coded_source_t ps;
  EXPECT_TRUE(true);
}

template <class T> using reg = ale::utils::hard_coded_source_t::registry<T>;

TEST(hard_coded_source_t,set_registry){
  using namespace ale::utils;
  using real_t = hard_coded_source_t::real_t;
  using string_t = hard_coded_source_t::string_t;
  hard_coded_source_t ps;
  reg<real_t> exp_reg = {{"CFL",257.4},{"dtmax",2.3e6}};
  ps.set_registry<real_t>(exp_reg);
  EXPECT_EQ(exp_reg,ps.get_registry<real_t>());

  reg<size_t> sz_reg = {{"n_steps",1000000000UL},{"n_zones_x",30000},
    {"n_zones_y",30000},{"n_zones_z",30000},{"CFL",12}};
  ps.set_registry<size_t>(sz_reg);
  EXPECT_EQ(sz_reg,ps.get_registry<size_t>());
  EXPECT_EQ(exp_reg,ps.get_registry<real_t>());

  reg<string_t> st_reg = {{"mesh_type","useful"},{"eos_type","fictional"},
    {"CFL","indubitable"}};
  ps.set_registry<string_t>(st_reg);
  EXPECT_EQ(st_reg,ps.get_registry<string_t>());
  EXPECT_EQ(sz_reg,ps.get_registry<size_t>());
  EXPECT_EQ(exp_reg,ps.get_registry<real_t>());
}

TEST(hard_coded_source_t,get_value){
  using namespace ale::utils;
  using real_t = hard_coded_source_t::real_t;
  using string_t = hard_coded_source_t::string_t;
  hard_coded_source_t ps;

  reg<real_t> exp_reg = {{"CFL",257.4},{"dtmax",2.3e6}};
  ps.set_registry<real_t>(exp_reg);
  reg<size_t> sz_reg = {{"n_steps",1000000000UL},{"n_zones_x",30000},
    {"n_zones_y",30000},{"n_zones_z",30000},{"CFL",12}};
  ps.set_registry<size_t>(sz_reg);
  reg<string_t> st_reg = {{"mesh_type","useful"},{"eos_type","fictional"},
    {"CFL","indubitable"}};
  ps.set_registry<string_t>(st_reg);

  {
    double d_cfl(-42.0);
    bool ok = ps.get_value("CFL",d_cfl);
    EXPECT_TRUE(ok);
    EXPECT_EQ(d_cfl,257.4);
    d_cfl = -42.0;
    ok = ps.get_value("CFM",d_cfl);
    EXPECT_FALSE(ok);
    EXPECT_EQ(d_cfl,-42.0);
  }
  {
    size_t n_steps(2);
    bool ok = ps.get_value("n_steps",n_steps);
    EXPECT_TRUE(ok);
    EXPECT_EQ(n_steps,1000000000UL);
    n_steps = 2;
    ok = ps.get_value("m_steps",n_steps);
    EXPECT_FALSE(ok);
    EXPECT_EQ(n_steps,2);
  }
} // TEST(hard_coded_source_t,get_value){

TEST(lua_source_t,instantiate){
  using namespace ale::utils;
  using string_t = lua_source_t::string_t;
  string_t fname("mock_box_2d.lua");
  lua_source_t ls(fname);
  EXPECT_TRUE(true);

}

template <class T> using array_t = std::array<T,2>;

TEST(lua_source_t,get_value){
  using namespace ale::utils;
  using string_t = lua_source_t::string_t;
  using real_t = lua_source_t::real_t;
  string_t fname("mock_box_2d.lua");
  lua_source_t ls(fname);
  {
    string_t prefix;
    bool ok = ls.get_value<string_t>("prefix",prefix);
    EXPECT_EQ("mock_box_2d",prefix);
    EXPECT_TRUE(ok);
  }
  {
    string_t suffix;
    bool ok = ls.get_value<string_t>("suffix",suffix);
    EXPECT_EQ("dat",suffix);
    EXPECT_TRUE(ok);
  }
  {
    string_t output_freq;
    bool ok = ls.get_value<string_t>("output_freq",output_freq);
    EXPECT_EQ("7",output_freq);
    EXPECT_TRUE(ok);
  }
  {
    string_t mesh_type;
    bool ok = ls.get_value<string_t>("mesh_type",mesh_type);
    EXPECT_EQ("box",mesh_type);
    EXPECT_TRUE(ok);
  }
  {
    string_t eos_type;
    bool ok = ls.get_value<string_t>("eos_type",eos_type);
    EXPECT_EQ("ideal_gas",eos_type);
    EXPECT_TRUE(ok);
  }
  {
    real_t final_time;
    bool ok = ls.get_value<real_t>("final_time",final_time);
    EXPECT_EQ(0.2,final_time);
    EXPECT_TRUE(ok);
  }
  {
    real_t CFL;
    bool ok = ls.get_value<real_t>("CFL",CFL);
    EXPECT_EQ(0.5,CFL);
    EXPECT_TRUE(ok);
  }
  {
    real_t gas_constant;
    bool ok = ls.get_value<real_t>("gas_constant",gas_constant);
    EXPECT_EQ(1.4,gas_constant);
    EXPECT_TRUE(ok);
  }
  {
    real_t specific_heat;
    bool ok = ls.get_value<real_t>("specific_heat",specific_heat);
    EXPECT_EQ(1.0,specific_heat);
    EXPECT_TRUE(ok);
  }
  {
    array_t<int> dimensions;
    bool ok = ls.get_value<array_t<int>>("dimensions",dimensions);
    array_t<int> exp_dims = {10,10};
    EXPECT_EQ(exp_dims,dimensions);
    EXPECT_TRUE(ok);
  }
  {
    array_t<real_t> xmin;
    bool ok = ls.get_value<array_t<real_t>>("xmin",xmin);
    array_t<real_t> exp_dims = {-0.5,-0.5};
    EXPECT_EQ(exp_dims,xmin);
    EXPECT_TRUE(ok);
  }
  {
    array_t<real_t> xmax;
    bool ok = ls.get_value<array_t<real_t>>("xmax",xmax);
    array_t<real_t> exp_dims = {0.5,0.5};
    EXPECT_EQ(exp_dims,xmax);
    EXPECT_TRUE(ok);
  }
}
// End of file
