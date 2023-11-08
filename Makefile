# login: xvlach24

CC=gcc
CFLAGS= 
# -std=gnu99 -Wall -Wextra -Werror -pedantic -lrt -pthread

all: tftp

tftp:
	$(CC) $(CFLAGS) ./src/tftp-server.c -o ./tftp-server
	$(CC) $(CFLAGS) ./src/tftp-client.c -o ./tftp-client

clean:
	rm -f tftp-client
	rm -f tftp-server