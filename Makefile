# Makefile

SRC = src/main.c src/terminal.c src/gui.c src/levels.c
OBJ = $(SRC:.c=.o)
CFLAGS = `sdl2-config --cflags` -Iinclude -Wall
LIBS   = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf -lSDL2_mixer

TerminalWar: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) TerminalWar

