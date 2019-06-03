CC = gcc
CFLAGS = -MMD -MP -Wall

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
DEP=$(SRC:.c=.d)

uinputchars: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

-include $(DEP)

.PHONY: clean
clean:
	rm -f $(OBJ) $(DEP) uinputchars
