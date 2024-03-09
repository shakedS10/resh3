CC = gcc
CFLAGS = -Wall -g

all: TCP_Receiver TCP_Sender

TCP_Receiver: TCP_Receiver.o 
	$(CC) $(CFLAGS) -o TCP_Receiver TCP_Receiver.o

TCP_Sender: TCP_Sender.o 
	$(CC) $(CFLAGS) -o TCP_Sender TCP_Sender.o

TCP_Receiver.o: TCP_Receiver.c MAXSIZES.h
	$(CC) $(CFLAGS) -c TCP_Receiver.c

TCP_Sender.o: TCP_Sender.c MAXSIZES.h
	$(CC) $(CFLAGS) -c TCP_Sender.c

.PHONY: clean all

clean:
	rm -f *.o TCP_Receiver TCP_Sender
