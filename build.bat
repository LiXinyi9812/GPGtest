@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
msbuild GPG.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Build
