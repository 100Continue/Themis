#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>
#include <ngx_themis_log.h>
#include <ngx_http_themis.h>


extern ngx_module_t  ngx_http_access_module;


static void *ngx_themis_access_mate_create_config(ngx_conf_t *cf);
static ngx_int_t ngx_themis_access_mate_update_config(ngx_cycle_t *cycle,
    void *config);
static ngx_int_t ngx_themis_access_mate_apply_config(ngx_http_request_t *r,
    void *config);


typedef struct {
    in_addr_t         mask;
    in_addr_t         addr;
    ngx_uint_t        deny;      /* unsigned  deny:1; */
} ngx_http_access_rule_t;


typedef struct {
    ngx_array_t      *rules;     /* array of ngx_http_access_rule_t */
#if (NGX_HAVE_INET6)
    ngx_array_t      *rules6;    /* array of ngx_http_access_rule6_t */
#endif
} ngx_http_access_loc_conf_t;


typedef struct {
    ngx_str_t     ip;
    ngx_uint_t    deny;
    ngx_pool_t   *pool;
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
    if (amcf == NULL) {
        return NULL;
    }

    amcf->pool = ngx_create_pool(ngx_pagesize, cf->log);
    if (amcf->pool == NULL) {
        return NULL;
    }

    return amcf;
}


static ngx_int_t
ngx_themis_access_mate_update_config(ngx_cycle_t *cycle, void *config)
{
    ngx_themis_access_mate_conf_t  *amcf = config;

    ngx_str_set(&amcf->ip, "127.0.0.1");
    amcf->deny = ngx_random() % 2;

    ngx_log_themis(NGX_LOG_DEBUG, cycle->log, 0, "access update config %V %i",
                   &amcf->ip, amcf->deny);

    return NGX_OK;
}


static ngx_int_t
ngx_themis_access_mate_apply_config(ngx_http_request_t *r, void *config)
{
    ngx_themis_access_mate_conf_t  *amcf = config;

    ngx_int_t                    rc;
    ngx_uint_t                   i;
    ngx_cidr_t                   cidr;
    ngx_http_access_rule_t      *rule, *rules;
    ngx_http_access_loc_conf_t  *alcf;

    ngx_log_themis(NGX_LOG_DEBUG, r->connection->log, 0,
                   "apply update config %V %i", &amcf->ip, amcf->deny); 

    ngx_memzero(&cidr, sizeof(ngx_cidr_t));
    alcf = ngx_http_get_module_loc_conf(r, ngx_http_access_module);
    if (alcf->rules == NULL) {
        alcf->rules = ngx_array_create(amcf->pool, 4,
                                       sizeof(ngx_http_access_rule_t));
        if (alcf->rules == NULL) {
            return NGX_ERROR;
        }
    }

    rc = ngx_ptocidr(&amcf->ip, &cidr);
    if (rc == NGX_ERROR) {
        ngx_log_themis(NGX_LOG_ERR, r->connection->log, 0, "ip format error");
        return NGX_ERROR;
    }

    if (rc == NGX_DONE) {
        ngx_log_themis(NGX_LOG_WARN, r->connection->log, 0,
                       "low address bits of %V are meaningless", &amcf->ip);
    }

    rules = alcf->rules->elts;
    for (i = 0; i < alcf->rules->nelts; i++) {
        rule = &rules[i];
        if (rule->mask == cidr.u.in.mask && rule->addr == cidr.u.in.addr) {
            rule->deny = amcf->deny;
            return NGX_OK;
        }
    }

    rule = ngx_array_push(alcf->rules);
    rule->mask = cidr.u.in.mask;
    rule->addr = cidr.u.in.addr;
    rule->deny = amcf->deny;

    return NGX_OK;
}
