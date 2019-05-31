REM @echo off


REM %1 Debug | Release
:main
	REM check arguments
	if "%1" == "" exit /b 1
	if "%2" == "" exit /b 1
	
	pushd %~dp0
	
	set config=%2
	set project=%1
	
	if "%project%" == "json2cxxstruct" (
	
		xcopy .\%config%\*.lib .\bin\ /ysdq
	
	) else if "%project%" == "json2cxxstructHelper" (
	
		if "%config%" == "Debug" (
		
			xcopy .\%config%\%project%d.exe .\bin\ /ysdq
			
		) else (
		
			xcopy .\%config%\%project%.exe .\bin\ /ysdq

		)
	)
	
	popd
	
	exit /b 0