SRC=main.c env.c game.c engine.c
INCLIDES=
INCLUDES += -Iexternal/lua
LIBS=-L. -lraylib
LIBS += external/lua/liblua.a -ldl -lm
all:
	gcc -ggdb -o prog $(SRC) $(INCLUDES) $(LIBS)
	./prog
win:
	zig cc -o prog.exe $(SRC) -L. -lraylib
	./prog.exe
check_leaks:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./prog
dbg:
	gdb --args ./prog
