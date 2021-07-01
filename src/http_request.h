#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <time.h>
#include "http.h"

#define LAN_AGAIN    EAGAIN

#define LAN_HTTP_PARSE_INVALID_METHOD        10
#define LAN_HTTP_PARSE_INVALID_REQUEST       11
#define LAN_HTTP_PARSE_INVALID_HEADER        12

#define LAN_HTTP_UNKNOWN                     0x0001
#define LAN_HTTP_GET                         0x0002
#define LAN_HTTP_HEAD                        0x0004
#define LAN_HTTP_POST                        0x0008

#define LAN_HTTP_OK                          200

#define LAN_HTTP_NOT_MODIFIED                304

#define LAN_HTTP_NOT_FOUND                   404

#define MAX_BUF 8124

typedef struct lan_http_request_s {
    void *root;
    int fd;
    int epfd;
    char buf[MAX_BUF];  /* ring buffer */
    size_t pos, last;
    int state;
    void *request_start;
    void *method_end;   /* not include method_end*/
    int method;
    void *uri_start;
    void *uri_end;      /* not include uri_end*/ 
    void *path_start;
    void *path_end;
    void *query_start;
    void *query_end;
    int http_major;
    int http_minor;
    void *request_end;

    struct list_head list;  /* store http header */
    void *cur_header_key_start;
    void *cur_header_key_end;
    void *cur_header_value_start;
    void *cur_header_value_end;

    void *timer;
} lan_http_request_t;

typedef struct {
    int fd;
    int keep_alive;
    time_t mtime;       /* the modified time of the file*/
    int modified;       /* compare If-modified-since field with mtime to decide whether the file is modified since last time*/

    int status;
} lan_http_out_t;

typedef struct lan_http_header_s {
    void *key_start, *key_end;          /* not include end */
    void *value_start, *value_end;
    list_head list;
} lan_http_header_t;

typedef int (*lan_http_header_handler_pt)(lan_http_request_t *r, lan_http_out_t *o, char *data, int len);

typedef struct {
    char *name;
    lan_http_header_handler_pt handler;
} lan_http_header_handle_t;

void lan_http_handle_header(lan_http_request_t *r, lan_http_out_t *o);
int lan_http_close_conn(lan_http_request_t *r);

int lan_init_request_t(lan_http_request_t *r, int fd, int epfd, lan_conf_t *cf);
int lan_free_request_t(lan_http_request_t *r);

int lan_init_out_t(lan_http_out_t *o, int fd);
int lan_free_out_t(lan_http_out_t *o);

const char *get_shortmsg_from_status_code(int status_code);

extern lan_http_header_handle_t     lan_http_headers_in[];

#endif

