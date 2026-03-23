@echo off
setlocal enabledelayedexpansion

set FILES=main.c env.c game.c

for %%f in (scripts\abilities\*.c scripts\projectiles\*.c scripts\enemies\*.c) do (
    set FILES=!FILES! %%f
)

echo %FILES%
zig cc -o prog.exe %FILES% -L..\rt\lib -lraylib
