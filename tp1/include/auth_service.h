#include "utilities.h"

uint32_t conectar();
uint32_t bloquear(uint32_t);
uint32_t login(char*);
char* get_nombres();
char* get_claves();
char* get_bloqueados();
void configurar_nombres();
void configurar_claves();
void configurar_bloqueados();


typedef struct {
  char nombre[USUARIO_NOMBRE_SIZE];
  char clave[USUARIO_CLAVE_SIZE];
  char bloqueado[USUARIO_BLOQUEADO_SIZE];
} Usuario;
