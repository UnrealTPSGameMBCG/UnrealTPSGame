rem this file is for intermediate checks (lesson 30. Тестирование в Unreal Engine. Публикация локального отчета по тестам. Автоматизация UE: https://www.youtube.com/watch?v=xXF_HEZSs5c)
rem is for installing bower packages
@echo off

call "%~dp0..\config.bat"

rem remove previous data folder
set TestsDir=%~dp0
set TestsDataDir=%~dp0data

rem install bower packages
pushd "%TestsDir%"
cd "%TestsDataDir%"
call bower install "%TestsDataDir%\bower.json"
popd