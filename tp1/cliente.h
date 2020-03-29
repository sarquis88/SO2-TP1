#include "utilidades.h"

void recepcion();
void escribir_a_servidor(uint32_t);
void enviar_a_servidor(char*);
uint32_t logueo();
uint32_t fin(char[TAM]);

ssize_t read(uint32_t fd, void *buf, size_t count);
ssize_t write(uint32_t fildes, const void *buf, size_t nbytes);
