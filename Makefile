ifndef OS
BIN := psrc
else
BIN := psrc.exe
endif

PBINFLAGS := -L. -L./src/lib

BINFLAGS := -lpthread -lm -lSDL2_mixer -lfreetype -lassimp
ifndef OS
BINFLAGS := $(BINFLAGS) -lglfw -ldl -lGL -lSDL2
else
BINFLAGS := $(BINFLAGS) -lmingw32 -lSDL2main -lSDL2 -lglfw3 -lgdi32 -lopengl32
endif

POBJFLAGS := -I. -I./src/include -I./src/include/freetype2
ifndef OS
POBJFLAGS := $(POBJFLAGS) -I/usr/include/freetype2
else
ifndef FT2PATH
FT2PATH := "C:\\Program Files\\mingw-w64\\mingw64\\x86_64-w64-mingw32\\include\\freetype2"
endif
POBJFLAGS := $(POBJFLAGS) -I$(FT2PATH)
endif

OBJFLAGS := 

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
	@$(CC) -o $@ $(CFLAGS) $(PBINFLAGS) $^ $(BINFLAGS)
	@echo "Built binary \"$@\""

$(OBJ):
ifndef OS
	@[ ! -d $(OBJ) ] && mkdir -p $(OBJ); true
else
	@if not exist $(OBJ) mkdir $(OBJ)
endif

$(OBJ)/%.o: $(SRC)/%.c $(DEPENDS) | $(OBJ)
	@echo "Compiling object \"$@\" from \"$<\""
	@$(CC) -o $@ $(CFLAGS) $(POBJFLAGS) -c $< $(OBJFLAGS)
	@echo "Compiled object \"$@\""

clean:
ifndef OS
	@rm -rf $(BIN) $(OBJ)/
else
	@if exist $(BIN) del /Q $(BIN)
	@if exist $(OBJ) rmdir /S /Q $(OBJ)
endif

