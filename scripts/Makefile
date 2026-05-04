CC = cc
CFLAGS = -Wall -Wextra -O2
RAYLIB_INCLUDE = /opt/homebrew/Cellar/raylib/5.5/include
RAYLIB_LIB = /opt/homebrew/Cellar/raylib/5.5/lib

LDFLAGS = -L$(RAYLIB_LIB) -lraylib \
          -framework Cocoa -framework IOKit -framework OpenGL

TARGET = bin/square
SRC = src/main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -I$(RAYLIB_INCLUDE) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
