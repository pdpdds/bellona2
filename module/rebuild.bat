cd startup
del debug\*.* /F /Q
nmake startup.mak

cd ..

cd stdlib
del debug\*.* /F /Q
nmake stdlib.mak

cd ..

cd \oh\test\module\gui
del debug\*.* /F /Q
nmake gui.mak

cd \oh\test\module\guiapp
del debug\*.* /F /Q
nmake guiapp.mak

cd \oh\test\module\jpeg
del debug\*.* /F /Q
nmake jpeg.mak

cd \oh\test\module\ush
del debug\*.* /F /Q
nmake ush.mak

cd \oh\test\module\login
del debug\*.* /F /Q
nmake login.mak

cd \oh\test\module\wall
del debug\*.* /F /Q
nmake wall.mak

cd clock
del debug\*.* /F /Q
nmake clock.mak

cd ..