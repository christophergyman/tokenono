CC = cc
CFLAGS = -Wall -Wextra -O2
PROJECT_INCLUDE = src/include
RAYLIB_INCLUDE = /opt/homebrew/Cellar/raylib/5.5/include
RAYLIB_LIB = /opt/homebrew/Cellar/raylib/5.5/lib

LDFLAGS = -L$(RAYLIB_LIB) -lraylib \
          -framework Cocoa -framework IOKit -framework OpenGL

TARGET = bin/square
SRC = src/main.c \
      src/core/arena.c \
      src/core/arena_tests.c \
      src/core/app_config.c
ARGS ?=

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -I$(PROJECT_INCLUDE) -I$(RAYLIB_INCLUDE) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -f $(TARGET)

.PHONY: run clean
