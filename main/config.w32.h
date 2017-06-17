/*
	Build Configuration Template for Win32.
	$Id$
*/

/* Define the minimum supported version */
/*
重新定义了这两个数值
*/
#undef _WIN32_WINNT
#undef NTDDI_VERSION
//这两个数值目前不是很明确
#define _WIN32_WINNT 0x0600
#define NTDDI_VERSION  0x06000100

/* Default PHP / PEAR directories */
#define PHP_CONFIG_FILE_PATH (getenv("SystemRoot")?getenv("SystemRoot"):"")
#define CONFIGURATION_FILE_PATH "php.ini"
#define PEAR_INSTALLDIR "C:\\php\\pear"
#define PHP_BINDIR "C:\\php"
#define PHP_DATADIR "C:\\php"
#define PHP_EXTENSION_DIR "C:\\php\\ext"
#define PHP_INCLUDE_PATH	".;C:\\php\\pear"
#define PHP_LIBDIR "C:\\php"
#define PHP_LOCALSTATEDIR "C:\\php"
#define PHP_PREFIX "C:\\php"
#define PHP_SYSCONFDIR "C:\\php"

/* PHP Runtime Configuration */
#define PHP_URL_FOPEN 1
#define USE_CONFIG_FILE 1
#define DEFAULT_SHORT_OPEN_TAG "1"

/* Platform-Specific Configuration. Should not be changed. */
#define PHP_SIGCHILD 0
#define HAVE_LIBBIND 1
#define HAVE_GETSERVBYNAME 1
#define HAVE_GETSERVBYPORT 1
#define HAVE_GETPROTOBYNAME 1
#define HAVE_GETPROTOBYNUMBER 1
#define HAVE_GETHOSTNAME 1
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define HAVE_ERRMSG_H 0
#undef HAVE_ADABAS
#undef HAVE_SOLID
#undef HAVE_LINK
#undef HAVE_SYMLINK

/* its in win32/time.c */
#define HAVE_USLEEP 1
#define HAVE_NANOSLEEP 1
#define PHP_SLEEP_NON_VOID 1

#define HAVE_GETHOSTNAME 1
#define HAVE_GETCWD 1
#define HAVE_POSIX_READDIR_R 1
#define NEED_ISBLANK 1
#define DISCARD_PATH 0
#undef HAVE_SETITIMER
#undef HAVE_SIGSETJMP
#undef HAVE_IODBC
#define HAVE_LIBDL 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_PUTENV 1
#define HAVE_LIMITS_H 1
#define HAVE_TZSET 1
#define HAVE_TZNAME 1
#undef HAVE_FLOCK
#define HAVE_ALLOCA 1
#undef HAVE_SYS_TIME_H
#define HAVE_SIGNAL_H 1
#undef HAVE_ST_BLKSIZE
#undef HAVE_ST_BLOCKS
#define HAVE_ST_RDEV 1
#define HAVE_UTIME_NULL 1
#define HAVE_VPRINTF 1
#define STDC_HEADERS 1
#define REGEX 1
#define HSREGEX 1
#define HAVE_GCVT 1
#define HAVE_GETLOGIN 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_PUTENV 1
#define HAVE_REGCOMP 1
#define HAVE_SETLOCALE 1
#define HAVE_LOCALECONV 1
#define HAVE_LOCALE_H 1
#ifndef HAVE_LIBBIND	//HAVE_LIBBIND的上面已经定义成是1了
# define HAVE_SETVBUF 1
#endif
#define HAVE_SHUTDOWN 1
#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_STRCASECMP 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRSTR 1
#define HAVE_TEMPNAM 1
#define HAVE_UTIME 1
#undef HAVE_DIRENT_H
#define HAVE_ASSERT_H 1
#define HAVE_FCNTL_H 1
#define HAVE_GRP_H 0
#undef HAVE_PWD_H
#define HAVE_STRING_H 1
#undef HAVE_SYS_FILE_H
#undef HAVE_SYS_SOCKET_H
#undef HAVE_SYS_WAIT_H
#define HAVE_SYSLOG_H 1
#undef HAVE_UNISTD_H
#define HAVE_SYS_TYPES_H 1
#define HAVE_STDARG_H 1
#undef HAVE_ALLOCA_H
#undef HAVE_KILL
#define HAVE_GETPID 1
#define HAVE_LIBM 1
#define HAVE_CUSERID 0
#undef HAVE_RINT
#define HAVE_STRFTIME 1
#define SIZEOF_SHORT 2
/* int and long are stll 32bit in 64bit compiles */
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
/* MSVC.6/NET don't allow 'long long' or know 'intmax_t' */
#define SIZEOF_LONG_LONG_INT 0
#define SIZEOF_LONG_LONG 8 /* defined as __int64 */
#define SIZEOF_INTMAX_T 0
#define ssize_t SSIZE_T
//_WIN64 用来判断编译环境是 x86 还是 x64
#ifdef _WIN64
# define SIZEOF_SIZE_T 8
# define SIZEOF_PTRDIFF_T 8
#else
# define SIZEOF_SIZE_T 4
# define SIZEOF_PTRDIFF_T 4
#endif
#define HAVE_FNMATCH
#define HAVE_GLOB
#define PHP_SHLIB_SUFFIX "dll"
#define HAVE_SQLDATASOURCES

/* Win32 supports strcoll */
#define HAVE_STRCOLL 1

/* Win32 supports socketpair by the emulation in win32/sockets.c */
#define HAVE_SOCKETPAIR 1
#define HAVE_SOCKLEN_T 1

/* Win32 support proc_open */
#define PHP_CAN_SUPPORT_PROC_OPEN 1

/* inet_ntop() / inet_pton() */
#define HAVE_INET_PTON 1
#define HAVE_INET_NTOP 1

#define HAVE_MBLEN

#undef HAVE_ATOF_ACCEPTS_NAN
#undef HAVE_ATOF_ACCEPTS_INF
#define HAVE_HUGE_VAL_NAN 0

/* vs.net 2005 has a 64-bit time_t.  This will likely break
 * 3rdParty libs that were built with older compilers; switch
 * back to 32-bit */
#ifndef _WIN64
# define _USE_32BIT_TIME_T 1
#endif
#define HAVE_STDLIB_H 1

#define _REENTRANT 1
#define HAVE_MBRLEN 1
#define HAVE_MBSTATE_T 1

#define HAVE_HUGE_VAL_INF 1

#define HAVE_GETRUSAGE

#define HAVE_FTOK 1

/* values determined by configure.js */

/* Configure line */
#define CONFIGURE_COMMAND "cscript /nologo configure.js  \"--disable-all\" \"--enable-cli\""

/* Detected compiler version */
#define COMPILER "MSVC14 (Visual C++ 2015)"

/* Compiler compatibility ID */
#define PHP_COMPILER_ID "VC14"

/* Detected compiler architecture */
#define ARCHITECTURE "x64"

#define HAVE_STRNLEN 1

/* have the wspiapi.h header file */
#define HAVE_WSPIAPI_H 1

#define HAVE_GETADDRINFO 1

#define HAVE_GAI_STRERROR 1

#define HAVE_IPV6 1

#define HAVE_USLEEP 1

#define HAVE_STRCOLL 1

/* Have date/time support */
#define HAVE_DATE 1

/* Have timelib_config.h */
#define HAVE_TIMELIB_CONFIG_H 1

/* Using bundled PCRE library */
#define HAVE_BUNDLED_PCRE 1

/* Have PCRE library */
#define HAVE_PCRE 1

/* Reflection support enabled */
#define HAVE_REFLECTION 1

#define HAVE_SPL 1

#define PHP_CONFIG_FILE_SCAN_DIR ""

#define PHP_USE_PHP_CRYPT_R 1

#define HAVE_ACOSH 1
#define HAVE_ASINH 1
#define HAVE_ATANH 1
