#include "../include/primary_server.h"

int32_t newsockfd, n, qid, sockfd, pid, puerto;
uint32_t clilen;
struct sockaddr_in serv_addr, cli_addr;
char buffer[BUFFER_SIZE], impresion[BUFFER_SIZE];

/**
 * Funcion main
 */
int32_t main( int32_t argc, char *argv[] ) {

	// chequeo de argumentos
	if ( argc < 2 ) {
		sprintf(impresion, "Uso: %s <puerto>\n", argv[0]);
    imprimir(1);
		exit(1);
	}

	// definicion de puerto
	puerto = atoi( argv[1] );

	// levantamiento de socket
	configurar_socket();

	// creacion de cola
	qid = get_cola('p');
	if(qid == -1) {
		sprintf(impresion, "error creando cola");
		imprimir(1);
		exit(1);
	}

	sprintf(impresion, "cola = %d\n", qid);
	imprimir();

	// empezar a escuchar
	listen( sockfd, 5 );
	clilen = sizeof( cli_addr );
	while(1) {
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );

		pid = fork();

		if ( pid == 0 ) {
			// proceso hijo que atiende a cliente

		    close( sockfd );
				int32_t intentos = 0;
				int32_t log;
				char* user_logueado;

				while(1) {

					recepcion();		// recepcion de credenciales por parte de cliente

					if( strcmp(buffer, "exit") == 0 )
						exit_command( "no-user" );

					enviar_a_cola_local((long) LOGIN_REQUEST, buffer, 'a'); // reenvio a auth_service

					char* respuesta = recibir_de_cola(LOGIN_RESPONSE, 'p').mtext; // respuesta de auth_service

					if( respuesta[0] == '0') {
						intentos++;
						if(intentos >= LIMITE_INTENTOS) {
							char* nombre = strtok(buffer, "-");
							sprintf(impresion, "usuario bloqueado: %s\n", nombre);
							imprimir();
							enviar_a_cola_local((long) BLOQUEAR_USUARIO, nombre, 'a');
							enviar_a_cliente("9");
							intentos = 0;
							exit(0);
						}
						else {
							char* nombre = strtok(buffer, "-");
							sprintf(impresion, "intento de logueo fallido: %s\n", nombre);
							imprimir();
							enviar_a_cliente("0");
						}
					}
					else if(respuesta[0] == '1') {
						user_logueado = malloc(strlen(strtok(buffer, "-")));
						strcpy(user_logueado, strtok(buffer, "-"));
						sprintf(impresion, "nuevo cliente: %s\n", user_logueado);
						imprimir();
						enviar_a_cliente("1");
						intentos = 0;
						log = 1;
						break;
					}
					else if(respuesta[0] == '9') {
						char* nombre = strtok(buffer, "-");
						sprintf(impresion, "usuario bloqueado: %s\n", nombre);
						imprimir();
						enviar_a_cliente("9");
						intentos = 0;
						exit(0);
					}
				}

				if(log == 1) {
					while(1) {
						recepcion();
						parse(user_logueado);
			    }
				}
		}
		else {
			// proceso original del servidor
			sprintf(impresion, "autenticando\n");
			imprimir();
			close( newsockfd );
		}
	}
	exit(0);
}

/**
 * Levantamiento de socket
 */
void configurar_socket() {
	sockfd = socket( AF_INET, SOCK_STREAM, 0);
	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serv_addr.sin_port = htons( puerto );
	if ( bind(sockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		sprintf(impresion, "error conectando socket");
		imprimir(1);
		exit(1);
	}
	else {
		sprintf(impresion, "iniciando\n");
		imprimir();
		sprintf(impresion, "proceso: %d - puerto: %d\n", getpid(), ntohs(serv_addr.sin_port));
		imprimir();
	}
}

/**
 * Recibe datos y los guarda en buffer
 */
void recepcion() {
	memset( buffer, 0, BUFFER_SIZE );
	n = recv( newsockfd, buffer, BUFFER_SIZE, 0 );
	if ( n < 0 ) {
		sprintf(impresion, "error leyendo de socket");
	  imprimir(1);
	  exit(1);
	}
}

/**
 * Envia datos por el socket
 */
void enviar_a_socket(int32_t socket, char* mensaje) {
	n = send( socket, mensaje, strlen(mensaje), 0 );
	if ( n < 0 ) {
		sprintf(impresion, "error enviando a socket");
	  imprimir(1);
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
void parse(char* usuario_logueado) {
	buffer[strlen(buffer)-1] = '\0';

	char *mensaje, *comando, *opcion, *argumento;
	int32_t i = 0;

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

	sprintf(impresion, "%s - %s %s %s\n", usuario_logueado, comando, opcion, argumento);
	imprimir();

	if( strcmp("exit", comando) == 0 )
		exit_command(usuario_logueado);
	else if( strcmp("user", comando) == 0 )
		user_command(usuario_logueado, opcion, argumento);
	else if( strcmp("file", comando) == 0 )
		file_command(opcion, argumento);
	else
		unknown_command();
}

/**
 * Reaccion a comando user
 */
void user_command(char* usuario, char *opcion, char *argumento) {
	if( strcmp("ls", opcion) == 0 )
		user_ls();
	else if( strcmp("passwd", opcion) == 0 && strcmp(" ", argumento) != 0)
		user_passwd(usuario, argumento);
	else {
		enviar_a_cliente(	" \nUso: user [opcion] <argumento>\n\n"
							"	- ls : listado de usuarios\n"
							"	- passwd <nueva contraseña> : cambio de contraseña\n");
	}
}

/**
 * Reaccion al comando user ls
 */
void user_ls() {
	enviar_a_cola_local((long) NOMBRES_REQUEST, "n", 'a');
	char* respuesta = recibir_de_cola(NOMBRES_RESPONSE, 'p').mtext; // respuesta de auth_service
	enviar_a_cliente(respuesta);
}

/**
 * Reaccion al comando user passwd
 */
void user_passwd(char* usuario, char* clave) {

	if( strlen(clave) < 3 || strlen(clave) > USUARIO_CLAVE_SIZE) {
		enviar_a_cliente("\nClave invalida\n");
		return;
	}

	char* aux = "-";
	char* tmp = malloc(strlen(usuario) + strlen(clave) + strlen(aux));
	strcat(tmp, usuario);
	strcat(tmp, aux);
	strcat(tmp, clave);
	enviar_a_cola_local((long) CAMBIAR_CLAVE_REQUEST, tmp, 'a');

	recibir_de_cola((long) CAMBIAR_CLAVE_RESPONSE, 'p');

	free(tmp);
	enviar_a_cliente("\nClave cambiada con exito\n");
}

/**
 * Reaccion a comando file
 */
void file_command(char *opcion, char *argumento) {
	if( strcmp("ls", opcion) == 0 )
		file_ls();
	else if( strcmp("down", opcion) == 0 && strcmp(" ", argumento) != 0)
		return;
	else {
		enviar_a_cliente(	" \nUso: file [opcion] <argumento>\n\n"
							"	- ls : listado de archivos\n"
							"	- down <archivo> : descarga de archivo\n");
	}
}

/**
 * Reaccion al comando file ls
 */
void file_ls() {
	enviar_a_cola_local((long) ARCHIVOS_REQUEST, "n", 'f');
	char* respuesta = recibir_de_cola(ARCHIVOS_RESPONSE, 'p').mtext; // respuesta de auth_service
	enviar_a_cliente(respuesta);
}

/**
 * Reaccion a comando exit
 */
void exit_command(char* usuario) {
	sprintf(impresion, "%s ha salido\n", usuario);
	imprimir();
	exit(0);
}

/**
 * Reaccion a comando desconocido
 */
void unknown_command() {
	enviar_a_cliente(	"\nComando no reconocido\n"
						"Comandos aceptados:\n"
						"	- user\n"
						"	- file\n"
						"	- exit\n");
}

/**
 * Enviar mensaje a cola de proceso
 */
void enviar_a_cola_local(long id, char* mensaje, char proceso) {
	if(enviar_a_cola(id, mensaje, proceso) == -1) {
		sprintf(impresion, "error enviando mensaje");
		imprimir(1);
		exit(1);
	}
}

/**
 * Imprimir en consola
 */
void imprimir(int32_t error) {
		if(error) {
			fprintf(stderr, "PRIMARY_SERVER: %s", impresion );
		}
		else {
			printf("PRIMARY_SERVER: %s", impresion);
		}
		fflush(stdout);
}
