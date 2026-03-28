# projects

`projects/` 是平台的项目配置层。

它的职责是描述：

- 当前项目名
- 对应 board
- 对应 app
- 支持的运行模式

当前这层还没有完全驱动 CLI 和 CMake，仍属于保留结构、逐步收口中的配置入口。

当前只保留第一版样板项目：

- `balance_chassis`
