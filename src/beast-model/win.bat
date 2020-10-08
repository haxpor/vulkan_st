@echo off
setlocal

set outputDir=.\
set outName=beast-model

rem if execute 'clean' then clean all artifact files
if "%1" == "clean" (
    call :DoClean
    echo "Removed all artifact files from build"
    EXIT /B %ERRORLEVEL%
)

if not exist %outputDir% (
    mkdir %outputDir%
)

rem /Z7 will produce embedded debugging info into .obj file, although .obj files are larger
rem but it is more convenient.
cl.exe /EHsc /c /O2 /std:c++17 /W3 /Z7 /I..\..\externals\include /I. VkBase.cpp /Fo:%outputDir%\VkBase.obj
cl.exe /EHsc /c /O2 /std:c++17 /W3 /Z7 /I..\..\externals\include /I. main.cpp /Fo:%outputDir%\main.obj
link.exe %outputDir%\VkBase.obj %outputDir%\main.obj /LIBPATH:..\..\externals\lib\glfw-vs2019 /LIBPATH:..\..\externals\lib\vulkan /OUT:%outputDir%\%outName%.exe /PDB:%outputDir%\%outName%.pdb glfw3dll.lib vulkan-1.lib

rem if compile or link operation failed then quit early
if %ERRORLEVEL% GEQ 1 (
    EXIT /B %ERRORLEVEL%
)

rem compile shader
pushd shaders
call compile.bat
popd

if not exist %outputDir%\glfw3.dll (
    copy /Y ..\..\externals\lib\glfw-vs2019\glfw3.dll %outputDir%\glfw3.dll
)

EXIT /B %ERRORLEVEL%

REM Delete all artifacts from build
:DoClean
del /Q %outputDir%\*.exe %outputDir%\*.ilk %outputDir%\*.obj %outputDir%\*.pdb
EXIT /B 0
