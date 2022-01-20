ifndef OS
BIN := psrc
else
BIN := psrc.exe
endif

PBINFLAGS := -Wall -Wextra -L. -L./src/lib -flto

BINFLAGS := -lpthread -lm -lSDL2_mixer -lfreetype -lassimp
ifndef OS
BINFLAGS := $(BINFLAGS) -lglfw -ldl -lGL -lSDL2
else
BINFLAGS := $(BINFLAGS) -lmingw32 -lSDL2main -lSDL2 -lglfw3 -lgdi32 -lopengl32
endif

POBJFLAGS := -Wall -Wextra -I. -I./src/include -I./src/include/freetype2
ifndef OS
POBJFLAGS := $(POBJFLAGS) -I/usr/include/freetype2
else
ifndef FT2PATH
FT2PATH := "%SYSTEMDRIVE%\\Program Files\\mingw-w64\\mingw64\\x86_64-w64-mingw32\\include\\freetype2"
endif
POBJFLAGS := $(POBJFLAGS) -I$(FT2PATH)
endif

OBJFLAGS := -Ofast -s -flto -g

SRC := src
OBJ := obj

SOURCES := $(wildcard $(SRC)/*.c)
DEPENDS := $(wildcard $(SRC)/*.h)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

ifdef CC
CC := gcc
endif

.PHONY: all build run clean $(OBJ)

all: run

build: $(BIN)

run: $(BIN)
	@echo "Running binary \"./$<\""
	@./$<
	@echo "\"./$<\" exited successfully"

$(BIN): $(OBJECTS)
	@echo "Building binary \"$@\" from \"$^\""
	@$(CC) -o $@ $(PCFLAGS) $(PBINFLAGS) $^ $(CFLAGS) $(BINFLAGS)
	@echo "Built binary \"$@\""

$(OBJ):
ifndef OS
	@[ ! -d $(OBJ) ] && mkdir -p $(OBJ); true
else
	@if not exist $(OBJ) mkdir $(OBJ)
endif

$(OBJ)/%.o: $(SRC)/%.c $(DEPENDS) | $(OBJ)
	@echo "Compiling object \"$@\" from \"$<\""
	@$(CC) -o $@ $(PCFLAGS) $(POBJFLAGS) -c $< $(CFLAGS) $(OBJFLAGS)
	@echo "Compiled object \"$@\""

clean:
ifndef OS
	@rm -rf $(BIN) $(OBJ)/
else
	@if exist $(BIN) del /Q $(BIN)
	@if exist $(OBJ) rmdir /S /Q $(OBJ)
endif

