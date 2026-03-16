all:
	cc -ggdb -o prog main.c -lm -L./ -lraylib
	./prog
check_leaks:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./prog
dbg:
	gdb --args ./prog
