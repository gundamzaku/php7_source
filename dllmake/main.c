#define ZEND_INCLUDE_FULL_WINDOWS_HEADERS

/*我自己配的定义，方便调试，PHP的不知道，好像是写在MAKEFILE里面了*/
/*
#define ZEND_WIN32 1
#define WIN32 1
#define PHP_WIN32 1
#define HAVE_SIGNAL_H 1
#define HAVE_SETLOCALE 1
#define _WIN64 1
#define ZEND_WIN32_FORCE_INLINE 1
#define ZTS 1
#define ZEND_DEBUG 0
#define PHP_EXPORTS 1
*/* 
/*结束*/

#include "php.h"
#include <stdio.h>
#include <fcntl.h>
#ifdef PHP_WIN32
#include "win32/time.h"
#include "win32/signal.h"
#include "win32/php_win32_globals.h"
#include "win32/winutil.h"
#include <process.h>
#elif defined(NETWARE)
#include <sys/timeval.h>
#ifdef USE_WINSOCK
#include <novsock2.h>
#endif
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_SETLOCALE
#include <locale.h>
#endif
#include "zend.h"
#include "zend_types.h"
#include "zend_extensions.h"
#include "php_ini.h"
#include "php_globals.h"
#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/php_string.h"
#include "ext/date/php_date.h"
#include "php_variables.h"
#include "ext/standard/credits.h"
#ifdef PHP_WIN32
#include <io.h>
#include "win32/php_registry.h"
#include "ext/standard/flock_compat.h"
#endif
#include "php_syslog.h"
#include "Zend/zend_exceptions.h"
#if PHP_SIGCHILD
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "zend_compile.h"
#include "zend_execute.h"
#include "zend_highlight.h"
#include "zend_extensions.h"
#include "zend_ini.h"
#include "zend_dtrace.h"

#include "php_content_types.h"
#include "php_ticks.h"
#include "php_streams.h"
#include "php_open_temporary_file.h"

#include "SAPI.h"
#include "rfc1867.h"

#if HAVE_MMAP || defined(PHP_WIN32)
# if HAVE_UNISTD_H
#  include <unistd.h>
#  if defined(_SC_PAGESIZE)
#    define REAL_PAGE_SIZE sysconf(_SC_PAGESIZE);
#  elif defined(_SC_PAGE_SIZE)
#    define REAL_PAGE_SIZE sysconf(_SC_PAGE_SIZE);
#  endif
# endif
# if HAVE_SYS_MMAN_H
#  include <sys/mman.h>
# endif
# ifndef REAL_PAGE_SIZE
#  ifdef PAGE_SIZE
#   define REAL_PAGE_SIZE PAGE_SIZE
#  else
#   define REAL_PAGE_SIZE 4096
#  endif
# endif
#endif

//PHPAPI int(*php_register_internal_extensions_func)(void) = php_register_internal_extensions;

#ifndef ZTS
php_core_globals core_globals;
#else
PHPAPI int core_globals_id;
#endif

#define SAFE_FILENAME(f) ((f)?(f):"-")

PHPAPI void show_dll()
{
	printf("fuck you");
}

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnSetPrecision)
{
	zend_long i;

	ZEND_ATOL(i, ZSTR_VAL(new_value));
	if (i >= -1) {
		EG(precision) = i;
		return SUCCESS;
	}
	else {
		return FAILURE;
	}
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnSetSerializePrecision)
{
	zend_long i;

	ZEND_ATOL(i, ZSTR_VAL(new_value));
	if (i >= -1) {
		PG(serialize_precision) = i;
		return SUCCESS;
	}
	else {
		return FAILURE;
	}
}
/* }}} */


/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnChangeMemoryLimit)
{
	if (new_value) {
		PG(memory_limit) = zend_atol(ZSTR_VAL(new_value), (int)ZSTR_LEN(new_value));
	}
	else {
		PG(memory_limit) = 1 << 30;		/* effectively, no limit */
	}
	return zend_set_memory_limit(PG(memory_limit));
}
/* }}} */

/* {{{ php_disable_functions
*/
static void php_disable_functions(void)
{
	char *s = NULL, *e;

	if (!*(INI_STR("disable_functions"))) {
		return;
	}

	e = PG(disable_functions) = strdup(INI_STR("disable_functions"));
	if (e == NULL) {
		return;
	}
	while (*e) {
		switch (*e) {
		case ' ':
		case ',':
			if (s) {
				*e = '\0';
				zend_disable_function(s, e - s);
				s = NULL;
			}
			break;
		default:
			if (!s) {
				s = e;
			}
			break;
		}
		e++;
	}
	if (s) {
		zend_disable_function(s, e - s);
	}
}
/* }}} */

/* {{{ php_disable_classes
*/
static void php_disable_classes(void)
{
	char *s = NULL, *e;

	if (!*(INI_STR("disable_classes"))) {
		return;
	}

	e = PG(disable_classes) = strdup(INI_STR("disable_classes"));

	while (*e) {
		switch (*e) {
		case ' ':
		case ',':
			if (s) {
				*e = '\0';
				zend_disable_class(s, e - s);
				s = NULL;
			}
			break;
		default:
			if (!s) {
				s = e;
			}
			break;
		}
		e++;
	}
	if (s) {
		zend_disable_class(s, e - s);
	}
}
/* }}} */

/* {{{ php_binary_init
*/
static void php_binary_init(void)
{
	char *binary_location;
#ifdef PHP_WIN32
	binary_location = (char *)malloc(MAXPATHLEN);
	if (GetModuleFileName(0, binary_location, MAXPATHLEN) == 0) {
		free(binary_location);
		PG(php_binary) = NULL;
	}
#else
	if (sapi_module.executable_location) {
		binary_location = (char *)malloc(MAXPATHLEN);
		if (!strchr(sapi_module.executable_location, '/')) {
			char *envpath, *path;
			int found = 0;

			if ((envpath = getenv("PATH")) != NULL) {
				char *search_dir, search_path[MAXPATHLEN];
				char *last = NULL;
				zend_stat_t s;

				path = estrdup(envpath);
				search_dir = php_strtok_r(path, ":", &last);

				while (search_dir) {
					snprintf(search_path, MAXPATHLEN, "%s/%s", search_dir, sapi_module.executable_location);
					if (VCWD_REALPATH(search_path, binary_location) && !VCWD_ACCESS(binary_location, X_OK) && VCWD_STAT(binary_location, &s) == 0 && S_ISREG(s.st_mode)) {
						found = 1;
						break;
					}
					search_dir = php_strtok_r(NULL, ":", &last);
				}
				efree(path);
			}
			if (!found) {
				free(binary_location);
				binary_location = NULL;
			}
		}
		else if (!VCWD_REALPATH(sapi_module.executable_location, binary_location) || VCWD_ACCESS(binary_location, X_OK)) {
			free(binary_location);
			binary_location = NULL;
		}
	}
	else {
		binary_location = NULL;
	}
#endif
	PG(php_binary) = binary_location;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateTimeout)
{
	if (stage == PHP_INI_STAGE_STARTUP) {
		/* Don't set a timeout on startup, only per-request */
		ZEND_ATOL(EG(timeout_seconds), ZSTR_VAL(new_value));
		return SUCCESS;
	}
	zend_unset_timeout();
	ZEND_ATOL(EG(timeout_seconds), ZSTR_VAL(new_value));
	zend_set_timeout(EG(timeout_seconds), 0);
	return SUCCESS;
}
/* }}} */

/* {{{ php_get_display_errors_mode() helper function
*/
static int php_get_display_errors_mode(char *value, int value_length)
{
	int mode;

	if (!value) {
		return PHP_DISPLAY_ERRORS_STDOUT;
	}
	//为什么这里要三个括号？？liudan
	if (value_length == 2 && !strcasecmp("on", value))) {
	mode = PHP_DISPLAY_ERRORS_STDOUT;
	}
	else if (value_length == 3 && !strcasecmp("yes", value))) {
	mode = PHP_DISPLAY_ERRORS_STDOUT;
	}
	else if (value_length == 4 && !strcasecmp("true", value))) {
	mode = PHP_DISPLAY_ERRORS_STDOUT;
	}
	else if (value_length == 6 && !strcasecmp(value, "stderr"))) {
	mode = PHP_DISPLAY_ERRORS_STDERR;
	}
	else if (value_length == 6 && !strcasecmp(value, "stdout"))) {
	mode = PHP_DISPLAY_ERRORS_STDOUT;
	}
	else {
		ZEND_ATOL(mode, value);
		if (mode && mode != PHP_DISPLAY_ERRORS_STDOUT && mode != PHP_DISPLAY_ERRORS_STDERR) {
			mode = PHP_DISPLAY_ERRORS_STDOUT;
		}
	}

	return mode;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateDisplayErrors)
{
	PG(display_errors) = (zend_bool)php_get_display_errors_mode(ZSTR_VAL(new_value), (int)ZSTR_LEN(new_value));

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI_DISP
*/
static PHP_INI_DISP(display_errors_mode)
{
	int mode, tmp_value_length, cgi_or_cli;
	char *tmp_value;

	if (type == ZEND_INI_DISPLAY_ORIG && ini_entry->modified) {
		tmp_value = (ini_entry->orig_value ? ZSTR_VAL(ini_entry->orig_value) : NULL);
		tmp_value_length = (int)(ini_entry->orig_value ? ZSTR_LEN(ini_entry->orig_value) : 0);
	}
	else if (ini_entry->value) {
		tmp_value = ZSTR_VAL(ini_entry->value);
		tmp_value_length = (int)ZSTR_LEN(ini_entry->value);
	}
	else {
		tmp_value = NULL;
		tmp_value_length = 0;
	}

	mode = php_get_display_errors_mode(tmp_value, tmp_value_length);

	/* Display 'On' for other SAPIs instead of STDOUT or STDERR */
	cgi_or_cli = (!strcmp(sapi_module.name, "cli") || !strcmp(sapi_module.name, "cgi"));

	switch (mode) {
	case PHP_DISPLAY_ERRORS_STDERR:
		if (cgi_or_cli) {
			PUTS("STDERR");
		}
		else {
			PUTS("On");
		}
		break;

	case PHP_DISPLAY_ERRORS_STDOUT:
		if (cgi_or_cli) {
			PUTS("STDOUT");
		}
		else {
			PUTS("On");
		}
		break;

	default:
		PUTS("Off");
		break;
	}
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateDefaultCharset)
{
	if (new_value) {
		OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
#ifdef PHP_WIN32
		php_win32_cp_do_update(ZSTR_VAL(new_value));
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateInternalEncoding)
{
	if (new_value) {
		OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
#ifdef PHP_WIN32
		php_win32_cp_do_update(ZSTR_VAL(new_value));
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateInputEncoding)
{
	if (new_value) {
		OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
#ifdef PHP_WIN32
		php_win32_cp_do_update(NULL);
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateOutputEncoding)
{
	if (new_value) {
		OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
#ifdef PHP_WIN32
		php_win32_cp_do_update(NULL);
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateErrorLog)
{
	/* Only do the safemode/open_basedir check at runtime */
	if ((stage == PHP_INI_STAGE_RUNTIME || stage == PHP_INI_STAGE_HTACCESS) && new_value && strcmp(ZSTR_VAL(new_value), "syslog")) {
		if (PG(open_basedir) && php_check_open_basedir(ZSTR_VAL(new_value))) {
			return FAILURE;
		}
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnUpdateMailLog)
{
	/* Only do the safemode/open_basedir check at runtime */
	if ((stage == PHP_INI_STAGE_RUNTIME || stage == PHP_INI_STAGE_HTACCESS) && new_value) {
		if (PG(open_basedir) && php_check_open_basedir(ZSTR_VAL(new_value))) {
			return FAILURE;
		}
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI_MH
*/
static PHP_INI_MH(OnChangeMailForceExtra)
{
	/* Don't allow changing it in htaccess */
	if (stage == PHP_INI_STAGE_HTACCESS) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* defined in browscap.c */
PHP_INI_MH(OnChangeBrowscap);

/* Need to be read from the environment (?):
* PHP_AUTO_PREPEND_FILE
* PHP_AUTO_APPEND_FILE
* PHP_DOCUMENT_ROOT
* PHP_USER_DIR
* PHP_INCLUDE_PATH
*/

/* Windows and Netware use the internal mail */
#if defined(PHP_WIN32) || defined(NETWARE)
# define DEFAULT_SENDMAIL_PATH NULL
#elif defined(PHP_PROG_SENDMAIL)
# define DEFAULT_SENDMAIL_PATH PHP_PROG_SENDMAIL " -t -i "
#else
# define DEFAULT_SENDMAIL_PATH "/usr/sbin/sendmail -t -i"
#endif
ZEND_API zend_compiler_globals compiler_globals;
/* {{{ PHP_INI
*/
/*
PHP_INI_BEGIN()
	PHP_INI_ENTRY_EX("highlight.comment", HL_COMMENT_COLOR, PHP_INI_ALL, NULL, php_ini_color_displayer_cb)
PHP_INI_END()
*/
/* }}} */


/* True globals (no need for thread safety */
/* But don't make them a single int bitfield */
static int module_initialized = 0;
static int module_startup = 1;
static int module_shutdown = 0;

/* {{{ php_during_module_startup */
static int php_during_module_startup(void)
{
	return module_startup;
}
/* }}} */

/* {{{ php_during_module_shutdown */
static int php_during_module_shutdown(void)
{
	return module_shutdown;
}
/* }}} */

/* {{{ php_get_module_initialized
*/
PHPAPI int php_get_module_initialized(void)
{
	return module_initialized;
}
/* }}} */

/* {{{ php_log_err_with_severity
*/
#define ZEND_ENABLE_STATIC_TSRMLS_CACHE 1
PHPAPI ZEND_COLD void php_log_err_with_severity(char *log_message, int syslog_type_int)
{
	int fd = -1;
	time_t error_time;

}
/* }}} */

