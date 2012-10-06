#include <ngx_http_themis.h>

extern ngx_themis_module_t ngx_themis_footer_mate;

ngx_themis_module_t *ngx_themis_modules[] = {
    &ngx_themis_footer_mate,
    NULL
};

ngx_uint_t ngx_themis_modules_max;
