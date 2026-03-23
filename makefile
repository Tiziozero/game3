SRC=main.c env.c game.c scripts/abilities/*.c scripts/projectiles/*.c scripts/enemies/*.c
INCLIDES=
INCLUDES += -Iexternal/lua
LIBS=-L. -lraylib -lm
all:
	gcc -ggdb -o prog $(SRC) $(INCLUDES) $(LIBS)
	./prog
win:
	zig cc -o prog.exe $(SRC)  $(INCLUDES) $(LIBS)
	./prog.exe
check_leaks:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./prog
dbg:
	gdb --args ./prog
