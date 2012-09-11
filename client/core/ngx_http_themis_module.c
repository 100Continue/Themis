#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>


typedef struct {
    ngx_flag_t  enable;
} ngx_http_themis_loc_conf_t;


static ngx_int_t ngx_http_themis_preconfiguration(ngx_conf_t *cf);
static ngx_int_t ngx_http_themis_init(ngx_conf_t *cf);
static char *ngx_http_themis_init_main_conf(ngx_conf_t *cf, void *conf);
static void *ngx_http_themis_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_themis_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);
static void *ngx_http_themis_create_main_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_themis_init_process(ngx_cycle_t *cycle);
static void ngx_http_themis_exit(ngx_cycle_t *cycle);


static ngx_command_t ngx_http_themis_commands[] = {

    { ngx_string("themis"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_MAIN_CONF_OFFSET,
      offsetof(ngx_http_themis_loc_conf_t, enable),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_themis_module_ctx = {
    ngx_http_themis_preconfiguration,         /* preconfiguration */
    ngx_http_themis_init,                     /* postconfiguration */

    ngx_http_themis_create_main_conf,         /* create main configuration */
    ngx_http_themis_init_main_conf,           /* init main configuration */

    NULL,                                     /* create server configuration */
    NULL,                                     /* merge server configuration */

    ngx_http_themis_create_loc_conf,          /* create location configration */
    ngx_http_themis_merge_conf                /* merge location configration */
};


ngx_module_t  ngx_http_themis_module = {
    NGX_MODULE_V1,
    &ngx_http_themis_module_ctx,              /* module context */
    ngx_http_themis_commands,                 /* module directives */
    NGX_HTTP_MODULE,                          /* module type */
    NULL,                                     /* init master */
    NULL,                                     /* init module */
    ngx_http_themis_init_process,             /* init process */
    NULL,                                     /* init thread */
    NULL,                                     /* exit thread */
    ngx_http_themis_exit,                     /* exit process */
    NULL,                                     /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_themis_preconfiguration(ngx_conf_t *cf)
{
    return NGX_OK;
}


static ngx_int_t
ngx_http_themis_init(ngx_conf_t *cf)
{
    return NGX_OK;
}


static char *
ngx_http_themis_init_main_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}


static void *
ngx_http_themis_create_loc_conf(ngx_conf_t *cf)
{
    return NULL;
}


static char *
ngx_http_themis_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    return NGX_CONF_OK;
}


static void *
ngx_http_themis_create_main_conf(ngx_conf_t *cf)
{
    return NULL;
}


static ngx_int_t
ngx_http_themis_init_process(ngx_cycle_t *cycle)
{
    return NGX_OK;
}


static void
ngx_http_themis_exit(ngx_cycle_t *cycle)
{

}
