cd bell_fs
del debug\*.* /F /Q
nmake bell_fs.mak

cd ..
del debug\*.* /F /Q
nmake test.mak

cd module\startup
del debug\*.* /F /Q
nmake startup.mak

cd ../..

cd module\stdlib
del debug\*.* /F /Q
nmake stdlib.mak

cd ../..

cd module\gui
del debug\*.* /F /Q
nmake gui.mak

cd ../..

cd module\guiapp
del debug\*.* /F /Q
nmake guiapp.mak

cd ../..

cd module\ush
del debug\*.* /F /Q
nmake ush.mak

cd ../..

cd module\login
del debug\*.* /F /Q
nmake login.mak

cd ../..

cd module\wall
del debug\*.* /F /Q
nmake wall.mak

cd ../..

cd module\clock
del debug\*.* /F /Q
nmake clock.mak

cd ../..
