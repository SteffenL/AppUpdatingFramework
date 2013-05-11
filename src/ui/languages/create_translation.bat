@echo off

set outputFile=%cd%\gen\create_translation_output.1.txt
set outputFile2=%cd%\gen\create_translation_output.2.txt
set cwd=%cd%

:: Create output folder if needed
if not exist gen mkdir gen

:: Generate list of file paths
pushd ..
dir /b /s /a-d | findstr /i "\.cpp$ \.h$ \.hpp$" > "%outputFile%"

:: Create .pro file
xgettext -C -n -k_ -kwxPLURAL:1,2 -kwxTRANSLATE "-f%outputFile%" -olanguages\gen\aufw.po -c -s -E
popd
if not %errorlevel% == 0 pause