CC=g++
CFLAGS= -Wall -Wextra

all: client

run: client
	./client

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm  *.o client
