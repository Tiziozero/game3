git submodule update --init --recursive
cd external/lua
gcc -c *.c -lm
ar rcs libluawin.a *.o
cd ../../

