@echo off
del boot2.bin
..\masm51\masm boot2;
..\masm51\link boot2;
..\masm51\exe2bin boot2.exe boot2.bin
del boot2.exe
del boot2.obj
copy boot2.bin ..\disk
dir *.bin
