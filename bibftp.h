#ifndef __BIBFTP_H__
#define __BIBFTP_H__

#include "csapp.h"
#define NB_PROC 10
#define PORT_FTP 2121
#define MAX_NAME_LEN 256
#define SERVER_DIR "./server_files/"
#define CLIENT_DIR "./client_files/"

typedef enum {
    GET = 0,
    PUT,
    LS
} typereq_t;

typedef struct request_t {
    typereq_t type;
    char fichier[MAXLINE];
} request_t;






























#endif /* __BIBFTP_H__ */