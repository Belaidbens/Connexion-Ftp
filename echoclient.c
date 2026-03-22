#include "bibftp.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>

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

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }

    host = argv[1];

    clientfd = Open_clientfd(host, PORT_FTP);

    printf("Connected to %s.\n", host);

    //demander nom fichier au client
    char command[MAXLINE];
    printf("ftp> ");
    Fgets(command, MAXLINE, stdin);
    command[strcspn(command, "\n")] = '\0';

    //"get filename"
    if (sscanf(command, "get %s", filename) != 1) {
        printf("Commande invalide\n");
        Close(clientfd);
        return 0;
}


    //construction  requête
    req.type = command[0] == 'g' ? GET : -1; // on a get seulement pour linstatn
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
    struct timeval start, end;
    gettimeofday(&start, NULL);
    size_t count = 0;
    while ((n = read(clientfd, buf, MAXLINE)) > 0) {
        write(fd, buf, n);
        count += n;   
    }

    // Afficher des informations sur le transfert

    printf("telechargment du fichier demandé (%zu bytes demandés)\n", (size_t)res.size);
    printf("Transfer successfully complete ");
    gettimeofday(&end, NULL);
    double temps = (end.tv_sec - start.tv_sec) +
                 (end.tv_usec - start.tv_usec) / 1000000.0;

    double debit = (count / 1024.0) / temps; // en Ko/s
    printf("(%zu bytes received in %.6f seconds (%.2f Kbytes/s)\n", count, temps, debit);

    if(count != (size_t)res.size) {
        fprintf(stderr, "Erreur: taille du fichier reçu ne correspond pas à la taille annoncée par le serveur\n");
        return -1;
    }
    printf("fichier reçu et sauvegardé dans %s\n", filepath);
    
    close(fd);


    Close(clientfd);
    return 0;
}