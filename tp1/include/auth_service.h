#include "utilities.h"

#define SOCKET_PATH "/tmp/auth_service"

int32_t conectar();
int32_t bloquear_usuario(char*);
int32_t cambiar_clave(char*);
int32_t set_ultima_conexion(char*);
int32_t login(char*);
int32_t refresh_datos();
char* get_nombres();
char* get_claves();
char* get_bloqueados();
char* get_ultimas_conexiones();
void configurar_nombres();
void configurar_claves();
void configurar_socket();
void enviar_a_cola_local(long, char*, char);

typedef struct {
  char nombre[USUARIO_NOMBRE_SIZE];
  char clave[USUARIO_CLAVE_SIZE];
  char bloqueado[USUARIO_BLOQUEADO_SIZE];
  char ultima_conexion[USUARIO_ULTIMA_CONEXION_SIZE];
} Usuario;
