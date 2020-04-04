#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>

#define TAM 256

#define USUARIO_CLAVE_SIZE 15
#define USUARIO_NOMBRE_SIZE 15
#define USUARIO_BLOQUEADO_SIZE 2
#define LINE_SIZE 33

#define LIMITE_INTENTOS 3

#define CANT_USUARIOS 3

ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
in_addr_t inet_addr(const char*);
pid_t getpid(void);
pid_t fork(void);
uint32_t close(uint32_t fd);
