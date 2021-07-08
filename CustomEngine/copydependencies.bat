@ECHO OFF
IF "%1"=="" GOTO BADTARGET
SETLOCAL
SET parent=%~dp0
IF /I %1==DEBUG32 GOTO DBG32
IF /I %1==DEBUG64 GOTO DBG64
IF /I %1==RELEASE32 GOTO RLS32
IF /I %1==RELEASE64 GOTO RLS64
:BADTARGET
ECHO Invalid Target!
GOTO QUIT
:DBG32
SET deptarget=%parent%Debug
GOTO COPY32
:DBG64
SET deptarget=%parent%x64\Debug
GOTO COPY64
:RLS32
SET deptarget=%parent%Release
GOTO COPY32
:RLS64
SET deptarget=%parent%x64\Release
GOTO COPY64
:COPY32
:: Copy DLLs
copy "%parent%lib\glew-2.1.0\bin\Release\Win32\glew32.dll"
copy "%parent%lib\assimp\assimp-vc140-mt.dll"
copy "%parent%lib\FMOD\lib\fmod.dll"
GOTO COPYUNI
:COPY64
:: Copy DLLs
copy "%parent%lib\glew-2.1.0\bin\Release\x64\glew32.dll"
copy "%parent%lib\assimp\assimp-vc140-mt.dll"
copy "%parent%lib\FMOD\lib\fmod64.dll"
GOTO COPYUNI
:COPYUNI
:: Copy Resource Files
xcopy /i /s /y "%parent%GAM200 Project\Resource" "%deptarget%\Resource"
:QUIT
ENDLOCAL
ECHO Finished Copying Dependencies