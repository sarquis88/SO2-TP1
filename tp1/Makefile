FLAGS= -std=gnu11 -Wall -Werror -pedantic -Wextra -lcrypto -O1 -Wconversion -g

make:
	cppcheck ./
	gcc $(FLAGS) src/server.c src/utilities.c -o bin/server
	gcc $(FLAGS) src/auth.c src/utilities.c -o bin/auth
	gcc $(FLAGS) src/fileserv.c src/utilities.c -o bin/fileserv
	gcc $(FLAGS) src/client.c src/utilities.c -o bin/client
	gcc $(FLAGS) src/launch.c -o bin/launch
clean:
	rm bin/server
	rm bin/auth
	rm bin/fileserv
	rm bin/client
	rm bin/launch
	rm -r html/ latex/
	rm resources/files/*
docs:
	doxygen Doxyfile
