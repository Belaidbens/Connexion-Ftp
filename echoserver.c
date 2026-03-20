#include "bibftp.h"


pid_t pids[NB_PROC];



void sigint_handler(int sig) {
    printf("Shutting down server...\n");
    signal(SIGINT, SIG_DFL); 
    for(int i = 0; i < NB_PROC; i++) {
        kill(pids[i], SIGINT); 
    }
    exit(0);
}



void file_server(int connfd)
{
    request_t req;
    char buf[MAXLINE];
    int fd;
    ssize_t n;
    int status;

    // Lire la requête STRUCTURE
    if (Rio_readn(connfd, &req, sizeof(request_t)) <= 0) {
        Close(connfd);
        return;
    }

    printf("Client demande le fichier : %s\n", req.fichier);

    // Verifier type 
    if (req.type == GET) {

        // ouvrir fichier
        char filepath[MAXLINE];
        snprintf(filepath, MAXLINE, "%s%s", SERVER_DIR, req.fichier);
        fd = open(filepath, O_RDONLY);// Serverdir/fichier

        if (fd < 0) {
            status = -1;
            Rio_writen(connfd, &status, sizeof(int));
            Close(connfd);
            return;
        }

        // envoyer acquitement de la requete pour dire oui au client 
        status = 0;
        Rio_writen(connfd, &status, sizeof(int));

        // 4. envoyer contenu
        while ((n = read(fd, buf, MAXLINE)) > 0) {
            Rio_writen(connfd, buf, n);
        }

        close(fd);
    }

    //fin connexion et fin transfert
    Close(connfd);
}





int main(int argc, char **argv)
{
    signal(SIGINT, sigint_handler);
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int i;

    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];



    listenfd = Open_listenfd(PORT_FTP);
    /* création du pool */
    for (i = 0; i < NB_PROC; i++) {
        pids[i] = fork();
        if (pids[i] == 0) { // fils
            Close(listenfd);
            signal(SIGINT, sigint_handler); // on retablit comportement par defaut de SIGINT
            while (1) {
                clientlen = sizeof(clientaddr);

                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

                /* info client */
                Getnameinfo((SA *) &clientaddr, clientlen,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);

                Inet_ntop(AF_INET, &clientaddr.sin_addr,
                          client_ip_string, INET_ADDRSTRLEN);

                printf("[PID %d] connected to %s (%s)\n",
                       getpid(), client_hostname, client_ip_string);

                struct sockaddr_in localaddr, peeraddr;
                socklen_t len = sizeof(struct sockaddr_in);

                getsockname(connfd, (SA *)&localaddr, &len);
                getpeername(connfd, (SA *)&peeraddr, &len);

                printf("[PID %d] local port: %d, client port: %d\n",
                       getpid(),
                       ntohs(localaddr.sin_port),
                       ntohs(peeraddr.sin_port));

                file_server(connfd);
                //Close(connfd);
            }
        }
    }

    // père attend
    while (1) {
        pause();
    }

    return 0;
}