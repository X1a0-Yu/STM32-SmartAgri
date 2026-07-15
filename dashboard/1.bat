@echo off
chcp 65001 >nul
echo ==============================================
echo STC串口乱码自动排查脚本
echo 端口固定 COM6，依次测试 4800 9600 19200 38400
echo 输出十六进制原始数据，方便判断是否通信正常
echo ==============================================
set "PYPATH=D:\CXCY-RJZZQ\STC89C52RC\Works\test1\pytools"

for %%b in (4800 9600 19200 38400) do (
    echo.
    echo ##########################################
    echo 正在测试波特率：%%b
    echo 命令：python -m serial.tools.miniterm COM6 %%b --hex
    echo 按 Ctrl+] 退出当前波特率，自动进入下一档
    echo ##########################################
    set PYTHONPATH=%PYPATH% && python -m serial.tools.miniterm COM6 %%b --hex
)
echo.
echo 全部波特率测试完毕！
pause