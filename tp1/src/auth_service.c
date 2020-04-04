#include "../include/auth_service.h"

char buffer[TAM];

uint32_t main( uint32_t argc, char *argv[] ) {

	int sockfd, newsockfd, servlen, clilen, n, buf, pid;
  struct sockaddr_un  cli_addr, serv_addr;

	/* Se toma el nombre del socket de la línea de comandos */
  if( argc != 2 ) {
    printf( "Uso: %s <nombre_de_socket>\n", argv[0] );
    exit( 1 );
  }

	if ( ( sockfd = socket( AF_UNIX, SOCK_STREAM, 0) ) < 0 ) {
    perror( "creación de  socket");
    exit(1);
  }

	/* Remover el nombre de archivo si existe */
	unlink ( argv[1] );

	memset( &serv_addr, 0, sizeof(serv_addr) );
  serv_addr.sun_family = AF_UNIX;
  strcpy( serv_addr.sun_path, argv[1] );
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  if( bind( sockfd,(struct sockaddr *)&serv_addr,servlen )<0 ) {
    perror( "ligadura" );
    exit(1);
  }
	else {
		printf("auth_service: Iniciando\n");
		printf("auth_service: Proceso: %d - Socket: %s\n", getpid(), serv_addr.sun_path);
	}

  listen( sockfd, 5 );
	clilen = sizeof( cli_addr );

  while(1) {

	}
}

/**
 * Recibe datos y los guarda en buffer

void recepcion() {
	memset( buffer, 0, TAM );
	n = recv( newsockfd, buffer, TAM, 0 );
	if ( n < 0 ) {
	  perror( "SERVIDOR: Error: lectura de socket" );
	  exit(1);
	}
}*/
