rem Just run tests and run test coverage, assuming that UE project is already built 

@echo off

call "%~dp0..\config.bat"

rem run tests
set TestRunner="%EditorPath%" "%ProjectPath%" -ExecCmds="Automation RunTests %TestNames%;Quit" ^
-log -abslog="%TestOutputLogPath%" -nosplash -ReportOutputPath="%ReportOutputPath%"

rem run code coverage
::set ExportType=cobertura:%ReportOutputPath%\Coverage\CodeCoverageReport.xml
set ExportType=html:%ReportOutputPath%\Coverage\CodeCoverageReport

call :NORMALIZEPATH %ProjectRoot%
set Module=%RETVAL%

call :NORMALIZEPATH %SourceCodePath%
set Sources=%RETVAL%

call :NORMALIZEPATH %ExludedPathForTestReport%
set ExludedSources=%RETVAL%

"%OpenCPPCoveragePath%" --modules="%Module%" --sources="%Sources%" ^
--excluded_sources="%ExludedSources%" --export_type="%ExportType%" -- %TestRunner% -v 

rem clean obsolete artifacts of test coverage
del /q LastCoverageResults.log

rem copy test artifacts
set TestsDir=%~dp0
set TestsDataDir=%~dp0data
robocopy "%TestsDataDir%" "%ReportOutputPath%" /E

rem start local server and show test report
set Port=8081
set Localhost=http://localhost:%Port%

rem running HTML reports with tests results and test coverage
pushd "%ReportOutputPath%"
start "" "%Localhost%"
start "" "%Localhost%\Coverage\CodeCoverageReport\index.html"
call http-server -p="%Port%"
popd

:: ========== FUNCTIONS ==========
goto:EOF

:: returning the full paths 
:NORMALIZEPATH
  SET RETVAL=%~f1
  EXIT /B