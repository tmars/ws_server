# компилятор
CC=gcc
CFLAGS=-std=c99
LD=$(CC)
LDFLAGS=

# директории
SRC=src
TEST=test
BIN=bin
OBJ=obj
TOOLS=tools
SRC_SUB = . lib

# скрипт проверки кода
CPPLINT=$(TOOLS)/cpplint/cpplint.py

OBJ_DIRS=$(addprefix $(OBJ)/, $(SRC_SUB) $(TEST))
SRC_DIRS=$(addprefix $(SRC)/, $(SRC_SUB))

SOURCES=$(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
OBJECTS=$(patsubst $(SRC)/%, $(OBJ)/%, $(SOURCES:.c=.o))

TEST_SOURCES=$(wildcard $(addsuffix /*.c, $(TEST)))
TEST_OBJECTS=$(addprefix $(OBJ)/, $(TEST_SOURCES:.c=.o))

PROGRAM=$(BIN)/server
TEST_CC=$(LD) $(LDFLAGS) -o $@ $^

all: $(PROGRAM)

$(PROGRAM): mk_dirs $(OBJECTS)
	$(LD) $(LDFLAGS) $(patsubst $(SRC)/%, $(OBJ)/%, $(OBJECTS)) -o $@

mk_dirs:
	mkdir -p $(OBJ_DIRS) $(BIN)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $^ -c -o $@

run:
	$(PROGRAM)

clean:
	rm -rf $(OBJ) $(BIN)

test_style: $(wildcard $(SRC)/*.h) $(wildcard $(SRC)/*.c)
	$(shell python $(CPPLINT) --root=$(SRC) $^)
	@echo Style test: OK!

# cборка объектного файла теста
$(OBJ)/$(TEST)/%.o: $(TEST)/%.c
	$(CC) $(CFLAGS) $^ -c -o $@ -I$(SRC)

# cборка испольняемого файла теста
$(BIN)/test_frame: $(OBJ)/$(TEST)/test_frame.o $(OBJ)/frame.o
	$(TEST_CC)

$(BIN)/test_websocket: $(OBJ)/$(TEST)/test_websocket.o $(OBJ)/websocket.o $(OBJ)/http.o $(OBJ)/lib/sha1.o $(OBJ)/lib/base64.o
	$(TEST_CC)

$(BIN)/test_http: $(OBJ)/$(TEST)/test_http.o $(OBJ)/http.o
	$(TEST_CC)

# вызов модульных тестов
test_units: mk_dirs $(BIN)/test_frame $(BIN)/test_websocket $(BIN)/test_http
	$(BIN)/test_frame
	$(BIN)/test_websocket
	$(BIN)/test_http
	@echo Unit tests: OK!

tests: test_style test_units