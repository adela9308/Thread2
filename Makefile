CC = gcc
CFLAGS = -Wall -g -ggdb

SRC = lib/thread.c lib/mutex.c lib/utility_methods.c 
OBJ = $(patsubst %.c,%.o,$(SRC))

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

thread_test: thread_test.o $(OBJ)
	$(CC) -o bin/thread_test thread_test.o $(OBJ)

mutex_test: mutex_test.o $(OBJ)
	$(CC) -o bin/mutex_test mutex_test.o $(OBJ)

deadlock_test: deadlock_test.o $(OBJ)
	$(CC) -o bin/deadlock_test deadlock_test.o $(OBJ)

clean:
	$(RM) -f *.o lib/*.o bin/*