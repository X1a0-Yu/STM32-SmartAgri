@echo off
setlocal
set ROOT=D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2
set BIN=D:\Keil_v5\ARM\ARMCC\bin
set INC1=D:\Keil_v5\ARM\Pack\Keil\STM32F1xx_DFP\1.0.5\Device\Include
set INC2=D:\Keil_v5\ARM\Pack\ARM\CMSIS\4.2.0\CMSIS\Include
set CC="%BIN%\armcc.exe" --cpu Cortex-M3 --c90 --apcs=interwork --split_sections -O1 -g --md --depend_format=unix_escaped -DSTM32F10X_MD -I"%INC1%" -I"%INC2%" -I"%ROOT%\User" -I"%ROOT%\Drivers" -I"%ROOT%\CMSIS"
if not exist "%ROOT%\ObjectsSTM32" mkdir "%ROOT%\ObjectsSTM32"
echo Compiling all sources...
for %%F in (main board system_tick app_control command_protocol lora_service wifi_service) do (
  %CC% -c "%ROOT%\User\%%F.c" -o "%ROOT%\ObjectsSTM32\%%F.o"
  if errorlevel 1 goto fail
)
for %%F in (actuators adc buzzer dht11 esp_uart key oled serial2_printf sx1278) do (
  %CC% -c "%ROOT%\Drivers\%%F.c" -o "%ROOT%\ObjectsSTM32\%%F.o"
  if errorlevel 1 goto fail
)
%CC% -c "%ROOT%\CMSIS\system_stm32f10x.c" -o "%ROOT%\ObjectsSTM32\system_stm32f10x.o"
if errorlevel 1 goto fail
echo All source compiled OK
echo Linking...
"%BIN%\armasm.exe" --cpu Cortex-M3 --apcs=interwork --pd "STM32F10X_MD SETA 1" -g "%ROOT%\CMSIS\startup_stm32f10x_md.s" -o "%ROOT%\ObjectsSTM32\startup.o"
if errorlevel 1 goto fail
"%BIN%\armlink.exe" --cpu Cortex-M3 --entry Reset_Handler --first __Vectors --ro-base 0x08000000 --rw-base 0x20000000 --map --list "%ROOT%\ObjectsSTM32\SmartAgri_STM32.map" --output "%ROOT%\ObjectsSTM32\SmartAgri_STM32.axf" "%ROOT%\ObjectsSTM32\startup.o" "%ROOT%\ObjectsSTM32\system_stm32f10x.o" "%ROOT%\ObjectsSTM32\main.o" "%ROOT%\ObjectsSTM32\board.o" "%ROOT%\ObjectsSTM32\system_tick.o" "%ROOT%\ObjectsSTM32\app_control.o" "%ROOT%\ObjectsSTM32\command_protocol.o" "%ROOT%\ObjectsSTM32\lora_service.o" "%ROOT%\ObjectsSTM32\wifi_service.o" "%ROOT%\ObjectsSTM32\actuators.o" "%ROOT%\ObjectsSTM32\adc.o" "%ROOT%\ObjectsSTM32\buzzer.o" "%ROOT%\ObjectsSTM32\dht11.o" "%ROOT%\ObjectsSTM32\esp_uart.o" "%ROOT%\ObjectsSTM32\key.o" "%ROOT%\ObjectsSTM32\oled.o" "%ROOT%\ObjectsSTM32\serial2_printf.o" "%ROOT%\ObjectsSTM32\sx1278.o"
if errorlevel 1 goto fail
echo Link OK, generating HEX...
"%BIN%\fromelf.exe" --i32combined --output "%ROOT%\ObjectsSTM32\SmartAgri_STM32.hex" "%ROOT%\ObjectsSTM32\SmartAgri_STM32.axf"
if errorlevel 1 goto fail
"%BIN%\fromelf.exe" --text -z "%ROOT%\ObjectsSTM32\SmartAgri_STM32.axf"
echo ===== BUILD SUCCESS =====
dir "%ROOT%\ObjectsSTM32\SmartAgri_STM32.hex"
exit /b 0
:fail
echo ===== BUILD FAILED =====
exit /b 1