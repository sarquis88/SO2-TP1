#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define TAM 256

#define USUARIO_CLAVE_SIZE 15
#define USUARIO_NOMBRE_SIZE 15
#define USUARIO_BLOQUEADO_SIZE 2
#define LINE_SIZE 33
#define LIMITE_INTENTOS 3
#define CANT_USUARIOS 3
#define PROJ_ID 66
#define DATA_FILE_NAME "resources/users_credentials"
#define PRIMARY_QUEUE_FILE_NAME "resources/primary_queue"
#define FILE_QUEUE_FILE_NAME "resources/file_queue"
#define AUTH_QUEUE_FILE_NAME "resources/auth_queue"
#define QUEUE_MESAGE_SIZE 64

// ids de mensajes
#define LOGIN_REQUEST 80
#define LOGIN_RESPONSE 81
#define BLOQUEAR_USUARIO 82
#define NOMBRES_REQUEST 83
#define NOMBRES_RESPONSE 84
#define CAMBIAR_CLAVE_REQUEST 85
#define CAMBIAR_CLAVE_RESPONSE 86

uint32_t get_cola(char);
uint32_t enviar_a_cola(long, char*, char);
struct msgbuf recibir_de_cola(long, char);

ssize_t recv(int sockfd, void*, size_t, int);
ssize_t send(int sockfd, const void*, size_t, int);
in_addr_t inet_addr(const char*);
pid_t getpid(void);
pid_t fork(void);
uint32_t close(uint32_t);
int unlink(const char*);

struct msgbuf {
   long mtype;
   char mtext[QUEUE_MESAGE_SIZE];
};
