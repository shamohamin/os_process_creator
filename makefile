CC      := gcc
MKDIR   := mkdir
BIN     := ./bin
SRC     := ./src
OBJ     := ./obj
CFILES  := $(wildcard $(SRC)/*.c)
OBJS    := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(CFILES))
OUTFILE := $(BIN)/main.out
HEADERS := $(wildcard $(SRC)/*.h)
OPTIONS = c99

$(OUTFILE): $(OBJS) | $(BIN)
	$(CC) -I $(HEADERS) $^ -o $@ -std=$(OPTIONS)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) -c $< -o $@ -std=$(OPTIONS)

$(BIN) $(OBJ):
	$(MKDIR) $@

compile: $(OUTFILE)
    $<

run:
	$(MAKE) $(compile) && $(OUTFILE)

clean:
	rm -rf $(OBJ) $(BIN)