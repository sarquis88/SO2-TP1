FLAGS=-Wall -Werror -pedantic -Wextra -Wconversion -g
FLAGS=-Wall -Werror -pedantic -Wextra -lcrypto -g

make:
	gcc $(FLAGS) src/primary_server.c src/utilities.c -o bin/primary_server.o
	gcc $(FLAGS) src/auth_service.c src/utilities.c -o bin/auth_service.o
	gcc $(FLAGS) src/files_service.c src/utilities.c -o bin/files_service.o
	gcc $(FLAGS) src/client.c src/utilities.c -o bin/client.o
	gcc $(FLAGS) src/launch.c -o bin/launch.o
clean:
	rm bin/primary_server.o
	rm bin/auth_service.o
	rm bin/files_service.o
	rm bin/client.o
	rm bin/launch.o
