#include "../include/auth.h"

int32_t qid;
Usuario* usuarios[CANTIDAD_USUARIOS];
char impresion[BUFFER_SIZE];

/**
 * Funcion main
 */
int32_t main() {

	sprintf(impresion, "iniciando\n");
	imprimir(0);
	sprintf(impresion, "proceso: %d\n", getpid());
	imprimir(0);

	// levantar base de datos
	if(levantar_usuarios()) {
		sprintf(impresion, "error leyendo base de datos\n");
		imprimir(1);
		exit(1);
	}

	// creacion de cola
	qid = get_cola('a');
	if(qid == -1) {
		sprintf(impresion, "error creando la cola\n");
		imprimir(1);
		exit(1);
	}
	sprintf(impresion, "cola = %d\n", qid);
	imprimir(0);

	// empezar a escuchar mensajes en cola
  while(1) {

		// recibir cualquier tipo de mensajes
		struct msgbuf mensaje_str = recibir_de_cola((long) 0, 'a');

		// handler para login request
		if(mensaje_str.mtype == LOGIN_REQUEST) {
			sprintf(impresion, "login request: %s\n", mensaje_str.mtext);
			imprimir(0);
			int32_t log = login(mensaje_str.mtext);

			char rta[2];
			if(log == 0)
				sprintf(rta, "0");
			else if(log == 1)
				sprintf(rta, "1");
			else if(log == 9)
				sprintf(rta, "9");

			enviar_a_cola_local((long) LOGIN_RESPONSE, rta, 'p');
			sprintf(impresion, "login response (rta: %s)\n", rta);
			imprimir(0);
		}
		// handler para bloquear usuario
		else if(mensaje_str.mtype == BLOQUEAR_USUARIO) {
			sprintf(impresion, "bloquear usuario: %s\n", mensaje_str.mtext);
			imprimir(0);
			if(bloquear_usuario(mensaje_str.mtext) == -1) {
				sprintf(impresion, "error bloqueando usuario\n");
				imprimir(1);
				exit(1);
			}
		}
		// handler para nombre request
		else if(mensaje_str.mtype == NOMBRES_REQUEST) {
			sprintf(impresion, "nombres request\n");
			imprimir(0);

			char* primero = "[Usuario] - [Bloqueado] - [Ultima conexion]\n";
			size_t size = strlen(primero);
			char* guion = " - ";
			char* salto = "\n";
			char bloqueado[3] = "";
			for(int32_t i = 0; i < CANTIDAD_USUARIOS; i++) {
				size = size + strlen(usuarios[i]->nombre) + strlen(guion) * 2 +
				strlen(usuarios[i]->ultima_conexion) + strlen(bloqueado);
				if(i < CANTIDAD_USUARIOS - 1)
					size = size + strlen(salto);
			}

			char users_info[size];
			sprintf(users_info, "%s", primero);
			for(int32_t i = 0; i < CANTIDAD_USUARIOS; i++) {

				if(usuarios[i]->bloqueado[0] == '0')
					sprintf(bloqueado, "No");
				else
					sprintf(bloqueado, "Si");

				char tmp[strlen(users_info)];
				sprintf(tmp, "%s", users_info);
				sprintf(users_info, "%s%s%s%s%s%s",	tmp,
																						usuarios[i]->nombre,
																						guion,
																						bloqueado,
																						guion,
																						usuarios[i]->ultima_conexion);
				if(i < CANTIDAD_USUARIOS - 1)
					strcat(users_info, salto);
			}
			enviar_a_cola_local((long) NOMBRES_RESPONSE, users_info, 'p');
			sprintf(impresion, "nombres response\n");
			imprimir(0);
		}
		// handler para cambiar contraseña request
		else if(mensaje_str.mtype == CAMBIAR_CLAVE_REQUEST) {
			sprintf(impresion, "cambiar clave request\n");
			imprimir(0);
			cambiar_clave(mensaje_str.mtext);
			enviar_a_cola_local((long) CAMBIAR_CLAVE_RESPONSE, "n", 'p');
			sprintf(impresion, "cambiar clave response\n");
			imprimir(0);
		}
	}
	exit(0);
}

/**
 * Conexion a base de datos
 * Creacion de usuarios de acuerdo a lo que hay en resorces/auth_credentials/users_credentials
 */
int32_t levantar_usuarios() {

	char lineas[CANTIDAD_USUARIOS * 4][LINE_SIZE];
	FILE *file;
	file = fopen(CREDENTIALS_FILE_NAME, "r");

	if ( file != NULL ) {
      char line[LINE_SIZE];

			int32_t index = 0;
      while ( fgets( line, LINE_SIZE, file ) != NULL ) {
				sprintf(lineas[index], "%s", line);
				index++;
      }
      fclose ( file );
  }
	else
		return 1;

	for(int32_t i = 0; i < CANTIDAD_USUARIOS * 4;) {
		int32_t usuario_index = i / CANTIDAD_USUARIOS;
		usuarios[usuario_index] = malloc(sizeof(Usuario));

		sprintf(usuarios[usuario_index]->nombre, "%s", lineas[i++]);
		usuarios[usuario_index]->nombre[strlen(usuarios[usuario_index]->nombre) - 1] = '\0';
		sprintf(usuarios[usuario_index]->clave, "%s", lineas[i++]);
		usuarios[usuario_index]->clave[strlen(usuarios[usuario_index]->clave) - 1] = '\0';
		sprintf(usuarios[usuario_index]->bloqueado, "%s", lineas[i++]);
		usuarios[usuario_index]->bloqueado[strlen(usuarios[usuario_index]->bloqueado) - 1] = '\0';
		sprintf(usuarios[usuario_index]->ultima_conexion, "%s", lineas[i++]);
		usuarios[usuario_index]->ultima_conexion[strlen(usuarios[usuario_index]->ultima_conexion) - 1] = '\0';
	}

	return 0;
}

/**
 * Bloqueo de usuario
 * @param nombre_usuario nombre del usuario a bloquear
 * @return -1 en caso de error
 */
int32_t bloquear_usuario(char* nombre_usuario) {
	for(int32_t i = 0; i < CANTIDAD_USUARIOS; i++) {
		if( strcmp(nombre_usuario, usuarios[i]->nombre) == 0 ) {
			usuarios[i]->bloqueado[0] = '1';
			break;
		}
	}
	return refresh_datos();
}

/**
 * Cambio de clave de usuario
 * @param credenciales_nuevas "nombre_usuario-clave_nueva"
 * @return -1 en caso de error
 */
int32_t cambiar_clave(char* credenciales_nuevas) {
	char* aux = strtok(credenciales_nuevas, "-");
	char nombre[strlen(aux)];
	sprintf(nombre, "%s", aux);

	aux = strtok(NULL, "\0");
	char nueva_clave[strlen(aux)];
	sprintf(nueva_clave, "%s", aux);

	for(int32_t i = 0; i < CANTIDAD_USUARIOS; i++) {
		if( strcmp(nombre, usuarios[i]->nombre) == 0 ) {
			sprintf(usuarios[i]->clave, "%s", nueva_clave);
			sprintf(impresion, "nueva clave: %s\n", nueva_clave);
			imprimir(0);
			break;
		}
	}
	return refresh_datos();
}

/**
 * Actualizacion de ultima conexion al momento actual
 * Funcion llamada al logearse un usuario
 * @param usuario nombre de usuario recien conectado
 * @return -1 en caso de error
 */
int32_t set_ultima_conexion(char* usuario) {

	time_t t = time(NULL);
  struct tm tm = *localtime(&t);
	char nueva_ultima_conexion[USUARIO_ULTIMA_CONEXION_SIZE];
  sprintf(nueva_ultima_conexion, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
	 tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	for(int32_t i = 0; i < CANTIDAD_USUARIOS; i++) {
		if( strcmp(usuario, usuarios[i]->nombre) == 0 ) {
			sprintf(usuarios[i]->ultima_conexion, "%s", nueva_ultima_conexion);
			break;
		}
	}
	return refresh_datos();
}

/**
 * Proceso de login
 * Se chequea que las credenciales sean las correctas
 * @param credenciales "nombre_usuario-clave"
 * @return 0 para login fallido, 1 para login exitoso, 9 para usuario bloqueado
 */
int32_t login(char* credenciales) {

	char* login;

	login = strtok(credenciales, "-");
	char nombre[strlen(login)];
	sprintf(nombre, "%s", login);

	login = strtok(NULL, "-");
	char clave[strlen(login)];
	sprintf(clave, "%s", login);

	for(int32_t i = 0; i < CANTIDAD_USUARIOS	; i++) {
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
 * @return -1 en caso de error
 */
int32_t refresh_datos() {
	FILE *file;
	file = fopen(CREDENTIALS_FILE_NAME, "w+");

	if ( file != NULL ) {

			for(int32_t i = 0; i < CANTIDAD_USUARIOS; i++) {
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
 * @param id id de mensaje
 * @param mensaje mensaje a depositar
 * @param proceso 'p' para server, 'a' para auth, 'f' para file
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
 * @param error 1 en caso de imprimir error
 */
void imprimir(int32_t error) {
		printf("\033[1;31m");
		if(error)
			fprintf(stderr, "AUTH_SERVICE: %s", impresion );
		else
			printf("AUTH_SERVICE: %s", impresion);
		printf("\033[0m");
		fflush(stdout);
}
