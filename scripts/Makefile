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
      src/core/app_config.c \
      src/core/ecs.c
HDR = src/include/app_config.h \
      src/include/arena.h \
      src/include/arena_tests.h \
      src/include/ecs.h
ARGS ?=

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -I$(PROJECT_INCLUDE) -I$(RAYLIB_INCLUDE) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) $(ARGS)

dev:
	./scripts/dev $(ARGS)

clean:
	rm -f $(TARGET)

.PHONY: run dev clean
