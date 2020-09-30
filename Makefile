OBJS	= cumunisp.o mpc.o
SOURCE	= cumunisp.c mpc.c
HEADER	= mpc.h
OUT	= cumunisp
CC	 = gcc
FLAGS	 = -std=c99 -g -c -Wall -Wextra -pedantic
LFLAGS	 = -lm -ledit


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)


cumunisp.o: cumunisp.c
	$(CC) $(FLAGS) cumunisp.c -std=c99

mpc.o: mpc.c
	$(CC) $(FLAGS) mpc.c


clean:
	rm -f $(OBJS) $(OUT)
