#include "../include/client.h"

char buffer[BUFFER_SIZE];
int32_t socket_primary, socket_file, n, puerto_primary, puerto_file, logueo_response;
struct sockaddr_in serv_addr_primary;
struct sockaddr_in serv_addr_file;
struct hostent *server_primary;
struct hostent *server_file;

int32_t main( int32_t argc, char *argv[] ) {

	// chequeo de argumentos
	if ( argc < 3 ) {
		fprintf( stderr, "Uso %s <host> <puerto_primario>\n", argv[0]);
		exit( 0 );
	}

	// configuracion de puerto_primary y server
	puerto_primary = atoi( argv[2] );
	puerto_file = puerto_primary + 1;
	server_primary = gethostbyname( argv[1] );
	server_file = gethostbyname( argv[1] );

	// configuracion de socket
	conectar_a_primary();

	// activar handler para SIGINT
 	signal(SIGINT, salida);

	// intento de logueo
	while(1) {
		logueo_response = logueo();

		if(logueo_response == 0)
			printf("Login fallido, vuelva a intentar\n");
		else if(logueo_response == 1) {
			printf("Conectado\n");
			break;
		}
		else if(logueo_response == 9) {
			printf("\nUsuario bloqueado\n\n");
			salida(0);
		}
	}

	while(1) {

		// enviar comandos a server
		escribir_a_servidor(1);

		// chequeo si el mensaje recibido no es nulo
		if(buffer[0] != '\0') {

			// chequeo si el cliente quiere salir
			if( strcmp(buffer, "exit\n") == 0)
				salida(1);

			// recibo contestacion de servidor
			recepcion(socket_primary);

			if( strcmp(buffer, "descarga_no") == 0)
				printf("\nIndice de archivo no encontrado\n");
			else if( strcmp(buffer, "descarga_si") == 0)
				descargar();
			else
				printf("%s\n", buffer);
		}
	}
	exit(0);
}

/**
 * Levantamiento de socket primario
 */
void conectar_a_primary() {
	socket_primary = socket( AF_INET, SOCK_STREAM, 0 );
	memset( (char *) &serv_addr_primary, '0', sizeof(serv_addr_primary) );
	serv_addr_primary.sin_family = AF_INET;
	bcopy( (char *)server_primary->h_addr, (char *)&serv_addr_primary.sin_addr.s_addr, server_primary->h_length );
	serv_addr_primary.sin_port = htons( puerto_primary );

	if ( connect( socket_primary, (struct sockaddr *)&serv_addr_primary, sizeof(serv_addr_primary ) ) < 0 ) {
		perror( "CLIENTE: Error: conexion a primary" );
		exit( 1 );
	}
}

/**
 * Levantamiento de socket file
 */
void conectar_a_file() {
	socket_file = socket( AF_INET, SOCK_STREAM, 0 );
	memset( (char *) &serv_addr_file, '0', sizeof(serv_addr_file) );
	serv_addr_file.sin_family = AF_INET;
	bcopy( (char *)server_file->h_addr, (char *)&serv_addr_file.sin_addr.s_addr, server_file->h_length );
	serv_addr_file.sin_port = htons( puerto_file );

	if ( connect( socket_file, (struct sockaddr *)&serv_addr_file, sizeof(serv_addr_file ) ) < 0 ) {
		perror( "CLIENTE: Error: conexion a file" );
		exit( 1 );
	}
}

/**
 * Rececpion de datos y guardado en buffer
 */
void recepcion(int32_t socket) {
	memset( buffer, 0, BUFFER_SIZE );
	n = recv( socket, buffer, BUFFER_SIZE, 0 );
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
			n = send( socket_primary, buffer, strlen(buffer), 0 );
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
void enviar_a_socket(int32_t socket, char* mensaje) {
	n = send( socket, mensaje, strlen(mensaje), 0 );
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
	enviar_a_socket(socket_primary, login);

	// respuesta del servidor
	recepcion(socket_primary);

	if( buffer[0] == '1' )
		return 1;
	else if( buffer[0] == '0' )
		return 0;
	else
		return 9;
}

/**
 * Handler de salida del cliente
 */
void salida(int32_t sig) {
	if(sig > 0) {
		printf("Saliendo...\n");
		fflush(stdout);
		enviar_a_socket(socket_primary, "exit");
	}
	exit(0);
}

/**
 * Proceso de descarga de archivo
 */
void descargar() {

	conectar_a_file();

	// recibo nombre de archivo con formato
	recepcion(socket_file);

	char nombre_archivo[strlen(buffer)];
	strcpy(nombre_archivo, buffer);
	nombre_archivo[strlen(nombre_archivo)] = '\0';

	printf("Descargando hash de archivo\n");
	recepcion(socket_file);
	char hash_original[MD5_DIGEST_LENGTH * 2 + 1];
	strcpy(hash_original, "\0");
	strcat(hash_original, buffer);

	char* file_path = malloc(strlen(PATH_DOWNLOADS_DIR) + strlen(nombre_archivo));
	strcpy(file_path, "\0");
	strcat(file_path, PATH_DOWNLOADS_DIR);
	strcat(file_path, nombre_archivo);
	file_path[strlen(file_path)] = '\0';

	FILE* file = fopen( file_path, "wb" );
  if(file != NULL) {
		printf("Descargando archivo: %s\n", nombre_archivo);
		int32_t recibido = 0;
    while( (n = recv(socket_file, buffer, sizeof(buffer), 0) ) > 0 ) {
      fwrite(buffer, sizeof(char), n, file);
			recibido = recibido + n;
		}
		recibido = recibido * 8; // 8: tamaño de char
		printf("Descarga terminada\n");

    fclose(file);
  }
	else {
      perror("CLIENTE: error creando archivo de descarga\n");
			printf("%s\n", file_path);
			exit(1);
  }

	printf("Chequeando hashes\n");
	char* hash_descarga = get_md5(file_path);

	if( strcmp(hash_original, hash_descarga) == 0 )
		printf("Chequeo de hash exitoso\n");
	else
		printf("Chequeo de hash fallido: revisar descarga\n");

	free(hash_descarga);
	free(file_path);

	close( socket_file );
}
