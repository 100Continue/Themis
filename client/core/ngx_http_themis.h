#ifndef _NGX_HTTP_THEMIS_MODULE_H_INCLUDE_
#define _NGX_HTTP_THEMIS_MODULE_H_INCLUDE_

#include <ngx_core.h>

typedef void *(*ngx_themis_create_config_pt)(ngx_conf_t *cf);
typedef struct ngx_themis_module_s ngx_themis_module_t;


struct ngx_themis_module_s {
    ngx_uint_t                   index;

    ngx_str_t                    name;

    ngx_themis_create_config_pt  create_config;
};

extern ngx_themis_module_t *ngx_themis_modules[];
extern ngx_uint_t ngx_themis_modules_max;

#endif
