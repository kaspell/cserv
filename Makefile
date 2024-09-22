EXEC := main
CC := gcc
SRC := $(shell find . -maxdepth 2 -name "*.c")
OBJ = $(patsubst %.c, %.o, $(SRC))

$(EXEC): $(OBJ)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJ) $(LDLIBS)

clean:
	rm -f $(OBJ)