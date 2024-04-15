#include "apr_hash.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"
#include <memory.h>
#include "apr_pools.h"

server_rec *server = NULL;

static int post_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s) {
    (void)p;
    server = s;

    ap_log_error(APLOG_MARK, APLOG_INFO, 0, server, "post_config is called");

    return 0;
}

static char* create_test_string(apr_pool_t *p, apr_size_t size) {
    char* str = apr_palloc(p, size);
    if (!str)
        return "alloc failed";

    memcpy(str, "http://test&", 12);
    for (unsigned i = 12; i < size; i++) {
        switch (i % 4) {
            case 0:
                str[i] = 'a';
                break;
            case 1:
                str[i] = 'm';
                break;
            case 2:
                str[i] = 'p';
                break;
            case 3:
                str[i] = ';';
                break;
        }
    }

    str[size - 1] = '\0';
    return str;
}

static int my_main(apr_pool_t *p, server_rec *server) {
    char *test = create_test_string(p, 80);
    char *out = ap_escape_html2(p, test, 0);
    ap_log_error(APLOG_MARK, APLOG_INFO, 0, server, "in: %s", test);
    ap_log_error(APLOG_MARK, APLOG_INFO, 0, server, "out: %s", out);

    return 0;
}

static int example_handler(request_rec *rec) {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, rec->server, "example_handler: got request for %s", rec->uri);

    if (!rec->uri || strcmp(rec->uri + 1, "run-main"))
        return DECLINED;

    my_main(rec->pool, rec->server);
    ap_set_content_type(rec, "text/html");
    ap_rprintf(rec, "Ok");

    return OK;
}

static void register_hooks(apr_pool_t *pool) {
    ap_hook_post_config(post_config, NULL, NULL, APR_HOOK_MIDDLE);

    ap_hook_handler(example_handler, NULL, NULL, APR_HOOK_LAST);
}

AP_DECLARE_MODULE(playground) = {
    STANDARD20_MODULE_STUFF,
    NULL, // create_dir_conf,
    NULL, // merge_dir_conf,
    NULL, // create_svr_conf,
    NULL, // merge_svr_conf,
    NULL, // directives,
    register_hooks
};

