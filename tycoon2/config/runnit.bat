@echo off
rem Run the Notepad demo
rem
rem 1.3 97/11/26

if "%SWING_HOME%" == "" goto nohome
@echo on
java -classpath ".;%SWING_HOME%\swing.jar;%SWING_HOME%\motif.jar;%SWING_HOME%\jlf.jar;%SWING_HOME%\1.1_lib\swing.jar;%SWING_HOME%\1.1_lib\motif.jar;%SWING_HOME%\1.1_lib\jlf.jar;%CLASSPATH%" Configure
@echo off
goto done

:nohome
echo No SWING_HOME environment variable set.

:done
