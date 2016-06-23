#include "gtest/gtest.h"
#include "../xint.hpp"
#include "../libs1.hpp"
#include <exception>
#include <cmath>
#include <type_traits>


namespace test_xint {
namespace detail {


typedef long double t_correct1;

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<1024, 65536,
		boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> >
t_correct_int;

/*
template<typename T_INT,
typename std::enable_if<std::is_signed<T_INT>{}>::type* = nullptr >
signed long long int round_co(t_correct1 v) {
	return static_cast<signed long long int>( std::round( v ) );
}

template<typename T_INT,
typename std::enable_if<! std::is_signed<T_INT>{}>::type* = nullptr >
unsigned long long int round_co(t_correct1 v) {
	return static_cast<unsigned long long int>( std::round( v ) );
}
*/
template<typename T_INT>
void math_tests_noproblem() {
	//_mark("Doing tests, on integral with sizeof="<<sizeof(T_INT));

	vector<int> testsize_tab = { 1, 2, 50, 1000, 10000 , 1000000 };
	for (auto testsize : testsize_tab) {
		T_INT a=0;
		int n=testsize;
		for (int i=1; i<n; ++i) a = a + i;
		t_correct_int n_corr = n;
		t_correct_int a_corr = (((n_corr-1)*1)* n_corr) /2;
		EXPECT_EQ(a, a_corr);
	}
}

template <typename T_INT> bool is_safe_int() {
	bool safetype =
		(typeid(T_INT) == typeid(xint))
		|| (typeid(T_INT) == typeid(uxint))
	;
	return safetype;
}

template<typename T_INT>
void math_tests_overflow_incr(T_INT val) { bool safetype = is_safe_int<T_INT>();
	T_INT a = val;	t_correct_int a_corr = a;
	auto func = [&]() { a_corr+=1; a++; } ;
	#define db do { _mark("safe="<<safetype<<"; a="<<a<<" a_corr="<<a_corr); } while(0)

	EXPECT_NO_THROW( func() );
	EXPECT_EQ(a,a_corr); // this should fit for given starting val
	// next icrement is problematic:
	if (safetype) { EXPECT_THROW( func() , std::runtime_error ); }
	else { // should not fit for given val
		EXPECT_NO_THROW( func() ); // usafe type fils to throw
		EXPECT_NE(a , a_corr); // unsafe type has mathematically-invalid value
	}
	#undef db
}

template<typename T_INT>
void math_tests_overflow_decr(T_INT val) { bool safetype = is_safe_int<T_INT>();
	T_INT a = val;	t_correct_int a_corr = a;
	auto func = [&]() { a_corr-=1; a--; } ;
	#define db do { _mark("safe="<<safetype<<"; a="<<a<<" a_corr="<<a_corr); } while(0)
	db;

	EXPECT_NO_THROW( func() );
	EXPECT_EQ(a,a_corr); // this should fit for given starting val
	// next icrement is problematic:
	if (safetype) { db; EXPECT_THROW( func() , std::runtime_error ); db; }
	else { // should not fit for given val
		EXPECT_NO_THROW( func() ); // usafe type fils to throw
		EXPECT_NE(a , a_corr); // unsafe type has mathematically-invalid value
	}
	#undef db
}

} // namespace
} // namespace


// TODO move to general gtest_examples_test.cpp later (and rename from "xint")
TEST(xint, gtest_throw) {
	EXPECT_THROW( { throw std::runtime_error("testerror"); } , std::runtime_error );
}
TEST(xint, gtest_nothrow) {
	EXPECT_NO_THROW( { int a=5; int b=10; int c=55; if (c<a+b) throw std::runtime_error("testerror"); } );
}

TEST(xint, gtest_throw_lambda) {
	// EXPECT_NO_THROW( { int a=5, int b=10, int c=55; if (c>a+b) throw std::runtime_error("testerror"); } );
	auto func = []() { int a=5, b=10, c=55; if (c>a+b) throw std::runtime_error("testerror"); } ;
	EXPECT_THROW( func() , std::runtime_error );
}
TEST(xint, gtest_nothrow_lambda) {
	// EXPECT_NO_THROW( { int a=5, b=10, c=55; if (c>a+b) throw std::runtime_error("testerror"); } );
	auto func = []() { int a=5, b=10, c=55; if (c<a+b) throw std::runtime_error("testerror"); } ;
	EXPECT_NO_THROW( func() );
}

TEST(xint, math1) {
	test_xint::detail::math_tests_noproblem<long int>();
	test_xint::detail::math_tests_noproblem<unsigned long int>();
	test_xint::detail::math_tests_noproblem<xint>();
	test_xint::detail::math_tests_noproblem<uxint>();
}

//const test_xint::detail::t_correct_int maxni = 0xFFFFFFFFFFFFFFFF; // max "normal integer" on this platform
#define maxni 0xFFFFFFFFFFFFFFFFULL // max "normal integer" on this platform


//static_assert(maxni == std::numeric_limits<uint64_t>::max() , "Unexpected max size of normal integer");

#define generate_tests_for_types(FUNCTION, V1,V2,V3,V4) \
TEST(xint, FUNCTION ## _u_i) {	test_xint::detail::math_tests_ ## FUNCTION <uint64_t>(V1); } \
TEST(xint, FUNCTION ## _u_xint) {	test_xint::detail::math_tests_ ## FUNCTION <uxint>(V2); } \
TEST(xint, FUNCTION ## _s_i) {	test_xint::detail::math_tests_ ## FUNCTION <int64_t>(V3); } \
TEST(xint, FUNCTION ## _s_xint) {	test_xint::detail::math_tests_ ## FUNCTION <xint>(V4); }
// we use max_u64-1 for SIGNED xint too, because it can in fact express it it seems?

generate_tests_for_types( overflow_incr , maxni-1, maxni-1, maxni/2-1, maxni-1 )
// generate_tests_for_types( overflow_decr , +1, +1, -(maxni/2), -maxni )

TEST(xint, some_use) {
	xint a("0xFFFFFFFFFFFFFFFF");
	a--;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFE"));
	a--;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFD"));
	a++;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFE"));
	a++;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));

	a = xint("0x8888888888888888");
	a/=2;
	EXPECT_EQ(a , xint("0x4444444444444444"));
	a/=2;
	EXPECT_EQ(a , xint("0x2222222222222222"));
}

TEST(xint, range_u_incr) {
	uxint a("0xFFFFFFFFFFFFFFFE");
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
}
TEST(xint, range_u_decr) {
	uxint a("0x0000000000000001");
	EXPECT_NO_THROW( { a--; } );  EXPECT_EQ(a , xint("0x0000000000000000"));
	EXPECT_THROW( { a--; } , std::runtime_error );	EXPECT_EQ(a , xint("0x0000000000000000"));
	EXPECT_THROW( { a--; } , std::runtime_error );	EXPECT_EQ(a , xint("0x0000000000000000"));
}

TEST(xint, range_s_incr) {
	xint a("0xFFFFFFFFFFFFFFFE");
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
}
TEST(xint, range_s_decr) {
	xint b("0xFFFFFFFFFFFFFFFE");
	xint a(0); a -= b;
	EXPECT_NO_THROW( { a--; } );
	EXPECT_THROW( { a--; } , std::runtime_error );
	EXPECT_THROW( { a--; } , std::runtime_error );
	EXPECT_NO_THROW( { a += b; } );
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , xint("0x0000000000000000"));
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , xint("0x0000000000000001"));
	EXPECT_NO_THROW( { a+=b; } );	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
}

TEST(xint, range_u_to_sizet) {
	size_t s1 = 0xFFFFFFFFFFFFFFFF - 10, s2=8, s3=2, s4=1;
	vector<int> tab10(10);
	vector<int> tabBig(10*1000000);
	ASSERT_EQ(tab10.size(),10u);
	size_t sm = 0xFFFFFFFFFFFFFFFF;
	uxint a = s1;
	xint as = s1;
	UNUSED(a);
	UNUSED(as);
	EXPECT_THROW( { uxint x = uxint(s1)+uxint(s2)+uxint(s3)+uxint(s4); UNUSED(x); } , std::runtime_error );
	{               uxint x = uxint(s1)+uxint(s2)+uxint(s3);  EXPECT_EQ(x,sm); }
	EXPECT_THROW( { uxint x = uxint(s1)+uxint(tab10.size())+uxint(s4); UNUSED(x); } , std::runtime_error );
	EXPECT_THROW( { uxint x = uxint(s1)+uxint(tab10.size())+1-1+1; UNUSED(x); } , std::runtime_error );
	{               uxint x = uxint(s1)+uxint(tab10.size());  EXPECT_EQ(x,sm); }

	EXPECT_THROW( { uxint x = uxint(tabBig.size()) * uxint(tabBig.size()) * uxint(tabBig.size()); UNUSED(x); } , std::runtime_error );
	EXPECT_THROW( { uxint x = xsize(tabBig) * xsize(tabBig) * xsize(tabBig); UNUSED(x); } , std::runtime_error );
}


TEST(xint, range_b_to_sizet) {
	size_t s1 = 0xFFFFFFFFFFFFFFFF - 10, s2=8, s3=2, s4=1;
	vector<int> tab10(10);
	vector<int> tabBig(10*1000000);
	ASSERT_EQ(tab10.size(),10u);
	size_t sm = 0xFFFFFFFFFFFFFFFF;
	uxbigint points = s1, value=5000;
	points *= value;
	EXPECT_THROW( { uxint points_size( points ); size_t s( points_size ); UNUSED(s); }  , std::runtime_error );
	              { uxint points_size( points/value ); size_t s( points_size ); EXPECT_EQ(s,s1); }

	EXPECT_THROW( { size_t s( points ); UNUSED(s); }  , std::runtime_error );
	              { size_t s( points/value ); UNUSED(s); }

	EXPECT_THROW( { size_t s( xsize(tabBig)*xsize(tabBig)*xsize(tabBig) ); UNUSED(s); }  , std::runtime_error );
	              { size_t s( xsize(tabBig)*xsize(tabBig) ); UNUSED(s); }
}




#undef maxni








