# NetSpeed Makefile compile and link sources and gui files

NAME=netspeed
GTKFLAGS=`pkg-config --cflags --libs gtk+-2.0` 
SRCS=main.c gui.o
CC=g++

all: main


main:
	ld -r -b binary -o gui.o gui.glade
	$(CC) $(SRCS) -o $(NAME) $(GTKFLAGS)

clean:
	rm -r $(NAME)
	rm -r gui.o

install:
	cp $(NAME) /usr/bin/$(NAME)

remove:
	rm -r /usr/bin/$(NAME)

