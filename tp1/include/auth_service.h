#include "utilities.h"

uint32_t conectar();
uint32_t bloquear_usuario(char*);
uint32_t cambiar_clave(char*);
uint32_t login(char*);
char* get_nombres();
char* get_claves();
char* get_bloqueados();
void configurar_nombres();
void configurar_claves();
void configurar_bloqueados();
void enviar_a_cola_local(long, char*, char);

typedef struct {
  char nombre[USUARIO_NOMBRE_SIZE];
  char clave[USUARIO_CLAVE_SIZE];
  char bloqueado[USUARIO_BLOQUEADO_SIZE];
} Usuario;
