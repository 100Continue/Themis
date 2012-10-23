#include <ngx_event.h>
#include <ngx_channel.h>
#include <ngx_themis_core.h>


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
static ngx_int_t ngx_http_themis_apply_conf_handler(ngx_http_request_t *r);
static void ngx_http_themis_channel_handler(ngx_event_t *ev);


extern ngx_socket_t ngx_themis_socketpairs[NGX_MAX_PROCESSES][2];
/* just for test */
ngx_event_t  themis_timer;
static void ngx_themis_timer_handler(ngx_event_t *ev);


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
    return NGX_OK;
}


static ngx_int_t
ngx_http_themis_init(ngx_conf_t *cf)
{
    void                         *c, **module_configs;
    ngx_uint_t                    i, j;
    ngx_hash_key_t               *nodes;
    ngx_hash_init_t               configs_hash;
    ngx_http_handler_pt          *h;
    ngx_themis_module_t          *m;
    ngx_http_core_main_conf_t    *cmcf;
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
            ngx_pcalloc(cf->pool, sizeof(void *) * ngx_max_module);

        nodes[i].value = module_configs;
        for (j = 0; ngx_modules[j]; j++) {
            if (ngx_modules[j]->type != NGX_THEMIS_MODULE) {
                continue;
            }

            m = ngx_modules[j]->ctx;
            if (!m->create_config) {
                continue;
            }

            c = m->create_config(cf);
            if (c == NULL) {
                ngx_log_themis(NGX_LOG_ERR, cf->log, 0, "%V create conf error",
                               &m->name);
                return NGX_ERROR;
            }
            module_configs[j] = c;
        }
    }

    if (ngx_hash_init(&configs_hash, tmcf->configs.elts, tmcf->configs.nelts)
        != NGX_OK)
    {
        ngx_log_themis(NGX_LOG_ERR, cf->log, 0, "http themis init hash error");
        return NGX_ERROR;
    }

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_PREACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }
    *h = ngx_http_themis_apply_conf_handler;

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
    tlcf->module_configs = NGX_CONF_UNSET_PTR;

    return tlcf;
}


static char *
ngx_http_themis_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_themis_loc_conf_t  *prev = parent;
    ngx_http_themis_loc_conf_t  *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);
    ngx_conf_merge_str_value(conf->name, prev->name, "");
    ngx_conf_merge_ptr_value(conf->module_configs, prev->module_configs, NULL);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_themis_init_process(ngx_cycle_t *cycle)
{
    ngx_int_t   i;

    ngx_log_themis_debug0(NGX_LOG_DEBUG_THEMIS, cycle->log, 0, "init process");

    for (i = 0; i <= ngx_last_process; i++) {
        if (ngx_processes[i].pid == -1) {
            continue;
        }

        if (i == ngx_process_slot) {
            continue;
        }

        if (ngx_themis_socketpairs[i][1] == 0) {
            continue;
        }

        ngx_log_themis_debug1(NGX_LOG_DEBUG_THEMIS, cycle->log, 0,
                              "close useless channel socket %i", i);

        if (close(ngx_themis_socketpairs[i][0]) == -1) {
            ngx_log_themis(NGX_LOG_ALERT, cycle->log, ngx_errno,
                           "close() failed");
        }
        ngx_themis_socketpairs[i][0] = 0;

        if (close(ngx_themis_socketpairs[i][1]) == -1) {
            ngx_log_themis(NGX_LOG_ALERT, cycle->log, ngx_errno,
                           "close() failed");
        }
        ngx_themis_socketpairs[i][1] = 0;
    }

    if (close(ngx_themis_socketpairs[ngx_process_slot][0]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "close() socketpair failed");
    }
    ngx_themis_socketpairs[ngx_process_slot][0] = 0;

    if (ngx_add_channel_event(cycle,
                              ngx_themis_socketpairs[ngx_process_slot][1],
                              NGX_READ_EVENT, ngx_http_themis_channel_handler)
        == NGX_ERROR)
    {
        ngx_log_themis(NGX_LOG_ERR, cycle->log, ngx_errno,
                       "add channel event error");
        return NGX_ERROR;
    }

    /*
      demo of dynamic conf update
      TODO: use channel event to replace timer
    */

    ngx_memzero(&themis_timer, sizeof(ngx_event_t));

    themis_timer.handler = ngx_themis_timer_handler;
    themis_timer.log = cycle->log;
    themis_timer.data = cycle;

    ngx_add_timer(&themis_timer, 1000);

    return NGX_OK;
}


static void
ngx_http_themis_exit(ngx_cycle_t *cycle)
{
    ngx_int_t   i;

    ngx_log_themis_debug0(NGX_LOG_DEBUG_THEMIS, cycle->log, 0, "exit process");

    for (i = 0; i <= ngx_last_process; i++) {
        ngx_log_themis_debug1(NGX_LOG_DEBUG_THEMIS, cycle->log, 0,
                              "close channel socket %i", i);

        if (ngx_processes[i].pid == -1) {
            continue;
        }

        if (ngx_themis_socketpairs[i][0] != 0) {
            if (close(ngx_themis_socketpairs[i][0]) == -1) {
                ngx_log_themis(NGX_LOG_ALERT, cycle->log, ngx_errno,
                               "close() failed");
            }
        }
        if (ngx_themis_socketpairs[i][1] != 0) {
            if (close(ngx_themis_socketpairs[i][1]) == -1) {
                ngx_log_themis(NGX_LOG_ALERT, cycle->log, ngx_errno,
                               "close() failed");
            }
        }
    }
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
    tmcf->enable = 1;

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


static void
ngx_themis_timer_handler(ngx_event_t *ev)
{
    /* TODO: channel event handler */

    void                        **configs;
    ngx_int_t                     hash, rc;
    ngx_uint_t                    i, j;
    ngx_cycle_t                  *cycle;
    ngx_hash_key_t               *key, *keys;
    ngx_themis_module_t          *m;
    ngx_http_themis_main_conf_t  *tmcf;

    cycle = ev->data;
    tmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_themis_module);

    keys = tmcf->configs.elts;
    for (i = 0; i < tmcf->configs.nelts; i++) {
        key = &keys[i];

        for (j = 0; ngx_modules[j]; j++) {
            if (ngx_modules[j]->type != NGX_THEMIS_MODULE) {
                continue;
            }

            m = ngx_modules[j]->ctx;
            if (!m->update_config) {
                continue;
            }

            hash = ngx_hash_key_lc(key->key.data, key->key.len);
            configs = ngx_hash_find(&tmcf->configs_hash, hash,
                                    key->key.data, key->key.len);
            if (!configs[j]) {
                continue;
            }

            rc = m->update_config(cycle, (void *)((uintptr_t) configs[j]
                                                  & (uintptr_t) ~0x1));
            if (rc != NGX_OK) {
                return;
            }

            configs[j] = (void *)((uintptr_t) configs[j] | (uintptr_t) 0x1);
        }
    }

    ngx_add_timer(ev, 10000);
}


static ngx_int_t
ngx_http_themis_apply_conf_handler(ngx_http_request_t *r)
{
    ngx_uint_t                   i;
    ngx_themis_module_t         *m;
    ngx_http_themis_loc_conf_t  *tlcf;

    tlcf = ngx_http_get_module_loc_conf(r, ngx_http_themis_module);
    if (!tlcf->enable) {
        return NGX_DECLINED;
    }

    for (i = 0; ngx_modules[i] ; i++) {
        if (ngx_modules[i]->type != NGX_THEMIS_MODULE) {
            continue;
        }

        m = ngx_modules[i]->ctx;
        if (!m->apply_config) {
            continue;
        }

        if (!((uintptr_t) (*tlcf->module_configs)[i] & (uintptr_t) 0x1)) {
            continue;
        }

        (*tlcf->module_configs)[i] =
            (void *)((uintptr_t) (*tlcf->module_configs)[i] & (uintptr_t) ~0x1);

        ngx_log_themis(NGX_LOG_DEBUG, r->connection->log, 0,
                       "%V %V apply http config", &m->name, &tlcf->name);
        m->apply_config(r, (*tlcf->module_configs)[i]);
    }

    return NGX_DECLINED;
}


static void
ngx_http_themis_channel_handler(ngx_event_t *ev)
{
    /* TODO */
}


void *
ngx_themis_get_masked_module_conf(ngx_str_t *name, ngx_int_t index)
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
