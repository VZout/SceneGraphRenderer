@echo off
set prefix=[INSTALLER]

:inst_deps
echo %prefix% Reloading CMake...

cd build-win64

call cmake . 

cd ..

echo %prefix% Finished Reloading CMake

pause
EXIT /B 0
