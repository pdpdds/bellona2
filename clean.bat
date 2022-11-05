@echo off
del *.bak /F /S /Q
del *.ncb /F /S /Q
del *.opt /F /S /Q
del *.plg /F /S /Q
del *.obj /F /S /Q
del *.idb /F /S /Q
del *.exp /F /S /Q
del *.ilk /F /S /Q
del *.pch /F /S /Q
del *.pdb /F /S /Q
del *.cod /F /S /Q
del *.sbr /F /S /Q
del *.bsc /F /S /Q

del module\tt\debug\*.* /F /Q
del module\gui\debug\*.* /F /Q
del module\gui\FontEdit\debug\*.* /F /Q
del module\gui\FontMk\debug\*.* /F /Q
del module\stdlib\debug\*.* /F /Q
del module\ush\debug\*.* /F /Q
del debug\*.* /F /Q
del bell_fs\debug\*.* /F /Q