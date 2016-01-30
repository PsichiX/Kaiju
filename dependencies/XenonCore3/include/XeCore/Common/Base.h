#ifndef __XE_CORE__COMMON__BASE__
#define __XE_CORE__COMMON__BASE__

#include <stdarg.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <stdint.h>

#ifndef FORCEINLINE
#if (__GNUC__ >= 3)
#define FORCEINLINE __inline  __attribute__((always_inline))
#else
#define FORCEINLINE __inline
#endif
#endif
#define CLAMP(v,mn,mx)			std::max( mn, std::min( mx, v ) )
#define REPEAT(c)				for( unsigned int __c_o_u_n_t_e_r__ = 0; __c_o_u_n_t_e_r__ < ( c ); __c_o_u_n_t_e_r__++ )
#define LERP(f,mn,mx)			( ( ( f ) * ( mx ) ) + ( ( 1.0f - ( f ) ) * ( mn ) ) )
#define DELETE_OBJECT(o)		{ if( o ){ delete o; o = 0; } }
#define DELETE_ARRAY(a)			{ if( a ){ delete[] a; a = 0; } }
#define DEGTORAD(deg)           ( ( deg ) * M_PI / 180.0 )
#define RADTODEG(rad)           ( ( rad ) * 180.0 / M_PI )
#ifndef NDEBUG
#define DEBUG
#else
#define RELEASE
#endif

typedef unsigned char           byte;
typedef unsigned short          word;
typedef unsigned int            dword;
typedef unsigned long           qword;
typedef size_t                  pointer;
typedef int8_t                  int8;
typedef int16_t                 int16;
typedef int32_t                 int32;
typedef int64_t                 int64;
typedef uint8_t                 uint8;
typedef uint16_t                uint16;
typedef uint32_t                uint32;
typedef uint64_t                uint64;

#ifdef _USE_STD_SUBSTITUTES
char* itoa( int value, char* str, int base );
#endif

#endif
