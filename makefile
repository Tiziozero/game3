all:
	cc -ggdb -o prog main.c -lm -L./ -lraylib  -I/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/include
	./prog
check_leaks:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./prog
dbg:
	gdb --args ./prog
