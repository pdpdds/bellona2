@echo off
del asm.obj
del bcmos.obj
del lib\asm.lib
ml /nologo /c /Cx /Zd /Zi /coff /Febcmos.obj /Flbcmos\bcmos.code bcmos\bcmos.asm
ml /nologo /c /Cx /Zd /Zi /coff /Feasm.obj /Flasm.code asm.asm
lib asm.obj bcmos.obj
copy asm.lib lib /y
del asm.lib

