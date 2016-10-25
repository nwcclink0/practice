#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "threadpool.h"

#define MAXEVENTS 64
#define TIMEOUT 500
#define BUF_SIZE 1024

#define NUM_THREAD 4

#define EPOLL_THPOOL_ERR_SUCCESS 0
#define EPOLL_THPOOL_ERR_ERROR -1
typedef struct connection_info{
    int epoll_fd;
    struct epoll_event *event;
    unsigned int n_size;
    char content[1024];
} connection_info;


static int create_and_bind (char *port)
{
    if(port == NULL) return EPOLL_THPOOL_ERR_ERROR;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int rc, server_socket_fd;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(NULL, port, &hints, &result);
    if (rc != EPOLL_THPOOL_ERR_SUCCESS) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return EPOLL_THPOOL_ERR_ERROR;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        server_socket_fd= socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_socket_fd == EPOLL_THPOOL_ERR_ERROR)
            continue;

        rc = bind(server_socket_fd, rp->ai_addr, rp->ai_addrlen);
        if (rc == EPOLL_THPOOL_ERR_SUCCESS) {
            /* We managed to bind successfully! */
            break;
        }

        close(server_socket_fd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        return EPOLL_THPOOL_ERR_ERROR;
    }

    freeaddrinfo(result);

    return server_socket_fd;
}

static int make_socket_non_blocking (int sfd)
{
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == EPOLL_THPOOL_ERR_ERROR) {
        perror ("fcntl");
        return EPOLL_THPOOL_ERR_ERROR;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == EPOLL_THPOOL_ERR_ERROR) {
        perror ("fcntl");
        return EPOLL_THPOOL_ERR_ERROR;
    }

    return 0;
}

void epoll_error_msg()
{
    switch(errno) {
    case EBADF:
        printf("epoll error is EBADF\n");
        break;
    case EEXIST:
        printf("epoll error is EEXIST\n");
        break;
    case EINVAL:
        printf("epoll error is EINVAL\n");
        break;
    case ENOENT:
        printf("epoll error is ENOENT\n");
        break;
    case ENOMEM:
        printf("epoll error is ENOMEM\n");
        break;
    case ENOSPC:
        printf("epoll error is ENOSPC\n");
        break;
    default:
        break;
    }
}

int socket_send(int sockfd)
{
    ssize_t tmp;
    char buf[1024] = {0};
    const char *p = buf;
    size_t total = 1024;

    sprintf(buf, "this is server echo\n");
    while(1) {
        tmp = send(sockfd, p, total, MSG_NOSIGNAL);
        if(tmp < 0) {
            if(errno == EINTR) continue;

            if(errno == EAGAIN) {
                usleep(1000);
                continue;
            }
            return EPOLL_THPOOL_ERR_ERROR;
        }

        if((size_t)tmp == total) {
            return 1024;
        }

        total -= tmp;
        p += tmp;
    }

    return tmp;
}

void epoll_send(void *args)
{
    connection_info *info = (connection_info*)args;
    int client_sock = info->event->data.fd;
    int rc = socket_send(client_sock);
    if(rc < 0) {
        printf("write error\n");
        return;
    }
    /* printf("write success\n"); */
    struct epoll_event *ev = (struct epoll_event*)malloc(sizeof(struct epoll_event));
    /* struct epoll_event ev; */
    ev->data.fd = client_sock;
    ev->events = EPOLLIN | EPOLLET;
    epoll_ctl(info->epoll_fd, EPOLL_CTL_MOD, client_sock, ev);

    free(ev);
    ev = NULL;
    free(info->event);
    info->event = NULL;
    free(info);
    info = NULL;
    return;
}

void epoll_recv(void *args)
{
    connection_info *info = (connection_info*)args;
    int client_sock = info->event->data.fd;
    if((client_sock = info->event->data.fd) < 0) {
        return;
    }

    ssize_t count = 0;
    ssize_t readed = 0;

    int rc;
    while((count = read(client_sock, info->content, sizeof(info->content))) > 0) {
        readed += count;
        rc = write(1, info->content, count);
    }

    if((count == -1) && (errno != EAGAIN)) {
        epoll_error_msg();
        close(client_sock);
        info->event->data.fd = -1;
    }

    if(count == 0) {
        close(client_sock);
        return;
    }


    struct epoll_event *ev = (struct epoll_event*)malloc(sizeof(struct epoll_event));
    /* struct epoll_event ev */;
    ev->data.fd = client_sock;
    ev->events = EPOLLOUT | EPOLLET;
    epoll_ctl(info->epoll_fd, EPOLL_CTL_MOD, client_sock, ev);
    free(ev);
    ev = NULL;
    free(info->event);
    info->event = NULL;
    free(info);
    info = NULL;
    return;
}

int main(int argc, char **argv)
{
    int epoll_fd = 0;
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    struct epoll_event *events = NULL;

    if(argc != 2){
        fprintf(stderr, "%s [port]", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket_fd = create_and_bind(argv[1]);
    int rc = make_socket_non_blocking(server_socket_fd);
    if(rc == EPOLL_THPOOL_ERR_ERROR) abort();

    rc = listen(server_socket_fd, MAXEVENTS);
    if(rc == EPOLL_THPOOL_ERR_ERROR){
        fprintf(stderr, "listen error\n");
        abort();
    }

    epoll_fd = epoll_create(256);
    if(epoll_fd == EPOLL_THPOOL_ERR_ERROR){
        fprintf(stderr, "epoll create error\n");
        abort();
    }

    event.data.fd = server_socket_fd;
    event.events = EPOLLIN | EPOLLET;
    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &event);
    if(rc == EPOLL_THPOOL_ERR_ERROR){
        fprintf(stderr, "epoll_ctl failed\n");
        abort();
    }

    events = calloc(MAXEVENTS, sizeof(event));

    printf("the server pid is: %d\n", getpid());

    //create thread poll
    threadpool tpl = thread_pool_init(NUM_THREAD);
    connection_info *info = NULL;
    while(1){
        int num_fds = 0;
        int i;
        num_fds = epoll_wait(epoll_fd, events, MAXEVENTS, TIMEOUT);
        for(i = 0; i < num_fds; i++){
            if(server_socket_fd == events[i].data.fd){
                while(1){
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int in_fd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof(in_addr);
                    in_fd = accept(server_socket_fd, &in_addr, &in_len);
                    if(in_fd == EPOLL_THPOOL_ERR_ERROR){
                        if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                            break;
                        }else{
                            break;
                        }
                    }

                    rc = getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
                    if(rc == EPOLL_THPOOL_ERR_SUCCESS){
                        printf("Accepted connection on descriptor %d "
                               "(host=%s, port=%s)\n", in_fd, hbuf, sbuf);
                    }

                    rc = make_socket_non_blocking(in_fd);
                    if(rc == EPOLL_THPOOL_ERR_ERROR) abort();

                    event.data.fd = in_fd;
                    event.events = EPOLLIN | EPOLLET;
                    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, in_fd, &event);
                    if(rc == EPOLL_THPOOL_ERR_ERROR){
                        fprintf(stderr, "epoll ctl add failed\n");
                        abort();
                    }
                }
                continue;
            }else if(events[i].events & EPOLLIN){
                info = (connection_info*)malloc(sizeof(connection_info));
                info->epoll_fd = epoll_fd;
                info->event = (struct epoll_event*)malloc(sizeof(struct epoll_event));
                info->event->events = events[i].events;
                info->event->data.fd = events[i].data.fd;
                thread_pool_add_task(tpl, (void*)epoll_recv, (void *)info);
            }else if(events[i].events & EPOLLOUT){
                info = (connection_info*)malloc(sizeof(connection_info));
                info->event = (struct epoll_event*)malloc(sizeof(struct epoll_event));
                info->event->events = events[i].events;
                info->event->data.fd = events[i].data.fd;
                thread_pool_add_task(tpl, (void*)epoll_send, (void *)info);
            }
        }
    }


    return 0;
}
