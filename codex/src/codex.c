/*
 +----------------------------------------------------------------------+
 | ChinaRoad license:                                                      |
 +----------------------------------------------------------------------+
 | Authors: Deng Tao <dengt@007ka.com>                                  |
 +----------------------------------------------------------------------+
 */

/* $ Id: $ */

#include "php_codex.h"
#include "syslog.h"

#if HAVE_CODEX

#include "zencode.c"

#define SALT "1d232c9c6373e992f3f54cc1afff9b12"

/* {{{ codex_functions[] */
function_entry codex_functions[] = {
    {   NULL, NULL, NULL}
};
/* }}} */

/* {{{ codex_module_entry */
zend_module_entry codex_module_entry = {
    STANDARD_MODULE_HEADER,
    "codex",
    codex_functions,
    PHP_MINIT(codex), /* Replace with NULL if there is nothing to do at php startup   */
    PHP_MSHUTDOWN(codex), /* Replace with NULL if there is nothing to do at php shutdown  */
    NULL, /* Replace with NULL if there is nothing to do at request start */
    NULL, /* Replace with NULL if there is nothing to do at request end   */
    PHP_MINFO(codex),
    PHP_CODEX_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CODEX
ZEND_GET_MODULE(codex)
#endif

/* {{{ globals and ini entries */
ZEND_DECLARE_MODULE_GLOBALS(codex)

#ifndef ZEND_ENGINE_2
#define OnUpdateLong OnUpdateInt
#endif
PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("codex.enable", "1", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_codex_globals, codex_globals)
STD_PHP_INI_ENTRY("codex.x", "ba1dc1ea0cde5f03", PHP_INI_SYSTEM, OnUpdateString, x, zend_codex_globals, codex_globals)
STD_PHP_INI_ENTRY("codex.header", "Codex Tech.", PHP_INI_SYSTEM, OnUpdateString, header, zend_codex_globals, codex_globals)
STD_PHP_INI_ENTRY("codex.display_ini_entries", "0", PHP_INI_SYSTEM, OnUpdateBool, display_ini_entries, zend_codex_globals, codex_globals)
PHP_INI_END()

static void php_codex_init_globals(zend_codex_globals *codex_globals) {
    codex_globals->enable = 1;
    codex_globals->x = "ba1dc1ea0cde5f03";
    codex_globals->x_len = strlen(codex_globals->x);
    codex_globals->header = "Codex Tech.";
    codex_globals->header_len = strlen(codex_globals->header);
    codex_globals->display_ini_entries = 0;
}

static void php_codex_shutdown_globals(zend_codex_globals *codex_globals) {
}/* }}} */

static FILE *codex_ext_fopen(FILE *fp) {
    struct stat stat_buf;
    char *datap, *newdatap;
    int datalen, newdatalen;
    int cryptkey_len = CODEX_G(x_len);
    int i;

    fstat(fileno(fp), &stat_buf);
    datalen = stat_buf.st_size - CODEX_G(header_len);
    datap = (char*)malloc(datalen);

    if (fread(datap, datalen, 1, fp) <= 0) {
        return NULL;
    }

    fclose(fp);

    for(i=0; i<datalen; i++) {
        datap[i] = (char)(CODEX_G(_x)[(datalen - i) % cryptkey_len]) ^ (~(datap[i]));
    }

    newdatap = zdecode(datap, datalen, &newdatalen);

    fp = tmpfile();
    if (fwrite(newdatap, newdatalen, 1, fp) <=0) {
        return NULL;
    }

    free(datap);
    free(newdatap);

    rewind(fp);
    return fp;
}

static char * sub_key(char * key) {
    int salt_len = 0, tmp_len = 0, i = 0;
    char *tmp_x;

    salt_len = strlen(SALT);
    tmp_len = strlen(key);

    tmp_x = (char *) malloc((tmp_len + 1) * sizeof(char));

    memset(tmp_x, 0, (tmp_len + 1));
    for (i = 0; i < tmp_len; i++) {
        tmp_x[i] = (char) (SALT[(tmp_len - i) % salt_len]) ^ (~key[i]);
    }
    return tmp_x;
}

ZEND_API zend_op_array *(*org_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);

ZEND_API zend_op_array *codex_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) {
    FILE *fp;
    char buf[CODEX_G(header_len) + 1];
    char fname[32];

    memset(fname, 0, sizeof fname);
    if (zend_is_executing(TSRMLS_C)) {
        if (get_active_function_name(TSRMLS_C)) {
            strncpy(fname, get_active_function_name(TSRMLS_C), sizeof fname - 2);
        }
    }
    if (fname[0]) {
        if ( strcasecmp(fname, "show_source") == 0
                || strcasecmp(fname, "highlight_file") == 0) {
            return NULL;
        }
    }

    if (!(fp = fopen(file_handle->filename, "r"))) {
        return org_compile_file(file_handle, type TSRMLS_CC);
    }

    if (!fread(buf, CODEX_G(header_len), 1, fp)) {
        fclose(fp);
        return org_compile_file(file_handle, type TSRMLS_CC);
    }

    if (memcmp(buf, CODEX_G(header), CODEX_G(header_len)) != 0) {
        fclose(fp);
        return org_compile_file(file_handle, type TSRMLS_CC);
    }

    if (file_handle->type == ZEND_HANDLE_FP) fclose(file_handle->handle.fp);
    if (file_handle->type == ZEND_HANDLE_FD) close(file_handle->handle.fd);
    file_handle->handle.fp = codex_ext_fopen(fp);
    file_handle->type = ZEND_HANDLE_FP;
    file_handle->opened_path = expand_filepath(file_handle->filename, NULL TSRMLS_CC);

    return org_compile_file(file_handle, type TSRMLS_CC);
}

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(codex) {
    ZEND_INIT_MODULE_GLOBALS(codex, php_codex_init_globals, php_codex_shutdown_globals)
    REGISTER_INI_ENTRIES();

    if (CODEX_G(enable)) {
        CODEX_G(_x) = sub_key(CODEX_G(x));
        if (CODEX_G(_x) == NULL) {
            CODEX_G(enable) = 0;
            php_syslog(LOG_ERR, "codex model init failed, memory alloc error!");
            return FAILURE;
        }

        CODEX_G(x_len) = strlen(CODEX_G(_x));
        CODEX_G(header_len) = strlen(CODEX_G(header));

        CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;
        org_compile_file = zend_compile_file;
        zend_compile_file = codex_compile_file;
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(codex) {
    UNREGISTER_INI_ENTRIES();

    if (CODEX_G(enable)) {
        CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;
        zend_compile_file = org_compile_file;
        free(CODEX_G(_x));
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(codex) {
    php_info_print_table_start();
    php_info_print_table_row(2, "Version", PHP_CODEX_VERSION);
    php_info_print_table_row(2, "Released", PHP_CODEX_RELEASED);
    php_info_print_table_row(2, "Authors", PHP_CODEX_AUTHORS);
    php_info_print_table_row(2, "Codex Support", CODEX_G(enable) ? "enabled" : "disabled");
    php_info_print_table_end();

    if (CODEX_G(display_ini_entries)) DISPLAY_INI_ENTRIES();
}
/* }}} */

#endif /* HAVE_CODEX */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
