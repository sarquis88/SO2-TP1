#include "../include/files_service.h"

// definicion de variables
int32_t qid, sockfd, servlen;
struct sockaddr_un serv_addr;
Archivo* archivos[CANT_ARCHIVOS];
char impresion[BUFFER_SIZE];

/**
 * Funcion main
 */
int32_t main() {

	// configuracion de socket
	configurar_socket();

	// levantar base de datos
	if(conectar()) {
		sprintf(impresion, "error leyendo archivos\n");
		imprimir(1);
		exit(1);
	}

	// creacion de cola
	qid = get_cola('f');
	if(qid == -1) {
		sprintf(impresion, "error creando cola\n");
		imprimir(1);
		exit(1);
	}
	sprintf(impresion, "cola = %d\n", qid);
	imprimir(0);

	// empezar a escuchar mensajes en cola y por socket
  listen( sockfd, 5 );
  while(1) {

		// recibir cualquier tipo de mensajes
		struct msgbuf mensaje_str = recibir_de_cola((long) 0, 'f');

    // handler para archivos request
		if(mensaje_str.mtype == ARCHIVOS_REQUEST) {
			sprintf(impresion, "archivos request\n");
			imprimir(0);

      char* primero = "\n[Indice] - [Nombre de archivos] - [Formato]\n";
      int32_t size = strlen(primero);
      char* salto = "\n";
      char* guion = " - ";
      char index[2];
      for(int32_t i = 0; i < CANT_ARCHIVOS; i++)
        size = size + strlen(index) + strlen(guion) * 2 +
        strlen(archivos[i]->nombre) + strlen(archivos[i]->formato) + strlen(salto);

      char files[size];
      strcpy(files, "\0");

      strcat(files, primero);
      for(int32_t i = 0; i < CANT_ARCHIVOS; i++) {
        sprintf(index, "%d", archivos[i]->index);
        strcat(files, index);
        strcat(files, guion);
        strcat(files, archivos[i]->nombre);
        strcat(files, guion);
        strcat(files, archivos[i]->formato);
        strcat(files, salto);
      }

			enviar_a_cola_local((long) ARCHIVOS_RESPONSE, files, 'p');
			sprintf(impresion, "archivos response\n");
			imprimir(0);
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
		sprintf(impresion, "error creando socket\n");
		imprimir(1);
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
		sprintf(impresion, "error conectando socket\n");
		imprimir(1);
    exit(1);
  }
	else {
		sprintf(impresion, "iniciando\n");
		imprimir(0);
		sprintf(impresion, "proceso: %d - socket: %s\n", getpid(), serv_addr.sun_path);
		imprimir(0);
	}
}

/**
 * Conexion a base de datos
 * Creacion de usuarios
 */
int32_t conectar() {

  DIR *dir;
  struct dirent *ent;
  int32_t index = 0;
  if ((dir = opendir (FILES_DIR_NAME)) != NULL) {
    while ((ent = readdir (dir)) != NULL) {
      if( ent->d_name[0] != '.' ) {

        archivos[index] = malloc(sizeof(Archivo));

        archivos[index]->index = index;
    		strcpy(archivos[index]->nombre, strtok(ent->d_name, "."));
    		archivos[index]->nombre[strlen(archivos[index]->nombre)] = '\0';
    		strcpy(archivos[index]->formato, strtok(NULL, "."));
    		archivos[index]->formato[strlen(archivos[index]->formato)] = '\0';
        index++;
      }
    }
    closedir (dir);
  }
  else {
		sprintf(impresion, "error creando archivos\n");
		imprimir(1);
    return EXIT_FAILURE;
  }
	return 0;
}

/**
 * Enviar mensaje a cola de mensaje
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
		if(error) {
			fprintf(stderr, "		FILES_SERVICE: %s", impresion );
		}
		else {
			printf("		FILES_SERVICE: %s", impresion);
		}
		fflush(stdout);
		strcpy(impresion, "\0");
}
