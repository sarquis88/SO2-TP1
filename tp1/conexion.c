#include "conexion.h"

uint32_t conectar(char* user, char* password) {

	FILE *file;
	file = fopen("usuarios", "r");

	char* usuario;
	char* clave;
	char* bloqueado;
	char* aux;

	if ( file != NULL ) {
      char line[LINE_SIZE];

      while ( fgets( line, LINE_SIZE, file ) != NULL ) {
				for(uint32_t i = 0; i < strlen(line); i++) {
					
				}
      }

      fclose ( file );
   }

	return 0;
}
