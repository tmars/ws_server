# компилятор
CC=gcc
CFLAGS=-std=c99
LD=$(CC)
LDFLAGS=

# директории
SRC=src
BIN=bin
OBJ=obj
TOOLS=tools
SRC_SUB = . lib

# скрипт проверки кода
CPPLINT=$(TOOLS)/cpplint/cpplint.py

OBJ_DIRS=$(addprefix $(OBJ)/, $(SRC_SUB))
SRC_DIRS=$(addprefix $(SRC)/, $(SRC_SUB))

SOURCES=$(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
OBJECTS=$(SOURCES:.c=.o)

PROGRAM=$(BIN)/server

all: $(PROGRAM)

$(PROGRAM): mk_dirs $(OBJECTS)
	$(LD) -o $@ $(patsubst $(SRC)/%, $(OBJ)/%, $(OBJECTS)) $(LDFLAGS)

mk_dirs:
	mkdir -p $(OBJ_DIRS) $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $(patsubst $(SRC)/%, $(OBJ)/%, $@)

run:
	$(PROGRAM)

clean:
	rm -rf $(OBJ) $(BIN)

test_style: $(wildcard $(SRC)/*.h) $(wildcard $(SRC)/*.c)
	$(shell python $(CPPLINT) --root=$(SRC) $^)