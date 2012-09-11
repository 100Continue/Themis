#include <ngx_event.h>
#include <ngx_channel.h>
#include <ngx_event_connect.h>
#include <ngx_http.h>


static void *ngx_proc_themis_create_conf(ngx_conf_t *cf);
static char *ngx_proc_themis_merge_conf(ngx_conf_t *cf,
    void *parent, void *child);
static ngx_int_t ngx_proc_themis_prev_start(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_themis_loop_proc(ngx_cycle_t *cycle);
static void ngx_proc_themis_exit(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_themis_init_process(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_themis_init_module(ngx_cycle_t *cycle);


static ngx_command_t ngx_proc_themis_commands[] = {
      ngx_null_command
};


static ngx_proc_module_t ngx_proc_themis_module_ctx = {
    ngx_string("themis"),
    NULL,
    NULL,
    ngx_proc_themis_create_conf,           /* create proc conf */
    ngx_proc_themis_merge_conf,            /* merge proc conf */
    ngx_proc_themis_prev_start,            /* prevstart */
    ngx_proc_themis_init_process,          /* process init */
    ngx_proc_themis_loop_proc,             /* process loop proc */
    ngx_proc_themis_exit                   /* process exit */
};


ngx_module_t  ngx_proc_themis_module = {
    NGX_MODULE_V1,
    &ngx_proc_themis_module_ctx,           /* module context */
    ngx_proc_themis_commands,              /* module directives */
    NGX_PROC_MODULE,                       /* module type */
    NULL,                                  /* init master */
    ngx_proc_themis_init_module,           /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_proc_themis_create_conf(ngx_conf_t *cf)
{
    return NULL;
}


static char *
ngx_proc_themis_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    return NULL;
}


static ngx_int_t
ngx_proc_themis_prev_start(ngx_cycle_t *cycle)
{
    return NGX_OK;
}


static ngx_int_t
ngx_proc_themis_loop_proc(ngx_cycle_t *cycle)
{
    return NGX_OK;
}


static void
ngx_proc_themis_exit(ngx_cycle_t *cycle)
{

}


static ngx_int_t
ngx_proc_themis_init_process(ngx_cycle_t *cycle)
{
    return NGX_OK;
}


static ngx_int_t
ngx_proc_themis_init_module(ngx_cycle_t *cycle)
{
    return NGX_OK;
}
