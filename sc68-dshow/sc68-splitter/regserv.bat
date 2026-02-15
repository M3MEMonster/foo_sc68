@REM Register COM server
@echo OFF
echo PWD=%cd%
echo Register server %1
del %2
runas /Administrators regsvr32 /s /u "%1" && echo Unregistred
runas /Administrators regsvr32 /s    "%1" && echo Registered && echo OK>%2
