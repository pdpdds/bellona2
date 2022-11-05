@echo off
del     v86lib.bin
..\masm51\masm    v86lib;
..\masm51\link   v86lib;
..\masm51\exe2bin v86lib.exe v86lib.bin
del     v86lib.obj
del     v86lib.exe
copy    v86lib.bin ..\disk /y
