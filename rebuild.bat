cd \oh\test\bell_fs
del debug\*.* /F /Q
nmake bell_fs.mak

cd \oh\test
del debug\*.* /F /Q
nmake test.mak

cd \oh\test\module\startup
del debug\*.* /F /Q
nmake startup.mak

cd \oh\test\module\stdlib
del debug\*.* /F /Q
nmake stdlib.mak

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

cd \oh\test\module\clock
del debug\*.* /F /Q
nmake clock.mak

cd \oh\test\module\tt
del debug\*.* /F /Q
nmake tt.mak

cd \oh\test