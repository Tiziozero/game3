SRC=main.c env.c
all:
	gcc -ggdb -o prog $(SRC) -lm -L./windows -lraylib
	./prog
win:
	zig cc -o prog.exe $(SRC) -L. -lraylib
	./prog.exe
check_leaks:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./prog
dbg:
	gdb --args ./prog
