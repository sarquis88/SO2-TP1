#include "../include/client.h"

char buffer[BUFFER_SIZE];
int32_t sockfd, n, puerto, terminar, logueo_response;
struct sockaddr_in serv_addr;
struct hostent *server;

int32_t main( int32_t argc, char *argv[] ) {

	// chequeo de argumentos
	if ( argc < 3 ) {
		fprintf( stderr, "Uso %s <host> <puerto>\n", argv[0]);
		exit( 0 );
	}

	// configuracion de puerto y server
	puerto = atoi( argv[2] );
	server = gethostbyname( argv[1] );

	// configuracion de socket
	configurar_socket();

	// activar handler para SIGINT
 	signal(SIGINT, salida);

	// intento de logueo
	while(1) {
		logueo_response = logueo();

		if(logueo_response == 0)
			printf("\nLogin fallido, vuelva a intentar\n\n");
		else if(logueo_response == 1) {
			printf("\nConectado\n\n");
			break;
		}
		else if(logueo_response == 9) {
			printf("\nUsuario bloqueado\n\n");
			salida(0);
		}
	}

	while(1) {

		terminar = 0;

		// enviar comandos a server
		escribir_a_servidor(1);

		// chequeo si el mensaje recibido no es nulo
		if(buffer[0] != '\0') {

			// chequeo si el cliente quiere salir
			if(fin(buffer))
				terminar = 1;

			// recibo contestacion de servidor
			recepcion();

			if(terminar == 0)
				printf("%s\n", buffer);
			else
				salida(0);
		}
	}
	exit(0);
}

/**
 * Levantamiento de socket
 */
void configurar_socket() {
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );

	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons( puerto );
	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
		perror( "CLIENTE: Error: conexion" );
		exit( 1 );
	}
}

/**
 * Rececpion de datos y guardado en buffer
 */
void recepcion() {
	memset( buffer, 0, BUFFER_SIZE );
	n = recv( sockfd, buffer, BUFFER_SIZE, 0 );
	if ( n < 0 ) {
	  perror( "CLIENTE: Error: lectura de socket" );
	  exit(1);
	}
}

/**
 * Envio de mensaje a servidor mediante input
 * Si simbolo == 1, se muestra un '>' para escribir
 */
void escribir_a_servidor(int32_t simbolo) {
	int32_t salir = 0;

	while(salir == 0) {
		memset( buffer, '\0', BUFFER_SIZE );

		if(simbolo == 1)
			printf( "> " );

		fgets( buffer, BUFFER_SIZE-1, stdin );

		if(buffer[0] != 10) {
			n = send( sockfd, buffer, strlen(buffer), 0 );
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
	n = send( sockfd, mensaje, strlen(mensaje), 0 );
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
int32_t logueo() {
	char usuario[USUARIO_NOMBRE_SIZE];
	char clave[USUARIO_CLAVE_SIZE];

	printf("Usuario: ");
	fgets( usuario, BUFFER_SIZE, stdin );
	usuario[strlen(usuario) - 1] = '\0';
	printf("Clave: ");
	fgets( clave, BUFFER_SIZE, stdin );
	clave[strlen(clave) - 1] = '\0';

	// formato de login: "usuario-clave"
	char* sep = "-";
	char login[strlen(usuario) + strlen(clave) + strlen(sep)];
	strcpy(login, "\0");
	strcat(login, usuario);
	strcat(login, sep);
	strcat(login, clave);

	// envio de credenciales a servidor
	enviar_a_servidor(login);

	// respuesta del servidor
	recepcion();

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
int32_t fin(char buf[BUFFER_SIZE]) {
	buf[strlen(buf)-1] = '\0';
	if( !strcmp( "exit", buf )) {
		return 1;
	}
	else {
		return 0;
	}
}

/**
 * Handler de salida del cliente
 */
void salida(int32_t sig) {
	if(!sig) {
		printf("Saliendo...\n");
		fflush(stdout);
		enviar_a_servidor("exit");
	}
	exit(0);
}
