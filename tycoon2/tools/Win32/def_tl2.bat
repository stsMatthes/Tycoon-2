@ECHO OFF
@REM
@REM Batch for setting Tycoon-2 Environment.
@REM
@REM V1.0  18.02.99  A.Willomat
@REM

@REM
@REM First set temporary variables by a C-Program (stupid DOS ...)
@REM

@Tools\Win32\DOSgetenv >$$tmp.bat
@CALL $$tmp.bat
@DEL $$tmp.bat >NUL


@REM
@REM Set base variables
@REM

@SET HOX_HOST=Win32_i386
@SET HOX_HOME=%HOX_CURR_DIR%
@SET TL2_PATH=.;%HOX_CURR_DIR%\src\TL2;

@SET SGML_CATALOG_FILES=%HOX_HOME%\lib\sgml\CATALOG
@SET SGML_SEARCH_PATH=%HOX_HOME%\lib\sgml

@REM
@REM Expand path, only one Tycoon-2-bin directory
@REM

@SET PATH=%HOX_CLEAN_PATH%;%HOX_HOME%\bin\%HOX_HOST%


@REM
@REM Set Jam Compiler path (if MSVC++ is found from DOSgetenv.exe)
@REM
@SET MSVCNT=%HOX_MSVC%


@REM
@REM Clear temporary variables
@REM

@SET HOX_MSVC=
@SET HOX_CURR_DIR=
@SET HOX_CLEAN_PATH=
@SET HOX_CURR_DIR_UNIX=
