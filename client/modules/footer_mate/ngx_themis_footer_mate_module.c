#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>
#include <ngx_themis_log.h>
#include <ngx_http_themis.h>

typedef struct ngx_themis_footer_mate_conf_s ngx_themis_footer_mate_conf_t;

static ngx_int_t ngx_themis_footer_mate_init(ngx_conf_t *cf);
static void *ngx_themis_footer_mate_create_config(ngx_conf_t *cf);


struct ngx_themis_footer_mate_conf_s {
    ngx_str_t             name;
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
    NULL,                                     /* init process */
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
