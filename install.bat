@echo off
set prefix=[INSTALLER]

:inst_deps
echo %prefix% Installing dependencies...

call git clone https://github.com/assimp/assimp deps/assimp
call git clone https://github.com/glfw/glfw deps/glfw

echo %prefix% Finished isntalling dependencies

mkdir build-win64
cd build-win64

echo %prefix% Initializing CMake (VS2017 Win64)...
call cmake -G "Visual Studio 15 2017 Win64" ..

cd ..

pause
EXIT /B 0
