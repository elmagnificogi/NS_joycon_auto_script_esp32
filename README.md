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



## 架构

- uorb 作为module之间传递数据的通信方式
- inode作为中间层连接c与c++，driver和app层

### module

- controllor 作为整个模拟手柄的控制器，有pro和joyconR,joyconL,joyconGrip
- notify 作为整个的状态显示，其实就是控制led作为运行状态的提示
- console 作为串口指令控制台，用于debug和输出
- ap-client 作为连接wifi的客户端，连接到公网络中
- ap-station 作为连接的 wifi station，允许用户连接进来
- server 
  - http 作为服务器后端，用于更新脚本，同时执行脚本
  - html 作为服务器前端，用于与用户交互，反馈脚本结果或者运行状态
  - virtual com 作为伊机控的透传串口



## 来源参考

BlueCubeMod

>  https://github.com/NathanReeves/BlueCubeMod

