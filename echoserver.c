#include "bibftp.h"


pid_t pids[NB_PROC];



void sigint_handler(int sig) {
    printf("Desativation du serveur...\n");
    for(int i = 0; i < NB_PROC; i++) {
        kill(pids[i], SIGINT); 
    }
    exit(0);
}



void file_server(int connfd)
{
    request_t req;
    response_t res;
    char buf[MAXLINE];
    int fd;
    ssize_t n;

    // Lire la requête STRUCTURE
    if (Rio_readn(connfd, &req, sizeof(request_t)) <= 0) {
        Close(connfd);
        return;
    }

    printf("Client demande le fichier : %s\n", req.fichier);



    //verif type de requete 
    if (req.type != GET) {
        res.status = RES_ERR_MAUVAISE_REQUEST;
        res.size = 0;
        strncpy(res.message, "Requete invalide", sizeof(res.message));
        Rio_writen(connfd, &res, sizeof(res));
        Close(connfd);
        return;
    }

    // ouvrir fichier
    char filepath[MAXLINE];
    int retour=snprintf(filepath, sizeof(filepath), "%s%s", SERVER_DIR, req.fichier);
    if (retour >= (int)sizeof(filepath)) {
        res.status = RES_ERR_INTERNE;
        res.size = 0;
        strncpy(res.message, "Nom de fichier trop long", sizeof(res.message));
        Rio_writen(connfd, &res, sizeof(res));
        fprintf(stderr, "tro long path\n");
        Close(connfd);
        return;
    }
    fd = open(filepath, O_RDONLY);// Serverdir/fichier

        if (fd < 0) {
            res.status = RES_ERR_NOT_FOUND;
            res.size = 0;
            snprintf(res.message, sizeof(res.message), "Fichier %.100s introuvable", req.fichier);
            Rio_writen(connfd, &res, sizeof(res));
            Close(connfd);
            return;
        }

        // envoyer acquitement de la requete pour dire oui au client 
        off_t filesize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        res.status = RES_VALIDE;
        res.size = filesize;
        snprintf(res.message, sizeof(res.message), "Fichier %.100s trouvé et la requete accéptée", req.fichier);
        Rio_writen(connfd, &res, sizeof(res));

        //envoyer contenu
        while ((n = read(fd, buf, MAXLINE)) > 0) {
            Rio_writen(connfd, buf, n);
        }
        

        close(fd);
    

    //fin connexion et fin transfert
    Close(connfd);
}





int main()
{
    signal(SIGINT, sigint_handler);
    int listenfd, connfd;
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
            //Close(listenfd); car c'est chaque processus qui va gérer les connexions, pas besoin de le partager
            signal(SIGINT, SIG_DFL); // on retablit comportement par defaut de SIGINT juste mourir
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