#include "utilities.h"

#define PATH_DOWNLOADS_DIR "resources/client_downloads/"

void recepcion(int32_t);
void escribir_a_servidor(int32_t);
void enviar_a_socket(int32_t, char*);
void salida(int32_t);
void conectar_a_primary();
void conectar_a_file();
void descargar();
int32_t logueo();
int32_t fin(char[BUFFER_SIZE]);
