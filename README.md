# ngx_http_rewrite_status_filter_module

# Name
ngx_http_rewrite_status_filter_module is a filter module used to rewrite response status code.

# Table of Content

* [Name](#name)
* [Status](#status)
* [Synopsis](#synopsis)
* [Installation](#installation)
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
        rewrite_status 404 if=$http_rsp_404_status;
        proxy_pass http://foo.com;
    }
}
```

# Installation

To use theses modules, configure your nginx branch with `--add-module=/path/to/ngx_http_rewrite_status_filter_module`.

# Directives

## rewrite_status

**Syntax:** *rewrite_status status \[if=condition\];*

**Default:** *-*

**Context:** *http, server, location*

Rewrite response status code.

# Author

Hanada im@hanada.info

# License

This Nginx module is licensed under [BSD 2-Clause License](LICENSE).
