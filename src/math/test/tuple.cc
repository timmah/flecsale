/*~-------------------------------------------------------------------------~~*
 *     _   ______________     ___    __    ______
 *    / | / / ____/ ____/    /   |  / /   / ____/
 *   /  |/ / / __/ /  ______/ /| | / /   / __/   
 *  / /|  / /_/ / /__/_____/ ___ |/ /___/ /___   
 * /_/ |_/\____/\____/    /_/  |_/_____/_____/   
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
/*!
 *
 * \file vector.cc
 * 
 * \brief Tests related to the vector class.
 *
 ******************************************************************************/

// system includes
#include <cinchtest.h>
#include <iostream>

// user includes
#include "ale/math/tuple.h"


// explicitly use some stuff
using namespace ale::math;

using std::make_tuple;

using real_t = double;
using tuple_1d_t = tuple_t<real_t>;
using tuple_2d_t = tuple_t<real_t,real_t>;
using std_tuple_2d = std::tuple<real_t,real_t>;



///////////////////////////////////////////////////////////////////////////////
//! \brief Test the intialization
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, init) {

  // { 1.0 }
  tuple_1d_t ans_1d{ 1.0 };
  //tuple_1d_t a1( 1.0 );    ASSERT_TRUE( a1 == ans_1d );
  tuple_1d_t a2{ 1.0 };    ASSERT_TRUE( a2 == ans_1d );
  //tuple_1d_t a3 = 1.0;     ASSERT_TRUE( a3 == ans_1d );
  tuple_1d_t a4 = make_tuple( 1.0 ); ASSERT_TRUE( a4 == ans_1d );
  tuple_1d_t a5 = a2;
  tuple_1d_t a6( a2 );
  fill( a2, 6.0 );
  ASSERT_TRUE( a5 == ans_1d );
  ASSERT_TRUE( a6 == ans_1d );

  // { 1.0, 2.0 }
  tuple_2d_t ans_2d{ 1.0, 2.0 };
  //tuple_2d_t b1( 1.0, 2.0 );      ASSERT_TRUE( b1 == ans_2d );
  tuple_2d_t b2{ 1.0, 2.0 };      ASSERT_TRUE( b2 == ans_2d );
  tuple_2d_t b3( b2 );            ASSERT_TRUE( b3 == ans_2d );
  tuple_2d_t b4 = make_tuple(1.0, 2.0);  ASSERT_TRUE( b4 == ans_2d );
  tuple_2d_t b5; 
  b5 = make_tuple(2.0, 1.0); ASSERT_FALSE( b5 == ans_2d );
  b5 = make_tuple(1.0, 2.0); ASSERT_TRUE ( b5 == ans_2d );

  // { 1.0, 1.0 }
  //ans_2d = { 1.0, 1.0 };
  //tuple_2d_t b6( 1.0 );    ASSERT_TRUE( b6 == ans_2d );
  //tuple_2d_t b7{ 1.0 };    ASSERT_TRUE( b7 == ans_2d );
  //tuple_2d_t b8 = {1.0};   ASSERT_TRUE( b8 == 1.0 );
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the addition
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, addition_1d) {

  // { 1.0 } + { 2.0 } = { 3.0 }
  tuple_1d_t ans{ 3.0 }; 
  tuple_1d_t a( 1.0 );
  tuple_1d_t b{ 2.0 };

  auto c = a;
  plus_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator+= with tuple";
 
  auto d = a + b; 
  ASSERT_TRUE( d == ans ) << " error in operator+ with tuple";


  // { 1.0 } + 2.0 = { 3.0 }
  ans = make_tuple( 3.0 );
  real_t val = 2.0;

  auto e = a;
  plus_equal( e, val );
  ASSERT_TRUE( e == ans ) << " error in operator+= with scalar";
  
  auto f = a + val; 
  ASSERT_TRUE( f == ans ) << " error in operator+ with scalar";
  
  auto g = val + a; 
  ASSERT_TRUE( g == ans ) << " error in operator+ with scalar";

}



///////////////////////////////////////////////////////////////////////////////
//! \brief Test the addition
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, addition_2d) {


  // { 1.0, 2.0 } + { 2.0, 1.0 } = { 3.0, 3.0 }
  tuple_2d_t a{ 1.0, 2.0 };
  tuple_2d_t b{ 2.0, 1.0 };
  tuple_2d_t ans{ 3.0, 3.0 };

  auto c = a;
  plus_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator+= with tuple";
  
  auto d = a + b; 
  ASSERT_TRUE( d == ans ) << " error in operator+ with tuple";

  // { 1.0, 2.0 } + 2.0 = { 3.0, 4.0 }
  ans = make_tuple( 3.0, 4.0 );
  real_t val = 2.0;

  auto e = a;
  plus_equal( e, val );
  ASSERT_TRUE( e == ans ) << " error in operator+= with scalar";
  
  auto f = a + val; 
  ASSERT_TRUE( f == ans ) << " error in operator+ with scalar";

  auto g = val + a; 
  ASSERT_TRUE( g == ans ) << " error in operator+ with scalar";
 
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the subtraction
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, subtraction_1d) {

  // { 1.0 } - { 2.0 } = { -1.0 }
  tuple_1d_t ans{ -1.0 }; 
  tuple_1d_t a( 1.0 );
  tuple_1d_t b{ 2.0 };

  auto c = a;
  minus_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator-= with tuple";
  
  auto d = a - b; 
  ASSERT_TRUE( d == ans ) << " error in operator- with tuple";

  // { 1.0 } - 2.0 = { -1.0 }
  ans = make_tuple( -1.0 );
  real_t val = 2.0;

  auto e = a;
  minus_equal( e, val );
  ASSERT_TRUE( e == ans ) << " error in operator+= with scalar";
  
  auto f = a - val; 
  ASSERT_TRUE( f == ans ) << " error in operator+ with scalar";

  // 2.0 - { 1.0 } = { 1.0 }
  ans = make_tuple( 1.0 );

  auto g = val - a; 
  ASSERT_TRUE( g == ans ) << " error in operator- with scalar";  

}


///////////////////////////////////////////////////////////////////////////////
//! \brief Test the subtraction
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, subtraction_2d) {


  // { 1.0, 2.0 } - { 2.0, 1.0 } = { -1.0, 1.0 }
  tuple_2d_t a{ 1.0, 2.0 };
  tuple_2d_t b{ 2.0, 1.0 };
  tuple_2d_t ans{ -1.0, 1.0 };

  auto c = a;
  minus_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator-= with tuple";
  
  auto d = a - b; 
  ASSERT_TRUE( d == ans ) << " error in operator- with tuple";

  // { 1.0, 2.0 } - 2.0 = { -1.0, 0.0 }
  ans = make_tuple( -1.0, 0.0 );
  real_t val = 2.0;

  auto e = a;
  minus_equal( e, val );
  ASSERT_TRUE( e == ans ) << " error in operator-= with scalar";
  
  auto f = a - val; 
  ASSERT_TRUE( f == ans ) << " error in operator- with scalar";
   
  // 2.0 - { 1.0, 2.0 } = { 1.0, 0.0 }
  ans = make_tuple( 1.0, 0.0 );

  auto g = val - a; 
  ASSERT_TRUE( g == ans ) << " error in operator- with scalar";

}


///////////////////////////////////////////////////////////////////////////////
//! \brief Test the multiplication
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, multiply_1d) {

  // { 2.0 } * { 6.0 } = { 6.0 }
  tuple_1d_t ans{ 6.0 }; 
  tuple_1d_t a{ 2.0 };
  tuple_1d_t b{ 3.0 };

  auto c = a;
  multiplies_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator*= with tuple";
  
  auto d = a * b; 
  ASSERT_TRUE( d == ans ) << " error in operator* with tuple";

  // { 2.0 } * 3.0 = { 6.0 }
  ans = make_tuple( 6.0 );
  real_t val = 3.0;

  auto e = a;
  multiplies_equal( e, val );
  ASSERT_TRUE( e == ans ) << " error in operator*= with scalar";
  
  auto f = a * val; 
  ASSERT_TRUE( f == ans ) << " error in operator* with scalar";
  
  // 3.0 * { 2.0 } = { 6.0 }
  ans = make_tuple( 6.0 );

  auto g = val * a; 
  ASSERT_TRUE( g == ans ) << " error in operator* with scalar";

}

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the multiplication
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, multiply_2d) {


  // { 2.0, 3.0 } + { 3.0, 2.0 } = { 6.0, 6.0 }
  tuple_2d_t a{ 2.0, 3.0 };
  tuple_2d_t b{ 3.0, 2.0 };
  tuple_2d_t ans{ 6.0, 6.0 };

  auto c = a;
  multiplies_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator*= with tuple";
  
  auto d = a * b; 
  ASSERT_TRUE( d == ans ) << " error in operator* with tuple";

  // { 2.0, 3.0 } * 2.0 = { 4.0, 6.0 }
  ans = make_tuple( 4.0, 6.0 );
  real_t val = 2.0;

  auto e = a;
  multiplies_equal( e, val );
  ASSERT_TRUE( e == ans ) << " error in operator*= with scalar";
  
  auto f = a * val; 
  ASSERT_TRUE( f == ans ) << " error in operator* with scalar";

  // 3.0 * { 2.0, 3.0 } = { 4.0, 6.0 }
  ans = make_tuple( 4.0, 6.0 );

  auto g = val * a; 
  ASSERT_TRUE( g == ans ) << " error in operator* with scalar";
 
}


///////////////////////////////////////////////////////////////////////////////
//! \brief Test the multiplication
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, divide_1d) {

  // { 6.0 } / { 2.0 } = { 3.0 }
  tuple_1d_t ans{ 3.0 }; 
  tuple_1d_t a{ 6.0 };
  tuple_1d_t b{ 2.0 };

  auto c = a;
  divides_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator/= with tuple";
  
  auto d = a / b; 
  ASSERT_TRUE( d == ans ) << " error in operator/ with tuple";

  // { 6.0 } / 3.0 = { 2.0 }
  ans = make_tuple( 2.0 );
  real_t val = 3.0;

  auto e = a;
  divides_equal( e, val );
  ASSERT_TRUE( e == ans ) << " error in operator/= with scalar";
  
  auto f = a / val; 
  ASSERT_TRUE( f == ans ) << " error in operator/ with scalar";
  
  // 12.0 / { 6.0 } = { 2.0 }
  ans = make_tuple( 2.0 );
  val = 12.0;

  auto g = val / a; 
  ASSERT_TRUE( g == ans ) << " error in operator/ with scalar";

}

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the multiplication
///////////////////////////////////////////////////////////////////////////////
TEST(tuple, divide_2d) {


  // { 12.0, 12.0 } / { 3.0, 2.0 } = { 8.0, 6.0 }
  tuple_2d_t a{ 24.0, 12.0 };
  tuple_2d_t b{ 3.0, 2.0 };
  tuple_2d_t ans{ 8.0, 6.0 };

  auto c = a;
  divides_equal( c, b );
  ASSERT_TRUE( c == ans ) << " error in operator/= with tuple";
  
  auto d = a / b; 
  ASSERT_TRUE( d == ans ) << " error in operator/ with tuple";

  // { 24.0, 12.0 } / 2.0 = { 12.0, 6.0 }
  ans = make_tuple( 12.0, 6.0 );
  real_t val = 2.0;

  auto e = a;
  divides_equal( e, 2.0 );
  ASSERT_TRUE( e == ans ) << " error in operator/= with scalar";
  
  auto f = a / 2.0; 
  ASSERT_TRUE( f == ans ) << " error in operator/ with scalar";

  // 48.0 / { 24.0, 12.0 } = { 2.0, 4.0 }
  ans = make_tuple( 2.0, 4.0 );
  val = 48.0;

  auto g = val / a; 
  ASSERT_TRUE( g == ans ) << " error in operator/ with scalar";
 
}


/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
