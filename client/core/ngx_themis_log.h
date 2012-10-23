#ifndef _NGX_THEMIS_LOG_H_INCLUDE_
#define _NGX_THEMIS_LOG_H_INCLUDE_


#define NGX_LOG_DEBUG_THEMIS  0x800


#if (NGX_HAVE_VARIADIC_MACROS)

#define ngx_log_themis(level, log, err, ...)                     \
    ngx_log_error(level, log, err, "[Themis] "__VA_ARGS__)

#elif (NGX_HAVE_VARIADIC_MACROS)

#define ngx_log_themis(level, log, err, args...)                 \
    ngx_log_error(level, log, err, "[Themis] "args)

#else
ngx_log_error(level, log, err, "[Themis] "...)

#endif

#define ngx_log_themis_debug0(level, log, err, fmt)                            \
    ngx_log_debug0(level, log, err, "[Themis] "fmt)
#define ngx_log_themis_debug1(level, log, err, fmt, arg1)                      \
    ngx_log_debug1(level, log, err, "[Themis] "fmt, arg1)
#define ngx_log_themis_debug2(level, log, err, fmt, arg1, arg2)                \
    ngx_log_debug2(level, log, err, "[Themis] "fmt, arg1, arg2)
#define ngx_log_themis_debug3(level, log, err, fmt, arg1, arg2, arg3)          \
    ngx_log_debug3(level, log, err, "[Themis] "fmt, arg1, arg2, arg3)
#define ngx_log_themis_debug4(level, log, err, fmt, arg1, arg2, arg3, arg4)    \
    ngx_log_debug4(level, log, err, "[Themis] "fmt, arg1, arg2, arg3, arg4)
#define ngx_log_themis_debug5(level, log, err, fmt,                            \
                              arg1, arg2, arg3, arg4, arg5)                    \
    ngx_log_debug5(level, log, err, "[Themis] "fmt,                            \
                   arg1, arg2, arg3, arg4, arg5)
#define ngx_log_themis_debug6(level, log, err, fmt,                            \
                              arg1, arg2, arg3, arg4, arg5, arg6)              \
    ngx_log_debug6(level, log, err, "[Themis] "fmt,                            \
                   arg1, arg2, arg3, arg4, arg5, arg6)
#define ngx_log_themis_debug7(level, log, err, fmt,                            \
                              arg1, arg2, arg3, arg4, arg5, arg6, arg7)        \
    ngx_log_debug7(level, log, err, "[Themis] "fmt,                            \
                   arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define ngx_log_themis_debug8(level, log, err, fmt,                            \
                              arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)  \
    ngx_log_debug8(level, log, err, "[Themis] "fmt,                            \
                   arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)


#endif
