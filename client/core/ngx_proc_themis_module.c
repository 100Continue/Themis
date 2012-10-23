#include <ngx_event.h>
#include <ngx_channel.h>
#include <ngx_event_connect.h>
#include <ngx_http.h>
#include <ngx_themis_core.h>


typedef struct ngx_proc_themis_conf_s ngx_proc_themis_conf_t;
typedef struct ngx_proc_themis_context_s ngx_proc_themis_context_t;


static void *ngx_proc_themis_create_conf(ngx_conf_t *cf);
static char *ngx_proc_themis_merge_conf(ngx_conf_t *cf,
    void *parent, void *child);
static ngx_int_t ngx_proc_themis_prev_start(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_themis_loop_proc(ngx_cycle_t *cycle);
static void ngx_proc_themis_exit(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_themis_init_process(ngx_cycle_t *cycle);
static ngx_int_t ngx_proc_themis_init_module(ngx_cycle_t *cycle);
static char *ngx_proc_themis_set_log(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static void ngx_proc_themis_channel_handler(ngx_event_t *ev);


extern ngx_socket_t ngx_themis_socketpairs[NGX_MAX_PROCESSES][2];


struct ngx_proc_themis_conf_s {
    ngx_str_t  server;

    ngx_log_t  log;
};


struct ngx_proc_themis_context_s {
    ngx_addr_t  server_addr;
};


static ngx_proc_themis_context_t ngx_proc_themis_ctx;


static ngx_command_t ngx_proc_themis_commands[] = {
    { ngx_string("server"),
      NGX_PROC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_PROC_CONF_OFFSET,
      offsetof(ngx_proc_themis_conf_t, server),
      NULL },

    { ngx_string("log"),
      NGX_PROC_CONF|NGX_CONF_TAKE12,
      ngx_proc_themis_set_log,
      NGX_PROC_CONF_OFFSET,
      offsetof(ngx_proc_themis_conf_t, server),
      NULL },

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


static char *
ngx_proc_themis_set_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_proc_themis_conf_t *ptcf = conf;

    ngx_int_t   rc;
    ngx_str_t  *value, name;

    if (ptcf->log.file) {
        return "is duplicate";
    }

    value = cf->args->elts;

    rc = ngx_log_target(cf->cycle, &value[1], &ptcf->log);
    if (rc == NGX_ERROR) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid themis logger \"%V\"", &value[1]);
        return NGX_CONF_ERROR;

    } else if (rc == NGX_OK) {
        if (ptcf->log.file != NULL) {
            name = ngx_log_error_backup;
            if (ngx_conf_full_name(cf->cycle, &name, 0) != NGX_OK) {
                return "fail to set backup";
            }

            ptcf->log.file->name = name;
        }
    } else {
        if (ngx_strcmp(value[1].data, "stderr") == 0) {
            ngx_str_null(&name);
        } else {
            name = value[1];
        }

        ptcf->log.file = ngx_conf_open_file(cf->cycle, &name);
        if (ptcf->log.file == NULL) {
            return NULL;
        }
    }

    if (cf->args->nelts == 2) {
        ptcf->log.log_level = NGX_LOG_ERR;
        return NGX_CONF_OK;
    }

    ptcf->log.log_level = 0;

    return ngx_log_set_levels(cf, &ptcf->log);
}


static void *
ngx_proc_themis_create_conf(ngx_conf_t *cf)
{
    ngx_proc_themis_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_proc_themis_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *
ngx_proc_themis_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_proc_themis_conf_t  *prev = parent;
    ngx_proc_themis_conf_t  *conf = child;

    ngx_conf_merge_str_value(prev->server, conf->server, "");

    if (!conf->log.file) {
        conf->log = prev->log;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_proc_themis_prev_start(ngx_cycle_t *cycle)
{
    ngx_http_themis_main_conf_t  *tmcf;

    tmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_themis_module);
    if (!tmcf->enable) {
        return NGX_DECLINED;
    }

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
    ngx_int_t                i;
    ngx_log_t               *log;
    ngx_proc_themis_conf_t  *ptcf;

    ptcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_themis_module);
    log = &ptcf->log;


    for (i = 0; i <= ngx_last_process; i++) {
        if (ngx_processes[i].pid == -1) {
            continue;
        }

        ngx_log_themis_debug1(NGX_LOG_DEBUG_THEMIS, log, 0,
                              "close channel socket %i", i);

        if (ngx_themis_socketpairs[i][0] != 0) {
            if (close(ngx_themis_socketpairs[i][0]) == -1) {
                ngx_log_themis(NGX_LOG_ALERT, log, ngx_errno,
                               "close() failed");
            }
        }

        if (ngx_themis_socketpairs[i][1] != 0) {
            if (close(ngx_themis_socketpairs[i][0]) == -1) {
                ngx_log_themis(NGX_LOG_ALERT, log, ngx_errno,
                               "close() failed");
            }
        }
    }
}


static ngx_int_t
ngx_proc_themis_init_process(ngx_cycle_t *cycle)
{
    ngx_url_t                u;
    ngx_int_t                i;
    ngx_proc_themis_conf_t  *ptcf;

    ptcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_themis_module);
    u.url = ptcf->server;
    u.no_resolve = 0;

    if (ngx_parse_url(cycle->pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_log_themis(NGX_LOG_ERR, &ptcf->log, 0,
                           "%s in config_server \"%V\"", u.err, &u.url);
        }

        return NGX_ERROR;
    }

    /* TODO: connect to themis backend-server */
    ngx_proc_themis_ctx.server_addr = *u.addrs;

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

        ngx_log_themis_debug1(NGX_LOG_DEBUG_THEMIS, &ptcf->log, 0,
                              "close useless channel socket %i", i);
        if (close(ngx_themis_socketpairs[i][1]) == -1) {
            ngx_log_themis(NGX_LOG_ALERT, cycle->log, ngx_errno,
                           "close() channel failed");
        }

        ngx_themis_socketpairs[i][1] = 0;

        if (ngx_add_channel_event(cycle, ngx_themis_socketpairs[i][0],
                                  NGX_READ_EVENT,
                                  ngx_proc_themis_channel_handler)
            == NGX_ERROR)
        {
            ngx_log_themis(NGX_LOG_ERR, cycle->log, ngx_errno,
                           "add channel event error");
            return NGX_ERROR;
        }
    }

    if (close(ngx_themis_socketpairs[ngx_process_slot][0]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "close() socketpair failed");
    }

    if (close(ngx_themis_socketpairs[ngx_process_slot][1]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "close() socketpair failed");
    }

    return NGX_OK;
}


static ngx_int_t
ngx_proc_themis_init_module(ngx_cycle_t *cycle)
{
    ngx_int_t                     i, s;
    ngx_log_t                    *log;
    ngx_socket_t                 *socks;
    ngx_core_conf_t              *ccf;
    ngx_proc_themis_conf_t       *ptcf;
    ngx_http_themis_main_conf_t  *tmcf;

    tmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_themis_module);
    if (!tmcf->enable) {
        return NGX_DECLINED;
    }

    if (ngx_get_conf(cycle->conf_ctx, ngx_procs_module) == NULL) {
        return NGX_OK;
    }

    ptcf = ngx_proc_get_conf(cycle->conf_ctx, ngx_proc_themis_module);
    if (ptcf == NULL) {
        ngx_log_themis(NGX_LOG_INFO, cycle->log, 0,
                      "no \"themis process\" in configuration");
        return NGX_OK;
    }

    log = &ptcf->log;
    ngx_log_themis_debug1(NGX_LOG_DEBUG_THEMIS, log, 0,
                          "process init module: %d", ngx_last_process);

    ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_core_module);

    s = 0;
    for (i = 0; i < ccf->worker_processes; i++, s++) {
        while (s < ngx_last_process && ngx_processes[s].pid != -1) {
            s++;
        }

        if (s == NGX_MAX_PROCESSES) {
            ngx_log_themis(NGX_LOG_ALERT, log, 0,
                          "no more than %d procs can be spawned",
                           NGX_MAX_PROCESSES);
            return NGX_ERROR;
        }

        ngx_log_themis_debug1(NGX_LOG_DEBUG_THEMIS, log, 0,
                              "init socketpair %i", s);

        socks = ngx_themis_socketpairs[s];
        if (socks[0] != 0) {
            ngx_close_channel(socks, log);
        }

        if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks) == -1) {
            return NGX_ERROR;
        }

        if (ngx_nonblocking(socks[0]) == -1) {
            ngx_close_channel(socks, log);
            return NGX_ERROR;
        }

        if (ngx_nonblocking(socks[1]) == -1) {
            ngx_close_channel(socks, log);
            return NGX_ERROR;
        }

        if (fcntl(socks[0], F_SETOWN, ngx_pid) == -1) {
            ngx_close_channel(socks, log);
            return NGX_ERROR;
        }

        if (fcntl(socks[0], F_SETFD, FD_CLOEXEC) == -1) {
            ngx_close_channel(socks, log);
            return NGX_ERROR;
        }

        if (fcntl(socks[1], F_SETFD, FD_CLOEXEC) == -1) {
            ngx_close_channel(socks, log);
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


static void
ngx_proc_themis_channel_handler(ngx_event_t *ev)
{
    /* TODO */

}
