#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAXEVENTS 64
#define EPOLL_RUN_TIMEOUT 1000

#define SUCCESS 0
#define ERROR 1
#define ERROR_SERVER_DISCONNECT 2


static int set_nonblocking_socket(int socket_fd);
static int socket_create_bind(char *address, int port)
{
    int s, socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sa = {0};
    struct sockaddr_in *psa = NULL;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    psa = &sa;

    printf("address is :%s\n", address);
    int rc = inet_pton(AF_INET, address, &psa->sin_addr.s_addr);
    if(rc == 0) {
        printf("inet_pton failed, invalid address\n");
        close(socket_fd);
        return ERROR;
    } else if(rc < 0) {
        printf("connect failed\n");
        close(socket_fd);
        return ERROR;
    }

    if(connect(socket_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        printf("connect failed");
        return ERROR;
    }
    set_nonblocking_socket(socket_fd);
    printf("success and the socket fd is: %d\n", socket_fd);
    return socket_fd;
}

static int set_nonblocking_socket(int socket_fd)
{
    int flags;
    int s;
    flags = fcntl (socket_fd, F_GETFL, 0);
    if(flags == -1) {
        printf("fctl %s %d\n", __func__, __LINE__);
        return ERROR;
    }

    flags |= O_NONBLOCK;
    s = fcntl(socket_fd, F_SETFL, flags);
    if( s == -1) {
        printf("fctl %s %d\n", __func__, __LINE__);
    }
    return SUCCESS;
}

int handle_write_event(int fd, void *ptr)
{
    char buf[1024] = {0};

    snprintf(buf, 1024, "this is epoll-client: %d, ptr: %d\n",
             getpid(), (int )ptr);
    if(write(fd, buf, 1024) < 0) {
        printf("write error, %s %d\n", __func__, __LINE__);
    }
    return 0;
}

int handle_read_event(int fd, void *ptr)
{
    ssize_t count;
    char buf[1024] = {0};

    count = read(fd, buf, 1024);
    if(((count == -1) && (errno != EAGAIN)) ||
       (count == 0)){
        close(fd);
        printf("close, ptr: %d\n", (int)ptr);
        return ERROR_SERVER_DISCONNECT;
    }

    printf("ptr: %d\n", (int)ptr);
    int sock = write(1, buf, count);
    if(sock == -1) {
        printf("write erro: %s %d\n", __func__, __LINE__);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf (stderr, "Usage: %s [host] [port start] [port end]\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    int efd;
    efd = epoll_create(MAXEVENTS);
    static struct epoll_event ev;
    static struct epoll_event *events;

    int client_sock;
    for(int n = 2; n < argc; n++) {
        int port = atoi(argv[n]);
        client_sock = socket_create_bind(argv[1], port);
        if(client_sock == ERROR) return 0;

        ev.data.fd = client_sock;
        /* ev.data.ptr = (void *)port; */
        ev.events = EPOLLOUT | EPOLLIN |EPOLLET;
        events = calloc(MAXEVENTS, sizeof(ev));
        int res = epoll_ctl(efd, EPOLL_CTL_ADD, client_sock, &ev);

    }

    while(1) {

        int nfds = epoll_wait(efd, events, MAXEVENTS, EPOLL_RUN_TIMEOUT);
        if(nfds < 0)
            printf("Error in epoll wait\n");

        for(int i = 0; i < nfds; i++) {
            if(events[i].events & EPOLLIN) {
                /* printf("get EPOLLIN event\n"); */
                handle_read_event(events[i].data.fd, events[i].data.ptr);
                /* ev.data.fd = events[i].data.fd; */
                /* ev.events = EPOLLET | EPOLLOUT; */
                /* epoll_ctl(efd, EPOLL_CTL_MOD, events[i].data.fd, &ev); */
            } else if(events[i].events & EPOLLOUT) {
                /* printf("get EPOLLOUT event\n"); */
                handle_write_event(events[i].data.fd, events[i].data.ptr);
                /* ev.data.fd = events[i].data.fd; */
                /* ev.events = EPOLLET | EPOLLIN; */
                /* epoll_ctl(efd, EPOLL_CTL_MOD, events[i].data.fd, &ev); */
            }
        }
    }

    close(client_sock);
    close(efd);
}
