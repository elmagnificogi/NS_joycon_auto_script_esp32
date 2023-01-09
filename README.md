# NS_joycon_auto_script_esp32

## 实现

目前修改了按键映射，移除了ps2手柄支持的部分

- 通过串口就能直接控制

- 修复进入游戏导致的手柄断连

- 目前稳定刷帧大概在3000帧左右，由于蓝牙自身延迟导致任何一个按键操作至少要50ms才能被switch响应
- 增加LED配对提示，未配对闪烁，配对后自动熄灭


## 废弃
请使用伊机控
> https://github.com/EasyConNS/EasyMCU_ESP32


## 来源参考

BlueCubeMod

>  https://github.com/NathanReeves/BlueCubeMod

