CC=gcc
CFLAGS=-Wall -ansi -pedantic -std=c99
LDFLAGS=-Wall -ansi -pedantic -std=c99

SOURCES=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SOURCES))

ifneq ($(DEBUG),)
CFLAGS += -g
endif

ifneq ($(ERR),)
CFLAGS += -Werror
endif

BIN=Assembler

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	-@rm $(OBJS) $(BIN)

.PHONY: clean
