ifndef OS
BIN := psrc
else
BIN := psrc.exe
endif

PBINFLAGS := -L. -Llib -g
BINFLAGS := -lpthread -lm -lSDL2 -lSDL2_mixer
ifndef OS
BINFLAGS := $(BINFLAGS) -lGL -lGLU -lGLEW
else
BINFLAGS := $(BINFLAGS) -lmingw32 -lSDL2main -lopengl32 -lglu32 -lglew32
endif
POBJFLAGS := -Wall -Wextra -I. -Ilib -g
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

$(BIN): $(OBJECTS)
	@echo "Building binary \"$@\" from \"$^\""
	@$(CC) -o $@ $(PBINFLAGS) $^ $(BINFLAGS)

$(OBJ)/%.o: $(SRC)/%.c $(DEPENDS)
ifndef OS
	@[ ! -d $(OBJ) ] && mkdir -p $(OBJ); true
else
	@if not exist $(OBJ) mkdir $(OBJ)
endif
	@echo "Compiling object \"$@\" from \"$<\""
	@$(CC) -o $@ $(POBJFLAGS) -c $< $(OBJFLAGS)

clean:
ifndef OS
	@rm -rf $(BIN) $(OBJ)/
else
	@if exist $(BIN) del /Q $(BIN)
	@if exist $(OBJ) rmdir /S /Q $(OBJ)
endif
