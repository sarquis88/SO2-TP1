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
#include <limits.h>
#include <errno.h>

#define CANTIDAD_USUARIOS 4
#define USUARIO_CLAVE_SIZE 15
#define USUARIO_NOMBRE_SIZE 15
#define USUARIO_BLOQUEADO_SIZE 2
#define USUARIO_ULTIMA_CONEXION_SIZE 64
#define LIMITE_INTENTOS 3
#define COMANDO_SIZE 32
#define LINE_SIZE 64

#define CANTIDAD_ARCHIVOS 4
#define ARCHIVO_NOMBRE_SIZE 128
#define ARCHIVO_FORMATO_SIZE 6
#define PATH_USB "/dev/sdb"

#define CANTIDAD_PARTICIONES 4
#define PARTICION_INFORMACION_SIZE 32
#define PARTICION_BOOTABLE_SIZE 3
#define PARTICION_TIPO_SIZE 3

#define BUFFER_SIZE 512
#define PROJ_ID 66

#define QUEUE_FILE_NAME "src/launch.c"
#define QUEUE_MESAGE_SIZE 480

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

#define SEC_TIME 0
#define NSEC_TIME 200

int32_t get_cola();
int32_t enviar_a_cola(long, char*);
char* recibir_de_cola(long, int32_t);
char* get_md5(char*, ssize_t);
void set_mbr_informacion(int32_t);
void set_mbr_tipo(int32_t);
void set_mbr_bootable(int32_t);
void set_mbr_size(int32_t);
void set_mbr_inicio(int32_t);
void set_mbr_final(int32_t);
void print_particion(int32_t);
void start_mbr_analisis();
void dormir();

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

typedef struct {
  int32_t index;
  char tipo[PARTICION_TIPO_SIZE];
  char informacion[PARTICION_INFORMACION_SIZE];
  char booteable[PARTICION_BOOTABLE_SIZE];
  int32_t inicio;
  int32_t final;
  int32_t size;
} Particion;
