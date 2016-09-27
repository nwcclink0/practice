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

#define MAXEVENTS 64

static int make_socket_non_blocking (int sfd)
{
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1) {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

static int create_and_bind (char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo (NULL, port, &hints, &result);
    if (s != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        }

        close (sfd);
    }

    if (rp == NULL) {
        fprintf (stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo (result);

    return sfd;
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
            return -1;
        }

        if((size_t)tmp == total) {
            return 1024;
        }

        total -= tmp;
        p += tmp;
    }

    return tmp;
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

int main (int argc, char *argv[])
{
    int sfd, s, client_sock;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = create_and_bind (argv[1]);
    if (sfd == -1)
        abort();

    s = make_socket_non_blocking (sfd);
    if (s == -1)
        abort();

    s = listen (sfd, SOMAXCONN);
    if (s == -1) {
        printf("listen");
        abort();
    }

    efd = epoll_create1 (0);
    if (efd == -1) {
        printf("epoll_create");
        abort();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        printf("epoll_ctl");
        abort();
    }

    /* Buffer where events are returned */
    events = calloc (MAXEVENTS, sizeof(event));

    printf("the server process is %d\n", getpid());
    /* The event loop */
    while (1) {
        int n, i;

        n = epoll_wait (efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++) {

            if (sfd == events[i].data.fd) {
                /* else if (sfd == events[i].data.fd) { */
                /* We have a notification on the listening socket, which
                   means one or more incoming connections. */
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof(in_addr);
                    infd = accept (sfd, &in_addr, &in_len);
                    if (infd == -1) {
                        if ((errno == EAGAIN) ||
                                (errno == EWOULDBLOCK)) {
                            /* We have processed all incoming
                               connections. */
                            break;
                        } else {
                            printf("accept");
                            break;
                        }
                    }

                    s = getnameinfo (&in_addr, in_len,
                                     hbuf, sizeof(hbuf),
                                     sbuf, sizeof(sbuf),
                                     NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        printf("Accepted connection on descriptor %d "
                               "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }

                    /* Make the incoming socket non-blocking and add it to the
                       list of fds to monitor. */
                    s = make_socket_non_blocking (infd);
                    if (s == -1)
                        abort();

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1) {
                        printf("epoll_ctl");
                        abort ();
                    }
                }
                continue;
            } else if(events[i].events & EPOLLIN) {
                if((client_sock = events[i].data.fd) < 0) {
                    continue;
                }

                ssize_t count = 0;
                ssize_t readed = 0;
                char buf[1024] = {0};

                while((count = read(events[i].data.fd, buf, 1024)) > 0) {
                    readed += count;
                }

                if((count == -1) && (errno != EAGAIN)) {
                    epoll_error_msg();
                    close(client_sock);
                    events[i].data.fd = -1;
                }

                if(count == 0) {
                    close(client_sock);
                    continue;
                }

                s = write (1, buf, count);

                event.data.fd = client_sock;
                event.events = EPOLLOUT | EPOLLET;
                epoll_ctl(efd, EPOLL_CTL_MOD, client_sock, &event);
            } else if(events[i].events & EPOLLOUT) {
                client_sock = events[i].data.fd;
                int rc = socket_send(client_sock);
                if(rc < 0) {
                    printf("write error\n");
                    continue;
                }
                printf("write success\n");
                event.data.fd = client_sock;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(efd, EPOLL_CTL_MOD, client_sock, &event);
            }
        }
    }

    free (events);

    close (sfd);

    return EXIT_SUCCESS;
}
