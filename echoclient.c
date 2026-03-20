#include "bibftp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host;
    request_t req;
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
    if (Rio_readn(clientfd, &status, sizeof(int)) <= 0) {
        printf("Erreur serveur\n");
        Close(clientfd);
        exit(0);
    }

    if (status == -1) {
        printf("Fichier introuvable sur le serveur\n");
        Close(clientfd);
        exit(0);
    }

    printf("telechargment du fichier demandé\n");

    //creation fichier local
    char filepath[MAXLINE];
    int retour=snprintf(filepath, sizeof(filepath), "%s%s", CLIENT_DIR, filename);
    if (retour >= (int)sizeof(filepath)) {
        fprintf(stderr, "tro long path\n");
        Close(clientfd);
        return -1;
        }

    fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    //recevoir fichier
    while ((n = read(clientfd, buf, MAXLINE)) > 0) {
        write(fd, buf, n);
    }

    close(fd);

    printf("fichier reçu et sauvegardé dans %s\n", filepath);

    Close(clientfd);
    return 0;
}