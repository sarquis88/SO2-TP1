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
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <openssl/md5.h>
#include <unistd.h>

#define BUFFER_SIZE 256

#define CANT_USUARIOS 4
#define USUARIO_CLAVE_SIZE 15
#define USUARIO_NOMBRE_SIZE 15
#define USUARIO_BLOQUEADO_SIZE 2
#define USUARIO_ULTIMA_CONEXION_SIZE 19

#define CANT_ARCHIVOS 4
#define ARCHIVO_NOMBRE_SIZE 128
#define ARCHIVO_FORMATO_SIZE 6

#define LINE_SIZE 64
#define LIMITE_INTENTOS 3
#define PROJ_ID 66

#define PRIMARY_QUEUE_FILE_NAME "resources/queues/primary_queue"
#define FILE_QUEUE_FILE_NAME "resources/queues/file_queue"
#define AUTH_QUEUE_FILE_NAME "resources/queues/auth_queue"
#define QUEUE_MESAGE_SIZE 200

// ids de mensajes
#define LOGIN_REQUEST 1
#define LOGIN_RESPONSE 2
#define BLOQUEAR_USUARIO 3
#define NOMBRES_REQUEST 4
#define NOMBRES_RESPONSE 5
#define CAMBIAR_CLAVE_REQUEST 6
#define CAMBIAR_CLAVE_RESPONSE 7
#define ARCHIVOS_REQUEST 8
#define ARCHIVOS_RESPONSE 9
#define DESCARGA_REQUEST 10
#define DESCARGA_RESPONSE 11

int32_t get_cola(char);
int32_t enviar_a_cola(long, char*, char);
struct msgbuf recibir_de_cola(long, char);
char* get_md5(char*);

ssize_t recv(int sockfd, void*, size_t, int);
ssize_t send(int sockfd, const void*, size_t, int);
in_addr_t inet_addr(const char*);
pid_t getpid(void);
pid_t fork(void);
int32_t close(int32_t);
int32_t unlink(const char*);


struct msgbuf {
   long mtype;
   char mtext[QUEUE_MESAGE_SIZE];
};
