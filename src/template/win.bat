@echo off
setlocal

REM Batch script to compile and build the project on Windows
REM Do not echo the command being executed on console, just result

REM configurable variables
REM Make sure variables are defined even before calling any function
set outputDir=bin
set outName=template

REM if execute 'clean' then clean all artifact files
if "%1" == "clean" (
    call :DoClean
    echo "Removed all artifact files from build"
    EXIT /B %ERRORLEVEL%
)

REM /WX (treat warning as error) is tried but due to warning from dependent libraries, so it's dropped
REM We link against dynamic library (.dll), but note that .lib is still need.
REM Make sure to use the proper .lib file accompanying a target .dll.
REM See https://softwareengineering.stackexchange.com/a/395455/60836 for more info of why .lib + .dll are needed.
REM
REM Note: Only vs2019 for glfw supported for now.
REM
REM .dll files need to be reachable.
REM     * vulkan - installed from the package, and it will automatically put relevant .dll file into C:\Windows\System32 so it is always found.
REM     * glfw - we need to manually copy it from <root>/externals/lib/glfw-vs2019/glfw3.dll to the target output directory of this script

if not exist %outputDir% (
    mkdir %outputDir%
)
cl.exe /EHsc /std:c++17 /Wall /Zi /I..\..\externals\include main.cpp /Fo:%outputDir%\main.obj /link /LIBPATH:..\..\externals\lib\glfw-vs2019 /LIBPATH:..\..\externals\lib\vulkan /OUT:%outputDir%\%outName%.exe /PDB:%outputDir%\%outName%.pdb glfw3dll.lib vulkan-1.lib

if not exist %outputDir%\glfw3.dll (
    copy /Y ..\..\externals\lib\glfw-vs2019\glfw3.dll %outputDir%\glfw3.dll
)

EXIT /B %ERRORLEVEL%

REM Delete all artifacts from build
:DoClean
del /Q %outputDir%\*.exe %outputDir%\*.ilk %outputDir%\*.obj %outputDir%\*.pdb
EXIT /B 0
