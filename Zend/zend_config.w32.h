/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2017 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_CONFIG_W32_H
#define ZEND_CONFIG_W32_H

#include <../main/config.w32.h>

#define _CRTDBG_MAP_ALLOC

#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

#include <string.h>

#ifndef ZEND_INCLUDE_FULL_WINDOWS_HEADERS
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>

#include <float.h>

//全转变成无符号类型
typedef unsigned long ulong;
typedef unsigned int uint;

#define HAVE_STDIOSTR_H 1
#define HAVE_CLASS_ISTDIOSTREAM
#define istdiostream stdiostream

#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
//定义了几个函数。
/*
比较字符串
当s1<s2时，返回值<0
当s1=s2时，返回值=0
当s1>s2时，返回值>0
*/
#define strcasecmp(s1, s2) _stricmp(s1, s2
/*
比较字符串str1和str2的前n个字符串字典序的大小
*/
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)

//int _fpclass(double x)用于检验一个浮点数的类型。
//_fpclass的返回值有：
//_FPCLASS_SNAN     /* signaling NaN */
//_FPCLASS_QNAN     /* quiet NaN */
//_FPCLASS_NINF     /* negative infinity */
//_FPCLASS_NN       /* negative normal */
//_FPCLASS_ND       /* negative denormal */
//_FPCLASS_NZ       /* -0 */
//_FPCLASS_PZ       /* +0 */
//_FPCLASS_PD       /* positive denormal */
//_FPCLASS_PN       /* positive normal */
//_FPCLASS_PINF     /* positive infinity */

#define zend_isinf(a)	((_fpclass(a) == _FPCLASS_PINF) || (_fpclass(a) == _FPCLASS_NINF))
#define zend_finite(x)	_finite(x)
#define zend_isnan(x)	_isnan(x)

#define zend_sprintf sprintf

/* This will cause the compilation process to be MUCH longer, but will generate
 * a much quicker PHP binary
 */
#ifdef ZEND_WIN32_FORCE_INLINE
/* _ALLOW_KEYWORD_MACROS is only relevant for C++ */
# ifndef _ALLOW_KEYWORD_MACROS
#  define _ALLOW_KEYWORD_MACROS
# endif
# undef inline
# define inline __forceinline
#elif !defined(ZEND_WIN32_KEEP_INLINE)
# undef inline
# define inline
#endif

#ifdef LIBZEND_EXPORTS
#	define ZEND_API __declspec(dllexport)
#else
#	define ZEND_API __declspec(dllimport)
#endif

#define ZEND_DLEXPORT		__declspec(dllexport)
#define ZEND_DLIMPORT		__declspec(dllimport)

#endif /* ZEND_CONFIG_W32_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
