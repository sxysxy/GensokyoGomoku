# GensokyoGomoku
北京交通大学计算机综合训练

五子棋游戏，主要特性：

- 图形化界面
- AI
- 局域网联机对战

代码使用C++语言写成，在Windows平台上Visual Studio 2019 IDE编译。

# 编译代码注意事项：

- C++语言标准C++14及以上
- 第三方库需要: Qt5(5.9), SDL2, SDL2_image, SDL2_net
- 协程的Coroutine实现依赖Windows平台的Windows.h，移植到*nix平台时应注意
