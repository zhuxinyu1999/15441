# 
#	Makefile for 15441 project1
# http/https server
# Zhu Xinyu <zzxy1999@mail.ustc.edu.cn>
#

CC = gcc
CFLAGS = -g
LFLAGS = -lssl
OBJECTS = server.o util.o common.o pool.o httpio.o http.o request.o header.o tls.o sig.o

default: server

server: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LFLAGS) -o server

%.o: %.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm $(wildcard *.o)
	rm server
