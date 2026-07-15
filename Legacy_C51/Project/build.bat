@echo off
set KEIL=D:\Keil_v5\C51\BIN
set ROOT=D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2
set INC=D:\Keil_v5\C51\INC;%ROOT%\User;%ROOT%\Drivers

if not exist "%ROOT%\Objects" mkdir "%ROOT%\Objects"
if not exist "%ROOT%\Listings" mkdir "%ROOT%\Listings"

"%KEIL%\C51.exe" "%ROOT%\User\main.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\main.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\User\app_control.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\app_control.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\User\system_tick.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\system_tick.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\Drivers\serial2_printf.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\serial2_printf.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\Drivers\key.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\key.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\Drivers\display.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\display.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\Drivers\actuators.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\actuators.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\Drivers\dht11.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\dht11.obj")
if errorlevel 1 exit /b 1
"%KEIL%\C51.exe" "%ROOT%\Drivers\adc0832.c" INCDIR("D:\Keil_v5\C51\INC","%ROOT%\User","%ROOT%\Drivers") OBJECT("%ROOT%\Objects\adc0832.obj")
if errorlevel 1 exit /b 1

"%KEIL%\BL51.exe" "%ROOT%\Objects\main.obj","%ROOT%\Objects\app_control.obj","%ROOT%\Objects\system_tick.obj","%ROOT%\Objects\serial2_printf.obj","%ROOT%\Objects\key.obj","%ROOT%\Objects\display.obj","%ROOT%\Objects\actuators.obj","%ROOT%\Objects\dht11.obj","%ROOT%\Objects\adc0832.obj" TO "%ROOT%\Objects\SmartAgri" RAMSIZE(256)
if errorlevel 1 exit /b 1
"%KEIL%\OH51.exe" "%ROOT%\Objects\SmartAgri" HEXFILE("%ROOT%\Objects\SmartAgri.hex")
