@ECHO OFF
setlocal enabledelayedexpansion


:: assign default value to the output directory
set LIBDIR=monitor_lib

:: initializing support variables
set /a nextVar=0

:: obtaining the script inputs
for %%x in (%*) do (
		if !nextVar! == 0 ( :: case we are expecting an option flag
			if %%x == -h (
				call :printusage
				exit /b 0
			) else if %%~x == -d (
				set /a nextVar=1
			) else if %%~x == -n (
				set /a nextVar=2
			) else if %%~x == -b (
				set /a nextVar=3
			) else if %%~x == -f (
				set /a nextVar=4
			) else ( :: case in which the input is not an option
				call :printusage
				exit /b 1
			)
		) ELSE ( :: case we are expecting an option argument
			if !nextVar! == 1 (
				set LIBDIR=%%~x
				call :normalizepath LIBDIR !LIBDIR!
			)
			if !nextVar! == 2 (
				set LIBNAME=%%~x
			)
			if !nextVar! == 3 (
				set BROWNAME=%%~x
			)
			if !nextVar! == 4 (
				set FORMULAFILE=%%~x
				call :normalizepath FORMULAFILE !FORMULAFILE!
			)
			set /a nextVar=0		
		)
)

:: case in which we are still expecting an option argument
if !nextVar! NEQ 0 (
	echo Error: expecting option argument
	call :printusage
	exit /b 1
)

:: assign default value to the library name
if not defined LIBNAME (
	call :setbasename LIBNAME !LIBDIR!
)


:: assign default value to the library browser name
if not defined BROWNAME (
	set BROWNAME=!LIBNAME!
)

:: check if matlab is in path
WHERE matlab >nul 2>nul
if !ERRORLEVEL! == 1 (
	echo Please install matlab or add the matlab binary to PATH
	exit /b 1
)

:: check if the formula file was given
if not defined FORMULAFILE (
  echo Please insert the name of the file containing the formulae
  call :printusage
  exit /b 1
)

:: current directory is set to the script directory
cd %~dp0"\src"

:: executing system
matlab -nodesktop -nosplash -nodisplay -r "bin.libgen('!FORMULAFILE!','!LIBDIR!','!LIBNAME!','!BROWNAME!');quit;"
exit /b 0



:: support functions===========================================================
:setbasename 
	set "%~1=%~n2"
exit /b 0

:printusage
	echo Usage: %~nx0 [-h] [-d output-library] [-n library-name] [-b library-name-in-browser] -f formulae-file 
exit /b 0

:normalizepath
  set "%~1=%~dpfn2"
  exit /B

endlocal