FLAGS= -std=gnu11 -Wall -Werror -pedantic -Wextra -lcrypto -Wconversion -g

make:
	cppcheck ./
	gcc $(FLAGS) src/server.c src/utilities.c -o bin/server
	gcc $(FLAGS) src/auth.c src/utilities.c -o bin/auth
	gcc $(FLAGS) src/fileserv.c src/utilities.c -o bin/fileserv
	gcc $(FLAGS) src/launch.c -o bin/launch
	gcc $(FLAGS) src/client.c src/utilities.c -o bin/client
	gcc $(FLAGS) src/launch.c -o bin/launch
clean:
	rm bin/*
	rm -r html/ latex/
	rm resources/files/*
docs:
	doxygen Doxyfile
