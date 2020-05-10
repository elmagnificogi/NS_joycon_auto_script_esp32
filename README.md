# NS_joycon_auto_script_esp32

## 实现

目前修改了按键映射，移除了ps2手柄支持的部分

- 通过串口就能直接控制

- 修复进入游戏导致的手柄断连

- 目前稳定刷帧大概在3000帧左右，由于蓝牙自身延迟导致任何一个按键操作至少要50ms才能被switch响应
- 增加LED配对提示，未配对闪烁，配对后自动熄灭



## TODO

- 更改手柄颜色
- 手柄稳定长连接，而不需要每次配对，自动重连
- 更改整体数据结构，可以解析全命令
- 分module，封装整个controller
- AP功能
- 在线刷固件
- 远程授权认证，OPT固件支持
- 远程获取脚本更新等等
- 纯串口穿透支持，作为伊机控串口使用



## 来源参考

BlueCubeMod

>  https://github.com/NathanReeves/BlueCubeMod

