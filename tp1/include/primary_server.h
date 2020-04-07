#include "utilities.h"

void recepcion();
void enviar_a_socket(int32_t, char*);
void enviar_a_cliente(char*);
void parse(char*);
void user_command(char*, char*, char*);
void user_ls();
void file_ls();
void user_passwd(char*, char*);
void file_command(char*, char*);
void exit_command();
void unknown_command();
void enviar_a_cola_local(long, char*, char);
void configurar_socket();
void imprimir(int32_t);
