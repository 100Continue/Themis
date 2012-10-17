# Developing... #

A set of Nginx modules to collect requests'information and send to backend server for analysing and feedback to support Nginx could update config immediately to against attacks.

## Config Example: ##

    worker_processes  1;

    error_log  logs/error.log debug;
    #pid        logs/nginx.pid;

    daemon off;
    events {
        worker_connections  10240;
    }


    http {
        include       mime.types;
        default_type  application/octet-stream;

        server {
            listen       8080;
            server_name  localhost;
        
            themis app1;
            location / {
                themis app2;
                root   html;
                index  index.html index.htm;
            }
            error_page   500 502 503 504  /50x.html;
            location = /50x.html {
                root   html;
            }
        }
    }
