rem this file is for intermediate checks (lesson 30. Тестирование в Unreal Engine. Публикация локального отчета по тестам. Автоматизация UE: https://www.youtube.com/watch?v=xXF_HEZSs5c)
@echo off

call "%~dp0..\config.bat"

goto:skipbuild
rem build sources
call "%RunUATPath%" BuildCookRun ^
-project="%ProjectPath%" ^
-platform="%Platform%" ^
-clientconfig="%Configuration%" ^
-build -cook -ubtargs="-UnoptimizedCode" -noturnkeyvariables
:skipbuild

rem run tests
"%EditorPath%" "%ProjectPath%" -ExecCmds="Automation RunTests TPSGame;Quit" ^
-log -abslog="%TestOutputLogPath%" -nosplash -ReportOutputPath="%ReportOutputPath%"

rem copy test artifacts
set TestsDir=%~dp0
set TestsDataDir=%~dp0data
robocopy "%TestsDataDir%" "%ReportOutputPath%" /E


rem start local server and show report
set Port=8081
set Localhost=http://localhost:%Port%

pushd "%ReportOutputPath%"
start "" "%Localhost%"
call http-server -p="%Port%"
popd