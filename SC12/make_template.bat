@echo off
rem Load ssh key
//load .ppk//

rem delete old source files
del /Q //network path//*.h
del /Q //network path//*.c

rem copy over new source files
copy /B /Y *.c //network path//
copy /B /Y *.h //network path//

rem open ssh session and execute commands
plink -batch -ssh //user@host// "cd sc12; sh compile.sh"