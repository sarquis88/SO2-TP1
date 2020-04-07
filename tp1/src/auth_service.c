#include "../include/auth_service.h"

// definicion de variables
int32_t qid, sockfd, servlen;
struct sockaddr_un serv_addr;
Usuario* usuarios[CANT_USUARIOS];

/**
 * Funcion main
 */
int32_t main() {

	// configuracion de socket
	configurar_socket();

	// levantar base de datos
	if(conectar()) {
		perror("	AUTH_SERVICE: error leyendo base de datos: ");
		exit(1);
	}

	// creacion de cola
	qid = get_cola('a');
	if(qid == -1) {
		perror("	AUTH_SERVICE: error creando cola: ");
		exit(1);
	}
	printf("	AUTH_SERVICE: cola = %d\n", qid);
	fflush(stdout);

	// empezar a escuchar mensajes en cola y por socket
  listen( sockfd, 5 );
  while(1) {

		// recibir cualquier tipo de mensajes
		struct msgbuf mensaje_str = recibir_de_cola((long) 0, 'a');

		// handler para login request
		if(mensaje_str.mtype == LOGIN_REQUEST) {
			printf("	AUTH_SERVICE: login request: %s\n", mensaje_str.mtext);
			fflush(stdout);
			int32_t log = login(mensaje_str.mtext);

			char rta[2];
			if(log == 0)
				strcpy(rta, "0");
			else if(log == 1)
				strcpy(rta, "1");
			else if(log == 9)
				strcpy(rta, "9");

			enviar_a_cola_local((long) LOGIN_RESPONSE, rta, 'p');
			printf("	AUTH_SERVICE: login response (rta: %s)\n", rta);
			fflush(stdout);
		}
		// handler para bloquear usuario
		else if(mensaje_str.mtype == BLOQUEAR_USUARIO) {
			printf("	AUTH_SERVICE: bloquear usuario: %s\n", mensaje_str.mtext);
			fflush(stdout);
			if(bloquear_usuario(mensaje_str.mtext) == -1) {
					perror("	AUTH_SERVICE: error bloquendo usuario: ");
					exit(1);
			}
		}
		// handler para nombre request
		else if(mensaje_str.mtype == NOMBRES_REQUEST) {
			printf("	AUTH_SERVICE: nombres request\n");
			fflush(stdout);

			char* primero = "\n[Usuario] - [Ultima conexion]\n";
			int32_t size = strlen(primero);
			char* aux = " - ";
			char* salto = "\n";
			for(int32_t i = 0; i < CANT_USUARIOS; i++)
				size = size + strlen(usuarios[i]->nombre) + strlen(aux) + strlen(usuarios[i]->ultima_conexion) + strlen(salto);

			char users_info[size];
			strcpy(users_info, "\0");

			strcat(users_info, primero);
			for(int32_t i = 0; i < CANT_USUARIOS; i++) {
				strcat(users_info, usuarios[i]->nombre);
				strcat(users_info, aux);
				strcat(users_info, usuarios[i]->ultima_conexion);
				strcat(users_info, salto);
			}
			enviar_a_cola_local((long) NOMBRES_RESPONSE, users_info, 'p');
			printf("	AUTH_SERVICE: nombres response\n");
			fflush(stdout);
		}
		// handler para cambiar contraseña request
		else if(mensaje_str.mtype == CAMBIAR_CLAVE_REQUEST) {
			printf("	AUTH_SERVICE: cambiar clave request\n");
			fflush(stdout);

			cambiar_clave(mensaje_str.mtext);
			enviar_a_cola_local((long) CAMBIAR_CLAVE_RESPONSE, "n", 'p');
			printf("	AUTH_SERVICE: cambiar clave response\n");
			fflush(stdout);
		}
	}
	exit(0);
}

/**
 * Levantamiento de socket
 */
void configurar_socket() {
	// creacion de socket
	if ( ( sockfd = socket( AF_UNIX, SOCK_STREAM, 0) ) < 0 ) {
    perror( "Creación de  socket");
    exit(1);
  }

	// Remover el nombre de archivo si existe
	unlink ( SOCKET_PATH );

	// configuracion de socket
	memset( &serv_addr, 0, sizeof(serv_addr) );
  serv_addr.sun_family = AF_UNIX;
  strcpy( serv_addr.sun_path, SOCKET_PATH );
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	// conexion de socket
  if( bind( sockfd,(struct sockaddr *)&serv_addr,servlen )<0 ) {
    perror( "	AUTH_SERVICE: ligadura" );
    exit(1);
  }
	else {
		printf("	AUTH_SERVICE: iniciando\n");
		printf("	AUTH_SERVICE: proceso: %d - socket: %s\n", getpid(), serv_addr.sun_path);
		fflush(stdout);
	}
}

/**
 * Conexion a base de datos
 * Creacion de usuarios
 */
int32_t conectar() {

	char lineas[CANT_USUARIOS * 4][LINE_SIZE];
	FILE *file;
	file = fopen(CREDENTIALS_FILE_NAME, "r");

	if ( file != NULL ) {
      char line[LINE_SIZE];

			int32_t index = 0;
      while ( fgets( line, LINE_SIZE, file ) != NULL ) {
				strcpy(lineas[index], line);
				index++;
      }
      fclose ( file );
  }
	else
		return 1;

	for(int32_t i = 0; i < CANT_USUARIOS * 4;) {
		int32_t usuario_index = i / CANT_USUARIOS;
		usuarios[usuario_index] = malloc(sizeof(Usuario));

		strcpy(usuarios[usuario_index]->nombre, lineas[i++]);
		usuarios[usuario_index]->nombre[strlen(usuarios[usuario_index]->nombre) - 1] = '\0';
		strcpy(usuarios[usuario_index]->clave, lineas[i++]);
		usuarios[usuario_index]->clave[strlen(usuarios[usuario_index]->clave) - 1] = '\0';
		strcpy(usuarios[usuario_index]->bloqueado, lineas[i++]);
		usuarios[usuario_index]->bloqueado[strlen(usuarios[usuario_index]->bloqueado) - 1] = '\0';
		strcpy(usuarios[usuario_index]->ultima_conexion, lineas[i++]);
		usuarios[usuario_index]->ultima_conexion[strlen(usuarios[usuario_index]->ultima_conexion) - 1] = '\0';
	}

	return 0;
}

/**
 * Bloqueo de usuario
 */
int32_t bloquear_usuario(char* nombre_usuario) {
	for(int32_t i = 0; i < CANT_USUARIOS; i++) {
		if( strcmp(nombre_usuario, usuarios[i]->nombre) == 0 ) {
			usuarios[i]->bloqueado[0] = '1';
			break;
		}
	}
	return refresh_datos();
}

/**
 * Cambio de clave a usuario
 * Formato de credenciales_nuevas: "nombre-nueva_clave"
 */
int32_t cambiar_clave(char* credenciales_nuevas) {

	char* aux = strtok(credenciales_nuevas, "-");
	char nombre[strlen(aux)];
	strcpy(nombre, aux);

	aux = strtok(NULL, "\0");
	char nueva_clave[strlen(aux)];
	strcpy(nueva_clave, aux);

	for(int32_t i = 0; i < CANT_USUARIOS; i++) {
		if( strcmp(nombre, usuarios[i]->nombre) == 0 ) {
			strcpy(usuarios[i]->clave, nueva_clave);
			break;
		}
	}
	return refresh_datos();
}

/**
 * Actualizacion de ultima conexion
 * Funcion llamada al logearse un usuario
 */
int32_t set_ultima_conexion(char* usuario) {

	time_t t = time(NULL);
  struct tm tm = *localtime(&t);
	char nueva_ultima_conexion[USUARIO_ULTIMA_CONEXION_SIZE];
  sprintf(nueva_ultima_conexion, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
	 tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	for(int32_t i = 0; i < CANT_USUARIOS; i++) {
		if( strcmp(usuario, usuarios[i]->nombre) == 0 ) {
			strcpy(usuarios[i]->ultima_conexion, nueva_ultima_conexion);
			break;
		}
	}
	return refresh_datos();
}

/**
 * Proceso de login
 * Se chequea que las credenciales sean las correctas
 * Retorna:	0 -> login fallido
 *					1 -> login exitoso
 *					9 -> usuario bloqueado
 */
int32_t login(char* credenciales) {

	char* login;

	login = strtok(credenciales, "-");
	char nombre[strlen(login)];
	strcpy(nombre, login);

	login = strtok(NULL, "-");
	char clave[strlen(login)];
	strcpy(clave, login);

	for(int32_t i = 0; i < CANT_USUARIOS	; i++) {
		if( strcmp(nombre, usuarios[i]->nombre) == 0 ) {
			if( strcmp(clave, usuarios[i]->clave) == 0 ) {
				if( strcmp(usuarios[i]->bloqueado, "0") == 0 ) {
					set_ultima_conexion(nombre);
					return 1;
				}
				else
					return 9;
			}
		}
	}
	return 0;
}

/**
 * Escritura de datos en base de datos
 * Funcion llamada al cambiar algun dato
 */
int32_t refresh_datos() {
	FILE *file;
	file = fopen(CREDENTIALS_FILE_NAME, "w+");

	if ( file != NULL ) {

			for(int32_t i = 0; i < CANT_USUARIOS; i++) {
				fputs(usuarios[i]->nombre, file);
				fputs("\n", file);
				fputs(usuarios[i]->clave, file);
				fputs("\n", file);
				fputs(usuarios[i]->bloqueado, file);
				fputs("\n", file);
				fputs(usuarios[i]->ultima_conexion, file);
				fputs("\n", file);
			}
      fclose ( file );
			return 0;
  }
	else
		return -1;

	return 0;
}

/**
 * Enviar mensaje a cola de mensaje
 */
void enviar_a_cola_local(long id, char* mensaje, char proceso) {
	if(enviar_a_cola(id, mensaje, proceso) == -1) {
		perror("auth_service - enviando mensaje: ");
		exit(1);
	}
}
