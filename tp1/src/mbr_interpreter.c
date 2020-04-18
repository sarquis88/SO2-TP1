#include "../include/mbr_interpreter.h"

Particion* particiones[CANTIDAD_PARTICIONES];

void start_mbr_analisis() {

	for(int32_t particion = 0; particion < CANTIDAD_PARTICIONES; particion++) {

		particiones[particion] = malloc(sizeof(Particion));
		particiones[particion]->index = particion;

		set_informacion(particion);
		set_bootable(particion);
		set_tipo(particion);
		set_inicio(particion);
		set_size(particion);
		set_final(particion);

		print_particion(particion);

		free(particiones[particion]);
	}


}

void set_informacion(int32_t particion) {
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

void set_bootable(int32_t particion) {
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

void set_tipo(int32_t particion) {
	particiones[particion]->tipo[0] = particiones[particion]->informacion[8];
	particiones[particion]->tipo[1] = particiones[particion]->informacion[9];
	particiones[particion]->tipo[2] = '\0';
}

void set_final(int32_t particion) {
	particiones[particion]->final = particiones[particion]->inicio + particiones[particion]->size - 1;
}

void set_inicio(int32_t particion) {
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

void set_size(int32_t particion) {
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
