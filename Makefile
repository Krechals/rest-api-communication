CC=g++
CFLAGS= -Wall -Wextra

all: client

run: client
	./client

client: client.cpp requests.o helpers.o buffer.o commands.o
	$(CC) $(CFLAGS) -o $@ $^

requests.o: requests.cpp
	$(CC) $(CFLAGS) -c $^

helpers.o: helpers.cpp
	$(CC) $(CFLAGS) -c $^

buffer.o: buffer.cpp
	$(CC) $(CFLAGS) -c $^

commands.o: commands.cpp
	$(CC) $(CFLAGS) -c $^

clean:
	rm  *.o client
