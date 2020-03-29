#include "cliente.h"

char buffer[TAM];
uint32_t sockfd, n;

uint32_t main( uint32_t argc, char *argv[] ) {

	/////////////////////////////////////////////////////////////
	// CONFIGURACION SOCKET Y PUERTO DE CLIENTE
	/////////////////////////////////////////////////////////////

	uint32_t puerto;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	uint32_t terminar = 0;

	if ( argc < 3 ) {
		fprintf( stderr, "Uso %s host puerto\n", argv[0]);
		exit( 0 );
	}

	puerto = atoi( argv[2] );
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );

	server = gethostbyname( argv[1] );

	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons( puerto );
	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
		perror( "CLIENTE: Error: conexion" );
		exit( 1 );
	}

	/////////////////////////////////////////////////////////////
	// INTENTO DE LOGUEO
	/////////////////////////////////////////////////////////////

	uint32_t log;
	while(1) {
		log = logueo();

		if(log == 0) {
			printf("\nLogin fallido, vuelva a intentar\n\n");
		}
		else if(log == 1) {
			printf("\nConectado\n\n");
			break;
		}
		else if(log == 9) {
			printf("\nUsuario bloqueado\n\n");
			log = 0;
			break;
		}
	}

	if(log != 1)
		return 0;

	while(1) {

		/////////////////////////////////////////////////////////////
		// ENVIO Y RECEPCION DE MENSAJES A SERVIDOR
		/////////////////////////////////////////////////////////////

		escribir_a_servidor(1);

		if(buffer[0] != '\0') {
			if(fin(buffer))
				terminar = 1;

			recepcion();

			if(terminar == 0)
				printf("%s\n", buffer);
			else
				exit(0);
		}
	}
	return 0;
}

/**
 * Recibe datos y los guarda en buffer
 */
void recepcion() {
	memset( buffer, 0, TAM );
	n = read( sockfd, buffer, TAM-1 );
	if ( n < 0 ) {
	  perror( "CLIENTE: Error: lectura de socket" );
	  exit(1);
	}
}

/**
 * Envio de mensaje a servidor mediante input
 * Si simbolo == 1, se muestra un '>' para escribir
 */
void escribir_a_servidor(uint32_t simbolo) {
	uint32_t salir = 0;

	while(salir == 0) {
		memset( buffer, '\0', TAM );

		if(simbolo == 1)
			printf( "> " );

		fgets( buffer, TAM-1, stdin );

		if(buffer[0] != 10) {
			n = write( sockfd, buffer, strlen(buffer) );
			if ( n < 0 ) {
			  perror( "CLIENTE: Error: envio a socket\n");
			  exit( 1 );
			}
			else
				salir = 1;
		}
	}
}

/**
 * Envio de mensaje a servidor
 */
void enviar_a_servidor(char* mensaje) {
	n = write( sockfd, mensaje, strlen(mensaje) );
	if ( n < 0 ) {
	  perror( "CLIENTE: Error: envio a socket\n");
	  exit( 1 );
	}
}

/**
 * Proceso de inicio de sesion
 * @return	0 para login fallido
 *					1 para login exitoso
 *					9 para usuario bloqueado
 */
uint32_t logueo() {
	char usuario[USER_SIZE];
	char clave[PASSWORD_SIZE];
	char login[strlen(usuario) + strlen(clave) + 2];

	printf("Usuario: ");
	fgets( usuario, TAM-1, stdin );
	printf("Clave: ");
	fgets( clave, TAM-1, stdin );

	// formato de login: "usuario-clave"
	uint32_t i = 0;
	for(; i < strlen(usuario); i++) {
		if(usuario[i] == 10) {
			login[i] = '-';
			i++;
			break;
		}
		else
			login[i] = usuario[i];
	}
	for(uint32_t j = 0; j < strlen(clave); j++) {
		if(clave[j] == 10) {
			login[i] = '\0';
			break;
		}
		else {
			login[i] = clave[j];
			i++;
		}
	}
	// fin formato de login

	enviar_a_servidor(login); // envio de credenciales a servidor
	recepcion();	// respuesta del servidor

	if( buffer[0] == '1' )
		return 1;
	else if( buffer[0] == '0' )
		return 0;
	else
		return 9;
}

/**
 * Verifica si se termina la comunicacion
 */
uint32_t fin(char buf[TAM]) {
	buf[strlen(buf)-1] = '\0';
	if( !strcmp( "exit", buf )) {
		return 1;
	}
	else {
		return 0;
	}
}
