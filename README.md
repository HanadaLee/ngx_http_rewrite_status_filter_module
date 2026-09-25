# ngx_http_rewrite_status_filter_module

# Name
ngx_http_rewrite_status_filter_module is a filter module used to rewrite response status code.

# Table of Content

* [Name](#name)
* [Status](#status)
* [Synopsis](#synopsis)
* [Installation](#installation)
* [Conditional syntax](#conditional-syntax)
* [Directives](#directives)
  * [rewrite_status](#rewrite_status)
* [Author](#author)
* [License](#license)

# Status

This Nginx module is currently considered experimental. Issues and PRs are welcome if you encounter any problems.

# Synopsis

```nginx
server {
    listen 127.0.0.1:8080;
    server_name localhost;

    location / {
        expr rewrite_to_404 !is_empty $http_rsp_404_status;
        when rewrite_to_404 {
            rewrite_status 404;
        }
        proxy_pass http://foo.com;
    }
}
```

# Installation

To use theses modules, configure your nginx branch with `--add-module=/path/to/ngx_http_rewrite_status_filter_module`.

To enable named conditions, build `ngx_expr_module` and this module statically in the same nginx configuration.

# Conditional syntax

Conditional syntax is selected at compile time. With `ngx_expr_module`, place `rewrite_status` inside an `http`, `server`, or `location` `when` block; `if=` and `if!=` are rejected. Without it, `when` is unavailable and the legacy `if=`/`if!=` parameters remain supported.

# Directives

## rewrite_status

**Syntax:** *rewrite_status status;*

**Default:** *-*

**Context:** *http, server, location, http when, server when, location when

Rewrite response status code.

# Author

Hanada im@hanada.info

# License

This Nginx module is licensed under [BSD 2-Clause License](LICENSE).
