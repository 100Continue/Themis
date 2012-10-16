#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>
#include <ngx_themis_log.h>
#include <ngx_http_themis.h>


extern ngx_module_t  ngx_http_access_module;


static void *ngx_themis_access_mate_create_config(ngx_conf_t *cf);
static ngx_int_t ngx_themis_access_mate_update_config(ngx_cycle_t *cycle,
    void *conf);
static ngx_int_t ngx_themis_access_mate_apply_config(ngx_http_request_t *r,
    void *config);


typedef struct {
    ngx_str_t  demo;
} ngx_themis_access_mate_conf_t;


ngx_themis_module_t ngx_themis_access_mate_ctx = {
    ngx_string("access_mate"),
    ngx_themis_access_mate_create_config,
    ngx_themis_access_mate_update_config,
    ngx_themis_access_mate_apply_config
};


static ngx_command_t  ngx_themis_access_commands[] = {
      ngx_null_command
};


ngx_module_t  ngx_themis_access_mate_module = {
    NGX_MODULE_V1,
    &ngx_themis_access_mate_ctx,           /* module context */
    ngx_themis_access_commands,            /* module directives */
    NGX_THEMIS_MODULE,                     /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_themis_access_mate_create_config(ngx_conf_t *cf)
{
    ngx_themis_access_mate_conf_t  *amcf;

    amcf = ngx_pcalloc(cf->pool, sizeof(ngx_themis_access_mate_conf_t));

    return amcf;
}


static ngx_int_t
ngx_themis_access_mate_update_config(ngx_cycle_t *cycle, void *conf)
{
    ngx_log_themis(NGX_LOG_DEBUG, cycle->log, 0, "access update config");

    return NGX_OK;
}


static ngx_int_t
ngx_themis_access_mate_apply_config(ngx_http_request_t *r, void *config)
{
    ngx_log_themis(NGX_LOG_DEBUG, r->connection->log, 0, "apply update config");

    return NGX_OK;
}
