#include "../include/launch.h"

int32_t pid, error;

/**
 * Funcion main
 */
int32_t main( int32_t argc, char *argv[] ) {

  // chequeo de argumentos
	if ( argc < 2 ) {
        	fprintf( stderr, "Uso: %s <direccion_ip> <puerto_primario>\n", argv[0] );
		exit( 1 );
	}

  error = 0;

  pid = fork();
  if ( pid == 0 ) {
    if( execv(PATH_AUTH_SERVICE,  argv) == -1 ) {
      perror("launch: auth_service: ");
      error = 1;
      exit(1);
    }
    exit(0);
  }

  if(error == 1)
    exit(1);

  pid = fork();
  if ( pid == 0 ) {
    if( execv(PATH_FILES_SERVICE,  argv) == -1 ) {
      perror("launch: files_service: ");
      exit(1);
    }
    exit(0);
  }

  if(error == 1)
    exit(1);

  if( execv(PATH_PRIMARY_SERVER,  argv) == -1 ) {
    perror("launch: primary_server: ");
    exit(1);
  }
}
