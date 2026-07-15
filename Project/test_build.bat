@echo off
set ROOT=D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2
set BIN=D:\Keil_v5\ARM\ARMCC\bin
echo === building ===
"%BIN%\armcc.exe" --cpu Cortex-M3 --c90 -O1 -DSTM32F10X_MD -I"D:\Keil_v5\ARM\Pack\Keil\STM32F1xx_DFP\1.0.5\Device\Include" -I"D:\Keil_v5\ARM\Pack\ARM\CMSIS\4.2.0\CMSIS\Include" -I"%ROOT%\User" -I"%ROOT%\Drivers" -c "%ROOT%\User\main.c" -o "%ROOT%\ObjectsSTM32\main.obj" 2>&1 || goto fail
"%BIN%\armcc.exe" --cpu Cortex-M3 --c90 -O1 -DSTM32F10X_MD -I"D:\Keil_v5\ARM\Pack\Keil\STM32F1xx_DFP\1.0.5\Device\Include" -I"D:\Keil_v5\ARM\Pack\ARM\CMSIS\4.2.0\CMSIS\Include" -I"%ROOT%\User" -I"%ROOT%\Drivers" -c "%ROOT%\Drivers\serial2_printf.c" -o "%ROOT%\ObjectsSTM32\serial2_printf.obj" 2>&1 || goto fail
"%BIN%\armcc.exe" --cpu Cortex-M3 --c90 -O1 -DSTM32F10X_MD -I"D:\Keil_v5\ARM\Pack\Keil\STM32F1xx_DFP\1.0.5\Device\Include" -I"D:\Keil_v5\ARM\Pack\ARM\CMSIS\4.2.0\CMSIS\Include" -I"%ROOT%\User" -I"%ROOT%\Drivers" -c "%ROOT%\CMSIS\system_stm32f10x.c" -o "%ROOT%\ObjectsSTM32\system_stm32f10x.obj" 2>&1 || goto fail
"%BIN%\armasm.exe" --cpu Cortex-M3 --pd "STM32F10X_MD SETA 1" "%ROOT%\CMSIS\startup_stm32f10x_md.s" -o "%ROOT%\ObjectsSTM32\startup.obj" 2>&1 || goto fail
"%BIN%\armlink.exe" --cpu Cortex-M3 --entry Reset_Handler --first __Vectors --ro-base 0x08000000 --rw-base 0x20000000 --output "%ROOT%\ObjectsSTM32\SmartAgri_STM32.axf" "%ROOT%\ObjectsSTM32\startup.obj" "%ROOT%\ObjectsSTM32\system_stm32f10x.obj" "%ROOT%\ObjectsSTM32\main.obj" "%ROOT%\ObjectsSTM32\serial2_printf.obj" 2>&1 || goto fail
"%BIN%\fromelf.exe" --i32combined --output "%ROOT%\ObjectsSTM32\SmartAgri_STM32.hex" "%ROOT%\ObjectsSTM32\SmartAgri_STM32.axf" 2>&1 || goto fail
echo === done ===
dir "%ROOT%\ObjectsSTM32"
exit /b 0
:fail
echo === FAILED with error %errorlevel% ===
exit /b 1
