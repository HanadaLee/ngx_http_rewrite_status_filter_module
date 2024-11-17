
/*
 * Copyright (C) Hanada
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_uint_t                 status_code;
    ngx_http_complex_value_t  *filter;
    ngx_int_t                  negative;
} ngx_http_rewrite_status_rule_t;


typedef struct {
    ngx_array_t                *rules; /* Array of ngx_http_rewrite_status_rule_t */
} ngx_http_rewrite_status_conf_t;


static ngx_int_t ngx_http_rewrite_status_filter_init(ngx_conf_t *cf);
static void *ngx_http_rewrite_status_create_conf(ngx_conf_t *cf);
static char *ngx_http_rewrite_status_merge_conf(ngx_conf_t *cf,
    void *parent, void *child);
static char *ngx_http_rewrite_status(ngx_conf_t *cf,
    ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_rewrite_status_header_filter(ngx_http_request_t *r);


static ngx_http_module_t  ngx_http_rewrite_status_filter_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_rewrite_status_filter_init,   /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_rewrite_status_create_conf,   /* create location configuration */
    ngx_http_rewrite_status_merge_conf     /* merge location configuration */
};


static ngx_command_t  ngx_http_rewrite_status_filter_commands[] = {

    { ngx_string("rewrite_status"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE12,
      ngx_http_rewrite_status,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


ngx_module_t  ngx_http_rewrite_status_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_rewrite_status_filter_module_ctx, /* module context */
    ngx_http_rewrite_status_filter_commands,    /* module directives */
    NGX_HTTP_MODULE,                            /* module type */
    NULL,                                       /* init master */
    NULL,                                       /* init module */
    NULL,                                       /* init process */
    NULL,                                       /* init thread */
    NULL,                                       /* exit thread */
    NULL,                                       /* exit process */
    NULL,                                       /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;


static void *
ngx_http_rewrite_status_create_conf(ngx_conf_t *cf)
{
    ngx_http_rewrite_status_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_rewrite_status_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *
ngx_http_rewrite_status_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_rewrite_status_conf_t *prev = parent;
    ngx_http_rewrite_status_conf_t *conf = child;

    if (conf->rules == NULL) {
        conf->rules = prev->rules;
    }

    return NGX_CONF_OK;
}


static char *
ngx_http_rewrite_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_rewrite_status_conf_t    *rst_conf = conf;
    ngx_http_rewrite_status_rule_t    *rule;

    ngx_str_t                         *value;
    ngx_uint_t                         status_code;
    ngx_int_t                          n;
    ngx_str_t                          s;
    ngx_http_compile_complex_value_t   ccv;

    if (rst_conf->rules == NULL) {
        rst_conf->rules = ngx_array_create(cf->pool, 4, sizeof(ngx_http_rewrite_status_rule_t));
        if (rst_conf->rules == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    rule = ngx_array_push(rst_conf->rules);
    if (rule == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(rule, sizeof(ngx_http_rewrite_status_rule_t));

    value = cf->args->elts;

    n = ngx_atoi(value[1].data, value[1].len);
    if (n == NGX_ERROR || n < 100 || n > 999) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "invalid status code \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    status_code = (ngx_uint_t) n;
    rule->status_code = status_code;

    if (cf->args->nelts == 3) {
        if (ngx_strncmp(value[2].data, "if=", 3) == 0) {
            s.len = value[2].len - 3;
            s.data = value[2].data + 3;
            rule->negative = 0;
        } else if (ngx_strncmp(value[2].data, "if!=", 4) == 0){
            s.len = value[2].len - 4;
            s.data = value[2].data + 4;
            rule->negative = 1;
        } else {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                "invalid parameter \"%V\"", &value[2]);
            return NGX_CONF_ERROR;
        }

        ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

        ccv.cf = cf;
        ccv.value = &s;
        ccv.complex_value = ngx_palloc(cf->pool,
                                    sizeof(ngx_http_complex_value_t));
        if (ccv.complex_value == NULL) {
            return NGX_CONF_ERROR;
        }

        if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
            return NGX_CONF_ERROR;
        }

        rule->filter = ccv.complex_value;
    } else {
        rule->negative = 0;
        rule->filter = NULL;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_rewrite_status_header_filter(ngx_http_request_t *r)
{
    ngx_http_rewrite_status_conf_t  *conf;
    ngx_http_rewrite_status_rule_t  *rules;
    ngx_uint_t                       i;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_rewrite_status_filter_module);

    if (conf->rules == NULL || conf->rules->nelts == 0) {
        return ngx_http_next_header_filter(r);
    }

    rules = conf->rules->elts;

    for (i = 0; i < conf->rules->nelts; i++) {
        if (rules[i].filter) {
            ngx_str_t  val;
            if (ngx_http_complex_value(r, rules[i].filter, &val)
                    != NGX_OK) {
                return NGX_ERROR;
            }

            if ((val.len == 0 || (val.len == 1 && val.data[0] == '0'))) {
                if (!rules[i].negative) {
                    /* Skip due to filter*/
                    continue;
                }
            } else {
                if (rules[i].negative) {
                    /* Skip due to negative filter*/
                    continue;
                }
            }
        }

        // rewrite status code
        r->headers_out.status = rules[i].status_code;
        r->headers_out.status_line.len = 0;
        break;
    }

    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_rewrite_status_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_rewrite_status_header_filter;

    return NGX_OK;
}
