#include "vector.h"
#include <errno.h>
#include <netinet/ip.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

struct endpoint {
    struct sockaddr_in origin;
    const char *name;
};

int main(void) {
    // struct endpoint *endpoints;
    // endpoints = malloc(sizeof(struct endpoint*));
    vector v;
    vector_init(&v);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9119);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    // listen(s, 16);
    if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind failed");
        return 0;
    }
    socklen_t *len = malloc(sizeof(socklen_t));
    struct sockaddr_in *peer = malloc(sizeof(struct sockaddr_in));
    uint8_t *data = malloc(UINT16_MAX);
    while (true) {
        *len = sizeof(struct sockaddr_in);
        int r = recvfrom(s, data, UINT16_MAX, MSG_WAITALL, (struct sockaddr *)peer,
                         len);
        printf("%i\n", r);
        if (r > 0) {
            // data = realloc(data, r);
            printf("Media packet recieved.\n");
            if (r < 2) {
                // free(data);
                continue;
            }
            unsigned char str_len = *(data + 1);
            printf("%c\n", data[0]);
            printf("%hhu\n", str_len);
            if (data[0] == 'R') {
                if (str_len > 32) {
                    // free(data);
                    continue;
                }
                if ((str_len + 2) > r) {
                    // free(data);
                    continue;
                }
                // endpoints = realloc(endpoints, sizeof(struct endpoint*) *
                // (endpoint_len + 1));
                struct endpoint *help;
                help = malloc(sizeof(struct endpoint));
                uint8_t *name = malloc(str_len + 1);
                memcpy(name, data + 2, str_len);
                name[str_len] = 0;
                help->name = (char *)name;
                help->origin = *peer;
                vector_add(&v, help);
                continue;
            } else if (data[0] == 'D') {
                printf("Disconnect recieved.\n");
                for (size_t i = 0; i < vector_total(&v); i++) {
                    if (((struct endpoint *)vector_get(&v, i))->origin.sin_addr.s_addr ==
                            peer->sin_addr.s_addr) {
                        vector_delete(&v, i);
                    }
                }
                continue;
            } else if (data[0] == 'S') {
                if (str_len > 32) {
                    // free(data);
                    continue;
                }
                if ((str_len + 2) > r) {
                    // free(data);
                    continue;
                }
                uint8_t *name = malloc(str_len + 1);
                memcpy(name, data + 2, str_len);
                printf("%s\n", name);
                name[str_len] = 0;
                for (size_t i = 0; i < vector_total(&v); i++) {
                    struct endpoint *endp = vector_get(&v, i);
                    printf("%s\n", endp->name);
                    if (strcmp(endp->name, (char *)name) == 0) {
                        uint8_t *newdata;
                        uint8_t *sname = NULL;
                        for (size_t i = 0; i < vector_total(&v); i++) {
                            if (((struct endpoint *)vector_get(&v, i))
                                    ->origin.sin_addr.s_addr == peer->sin_addr.s_addr) {
                                sname = ((struct endpoint *)vector_get(&v, i))->name;
                                break;
                            }
                        }
                        if (!sname) {
                            sname = "anon";
                        }
                        newdata = malloc(2 + strlen(sname) + ((r)-str_len - 2));
                        newdata[0] = ':';
                        newdata[1] = (unsigned char)strlen(sname);
                        memcpy(newdata + 2, sname, strlen(sname));
                        memcpy(newdata + 2 + strlen(sname), data + 2 + str_len,
                               (r - str_len - 2));
                        sendto(s, newdata, 2 + strlen(name) + (r - str_len - 2), 0,
                               (struct sockaddr *)&((struct endpoint *)vector_get(&v, i))
                               ->origin,
                               sizeof(struct sockaddr_in));
                    }
                }
            }

        } else if (r == -1) {
            printf("Error recieved.\n");
            switch (errno) {
            case ECONNRESET:
                printf("ECONNRESET.\n");
                break;
            case EBADF:
                printf("EBADF.\n");
                break;
            case EAGAIN:
                printf("EAGAIN.\n");
                break;
            case EINTR:
                printf("EINTR.\n");
                break;
            case ENOTCONN:
                printf("ENOTCONN.\n");
                break;
            case ENOTSOCK:
                printf("ENOTSOCK.\n");
                break;
            case EOPNOTSUPP:
                printf("EOPNOTSUPP.\n");
                break;
            case ETIMEDOUT:
                printf("ETIMEDOUT.\n");
                break;
            }
            printf("%u\n", errno);
            continue;
        }
    }
}
