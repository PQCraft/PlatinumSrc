ifndef OS
BIN := psrc
else
BIN := psrc.exe
endif

PBINFLAGS := -L. -L./src/lib -Llib -g -Ofast
BINFLAGS := -lpthread -lm -lSDL2 -lSDL2_mixer
ifndef OS
BINFLAGS := $(BINFLAGS) -lglfw -ldl -lGL -lassimp
else
BINFLAGS := $(BINFLAGS) -lmingw32 -lSDL2main -lopengl32
endif
POBJFLAGS := -Wall -Wextra -I. -I./src/include -Ilib -g -Ofast
OBJFLAGS := 

SRC := src
OBJ := obj

SOURCES := $(wildcard $(SRC)/*.c)
DEPENDS := $(wildcard $(SRC)/*.h)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

ifdef OS
CC = gcc
endif

.PHONY: all build run clean

all: run

build: $(BIN)

run: $(BIN)
	@echo "Running binary \"./$<\""
	@./$<
	@echo "\"./$<\" exited successfully"

$(BIN): $(OBJECTS)
	@echo "Building binary \"$@\" from \"$^\""
	@$(CC) -o $@ $(PBINFLAGS) $^ $(BINFLAGS)
	@echo "Built binary \"$@\""

$(OBJ)/%.o: $(SRC)/%.c $(DEPENDS)
ifndef OS
	@[ ! -d $(OBJ) ] && mkdir -p $(OBJ); true
else
	@if not exist $(OBJ) mkdir $(OBJ)
endif
	@echo "Compiling object \"$@\" from \"$<\""
	@$(CC) -o $@ $(POBJFLAGS) -c $< $(OBJFLAGS)
	@echo "Compiled object \"$@\""

clean:
ifndef OS
	@rm -rf $(BIN) $(OBJ)/
else
	@if exist $(BIN) del /Q $(BIN)
	@if exist $(OBJ) rmdir /S /Q $(OBJ)
endif
