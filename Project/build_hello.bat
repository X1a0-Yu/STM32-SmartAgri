@echo off
set BIN=D:\Keil_v5\ARM\ARMCC\bin
set INC1=D:\Keil_v5\ARM\Pack\Keil\STM32F1xx_DFP\1.0.5\Device\Include
set INC2=D:\Keil_v5\ARM\Pack\ARM\CMSIS\4.2.0\CMSIS\Include
set ROOT=D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2
if not exist "%ROOT%\ObjectsSTM32" mkdir "%ROOT%\ObjectsSTM32"
"%BIN%\armcc.exe" --cpu Cortex-M3 --c90 -O1 -DSTM32F10X_MD -I"%INC1%" -I"%INC2%" -c "%ROOT%\Project\hello.c" -o "%ROOT%\ObjectsSTM32\hello.obj" & if errorlevel 1 goto fail
"%BIN%\armasm.exe" --cpu Cortex-M3 --pd "STM32F10X_MD SETA 1" "%ROOT%\CMSIS\startup_stm32f10x_md.s" -o "%ROOT%\ObjectsSTM32\startup.obj" & if errorlevel 1 goto fail
"%BIN%\armcc.exe" --cpu Cortex-M3 --c90 -O1 -DSTM32F10X_MD -I"%INC1%" -I"%INC2%" -c "%ROOT%\CMSIS\system_stm32f10x.c" -o "%ROOT%\ObjectsSTM32\system_stm32f10x.obj" & if errorlevel 1 goto fail
"%BIN%\armlink.exe" --cpu Cortex-M3 --entry Reset_Handler --first __Vectors --ro-base 0x08000000 --rw-base 0x20000000 --output "%ROOT%\ObjectsSTM32\hello.axf" "%ROOT%\ObjectsSTM32\startup.obj" "%ROOT%\ObjectsSTM32\system_stm32f10x.obj" "%ROOT%\ObjectsSTM32\hello.obj" & if errorlevel 1 goto fail
"%BIN%\fromelf.exe" --i32combined --output "%ROOT%\ObjectsSTM32\hello.hex" "%ROOT%\ObjectsSTM32\hello.axf"
echo HELLO BUILD OK
dir "%ROOT%\ObjectsSTM32\hello.hex"
exit /b 0
:fail
echo HELLO BUILD FAILED
exit /b 1
