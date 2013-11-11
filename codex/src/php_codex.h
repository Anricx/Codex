/*
 +----------------------------------------------------------------------+
 | ChinaRoad license:                                                      |
 +----------------------------------------------------------------------+
 | Authors: Deng Tao <dengt@007ka.com>                                  |
 +----------------------------------------------------------------------+
 */

/* $ Id: $ */

#ifndef PHP_CODEX_H
#define PHP_CODEX_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>

#ifdef HAVE_CODEX
#define PHP_CODEX_VERSION "1.0.0 (Release)"
#define PHP_CODEX_RELEASED "2013-11-04"
#define PHP_CODEX_AUTHORS "007ka 'www.007ka.com' (lead)\n"

#include <php_ini.h>
#include <SAPI.h>
#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>
#ifdef  __cplusplus
} // extern "C" 
#endif
#ifdef  __cplusplus
extern "C" {
#endif

extern zend_module_entry codex_module_entry;
#define phpext_codex_ptr &codex_module_entry

#ifdef PHP_WIN32
#define PHP_CODEX_API __declspec(dllexport)
#else
#define PHP_CODEX_API
#endif

PHP_MINIT_FUNCTION(codex);
PHP_MSHUTDOWN_FUNCTION(codex);
PHP_RINIT_FUNCTION(codex);
PHP_RSHUTDOWN_FUNCTION(codex);
PHP_MINFO_FUNCTION(codex);

#ifdef ZTS
#include "TSRM.h"
#endif

#define FREE_RESOURCE(resource) zend_list_delete(Z_LVAL_P(resource))

#define PROP_GET_LONG(name)    Z_LVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_LONG(name, l) zend_update_property_long(_this_ce, _this_zval, #name, strlen(#name), l TSRMLS_CC)

#define PROP_GET_DOUBLE(name)    Z_DVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_DOUBLE(name, d) zend_update_property_double(_this_ce, _this_zval, #name, strlen(#name), d TSRMLS_CC)

#define PROP_GET_STRING(name)    Z_STRVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_GET_STRLEN(name)    Z_STRLEN_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_STRING(name, s) zend_update_property_string(_this_ce, _this_zval, #name, strlen(#name), s TSRMLS_CC)
#define PROP_SET_STRINGL(name, s, l) zend_update_property_stringl(_this_ce, _this_zval, #name, strlen(#name), s, l TSRMLS_CC)

ZEND_BEGIN_MODULE_GLOBALS(codex)
zend_bool enable;
char * x;
char * _x;
int x_len;
char * header;
int header_len;
zend_bool display_ini_entries;

ZEND_END_MODULE_GLOBALS(codex)

#ifdef ZTS
#define CODEX_G(v) TSRMG(codex_globals_id, zend_codex_globals *, v)
#else
#define CODEX_G(v) (codex_globals.v)
#endif

#ifdef  __cplusplus
} // extern "C" 
#endif

#endif /* PHP_HAVE_CODEX */

#endif /* PHP_CODEX_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
