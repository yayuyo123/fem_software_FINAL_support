CFLAGS = -Wall -I../include
CC = gcc

main: main.c print_ffi.c
	$(CC) $(CFLAGS) -o ../frcs main.o print_ffi.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

print_ffi.o: print_ffi.c
	$(CC) $(CFLAGS) -c print_ffi.c