@ECHO OFF
@REM
@REM Simple batch to compile and copy DOSgetenv.c
@REM
@REM V1.0  18.02.99  A.Willomat
@REM

@cl DOSgetenv.c
@copy DOSgetenv.exe ..\Win32 >NUL
@del DOSgetenv.obj > NUL
@del DOSgetenv.exe > NUL
