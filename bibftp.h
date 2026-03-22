#ifndef __BIBFTP_H__
#define __BIBFTP_H__

#include "csapp.h"
#define NB_PROC 10
#define PORT_FTP 2121
#define MAX_NAME_LEN 256
#define SERVER_DIR "./server_files/"
#define CLIENT_DIR "./client_files/"

#define RES_ERR_NOT_FOUND -1
#define RES_ERR_MAUVAISE_REQUEST -2
#define RES_ERR_INTERNE -3
#define RES_VALIDE 0

typedef enum {
    GET = 0,
    PUT,
    LS
} typereq_t;

typedef struct request_t {
    typereq_t type;
    char fichier[MAXLINE];
} request_t;

typedef struct response_t {
    int status;   // 0 si correct, -1 si  erreur
    int size;     
    char message[MAXLINE]; // message d'erreur ou autre info    
} response_t;




























#endif /* __BIBFTP_H__ */