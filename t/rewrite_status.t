#!/usr/bin/perl

# Tests for ngx_http_rewrite_status_filter_module.

###############################################################################

use warnings;
use strict;

use Test::More;

BEGIN { use FindBin; chdir($FindBin::Bin); }

use Test::Nginx;

###############################################################################

select STDERR; $| = 1;
select STDOUT; $| = 1;

my $t = Test::Nginx->new()->has(qw/http rewrite ngx_expr_module
	ngx_http_rewrite_status_filter_module/)->plan(6);

$t->write_file_expand('nginx.conf', <<'EOF');

%%TEST_GLOBALS%%

daemon off;

events {
}

http {
    %%TEST_GLOBALS_HTTP%%

    server {
        listen       127.0.0.1:8080;
        server_name  localhost;

        expr selected str_eq $arg_mode selected;

        location /plain {
            return 201 plain;
        }

        location /always {
            rewrite_status 202;
            return 201 always;
        }

        location /conditional {
            when selected {
                rewrite_status 404;
            }
            rewrite_status 418;
            return 200 conditional;
        }

        location /parent/ {
            rewrite_status 203;

            location /parent/inherit {
                return 200 inherit;
            }

            location /parent/override {
                rewrite_status 204;
                return 200 override;
            }
        }
    }
}

EOF

$t->run();

###############################################################################

like(http_get('/plain'), qr/201 Created/, 'status unchanged');
like(http_get('/always'), qr/202 Accepted/, 'unconditional rewrite');
like(http_get('/conditional'), qr/418 /, 'conditional rule miss');
like(http_get('/conditional?mode=selected'), qr/404 Not Found/,
	'conditional rule hit');
like(http_get('/parent/inherit'), qr/203 /, 'inherit status rule');
like(http_get('/parent/override'), qr/204 No Content/,
	'local status rule replaces inherited rule');

###############################################################################
