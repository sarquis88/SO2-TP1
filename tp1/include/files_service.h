#include "utilities.h"

#define SOCKET_PATH "/tmp/file_service"
#define FILES_DIR_NAME "resources/files/"

int32_t conectar();
void enviar_a_cola_local(long, char*, char);
void configurar_socket();
void imprimir(int32_t);
void enviar_a_cliente(char*);
void enviar_archivo(int32_t);
void recepcion();

typedef struct {
  int32_t index;
  char nombre[ARCHIVO_NOMBRE_SIZE];
  char formato[ARCHIVO_FORMATO_SIZE];
} Archivo;
