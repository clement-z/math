//  (C) Copyright John Maddock 2005-2006.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_LOG1P_INCLUDED
#define BOOST_MATH_LOG1P_INCLUDED

#include <cmath>
#include <math.h> // platform's ::log1p
#include <boost/limits.hpp>
#include <boost/math/tools/config.hpp>
#include <boost/math/tools/series.hpp>
#include <boost/math/tools/precision.hpp>

#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
#  include <boost/static_assert.hpp>
#else
#  include <boost/assert.hpp>
#endif

namespace boost{ namespace math{

namespace detail
{
  // Functor log1p_series returns the next term in the Taylor series
  //   pow(-1, k-1)*pow(x, k) / k
  // each time that operator() is invoked.
  //
  template <class T>
  struct log1p_series
  {
     typedef T result_type;

     log1p_series(T x)
        : k(0), m_mult(-x), m_prod(-1){}

     T operator()()
     {
        m_prod *= m_mult;
        return m_prod / ++k;
     }

     int count()const
     {
        return k;
     }

  private:
     int k;
     const T m_mult;
     T m_prod;
     log1p_series(const log1p_series&);
     log1p_series& operator=(const log1p_series&);
  };

} // namespace detail

// Algorithm log1p is part of C99, but is not yet provided by many compilers.
//
// This version uses a Taylor series expansion for 0.5 > x > epsilon, which may
// require up to std::numeric_limits<T>::digits+1 terms to be calculated. 
// It would be much more efficient to use the equivalence:
//   log(1+x) == (log(1+x) * x) / ((1-x) - 1)
// Unfortunately optimizing compilers make such a mess of this, that it performs
// no better than log(1+x): which is to say not very well at all.
//
template <class T>
T log1p(T x)
{ // The function returns the natural logarithm of 1 + x.
  // A domain error occurs if x < -1.  TODO should there be a check?
   using namespace std;

   T a = fabs(x);
   if(a > T(0.5L))
      return log(1 + x);
   // Note that without numeric_limits specialisation support, 
   // epsilon just returns zero, and our "optimisation" will always fail:
   if(a < tools::epsilon<T>())
      return x;
   detail::log1p_series<T> s(x);
   boost::uintmax_t max_iter = BOOST_MATH_MAX_ITER;
#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
   T result = tools::sum_series(s, tools::digits<T>() + 2, max_iter);
#else
   T zero = 0;
   T result = tools::sum_series(s, tools::digits<T>() + 2, max_iter, zero);
#endif
   tools::check_series_iterations(BOOST_CURRENT_FUNCTION, max_iter);
   return result;
}

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
// These overloads work around a type deduction bug:
inline float log1p(float z)
{
   return log1p<float>(z);
}
inline double log1p(double z)
{
   return log1p<double>(z);
}
inline long double log1p(long double z)
{
   return log1p<long double>(z);
}
#endif

#ifdef log1p
#  ifndef BOOST_HAS_LOG1P
#     define BOOST_HAS_LOG1P
#  endif
#  undef log1p
#endif

#ifdef BOOST_HAS_LOG1P
#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
template <>
inline float log1p<float>(float x){ return ::log1pf(x); }
template <>
inline long double log1p<long double>(long double x){ return ::log1pl(x); }
#else
template <>
inline float log1p<float>(float x){ return ::log1p(x); }
#endif
template <>
inline double log1p<double>(double x){ return ::log1p(x); }
#elif defined(_MSC_VER)
//
// You should only enable this branch if you are absolutely sure
// that your compilers optimizer won't mess this code up!!
// Currently tested with VC8 and Intel 9.1.
//
template <>
inline double log1p<double>(double x)
{
   double u = 1+x;
   if(u == 1.0) 
      return x; 
   else
      return log(u)*(x/(u-1.0));
}
template <>
inline float log1p<float>(float x)
{
   return static_cast<float>(boost::math::log1p<double>(x));
}
template <>
inline long double log1p<long double>(long double x)
{
   long double u = 1+x;
   if(u == 1.0) 
      return x; 
   else
      return log(u)*(x/(u-1.0));
}
#endif

} // namespace math
} // namespace boost

#endif // BOOST_MATH_LOG1P_INCLUDED

