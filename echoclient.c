#include "bibftp.h"
#include <stddef.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int clientfd;
    char *host;
    request_t req;
    response_t res;
    char filename[MAXLINE];
    char buf[MAXLINE];
    int fd;
    ssize_t n;
    int status;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }

    host = argv[1];

    clientfd = Open_clientfd(host, PORT_FTP);

    printf("Connected to server\n");

    //demander nom fichier au client
    printf("Nom du fichier à récupérer: ");
    Fgets(filename, MAXLINE, stdin);
    filename[strcspn(filename, "\n")] = '\0';

    //construction  requête
    req.type = GET;
    strcpy(req.fichier, filename);

    //envoi requête
    Rio_writen(clientfd, &req, sizeof(request_t));

    //lire status pour savoir le serveur a trouvé le fichier ou pas
    if (Rio_readn(clientfd, &res, sizeof(response_t)) <= 0) {
        printf("Erreur serveur\n");
        Close(clientfd);
        exit(0);
    }

    if (res.status != RES_VALIDE) {
        printf("Erreur serveur: %s\n", res.message);
        Close(clientfd);
        exit(0);
    }

    printf("telechargment du fichier demandé (%zu bytes)\n", (size_t)res.size);

    //creation fichier local
    char filepath[MAXLINE];
    int retour=snprintf(filepath, sizeof(filepath), "%s%s", CLIENT_DIR, filename);
    if (retour >= (int)sizeof(filepath)) {
        fprintf(stderr, "tro long path\n");
        Close(clientfd);
        return -1;
        }

    fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        Close(clientfd);
        return -1;
    }

    //recevoir fichier
    while ((n = read(clientfd, buf, MAXLINE)) > 0) {
        write(fd, buf, n);
    }

    close(fd);
    printf("fichier reçu et sauvegardé dans %s\n", filepath);

    Close(clientfd);
    return 0;
}