#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

#define CR '\r'
#define LF '\n'
#define CRLFCRLF "\r\n\r\n"

int lan_http_parse_request_line(lan_http_request_t *r);
int lan_http_parse_request_body(lan_http_request_t *r);

#endif
