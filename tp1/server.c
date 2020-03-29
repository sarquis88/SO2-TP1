#include "server.h"

uint32_t newsockfd, n, intentos;
char buffer[TAM];

uint32_t main( uint32_t argc, char *argv[] ) {

	/////////////////////////////////////////////////////////////
	// CONFIGURACION SOCKET Y PUERTO DE SERVER
	/////////////////////////////////////////////////////////////

	uint32_t sockfd, puerto, clilen, pid;
	struct sockaddr_in serv_addr, cli_addr;

	if ( argc < 2 ) {
        	fprintf( stderr, "Uso: %s <puerto>\n", argv[0] );
		exit( 1 );
	}

	sockfd = socket( AF_INET, SOCK_STREAM, 0);

	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );
	puerto = atoi( argv[1] );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons( puerto );

	if ( bind(sockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		perror( "SERVIDOR: Error" );
		exit( 1 );
	}
	else {
		printf("SERVIDOR: Iniciando\n");
		printf("SERVIDOR: Proceso: %d - Puerto: %d\n", getpid(), ntohs(serv_addr.sin_port));
	}

	listen( sockfd, 5 );
	clilen = sizeof( cli_addr );

	/////////////////////////////////////////////////////////////
	// ESPERAR CLIENTES NUEVOS
	/////////////////////////////////////////////////////////////

	while(1) {
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );

		pid = fork();

		if ( pid == 0 ) {
			// proceso hijo que atiende a cliente

		    close( sockfd );
				intentos = 0;
				uint32_t log;

				while(1) {
					log = logueo();

					if(log == 1) {
			    	printf("%d NUEVO CLIENTE\n", getpid() );
						enviar_a_cliente("1");
						intentos = 0;
						break;
					}
			    else if(log == 0) {
						intentos++;
						if(intentos >= LIMITE_INTENTOS) {
							printf("%d USUARIO BLOQUEADO\n", getpid() );
							enviar_a_cliente("9");
							intentos = 0;
							break;
						}
						else {
				    	printf("%d LOGUEO INTENTO FALLIDO\n", getpid() );
							enviar_a_cliente("0");
						}
			    }
				}

				if(log == 1) {
					while(1) {
						recepcion();
						parse();
			    }
				}
		    else
					break;
		}
		else {
			// proceso original del servidor
			printf( "%d AUTENTICANDO\n", pid );
			close( newsockfd );
		}
	}
	return 0;
}

/**
 * Recibe datos y los guarda en buffer
 */
void recepcion() {
	memset( buffer, 0, TAM );
	n = read( newsockfd, buffer, TAM-1 );
	if ( n < 0 ) {
	  perror( "SERVIDOR: Error: lectura de socket" );
	  exit(1);
	}
}

/**
 * Envia datos por el socket
 */
void enviar_a_socket(uint32_t socket, char* mensaje) {
	n = write( socket, mensaje, strlen(mensaje) );
	if ( n < 0 ) {
	  perror( "SERVIDOR: Error: envio a socket\n");
	  exit( 1 );
	}
}

/**
 * Envia datos por el socket hacia el cliente
 */
void enviar_a_cliente(char* mensaje) {
	enviar_a_socket(newsockfd, mensaje);
}

/**
 * Crea el comando y la opcion recibidos
 * Reaccion a los mensajes recibidos
 * Funcion llamada despues de recepcion()
 */
void parse() {
	buffer[strlen(buffer)-1] = '\0';

	char *mensaje, *comando, *opcion, *argumento;
	uint32_t i = 0;

	mensaje = strtok(buffer, " ");
	opcion = " ";
	argumento = " ";

	while(mensaje != NULL) {
		if(i == 0) {
			comando = mensaje;
			i++;
		}
		else if(i == 1) {
			opcion = mensaje;
			i++;
		}
		else {
			argumento = mensaje;
			break;
		}
		mensaje = strtok(NULL, " ");
	}

	printf( "%d - %s %s %s\n", getpid(), comando, opcion, argumento);

	if( strcmp("exit", comando) == 0 )
		exit_command();
	else if( strcmp("user", comando) == 0 )
		user_command(opcion, argumento);
	else if( strcmp("file", comando) == 0 )
		file_command(opcion, argumento);
	else
		unknown_command(comando);
}

/**
 * Reaccion a comando user
 */
void user_command(char *opcion, char *argumento) {
	if( strcmp("ls", opcion) == 0 ) {
		enviar_a_cliente(	"\nMostrando usuarios:\n"
							"	- Elpe Lado\n"
							"	- Dicky del Solar\n");
	}
	else if( strcmp("passwd", opcion) == 0 && strcmp(" ", argumento) != 0) {
		char aux[] = "\nNueva contraseña: ";
		char aux1[] = "\n";

		char *respuesta = malloc(strlen(aux) + strlen(aux1) + strlen(argumento) + 1);
		strcat(respuesta, aux);
		strcat(respuesta, argumento);
		strcat(respuesta, aux1);
		enviar_a_cliente(respuesta);
		free(respuesta);
	}
	else {
		enviar_a_cliente(	" \nUso: user [opcion] <argumento>\n\n"
							"	- ls : listado de usuarios\n"
							"	- passwd <nueva contraseña> : cambio de contraseña\n");
	}
}

/**
 * Reaccion a comando file
 */
void file_command(char *opcion, char *argumento) {
	if( strcmp("ls", opcion) == 0 ) {
		enviar_a_cliente(	"\nMostrando archivos:\n"
							"	- Ubuntu\n"
							"	- CentOS\n");
	}
	else if( strcmp("down", opcion) == 0 && strcmp(" ", argumento) != 0) {
		char aux[] = "\nArchivo descargado: ";
		char aux1[] = "\n";

		char *respuesta = malloc(strlen(aux) + strlen(aux1) + strlen(argumento) + 1);
		strcat(respuesta, aux);
		strcat(respuesta, argumento);
		strcat(respuesta, aux1);
		enviar_a_cliente(respuesta);
		free(respuesta);
	}
	else {
		enviar_a_cliente(	" \nUso: file [opcion] <argumento>\n\n"
							"	- ls : listado de archivos\n"
							"	- down <archivo> : descarga de archivo\n");
	}
}

/**
 * Reaccion a comando exit
 */
void exit_command() {
	printf( "%d HA SALIDO\n", getpid() );
	exit(0);
}

/**
 * Reaccion a comando desconocido
 */
void unknown_command(char * comando) {
	enviar_a_cliente(	"\nComando no reconocido\n"
						"Comandos aceptados:\n"
						"	- user\n"
						"	- file\n"
						"	- exit\n");
}

/**
 * Proceso de inicio de sesion
 * @return	0 para login fallido
 *					1 para login exitoso
 *					9 para usuario bloqueado
 */
uint32_t logueo() {
	recepcion();	// recepcion de credenciales desde el cliente
	char* login;

	login = strtok(buffer, "-");
	char usuario[strlen(login)];
	strcpy(usuario, login);

	login = strtok(NULL, "-");
	char clave[strlen(login)];
	strcpy(clave, login);

	printf("%d USUARIO %s - CLAVE %s\n", getpid(), usuario, clave);

	conectar(usuario, clave);

	if(strcmp("Tomas", usuario) == 0) {
		return 1;
	}
	else {
		return 0;
	}
}
