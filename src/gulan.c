#include <stdint.h>
#include <sys/mman.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "util.h"
#include "timer.h"
#include "http.h"
#include "epoll.h"
#include "threadpool.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define CONF "gulan.conf"
#define PROGRAM_VERSION "0.1"
char * indexaddr;
extern struct epoll_event *events;
static void sighandler() 
{
    exit(0);
}

static const struct option long_options[]=
{
    {"help",no_argument,NULL,'?'},
    {"version",no_argument,NULL,'V'},
    {"conf",required_argument,NULL,'c'},
    {NULL,0,NULL,0}
};

static void usage() {
    fprintf(stderr,
            "gulan [option]... \n"
            "  -c|--conf <config file>  Specify config file. Default ./gulan.conf.\n"
            "  -?|-h|--help             This information.\n"
            "  -V|--version             Display program version.\n"
           );
}

int main(int argc, char* argv[]) {
    int rc;
    int opt = 0;
    int options_index = 0;
    char *conf_file = CONF;

    /*
     * parse argv 
     * more detail visit: http://www.gnu.org/software/libc/manual/html_node/Getopt.html
     */

    if (argc == 1) {
        usage();
        return 0;
    }

    while ((opt=getopt_long(argc, argv,"Vc:?h",long_options,&options_index)) != EOF) {
        switch (opt) {
            case  0 : break;
            case 'c':
                      conf_file = optarg;
                      break;
            case 'V':
                      printf(PROGRAM_VERSION"\n");
                      return 0;
            case ':':
            case 'h':
            case '?':
                      usage();
                      return 0;
        }
    }

    debug("conffile = %s", conf_file);

    if (optind < argc) {
        log_err("non-option ARGV-elements: ");
        while (optind < argc)
            log_err("%s ", argv[optind++]);
        return 0;
    }

    /*
     * read confile file
     */
    char conf_buf[BUFLEN];
    lan_conf_t cf;
    rc = read_conf(conf_file, &cf, conf_buf, BUFLEN);
    check(rc == LAN_CONF_OK, "read conf err");

    /*
     *   install signal handle for SIGPIPE
     *   when a fd is closed by remote, writing to this fd will cause system send
     *   SIGPIPE to this process, which exit the program
     */
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) {
        log_err("install sigal handler for SIGPIPE failed");
        return 0;
    }

    /* installl USR1 for profile
    */
    signal( SIGUSR1, sighandler );
    /*
     * initialize listening socket
     */
    int listenfd;
    struct sockaddr_in clientaddr;
    // initialize clientaddr and inlen to solve "accept Invalid argument" bug
    socklen_t inlen = 1;
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));  

    listenfd = open_listenfd(cf.port);
    rc = make_socket_non_blocking(listenfd);
    check(rc == 0, "make_socket_non_blocking");

    /*
     * create epoll and add listenfd to ep
     */
    int epfd = lan_epoll_create(0);
    struct epoll_event event;

    lan_http_request_t *request = (lan_http_request_t *)malloc(sizeof(lan_http_request_t));
    lan_init_request_t(request, listenfd, epfd, &cf);

    event.data.ptr = (void *)request;
    event.events = EPOLLIN | EPOLLET;
    lan_epoll_add(epfd, listenfd, &event);

    /*
     * create thread pool
     */

    //    lan_threadpool_t *tp = threadpool_init(cf.thread_num);
    //    check(tp != NULL, "threadpool_init error");


    /*
     * initialize timer
     */
    //    lan_timer_init();

    log_info("gulan started.");

    //    int idlefd =open("/dev/null", O_RDONLY | O_CLOEXEC);
    //    check(idlefd>0," idle fd is ready");
    int n;
    //    int time;

    //cache indexfile
    char indexfile[]="./html/index.html";
    int indexfd = open(indexfile, O_RDONLY, 0);
    check(indexfd > 2, "open error");
    struct stat sb;
    if (fstat(indexfd, &sb) == -1)           /* To obtain file size */
        log_err("fstat");
    indexaddr = mmap(NULL,(size_t)sb.st_size, PROT_READ, MAP_PRIVATE, indexfd, 0);
    check(indexaddr != (void *) -1, "mmap error");
    close(indexfd);

    /* epoll_wait loop */
    while (1) {
        //        time = lan_find_timer();
        //        debug("wait time = %d", time);
        n = lan_epoll_wait(epfd, events, MAXEVENTS,10*1000);
        //        lan_handle_expire_timers();

        for (int i = 0; i < n; i++) {
            lan_http_request_t *r = (lan_http_request_t *)events[i].data.ptr;
            int fd = r->fd;

            if (listenfd == fd) {
                /* we hava one or more incoming connections */

                int infd;
                while(1) {
                    infd = accept(listenfd, (struct sockaddr *)&clientaddr, &inlen);
                    if (infd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /* we have processed all incoming connections */
                            break;
                        }
                        //                        else if(errno ==EMFILE)
                        //                        {
                        //                            log_err("EMFILE problem");
                        //                            close(idlefd);
                        //
                        //                            //接受 clientfd
                        //                            int  clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &inlen);
                        //                            //关闭 clientfd，防止一直触发 listenfd 上的可读事件
                        //                            close(clientfd);
                        //
                        //                            //重新占领 "坑"位
                        //                            idlefd =open("/dev/null", O_RDONLY | O_CLOEXEC);
                        //                        }
                        else {
                            log_err("accept");
                            break;
                        }
                    }

                    rc = make_socket_non_blocking(infd);
                    check(rc == 0, "make_socket_non_blocking");
                    log_info("new connection fd %d", infd);

                    lan_http_request_t *request = (lan_http_request_t *)malloc(sizeof(lan_http_request_t));
                    if (request == NULL) {
                        log_err("malloc(sizeof(lan_http_request_t))");
                        break;
                    }

                    lan_init_request_t(request, infd, epfd, &cf);
                    event.data.ptr = (void *)request;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

                    lan_epoll_add(epfd, infd, &event);
                    //                    lan_add_timer(request, TIMEOUT_DEFAULT, lan_http_close_conn);
                }   // end of while of accept

            } else {
                if ((events[i].events & EPOLLERR) ||
                        (events[i].events & EPOLLHUP) ||
                        (!(events[i].events & EPOLLIN))) {
                    log_err("epoll error fd: %d", r->fd);
                    close(fd);
                    continue;
                }

                log_info("new data from fd %d", fd);
                //              rc = threadpool_add(tp, do_request, events[i].data.ptr);
                //                check(rc == 0, "threadpool_add");

                do_request(events[i].data.ptr);
            }
        }   //end of for
    }   // end of while(1)



    //if (threadpool_destroy(tp, 1) < 0) {
    //    log_err("destroy threadpool failed");
    // }


    return 0;
}
