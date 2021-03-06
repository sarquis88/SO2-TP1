#include "../include/utilities.h"

Particion* particiones[CANTIDAD_PARTICIONES];

/**
 * Creacion de cola
 * @return id cola, -1 para error
 */
int32_t get_cola() {

  key_t qkey;
  qkey = ftok(QUEUE_FILE_NAME, PROJ_ID);

  if (qkey == -1) {
    perror("error bteniendo token");
    exit(1);
  }

  return msgget(qkey, 0666 | IPC_CREAT);
}

/**
 * Envio de mensaje a cola
 * @param id_mensaje tipo del mensaje
 * @param mensaje mensaje a enviar
 */
int32_t enviar_a_cola(long id_mensaje, char mensaje[QUEUE_MESAGE_SIZE]) {

  if(strlen(mensaje) > QUEUE_MESAGE_SIZE) {
    perror("error, mensaje muy grande\n");
    exit(1);
  }
  struct msgbuf mensaje_str;
  mensaje_str.mtype = id_mensaje;
  sprintf(mensaje_str.mtext, "%s", mensaje);

  return msgsnd(get_cola(), &mensaje_str, sizeof mensaje_str.mtext, 0 );
}

/**
 * Recepcion de mensaje de la cola
 * @id_mensaje para tipo de mensaje a recibir
 * @flag flag para recepcion
 * @return mensaje de cola
 */
char* recibir_de_cola(long id_mensaje, int32_t flag) {
  errno = 0;
  struct msgbuf mensaje_str = {id_mensaje, {0}};

  if(msgrcv(get_cola(), &mensaje_str, sizeof mensaje_str.mtext, id_mensaje, flag) == -1) {
      if(errno != ENOMSG) {
        perror("error recibiendo mensaje de cola");
        exit(1);
      }
  }
  char* mensaje = malloc(strlen(mensaje_str.mtext));
  sprintf(mensaje, mensaje_str.mtext);
  return mensaje;
}

/**
 * Retona el hash md5 del archivo ingresado
 * @param file_path direccion del archivo
 * @param tamaño del archivo, si no es necesario ingresar 0
 */
char* get_md5(char* file_path, ssize_t fin) {

    sync();

		FILE* file = fopen(file_path, "rb");	// rb para archivos de no-texto;
    char* buffer[BUFFER_SIZE];
    size_t n;
    unsigned char c[MD5_DIGEST_LENGTH];
		MD5_CTX mdContext;

    MD5_Init (&mdContext);

    if(fin == 0) {
		    while (( n = fread (buffer, sizeof(char), sizeof(buffer), file) ) != 0)
          MD5_Update (&mdContext, buffer, n);
    }
    else {
      ssize_t acumulado = 0;
      while ( acumulado < fin ) {
        n = fread (buffer, sizeof(char), sizeof(buffer), file);
        MD5_Update (&mdContext, buffer, n);
        acumulado = acumulado + (ssize_t) n;
      }
    }
    MD5_Final (c,&mdContext);

		fclose(file);

    char* md5string = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    if(md5string == NULL) {
			perror("error alocando memoria en md5string\n");
			exit(1);
		}
    for (int32_t i = 0; i < MD5_DIGEST_LENGTH; ++i)
      sprintf(&md5string[i * 2], "%02x", (unsigned int)c[i]);

    md5string[strlen(md5string)] = '\0';

		return md5string;
}

/**
 * Comienzo de analisis de tabla MBR en PATH_USB
 */
void start_mbr_analisis() {

	for(int32_t particion = 0; particion < CANTIDAD_PARTICIONES; particion++) {

		particiones[particion] = malloc(sizeof(Particion));
    if(particiones[particion] == NULL) {
			perror("error alocando memoria en Particion\n");
			exit(1);
		}
		particiones[particion]->index = particion;

		set_mbr_informacion(particion);
		set_mbr_bootable(particion);
		set_mbr_tipo(particion);
		set_mbr_inicio(particion);
		set_mbr_size(particion);
		set_mbr_final(particion);

		print_particion(particion);

		free(particiones[particion]);
	}


}

/**
 * Seteo de bits de informacion a particion
 * @param particion numero de particion
 */
void set_mbr_informacion(int32_t particion) {
	sync();

	FILE* file = fopen( PATH_USB, "rb" );

	fseek(file, 0L, SEEK_SET);
	fseek(file, 446 + 16 * particion, SEEK_CUR);	// comienzo de particionado

	if(file != NULL) {
		char buf[BUFFER_SIZE];
		int32_t pos = 0;
		char aux[3];
		while( fread(buf, sizeof(char), 1, file) > 0 ) {
			sprintf(aux, "%02x", buf[0] & 0xff);
			strcat(particiones[particion]->informacion, aux);
			pos++;
			if(pos == 16)
				break;
		}
		fclose(file);
		particiones[particion]->informacion[strlen(particiones[particion]->informacion)] = '\0';
	}
}

/**
 * Seteo de booteable a particion
 * @param particion numero de particion
 */
void set_mbr_bootable(int32_t particion) {
	char bootable[3];

	bootable[0] = particiones[particion]->informacion[0];
	bootable[1] = particiones[particion]->informacion[1];
	bootable[2] = '\0';

	char boot[3];
	if( strcmp(bootable, "80") == 0 )
		sprintf(boot, "Si");
	else if( strcmp(bootable, "00") == 0 )
		sprintf(boot, "No");
	else
		sprintf(boot, "Er");

	sprintf(particiones[particion]->booteable, "%s", boot);
}

/**
 * Seteo de tipo a particion
 * @param particion numero de particion
 */
void set_mbr_tipo(int32_t particion) {
	particiones[particion]->tipo[0] = particiones[particion]->informacion[8];
	particiones[particion]->tipo[1] = particiones[particion]->informacion[9];
	particiones[particion]->tipo[2] = '\0';
}

/**
 * Seteo de final a particion
 * @param particion numero de particion
 */
void set_mbr_final(int32_t particion) {
	particiones[particion]->final = particiones[particion]->inicio +
   (particiones[particion]->size) * 2048 - 1;
}

/**
 * Seteo de inicio a particion
 * @param particion numero de particion
 */
void set_mbr_inicio(int32_t particion) {
	char inicio[9];

	inicio[0] = particiones[particion]->informacion[22];
	inicio[1] = particiones[particion]->informacion[23];
	inicio[2] = particiones[particion]->informacion[20];
	inicio[3] = particiones[particion]->informacion[21];
	inicio[4] = particiones[particion]->informacion[18];
	inicio[5] = particiones[particion]->informacion[19];
	inicio[6] = particiones[particion]->informacion[16];
	inicio[7] = particiones[particion]->informacion[17];
	inicio[8] = '\0';

	particiones[particion]->inicio = (int32_t) strtol(inicio, NULL, 16);
}

/**
 * Seteo de tamaño a particion
 * @param particion numero de particion
 */
void set_mbr_size(int32_t particion) {
	char size[9];

	size[0] = particiones[particion]->informacion[30];
	size[1] = particiones[particion]->informacion[31];
	size[2] = particiones[particion]->informacion[28];
	size[3] = particiones[particion]->informacion[29];
	size[4] = particiones[particion]->informacion[26];
	size[5] = particiones[particion]->informacion[27];
	size[6] = particiones[particion]->informacion[24];
	size[7] = particiones[particion]->informacion[25];
	size[8] = '\0';

	particiones[particion]->size = (int32_t) strtol(size, NULL, 16) / 2048;

}

/**
 * Impresion en consola de informacion acerca de particion
 * @param particion numero de particion
 */
void print_particion(int32_t particion) {

	if(particiones[particion]->size > 0) {
		printf("--------> Informacion acerca de particion %d <--------\n", particion + 1);
		int32_t c = 0;
		int32_t d = 0;
		for(size_t i = 0; i < strlen(particiones[particion]->informacion); i++) {
			printf("%c", particiones[particion]->informacion[i]);
			c++;
			d++;
			if(c == 2) {
				printf(" ");
				c = 0;
				if(d == 16)
					printf("   ");
			}
		}
		printf("\n");
		printf("Particion booteable: %s\n", particiones[particion]->booteable);
		printf("Tipo de particion: %s\n", particiones[particion]->tipo);
		printf("Inicio de particion: %d\n", particiones[particion]->inicio);
		printf("Final de particion: %d\n", particiones[particion]->final);
		printf("Tamaño de particion: %d [MB]\n", particiones[particion]->size);
	}
}

/**
 * Duerme al hilo durante NSEC_TIME + SEC_TIME
 */
void dormir() {
  struct timespec time;
  time.tv_sec = SEC_TIME;
  time.tv_nsec = NSEC_TIME;
  if(nanosleep(&time ,NULL) < 0)
    perror("error durmiendo");
}
