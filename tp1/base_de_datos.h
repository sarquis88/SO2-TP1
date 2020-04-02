#include "utilidades.h"

uint32_t conectar();
char* get_nombres();
char* get_claves();
char* get_bloqueados();

typedef struct {
  char nombre[USUARIO_NOMBRE_SIZE];
  char clave[USUARIO_CLAVE_SIZE];
  char bloqueado[USUARIO_BLOQUEADO_SIZE];
} Usuario;
