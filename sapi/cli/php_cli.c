/*
+----------------------------------------------------------------------+
| PHP Version 7                                                        |
+----------------------------------------------------------------------+
| Copyright (c) 1997-2017 The PHP Group                                |
+----------------------------------------------------------------------+
| This source file is subject to version 3.01 of the PHP license,      |
| that is bundled with this package in the file LICENSE, and is        |
| available through the world-wide-web at the following url:           |
| http://www.php.net/license/3_01.txt                                  |
| If you did not receive a copy of the PHP license and are unable to   |
| obtain it through the world-wide-web, please send a note to          |
| license@php.net so we can mail you a copy immediately.               |
+----------------------------------------------------------------------+
| Author: Edin Kadribasic <edink@php.net>                              |
|         Marcus Boerger <helly@php.net>                               |
|         Johannes Schlueter <johannes@php.net>                        |
|         Parts based on CGI SAPI Module by                            |
|         Rasmus Lerdorf, Stig Bakken and Zeev Suraski                 |
+----------------------------------------------------------------------+
*/

/* $Id$ */
#include "php.h"
#include "php_globals.h"
#include "php_variables.h"
#include "zend_hash.h"
#include "zend_modules.h"
#include "zend_interfaces.h"

#include "ext/reflection/php_reflection.h"

#include "SAPI.h"

#include <stdio.h>
#include "php.h"
#ifdef PHP_WIN32
#include "win32/time.h"
#include "win32/signal.h"
#include <process.h>
#include <shellapi.h>
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
#include "zend_extensions.h"
#include "php_ini.h"
#include "php_globals.h"
#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"
#include "cli.h"
#ifdef PHP_WIN32
#include <io.h>
#include <fcntl.h>
#include "win32/php_registry.h"
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef __riscos__
#include <unixlib/local.h>
#endif

#include "zend_compile.h"
#include "zend_execute.h"
#include "zend_highlight.h"
#include "zend_exceptions.h"

#include "php_getopt.h"

#ifndef PHP_CLI_WIN32_NO_CONSOLE
#include "php_cli_server.h"
#endif

#include "ps_title.h"
#include "php_cli_process_title.h"

#ifndef PHP_WIN32
# define php_select(m, r, w, e, t)	select(m, r, w, e, t)
#else
# include "win32/select.h"
#endif

#if defined(PHP_WIN32) && defined(HAVE_OPENSSL)
# include "openssl/applink.c"
#endif

PHPAPI extern char *php_ini_opened_path;
PHPAPI extern char *php_ini_scanned_path;
PHPAPI extern char *php_ini_scanned_files;

#if defined(PHP_WIN32)
#if defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE()
#endif
static DWORD orig_cp = 0;
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define PHP_MODE_STANDARD      1
#define PHP_MODE_HIGHLIGHT     2
#define PHP_MODE_LINT          4
#define PHP_MODE_STRIP         5
#define PHP_MODE_CLI_DIRECT    6
#define PHP_MODE_PROCESS_STDIN 7
#define PHP_MODE_REFLECTION_FUNCTION    8
#define PHP_MODE_REFLECTION_CLASS       9
#define PHP_MODE_REFLECTION_EXTENSION   10
#define PHP_MODE_REFLECTION_EXT_INFO    11
#define PHP_MODE_REFLECTION_ZEND_EXTENSION 12
#define PHP_MODE_SHOW_INI_CONFIG        13

cli_shell_callbacks_t cli_shell_callbacks = { NULL, NULL, NULL };
PHP_CLI_API cli_shell_callbacks_t *php_cli_get_shell_callbacks()
{
	return &cli_shell_callbacks;
}

const char HARDCODED_INI[] =
"html_errors=0\n"
"register_argc_argv=1\n"
"implicit_flush=1\n"
"output_buffering=0\n"
"max_execution_time=0\n"
"max_input_time=-1\n\0";


const opt_struct OPTIONS[] = {
	{ 'a', 0, "interactive" },
	{ 'B', 1, "process-begin" },
	{ 'C', 0, "no-chdir" }, /* for compatibility with CGI (do not chdir to script directory) */
	{ 'c', 1, "php-ini" },
	{ 'd', 1, "define" },
	{ 'E', 1, "process-end" },
	{ 'e', 0, "profile-info" },
	{ 'F', 1, "process-file" },
	{ 'f', 1, "file" },
	{ 'h', 0, "help" },
	{ 'i', 0, "info" },
	{ 'l', 0, "syntax-check" },
	{ 'm', 0, "modules" },
	{ 'n', 0, "no-php-ini" },
	{ 'q', 0, "no-header" }, /* for compatibility with CGI (do not generate HTTP headers) */
	{ 'R', 1, "process-code" },
	{ 'H', 0, "hide-args" },
	{ 'r', 1, "run" },
	{ 's', 0, "syntax-highlight" },
	{ 's', 0, "syntax-highlighting" },
	{ 'S', 1, "server" },
	{ 't', 1, "docroot" },
	{ 'w', 0, "strip" },
	{ '?', 0, "usage" },/* help alias (both '?' and 'usage') */
	{ 'v', 0, "version" },
	{ 'z', 1, "zend-extension" },
	{ 10,  1, "rf" },
	{ 10,  1, "rfunction" },
	{ 11,  1, "rc" },
	{ 11,  1, "rclass" },
	{ 12,  1, "re" },
	{ 12,  1, "rextension" },
	{ 13,  1, "rz" },
	{ 13,  1, "rzendextension" },
	{ 14,  1, "ri" },
	{ 14,  1, "rextinfo" },
	{ 15,  0, "ini" },
	{ '-', 0, NULL } /* end of args */
};

static int print_module_info(zval *element) /* {{{ */
{
	zend_module_entry *module = (zend_module_entry*)Z_PTR_P(element);
	php_printf("%s\n", module->name);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static int module_name_cmp(const void *a, const void *b) /* {{{ */
{
	Bucket *f = (Bucket *)a;
	Bucket *s = (Bucket *)b;

	return strcasecmp(((zend_module_entry *)Z_PTR(f->val))->name,
		((zend_module_entry *)Z_PTR(s->val))->name));//编译的时候报错说少个括号？可是我能编译啊。
}
/* }}} */

static void print_modules(void) /* {{{ */
{
	HashTable sorted_registry;

	zend_hash_init(&sorted_registry, 50, NULL, NULL, 0);
	zend_hash_copy(&sorted_registry, &module_registry, NULL);
	zend_hash_sort(&sorted_registry, module_name_cmp, 0);
	zend_hash_apply(&sorted_registry, print_module_info);
	zend_hash_destroy(&sorted_registry);
}
/* }}} */

static int print_extension_info(zend_extension *ext, void *arg) /* {{{ */
{
	php_printf("%s\n", ext->name);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static int extension_name_cmp(const zend_llist_element **f, const zend_llist_element **s) /* {{{ */
{
	zend_extension *fe = (zend_extension*)(*f)->data;
	zend_extension *se = (zend_extension*)(*s)->data;
	return strcmp(fe->name, se->name);
}
/* }}} */

static void print_extensions(void) /* {{{ */
{
	zend_llist sorted_exts;

	zend_llist_copy(&sorted_exts, &zend_extensions);
	sorted_exts.dtor = NULL;
	zend_llist_sort(&sorted_exts, extension_name_cmp);
	zend_llist_apply(&sorted_exts, (llist_apply_func_t)print_extension_info);
	zend_llist_destroy(&sorted_exts);
}
/* }}} */

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

static inline int sapi_cli_select(int fd)
{
	fd_set wfd, dfd;
	struct timeval tv;
	int ret;

	FD_ZERO(&wfd);
	FD_ZERO(&dfd);

	PHP_SAFE_FD_SET(fd, &wfd);

	tv.tv_sec = (long)FG(default_socket_timeout);
	tv.tv_usec = 0;

	ret = php_select(fd + 1, &dfd, &wfd, &dfd, &tv);

	return ret != -1;
}

PHP_CLI_API size_t sapi_cli_single_write(const char *str, size_t str_length) /* {{{ */
{
#ifdef PHP_WRITE_STDOUT
	zend_long ret;
#else
	size_t ret;
#endif

	if (cli_shell_callbacks.cli_shell_write) {
		cli_shell_callbacks.cli_shell_write(str, str_length);
	}

#ifdef PHP_WRITE_STDOUT
	do {
		ret = write(STDOUT_FILENO, str, str_length);
	} while (ret <= 0 && errno == EAGAIN && sapi_cli_select(STDOUT_FILENO));

	if (ret <= 0) {
		return 0;
	}

	return ret;
#else
	ret = fwrite(str, 1, MIN(str_length, 16384), stdout);
	return ret;
#endif
}
/* }}} */

static size_t sapi_cli_ub_write(const char *str, size_t str_length) /* {{{ */
{
	const char *ptr = str;
	size_t remaining = str_length;
	size_t ret;

	if (!str_length) {
		return 0;
	}

	if (cli_shell_callbacks.cli_shell_ub_write) {
		size_t ub_wrote;
		ub_wrote = cli_shell_callbacks.cli_shell_ub_write(str, str_length);
		if (ub_wrote != (size_t)-1) {
			return ub_wrote;
		}
	}

	while (remaining > 0)
	{
		ret = sapi_cli_single_write(ptr, remaining);
		if (!ret) {
#ifndef PHP_CLI_WIN32_NO_CONSOLE
			php_handle_aborted_connection();
#endif
			break;
		}
		ptr += ret;
		remaining -= ret;
	}

	return (ptr - str);
}
/* }}} */

static void sapi_cli_flush(void *server_context) /* {{{ */
{
	/* Ignore EBADF here, it's caused by the fact that STDIN/STDOUT/STDERR streams
	* are/could be closed before fflush() is called.
	*/
	if (fflush(stdout) == EOF && errno != EBADF) {
#ifndef PHP_CLI_WIN32_NO_CONSOLE
		php_handle_aborted_connection();
#endif
	}
}
/* }}} */

static char *php_self = "";
static char *script_filename = "";

static void sapi_cli_register_variables(zval *track_vars_array) /* {{{ */
{
	size_t len;
	char   *docroot = "";

	/* In CGI mode, we consider the environment to be a part of the server
	* variables
	*/
	php_import_environment_variables(track_vars_array);

	/* Build the special-case PHP_SELF variable for the CLI version */
	len = strlen(php_self);
	if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &php_self, len, &len)) {
		php_register_variable("PHP_SELF", php_self, track_vars_array);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_NAME", &php_self, len, &len)) {
		php_register_variable("SCRIPT_NAME", php_self, track_vars_array);
	}
	/* filenames are empty for stdin */
	len = strlen(script_filename);
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_FILENAME", &script_filename, len, &len)) {
		php_register_variable("SCRIPT_FILENAME", script_filename, track_vars_array);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "PATH_TRANSLATED", &script_filename, len, &len)) {
		php_register_variable("PATH_TRANSLATED", script_filename, track_vars_array);
	}
	/* just make it available */
	len = 0U;
	if (sapi_module.input_filter(PARSE_SERVER, "DOCUMENT_ROOT", &docroot, len, &len)) {
		php_register_variable("DOCUMENT_ROOT", docroot, track_vars_array);
	}
}
/* }}} */

static void sapi_cli_log_message(char *message, int syslog_type_int) /* {{{ */
{
	fprintf(stderr, "%s\n", message);
#ifdef PHP_WIN32
	fflush(stderr);
#endif
}
/* }}} */

static int sapi_cli_deactivate(void) /* {{{ */
{
	fflush(stdout);
	if (SG(request_info).argv0) {
		free(SG(request_info).argv0);
		SG(request_info).argv0 = NULL;
	}
	return SUCCESS;
}
/* }}} */

static char* sapi_cli_read_cookies(void) /* {{{ */
{
	return NULL;
}
/* }}} */

static int sapi_cli_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s) /* {{{ */
{
	return 0;
}
/* }}} */

static int sapi_cli_send_headers(sapi_headers_struct *sapi_headers) /* {{{ */
{
	/* We do nothing here, this function is needed to prevent that the fallback
	* header handling is called. */
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}
/* }}} */

static void sapi_cli_send_header(sapi_header_struct *sapi_header, void *server_context) /* {{{ */
{
}
/* }}} */

static int php_cli_startup(sapi_module_struct *sapi_module) /* {{{ */
{
	/*
	if (php_module_startup(sapi_module, NULL, 0) == FAILURE) {
		return FAILURE;
	}*/
	
	return SUCCESS;
}
/* }}} */

/* {{{ sapi_cli_ini_defaults */

/* overwriteable ini defaults must be set in sapi_cli_ini_defaults() */
#define INI_DEFAULT(name,value)\
	ZVAL_NEW_STR(&tmp, zend_string_init(value, sizeof(value)-1, 1));\
	zend_hash_str_update(configuration_hash, name, sizeof(name)-1, &tmp);\

static void sapi_cli_ini_defaults(HashTable *configuration_hash)
{
	zval tmp;
	INI_DEFAULT("report_zend_debug", "0");
	INI_DEFAULT("display_errors", "1");
}
/* }}} */

/* {{{ sapi_module_struct cli_sapi_module
*/
static sapi_module_struct cli_sapi_module = {
	"cli" 
	"Command Line Interface"    	/* pretty name */
};
/* }}} */
//#pragma comment (lib, "dllmake.lib")
void main() {
	//show_dll();
	printf("hello world\n");
	//printf("%d", LIUDAN);
}