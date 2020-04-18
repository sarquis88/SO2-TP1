#include "../include/primary_server.h"

// definicion de variables
int32_t socket_cliente, qid, sockfd, pid, puerto, salida;
ssize_t n;
uint32_t clilen;
struct sockaddr_in serv_addr, cli_addr;
char buffer[BUFFER_SIZE], impresion[BUFFER_SIZE], direccion[16];

/**
 * Funcion main
 */
int32_t main( int32_t argc, char *argv[] ) {

	// chequeo de argumentos
	if ( argc < 3 ) {
		sprintf(impresion, "Uso: %s <direccion ip> <puerto>\n", argv[0]);
    imprimir(1);
		exit(1);
	}

	// definicion de puerto y direccion
	sprintf(direccion, "%s", argv[1]);
	puerto = atoi( argv[2] );

	// levantamiento de socket
	configurar_socket();

	sprintf(impresion, "iniciando\n");
	imprimir(0);
	sprintf(impresion, "proceso: %d - puerto: %d\n", getpid(), ntohs(serv_addr.sin_port));
	imprimir(0);

	// creacion de cola
	qid = get_cola('p');
	if(qid == -1) {
		sprintf(impresion, "error creando cola\n");
		imprimir(1);
		exit(1);
	}

	sprintf(impresion, "cola = %d\n", qid);
	imprimir(0);

	// empezar a escuchar
	listen( sockfd, 5 );
	clilen = sizeof( cli_addr );
	while(1) {
		socket_cliente = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );

		int32_t intentos = 0;
		salida = 0;
		int32_t log = 0;
		char user_logueado[USUARIO_NOMBRE_SIZE];

		sprintf(impresion, "autenticando\n");
		imprimir(0);

		while(1) {

			recepcion();		// recepcion de credenciales por parte de cliente

			if( strcmp(buffer, "exit\n") == 0 ) {
				exit_command( "no-user" );
				break;
			}

			enviar_a_cola_local((long) LOGIN_REQUEST, buffer, 'a'); // reenvio a auth_service

			char* respuesta = recibir_de_cola(LOGIN_RESPONSE, 'p').mtext; // respuesta de auth_service

			if( respuesta[0] == '0') {
				intentos++;
				if(intentos >= LIMITE_INTENTOS) {
					char* nombre = strtok(buffer, "-");
					sprintf(impresion, "usuario bloqueado: %s\n", nombre);
					imprimir(0);
					enviar_a_cola_local((long) BLOQUEAR_USUARIO, nombre, 'a');
					enviar_a_cliente("9");
					intentos = 0;
					log = 0;
					break;
				}
				else {
					char* nombre = strtok(buffer, "-");
					sprintf(impresion, "intento de logueo fallido: %s\n", nombre);
					imprimir(0);
					enviar_a_cliente("0");
					log = 0;
				}
			}
			else if(respuesta[0] == '1') {
				sprintf(user_logueado, "%s", strtok(buffer, "-"));
				sprintf(impresion, "nuevo cliente: %s\n", user_logueado);
				imprimir(0);
				enviar_a_cliente("1");
				intentos = 0;
				log = 1;
				break;
			}
			else if(respuesta[0] == '9') {
				char* nombre = strtok(buffer, "-");
				sprintf(impresion, "usuario bloqueado: %s\n", nombre);
				imprimir(0);
				enviar_a_cliente("9");
				intentos = 0;
				log = 0;
				break;
			}
		}

		if(log == 1) {
			while(salida == 0) {
				recepcion();
				parse(user_logueado);
			}
			salida = 0;
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
	serv_addr.sin_addr.s_addr = inet_addr(direccion);
	serv_addr.sin_port = htons( (uint16_t) puerto );
	if ( bind(sockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		sprintf(impresion, "error conectando socket\n");
		imprimir(1);
		exit(1);
	}
}

/**
 * Recibe datos y los guarda en buffer
 */
void recepcion() {
	memset( buffer, 0, BUFFER_SIZE );
	n = recv( socket_cliente, buffer, BUFFER_SIZE, 0 );
	if ( n < 0 ) {
		sprintf(impresion, "error leyendo de socket\n");
	  imprimir(1);
	  exit(1);
	}
}

/**
 * Envia datos por el socket
 */
void enviar_a_cliente(char* mensaje) {
	n = send( socket_cliente, mensaje, strlen(mensaje), 0 );
	if ( n < 0 ) {
		sprintf(impresion, "error enviando a cliente\n");
	  imprimir(1);
	  exit( 1 );
	}
}

/**
 * Crea el comando y la opcion recibidos
 * Reaccion a los mensajes recibidos
 * Funcion llamada despues de recepcion()
 */
void parse(char* usuario_logueado) {
	buffer[strlen(buffer)-1] = '\0';

	char* mensaje;
	char opcion[COMANDO_SIZE];
	char argumento[COMANDO_SIZE];
	char comando[COMANDO_SIZE];
	int32_t i = 0;

	mensaje = strtok(buffer, " ");
	sprintf(opcion, " ");
	sprintf(argumento, " ");

	while(mensaje != NULL) {
		if(i == 0) {
			sprintf(comando, "%s", mensaje);
			i++;
		}
		else if(i == 1) {
			sprintf(opcion, "%s", mensaje);
			i++;
		}
		else {
			sprintf(argumento, "%s", mensaje)	;
			break;
		}
		mensaje = strtok(NULL, " ");
	}

	sprintf(impresion, "%s - %s %s %s\n", usuario_logueado, comando, opcion, argumento);
	imprimir(0);

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
		enviar_a_cliente(	" Uso: user [opcion] <argumento>\n\n"
							"	- ls : listado de usuarios\n"
							"	- passwd <nueva contraseña> : cambio de contraseña");
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
		enviar_a_cliente("Clave invalida");
		return;
	}

	char* aux = "-";
	char* tmp = malloc(strlen(usuario) + strlen(clave) + strlen(aux));
	sprintf(tmp,"%s%s%s", usuario, aux, clave);
	enviar_a_cola_local((long) CAMBIAR_CLAVE_REQUEST, tmp, 'a');

	recibir_de_cola((long) CAMBIAR_CLAVE_RESPONSE, 'p');

	free(tmp);
	enviar_a_cliente("Clave cambiada con exito");
}

/**
 * Reaccion a comando file
 */
void file_command(char *opcion, char *argumento) {
	if( strcmp("ls", opcion) == 0 )
		file_ls();
	else if( strcmp("down", opcion) == 0 && strcmp(" ", argumento) != 0)
		file_down(argumento);
	else {
		enviar_a_cliente(	" Uso: file [opcion] <argumento>\n\n"
							"	- ls : listado de archivos\n"
							"	- down <archivo> : descarga de archivo");
	}
}

/**
 * Reaccion al comando file ls
 */
void file_ls() {
	enviar_a_cola_local((long) ARCHIVOS_REQUEST, "n", 'f');
	char* respuesta = recibir_de_cola(ARCHIVOS_RESPONSE, 'p').mtext;
	enviar_a_cliente(respuesta);
}

/**
 * Reaccion al comando file down
 */
void file_down(char* archivo_index) {
	enviar_a_cola_local((long) DESCARGA_REQUEST, archivo_index, 'f');
	char* respuesta = recibir_de_cola(DESCARGA_RESPONSE, 'p').mtext;
	enviar_a_cliente(respuesta);
}

/**
 * Reaccion a comando exit
 */
void exit_command(char* usuario) {
	sprintf(impresion, "%s ha salido\n", usuario);
	imprimir(0);
	salida = 1;
}

/**
 * Reaccion a comando desconocido
 */
void unknown_command() {
	enviar_a_cliente(	"Comando no reconocido\n"
						"Comandos aceptados:\n"
						"	- user\n"
						"	- file\n"
						"	- exit");
}

/**
 * Enviar mensaje a cola de proceso
 */
void enviar_a_cola_local(long id, char* mensaje, char proceso) {
	if(enviar_a_cola(id, mensaje, proceso) == -1) {
		sprintf(impresion, "error enviando mensaje\n");
		imprimir(1);
		exit(1);
	}
}

/**
 * Imprimir en consola
 */
void imprimir(int32_t error) {
		printf("\033[1;34m");
		if(error)
			fprintf(stderr, "PRIMARY_SERVER: %s", impresion );
		else
			printf("PRIMARY_SERVER: %s", impresion);
		fflush(stdout);
		printf("\033[0m");
}
