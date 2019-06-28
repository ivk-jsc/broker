@echo off
SETX TERM msys
chcp 65001
cls

where /q git.exe || goto msg1

set PROTO_NAME=libs\libupmqprotocol\protocol.proto
set FILE_NAME=share/Version.hpp
set FILE_NAME2=VERSION.txt
set FILE_NAME3=COMMIT.txt

for /f %%i in ('git rev-list HEAD --count') do set BUILD_VER=%%i
for /f "delims=" %%i in ('git log -n 1 "--pretty=format:%%H"') 	do set COMMITTER_FULLSHA=%%i
for /f "delims=" %%i in ('git log -n 1 "--pretty=format:%%h"') 	do set COMMITTER_SHORTSHA=%%i
for /f "delims=" %%i in ('git log -n 1 "--pretty=format:%%cn"') do set COMMITTER_NAME=%%i
for /f "delims=" %%i in ('git log -n 1 "--pretty=format:%%ce"') do set COMMITTER_EMAIL=%%i
for /f "delims=" %%i in ('git log -n 1 "--pretty=format:%%ci"') do set COMMITTER_DATE=%%i
for /f "delims=" %%i in ('git log -n 1 "--pretty=format:%%s"') 	do set COMMITTER_NOTE=%%i

if "%BUILD_VER%"=="" set BUILD_VER=0

for /F "delims=" %%a in ('findstr /c:"server_vendor_id" %PROTO_NAME%') do set BrokerNameString=%%a
SET BrokerName=%BrokerNameString:~-7,-3%
echo Broker name: 		%BrokerName%

for /F "delims=" %%b in ('findstr /c:"server_major_version" %PROTO_NAME%') do set BrokerMajorString=%%b
SET BrokerMajor=%BrokerMajorString:~-3,-2%
for /F "delims=" %%c in ('findstr /c:"server_minor_version" %PROTO_NAME%') do set BrokerMinorString=%%c
SET BrokerMinor=%BrokerMinorString:~-3,-2%
echo Broker version: 	%BrokerMajor%.%BrokerMinor%.%BUILD_VER%

for /F "delims=" %%d in ('findstr /c:"client_major_version" %PROTO_NAME%') do set ClientMajorString=%%d
SET ClientMajor=%ClientMajorString:~-3,-2%
for /F "delims=" %%e in ('findstr /c:"client_minor_version" %PROTO_NAME%') do set ClientMinorString=%%e
SET ClientMinor=%ClientMinorString:~-3,-2%
echo Client version: 	%ClientMajor%.%ClientMinor%.%BUILD_VER%


echo #ifndef VERSION_H												>  %FILE_NAME%
echo #define VERSION_H												>> %FILE_NAME%
echo.																>> %FILE_NAME%
echo #include ^<string^>											>> %FILE_NAME%
echo #include ^<sstream^>											>> %FILE_NAME%
echo.																>> %FILE_NAME%
echo.																>> %FILE_NAME%
echo #define MQ_VERSION_REVISION 		%BUILD_VER%					>> %FILE_NAME% 
echo #define MQ_VERSION_MAJOR			%BrokerMajor%				>> %FILE_NAME%
echo #define MQ_VERSION_MINOR			%BrokerMinor%				>> %FILE_NAME%
echo.																>> %FILE_NAME%
echo #define MQ_COMMITTER_NAME 			"%COMMITTER_NAME%"			>> %FILE_NAME% 
echo #define MQ_COMMITTER_EMAIL 		"%COMMITTER_EMAIL%"			>> %FILE_NAME%
echo.																>> %FILE_NAME%
echo #define MQ_COMMITTER_FULLSHA		"%COMMITTER_FULLSHA%"		>> %FILE_NAME%
echo #define MQ_COMMITTER_SHORTSHA		"%COMMITTER_SHORTSHA%"		>> %FILE_NAME%
echo #define MQ_COMMITTER_DATE 			"%COMMITTER_DATE%"			>> %FILE_NAME%
echo #define MQ_COMMITTER_NOTE 			"%COMMITTER_NOTE%"			>> %FILE_NAME% 
echo.																>> %FILE_NAME%
echo #endif // VERSION_H											>> %FILE_NAME%
echo.																>> %FILE_NAME%


echo %BrokerMajor%.%BrokerMinor%.%BUILD_VER%> %FILE_NAME2%
echo %COMMITTER_SHORTSHA%> %FILE_NAME3%


echo.
echo Version file  : %FILE_NAME%
echo Version number: %BUILD_VER%
echo Committer SHA:		%COMMITTER_FULLSHA%
echo Committer SHA:		%COMMITTER_SHORTSHA%
echo Committer name: 	%COMMITTER_NAME%
echo Committer email: 	%COMMITTER_EMAIL%
echo Committer date: 	%COMMITTER_DATE%
rem echo Committer note : %COMMITTER_NOTE%
echo.

goto end

:msg1
echo {-----------------------------------------------------------------------------------}
echo {                                                                                   }
echo {                                                                                   }
echo { Please set the Path variable in your environment to match the location of git.exe }
echo {                                                                                   }
echo {                                                                                   }
echo {-----------------------------------------------------------------------------------}

:end