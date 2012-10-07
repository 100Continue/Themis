#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>
#include <ngx_themis_log.h>
#include <ngx_http_themis.h>

extern ngx_module_t  ngx_http_footer_filter_module;

typedef struct {
    ngx_hash_t                          types;
    ngx_array_t                        *types_keys;
    ngx_http_complex_value_t           *variable;
} ngx_http_themis_footer_loc_conf_t;


static char  *week[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
                         "Friday", "Saturday" };

static char  *months[] = { "January", "February", "March", "April", "May",
                           "June", "July", "August", "Semptember", "October",
                           "November", "December" };


static ngx_conf_t  ngx_themis_footer_mate_conf_tmp;
static ngx_event_t ngx_themis_footer_mate_timer;


typedef struct ngx_themis_footer_mate_conf_s ngx_themis_footer_mate_conf_t;


static ngx_int_t ngx_themis_footer_mate_init(ngx_conf_t *cf);
static void *ngx_themis_footer_mate_create_config(ngx_conf_t *cf);
static ngx_int_t ngx_themis_footer_mate_init_process(ngx_cycle_t *cycle);
static void ngx_themis_footer_mate_timer_handler(ngx_event_t *ev);
static ngx_int_t ngx_themis_footer_mate_update_conf_handler(
    ngx_http_request_t *r);


struct ngx_themis_footer_mate_conf_s {
    ngx_http_complex_value_t  *variable;
};


static ngx_command_t ngx_themis_footer_mate_commands[] = {
      ngx_null_command
};


static ngx_http_module_t  ngx_themis_footer_mate_module_ctx = {
    NULL,
    ngx_themis_footer_mate_init,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};


ngx_module_t  ngx_themis_footer_mate_module = {
    NGX_MODULE_V1,
    &ngx_themis_footer_mate_module_ctx,       /* module context */
    ngx_themis_footer_mate_commands,          /* module directives */
    NGX_HTTP_MODULE,                          /* module type */
    NULL,                                     /* init master */
    NULL,                                     /* init module */
    ngx_themis_footer_mate_init_process,      /* init process */
    NULL,                                     /* init thread */
    NULL,                                     /* exit thread */
    NULL,                                     /* exit process */
    NULL,                                     /* exit master */
    NGX_MODULE_V1_PADDING
};


ngx_themis_module_t ngx_themis_footer_mate = {
    0,
    ngx_string("footer_mate"),
    ngx_themis_footer_mate_create_config
};


static ngx_int_t
ngx_themis_footer_mate_init(ngx_conf_t *cf)
{
    /* TODO: move this function to thmeis module, this is a demo */
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_PREACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }
    *h = ngx_themis_footer_mate_update_conf_handler;

    return NGX_OK;
}


static void *
ngx_themis_footer_mate_create_config(ngx_conf_t *cf)
{
    ngx_themis_footer_mate_conf_t  *fmcf;

    fmcf = ngx_pcalloc(cf->pool, sizeof(ngx_themis_footer_mate_conf_t));
    if (fmcf == NULL) {
        return NULL;
    }

    return fmcf;
}


static ngx_int_t
ngx_themis_footer_mate_init_process(ngx_cycle_t *cycle)
{
    /*
      demo of dynamic conf update
      TODO: use channel event to replace timer
    */
    ngx_memzero(&ngx_themis_footer_mate_timer, sizeof(ngx_event_t));
    ngx_themis_footer_mate_timer.handler = ngx_themis_footer_mate_timer_handler;
    ngx_themis_footer_mate_timer.log = cycle->log;
    ngx_themis_footer_mate_timer.data = cycle;

    ngx_memzero(&ngx_themis_footer_mate_conf_tmp, sizeof(ngx_conf_t));

    ngx_add_timer(&ngx_themis_footer_mate_timer, 1000);

    return NGX_OK;
}


static void
ngx_themis_footer_mate_timer_handler(ngx_event_t *ev)
{
    static u_char                       buf[256];

    void                              **module_configs;
    u_char                             *p;
    ngx_str_t                           content;
    ngx_log_t                          *log;
    ngx_uint_t                          i;
    ngx_pool_t                         *pool;
    ngx_cycle_t                        *cycle;
    ngx_hash_key_t                     *key, *keys;
    ngx_http_complex_value_t           *cv;
    ngx_http_themis_main_conf_t        *tmcf;
    ngx_themis_footer_mate_conf_t      *fcf;
    ngx_http_compile_complex_value_t    ccv;

    log = ev->log;
    cycle = ev->data;

    if (ngx_themis_footer_mate_conf_tmp.pool != NULL) {
        ngx_destroy_pool(ngx_themis_footer_mate_conf_tmp.pool);
        ngx_themis_footer_mate_conf_tmp.pool = NULL;
    }

    pool = ngx_create_pool(ngx_pagesize, log);
    if (pool == NULL) {
        return;
    }

    ngx_themis_footer_mate_conf_tmp.ctx = cycle->conf_ctx;
    ngx_themis_footer_mate_conf_tmp.cycle = cycle;
    ngx_themis_footer_mate_conf_tmp.pool = pool;
    ngx_themis_footer_mate_conf_tmp.log = log;
    ngx_themis_footer_mate_conf_tmp.ctx =
        cycle->conf_ctx[ngx_http_module.index];

    p = ngx_sprintf(buf, "$remote_addr %s, %s, %d, %d, %d:%d:%d-%s",
                    week[ngx_cached_tm->tm_wday],
                    months[ngx_cached_tm->tm_mon],
                    ngx_cached_tm->tm_mday, ngx_cached_tm->tm_year,
                    ngx_cached_tm->tm_hour, ngx_cached_tm->tm_min,
                    ngx_cached_tm->tm_sec, ngx_cached_tm->tm_zone);

    content.data = buf;
    content.len = p - buf;

    ngx_log_themis(NGX_LOG_INFO, log, 0, "%V", &content);

    /* TODO: move to the frame */

    tmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_themis_module);
    keys = tmcf->configs.elts;
    for (i = 0; i < tmcf->configs.nelts; i++) {
        key = &keys[i];
        ngx_log_themis(NGX_LOG_DEBUG, log, 0, "%V create complex value",
                       &key->key);

        module_configs = ngx_hash_find(&tmcf->configs_hash, key->key_hash,
                                       key->key.data, key->key.len);
        if (module_configs == NULL) {
            continue;
        }

        fcf = module_configs[ngx_themis_footer_mate.index];

        cv = ngx_palloc(pool, sizeof(ngx_http_complex_value_t));
        if (cv == NULL) {
            ngx_log_themis(NGX_LOG_ERR, log, 0, "%V create complex value error",
                           &key->key);
            return;
        }
        ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));
        ccv.cf = &ngx_themis_footer_mate_conf_tmp;
        ccv.value = &content;
        ccv.complex_value = cv;

        if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
            ngx_log_themis(NGX_LOG_ERR, log, 0,
                           "%V compile complex value error", &key->key);
            continue;
        }
        fcf->variable = cv;
    }

    ngx_add_timer(&ngx_themis_footer_mate_timer, 1000);
}


static ngx_int_t
ngx_themis_footer_mate_update_conf_handler(ngx_http_request_t *r)
{
    ngx_http_themis_loc_conf_t         *tlcf;
    ngx_themis_footer_mate_conf_t      *fcf;
    ngx_http_themis_footer_loc_conf_t  *flcf;

    tlcf = ngx_http_get_module_loc_conf(r, ngx_http_themis_module);
    flcf = ngx_http_get_module_loc_conf(r, ngx_http_footer_filter_module);

    fcf = (*tlcf->module_configs)[ngx_themis_footer_mate.index];

    ngx_log_themis(NGX_LOG_ERR, r->connection->log, 0,
                   "update complex value %i", fcf->variable);

    flcf->variable = fcf->variable;

    return NGX_OK;
}
