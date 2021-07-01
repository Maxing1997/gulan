#include "http.h"
#include "http_parse.h"
#include "error.h"

//解析"GET / HTTP/1.1\r\n"
int lan_http_parse_request_line(lan_http_request_t *r) {
    u_char ch, *p, *m;
    size_t pi;

    enum {
        sw_start = 0,//开始解析
        sw_method,//方法
        sw_spaces_before_uri,
        sw_after_slash_in_uri,
        sw_http,
        sw_http_H,
        sw_http_HT,
        sw_http_HTT,
        sw_http_HTTP,
        sw_first_major_digit,
        sw_major_digit,
        sw_first_minor_digit,
        sw_minor_digit,
        sw_spaces_after_digit,
        sw_almost_done
    } state;

    state = r->state;

    // log_info("ready to parese request line, start = %d, last= %d", (int)r->pos, (int)r->last);
    for (pi = r->pos; pi < r->last; pi++) {
        p = (u_char *)&r->buf[pi % MAX_BUF];
        ch = *p;

        switch (state) {

        /* HTTP methods: GET, HEAD, POST */
        case sw_start:
            r->request_start = p;

            if (ch == CR || ch == LF) {
                break;
            }

            if ((ch < 'A' || ch > 'Z') && ch != '_') {
                return LAN_HTTP_PARSE_INVALID_METHOD;
            }

            state = sw_method;//状态机转移,开始解析方法
            break;

        case sw_method:
            if (ch == ' ') {
                r->method_end = p;
                m = r->request_start;

                switch (p - m) {

                case 3:
                    if (lan_str3_cmp(m, 'G', 'E', 'T', ' ')) {
                        r->method = LAN_HTTP_GET;
                        break;
                    }

                    break;

                case 4:
                    if (lan_str3Ocmp(m, 'P', 'O', 'S', 'T')) {
                        r->method = LAN_HTTP_POST;
                        break;
                    }

                    if (lan_str4cmp(m, 'H', 'E', 'A', 'D')) {
                        r->method = LAN_HTTP_HEAD;
                        break;
                    }

                    break;
                default:
                    r->method = LAN_HTTP_UNKNOWN;
                    break;
                }
                state = sw_spaces_before_uri;//开始解析uri前面的空格
                break;
            }

            if ((ch < 'A' || ch > 'Z') && ch != '_') {
                return LAN_HTTP_PARSE_INVALID_METHOD;
            }

            break;

        /* space* before URI */
        case sw_spaces_before_uri:

            if (ch == '/') {
                r->uri_start = p;
                state = sw_after_slash_in_uri;//开始解析uri后面的空格
                break;
            }

            switch (ch) {
                case ' ':
                    break;
                default:
                    return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_after_slash_in_uri:

            switch (ch) {
            case ' ':
                r->uri_end = p;
                state = sw_http;//开始解析HTTP/...
                break;
            default:
                break;
            }
            break;

        /* space+ after URI */
        case sw_http:
            switch (ch) {
            case ' ':
                break;
            case 'H':
                state = sw_http_H;//解析出H
                break;
            default:
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_H:
            switch (ch) {
            case 'T':
                state = sw_http_HT;//解析出HT
                break;
            default:
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HT:
            switch (ch) {
            case 'T':
                state = sw_http_HTT;//解析出HTT
                break;
            default:
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HTT:
            switch (ch) {
            case 'P':
                state = sw_http_HTTP;//解析出HTTP
                break;
            default:
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HTTP:
            switch (ch) {
            case '/':
                state = sw_first_major_digit;//开始解析HTTP版本第一个数字
                break;
            default:
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* first digit of major HTTP version */
        case sw_first_major_digit:
            if (ch < '1' || ch > '9') {
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_major = ch - '0';
            state = sw_major_digit;
            break;

        /* major HTTP version or dot */
        case sw_major_digit:
            if (ch == '.') {
                state = sw_first_minor_digit;//开始解析小版本第一个数字
                break;
            }

            if (ch < '0' || ch > '9') {
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_major = r->http_major * 10 + ch - '0';//23.1
            break;

        /* first digit of minor HTTP version */
        case sw_first_minor_digit:
            if (ch < '0' || ch > '9') {
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_minor = ch - '0';
            state = sw_minor_digit;
            break;

        /* minor HTTP version or end of request line */
        case sw_minor_digit:
            if (ch == CR) {//\r
                state = sw_almost_done;
                break;
            }

            if (ch == LF) {//\n
                goto done;
            }

            if (ch == ' ') {
                state = sw_spaces_after_digit;
                break;
            }

            if (ch < '0' || ch > '9') {
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_minor = r->http_minor * 10 + ch - '0';
            break;

        case sw_spaces_after_digit:
            switch (ch) {
            case ' ':
                break;
            case CR:
                state = sw_almost_done;
                break;
            case LF:
                goto done;
            default:
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* end of request line */
        case sw_almost_done:
            r->request_end = p - 1;
            switch (ch) {
            case LF://\n
                goto done;
            default:
                return LAN_HTTP_PARSE_INVALID_REQUEST;
            }
        }
    }

    r->pos = pi;
    r->state = state;

    return LAN_AGAIN;

done:

    r->pos = pi + 1;

    if (r->request_end == NULL) {
        r->request_end = p;
    }

    r->state = sw_start;

    return LAN_OK;
}

//解析"Host: 127.0.0.1:3000\r\nUser-Agent: Moz...."
int lan_http_parse_request_body(lan_http_request_t *r) {
    u_char ch, *p;
    size_t pi;

    enum {
        sw_start = 0,
        sw_key,
        sw_spaces_before_colon,//colon是冒号
        sw_spaces_after_colon,
        sw_value,
        sw_cr,
        sw_crlf,
        sw_crlfcr
    } state;

    state = r->state;
    check(state == 0, "state should be 0");

    //log_info("ready to parese request body, start = %d, last= %d", r->pos, r->last);

    lan_http_header_t *hd; 
    for (pi = r->pos; pi < r->last; pi++) {
        p = (u_char *)&r->buf[pi % MAX_BUF];
        ch = *p;

        switch (state) {
        case sw_start:
            if (ch == CR || ch == LF) {
                break;
            }

            r->cur_header_key_start = p;
            state = sw_key;
            break;
        case sw_key:
            if (ch == ' ') {
                r->cur_header_key_end = p;
                state = sw_spaces_before_colon;
                break;
            }

            if (ch == ':') {
                r->cur_header_key_end = p;//记录位置
                state = sw_spaces_after_colon;
                break;
            }

            break;
        case sw_spaces_before_colon:
            if (ch == ' ') {
                break;
            } else if (ch == ':') {
                state = sw_spaces_after_colon;
                break;
            } else {
                return LAN_HTTP_PARSE_INVALID_HEADER;
            }
        case sw_spaces_after_colon:
            if (ch == ' ') {
                break;
            }

            state = sw_value;//解析值
            r->cur_header_value_start = p;
            break;
        case sw_value:
            if (ch == CR) {//\r
                r->cur_header_value_end = p;
                state = sw_cr;
            }

            if (ch == LF) {//\n
                r->cur_header_value_end = p;
                state = sw_crlf;
            }
            
            break;
        case sw_cr:
            if (ch == LF) {
                state = sw_crlf;
                // save the current http header
                hd = (lan_http_header_t *)malloc(sizeof(lan_http_header_t));
                hd->key_start   = r->cur_header_key_start;
                hd->key_end     = r->cur_header_key_end;
                hd->value_start = r->cur_header_value_start;
                hd->value_end   = r->cur_header_value_end;

                list_add(&(hd->list), &(r->list));//加入链表

                break;
            } else {
                return LAN_HTTP_PARSE_INVALID_HEADER;
            }

        case sw_crlf:
            if (ch == CR) {
                state = sw_crlfcr;
            } else {
                r->cur_header_key_start = p;
                state = sw_key;
            }
            break;

        case sw_crlfcr://\r\n\r\n代表header解析完毕
            switch (ch) {
            case LF:
                goto done;
            default:
                return LAN_HTTP_PARSE_INVALID_HEADER;
            }
            break;
        }   
    }

    r->pos = pi;
    r->state = state;

    return LAN_AGAIN;

done:
    r->pos = pi + 1;

    r->state = sw_start;

    return LAN_OK;
}
