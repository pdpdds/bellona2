@echo off
del boot.bin
..\masm51\masm boot;
..\masm51\link boot;
..\masm51\exe2bin boot.exe boot.bin
del boot.exe
del boot.obj
copy boot.bin ..\disk
dir *.bin