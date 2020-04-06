FLAGS=-Wall -Werror -pedantic -Wextra -Wconversion -g
FLAGS=-Wall -Werror -g

make:
	gcc $(FLAGS) src/primary_server.c src/utilities.c -o bin/primary_server.o
	gcc $(FLAGS) src/auth_service.c src/utilities.c -o bin/auth_service.o
	gcc $(FLAGS) src/client.c src/utilities.c -o bin/client.o
clean:
	rm bin/primary_server.o
	rm bin/auth_service.o
	rm bin/client.o
