#include "utilities.h"

#define DATA_FILE_NAME "resources/users_credentials"

uint32_t conectar();
char* get_nombres();
char* get_claves();
char* get_bloqueados();
uint32_t bloquear(uint32_t);


typedef struct {
  char nombre[USUARIO_NOMBRE_SIZE];
  char clave[USUARIO_CLAVE_SIZE];
  char bloqueado[USUARIO_BLOQUEADO_SIZE];
} Usuario;
