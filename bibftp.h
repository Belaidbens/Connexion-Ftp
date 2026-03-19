#ifndef __BIBFTP_H__
#define __BIBFTP_H__

typedef enum {
    GET = 0,
    PUT,
    LS
} typereq_t;

struct request {
    typereq_t type;
    char fichier[256];
};








































#endif /* __BIBFTP_H__ */