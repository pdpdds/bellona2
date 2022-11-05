del d:\oh\test\debug\*.* /F/Q
del d:\oh\test\bell_fs\debug\*.* /F/Q

cd d:\oh\test\bell_fs
nmake bell_fs.mak
cd ..
nmake test.mak
