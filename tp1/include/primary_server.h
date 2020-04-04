#include "utilities.h"
#include "data_base.h"

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
void bloquear_usuario(char*);
uint32_t logueo();
