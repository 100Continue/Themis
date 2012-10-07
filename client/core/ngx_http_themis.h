#ifndef _NGX_HTTP_THEMIS_MODULE_H_INCLUDE_
#define _NGX_HTTP_THEMIS_MODULE_H_INCLUDE_

#include <ngx_core.h>

typedef void *(*ngx_themis_create_config_pt)(ngx_conf_t *cf);
typedef struct ngx_themis_module_s ngx_themis_module_t;
typedef struct ngx_http_themis_loc_conf_s ngx_http_themis_loc_conf_t;
typedef struct ngx_http_themis_main_conf_s ngx_http_themis_main_conf_t;


struct ngx_themis_module_s {
    ngx_uint_t                   index;

    ngx_str_t                    name;

    ngx_themis_create_config_pt  create_config;
};


struct ngx_http_themis_main_conf_s {
    ngx_array_t  configs;        /* ngx_hash_key_t: ngx_themis_config_t */

    ngx_hash_t   configs_hash;

    ngx_uint_t   configs_hash_max_size;
    ngx_uint_t   configs_hash_bucket_size;
};


struct ngx_http_themis_loc_conf_s {
    ngx_str_t             name;
    ngx_flag_t            enable;
    void               ***module_configs; /* point to *module_configs[] */
};


extern ngx_themis_module_t *ngx_themis_modules[];
extern ngx_uint_t ngx_themis_modules_max;
extern ngx_module_t  ngx_http_themis_module;

#endif
