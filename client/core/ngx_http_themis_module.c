#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>
#include <ngx_themis_log.h>
#include <ngx_http_themis.h>


static ngx_int_t ngx_http_themis_preconfiguration(ngx_conf_t *cf);
static ngx_int_t ngx_http_themis_init(ngx_conf_t *cf);
static void *ngx_http_themis_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_themis_init_main_conf(ngx_conf_t *cf, void *conf);
static void *ngx_http_themis_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_themis_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_themis_init_process(ngx_cycle_t *cycle);
static void ngx_http_themis_exit(ngx_cycle_t *cycle);
static char *ngx_themis_conf_set_config(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);



static ngx_command_t ngx_http_themis_commands[] = {

    { ngx_string("themis"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_themis_conf_set_config,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
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
    ngx_uint_t            i;
    ngx_themis_module_t  *m;

    ngx_themis_modules_max = 0;
    for (i = 0; ngx_themis_modules[i]; i++) {
        m = ngx_themis_modules[i];
        m->index = i;
    }

    ngx_themis_modules_max = i;

    return NGX_OK;
}


static ngx_int_t
ngx_http_themis_init(ngx_conf_t *cf)
{
    void                         *m, **module_configs;
    ngx_uint_t                    i, j;
    ngx_hash_key_t               *nodes;
    ngx_hash_init_t               configs_hash;
    ngx_http_themis_main_conf_t  *tmcf;

    tmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_themis_module);

    configs_hash.hash = &tmcf->configs_hash;
    configs_hash.key = ngx_hash_key_lc;
    configs_hash.max_size = tmcf->configs_hash_max_size;
    configs_hash.bucket_size = tmcf->configs_hash_bucket_size;
    configs_hash.name = "themis_configs_hash";
    configs_hash.pool = cf->pool;
    configs_hash.temp_pool = NULL;

    nodes = tmcf->configs.elts;
    for (i = 0; i < tmcf->configs.nelts; i++) {
        module_configs =
            ngx_pcalloc(cf->pool, sizeof(void *) * ngx_themis_modules_max);
        nodes[i].value = module_configs;

        for (j = 0; ngx_themis_modules[j]; j++) {
            m = ngx_themis_modules[j]->create_config(cf);
            if (m == NULL) {
                ngx_log_themis(NGX_LOG_ERR, cf->log, 0, "%V create conf error",
                               &ngx_themis_modules[j]->name);
                return NGX_ERROR;
            }
            module_configs[j] = m;
        }
    }

    if (ngx_hash_init(&configs_hash, tmcf->configs.elts, tmcf->configs.nelts)
        != NGX_OK)
    {
        ngx_log_themis(NGX_LOG_ERR, cf->log, 0, "http themis init hash error");
        return NGX_ERROR;
    }

    return NGX_OK;
}


static void *
ngx_http_themis_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_themis_main_conf_t  *tmcf;

    tmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_themis_main_conf_t));
    if (tmcf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&tmcf->configs, cf->pool, 4, sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NULL;
    }

    tmcf->configs_hash_max_size = NGX_CONF_UNSET_UINT;
    tmcf->configs_hash_bucket_size = NGX_CONF_UNSET_UINT;

    return tmcf;
}


static char *
ngx_http_themis_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_themis_main_conf_t *tmcf = conf;

    if (tmcf->configs_hash_max_size == NGX_CONF_UNSET_UINT) {
        tmcf->configs_hash_max_size = 512;
    }

    if (tmcf->configs_hash_bucket_size == NGX_CONF_UNSET_UINT) {
        tmcf->configs_hash_bucket_size = ngx_cacheline_size;
    }

    tmcf->configs_hash_bucket_size =
              ngx_align(tmcf->configs_hash_bucket_size, ngx_cacheline_size);

    return NGX_CONF_OK;
}


static void *
ngx_http_themis_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_themis_loc_conf_t  *tlcf;

    tlcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_themis_loc_conf_t));
    if (tlcf == NULL) {
        return NULL;
    }

    tlcf->enable = NGX_CONF_UNSET;

    return tlcf;
}


static char *
ngx_http_themis_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_themis_loc_conf_t  *prev = parent;
    ngx_http_themis_loc_conf_t  *conf = child;

    ngx_conf_merge_value(prev->enable, conf->enable, 0);
    ngx_conf_merge_str_value(prev->name, conf->name, "");

    return NGX_CONF_OK;
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


static char *
ngx_themis_conf_set_config(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_themis_loc_conf_t   *tlcf = conf;

    ngx_str_t                    *value;
    ngx_uint_t                    i;
    ngx_hash_key_t               *node, *nodes;
    ngx_http_conf_ctx_t          *http_ctx;
    ngx_http_themis_main_conf_t  *tmcf;

    value = cf->args->elts;
    tlcf->name = value[1];
    tlcf->enable = 1;
    http_ctx = cf->ctx;
    tmcf = http_ctx->main_conf[ngx_http_themis_module.ctx_index];

    nodes = tmcf->configs.elts;
    for (i = 0; i < tmcf->configs.nelts; i++) {
        if (ngx_strncmp(tlcf->name.data, nodes[i].key.data, tlcf->name.len)
            == 0)
        {
            tlcf->module_configs = (void *) &nodes[i].value;
            goto found;
        }
    }

    node = ngx_array_push(&tmcf->configs);
    if (node == NULL) {
        return NGX_CONF_ERROR;
    }

    node->key = tlcf->name;
    node->key_hash = ngx_hash_key_lc(node->key.data, node->key.len);
    node->value = NULL;

    tlcf->module_configs = (void *) &node->value;

found:

    return NGX_CONF_OK;
}


void *
ngx_themis_get_module_conf(ngx_str_t *name, ngx_int_t index)
{
    void                        **configs;
    ngx_int_t                     key;
    ngx_http_themis_main_conf_t  *tmcf;

    tmcf = ngx_http_cycle_get_module_main_conf(ngx_cycle,
                                               ngx_http_themis_module);

    key = ngx_hash_key_lc(name->data, name->len);
    configs = ngx_hash_find(&tmcf->configs_hash, key, name->data, name->len);
    if (configs == NULL) {
        return NULL;
    }

    return configs[index];
}
