#include "utilidades.h"
#include "base_de_datos.h"

void recepcion();
void enviar_a_socket(uint32_t, char*);
void enviar_a_cliente(char*);
void parse();
void user_command(char*, char*);
void user_ls();
void file_command(char*, char*);
void exit_command();
void unknown_command(char*);
void configurar_nombres(char*);
void configurar_claves(char*);
void configurar_bloqueados(char*);
uint32_t logueo();

in_addr_t inet_addr(const char*);
pid_t getpid(void);
pid_t fork(void);
uint32_t close(uint32_t fd);
ssize_t read(uint32_t fd, void *buf, size_t count);
ssize_t write(uint32_t fildes, const void *buf, size_t nbytes);
