PROJECT := cserv
CC=gcc
SRC := $(shell find . -maxdepth 2 -name "*.c")
OBJ = $(patsubst %.c, %.o, $(SRC))

$(PROJECT): $(OBJ)
	$(CC) $(LDFLAGS) -o main $(OBJ) $(LDLIBS)

clean:
	rm -f $(OBJ)