# Sockets Makefile
#
# Written by Matthew Campbell.
#
# Do not define CC if you are using C4droid.
#
# Compiler to use:
#
#CC = gcc
#
# Compiler flags:
#
CFLAGS = -c -pedantic -std=c17 -Wall
#
# Linker flags:
#
LFLAGS = -lm
#
# Define the include/header file.
#
INC = sockets.h
#
# Define the source code files.
#
SRC = convert_endian.c \
      list_sockets.c \
      print_domain_menu.c \
      read_stdin.c \
      shutdown_sockets.c \
      setup_af_bluetooth.c \
      setup_af_inet.c \
      setup_af_inet6.c \
      setup_af_unix.c \
      setup_sockets.c \
      show_socket_options.c \
      sockets.c
#
# Define the object files.
#
OBJ = convert_endian.o \
      list_sockets.o \
      print_domain_menu.o \
      read_stdin.o \
      shutdown_sockets.o \
      setup_af_bluetooth.o \
      setup_af_inet.o \
      setup_af_inet6.o \
      setup_af_unix.o \
      setup_sockets.o \
      show_socket_options.o \
      sockets.o
#
# Define the default target.
#
all: sockets
#
# Define the sockets target.
#
sockets: objects
	@echo "Linking the object files with library files."
	@echo
	$(CC) $(LFLAGS) $(OBJ) -o sockets
	@echo
#
# Define the objects target.
#
objects: $(SRC) $(INC)
	@echo
	@echo "Compiling the source code files."
	@echo
	$(CC) $(CFLAGS) $(SRC)
	@echo
#
# Define the clean target.
#
clean:
	@echo
	@echo "Cleaning up."
	@echo
	rm -f *.o sockets
	@echo
#
# EOF
