#ifndef _NGX_THEMIS_LOG_H_INCLUDE_
#define _NGX_THEMIS_LOG_H_INCLUDE_

#if (NGX_HAVE_VARIADIC_MACROS)

#define ngx_log_themis(level, log, err, ...)             \
    ngx_log_error(level, log, err, "[Themis] "__VA_ARGS__)

#elif (NGX_HAVE_VARIADIC_MACROS)

#define ngx_log_themis(level, log, err, args...)    \
    ngx_log_error(level, log, err, "[Themis] "args)

#else
    ngx_log_error(level, log, err, "[Themis] "...)

#endif

#endif
