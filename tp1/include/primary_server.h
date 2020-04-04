#include "utilities.h"

void recepcion();
void enviar_a_socket(uint32_t, char*);
void enviar_a_cliente(char*);
void parse();
void user_command(char*, char*);
void user_ls();
void file_command(char*, char*);
void exit_command();
void unknown_command(char*);
void enviar_a_cola_local(long, char*, char);
