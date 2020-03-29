#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define TAM 256

#define PASSWORD_SIZE 15
#define USER_SIZE 15
#define LINE_SIZE 33

#define LIMITE_INTENTOS 3
