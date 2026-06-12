# 硬件产品全生命周期管理系统（HPLM）

## 运行环境

- PostgreSQL 14+
- Qt 6.5 LTS（Widgets / Sql / Charts / Network 模块）
- CMake 3.20+
- C++17 编译器（MSVC 2022 / GCC 11+ / Clang 14+）
- QPSQL 驱动（Qt6 SQL PostgreSQL 插件）

## 安装步骤

1. 安装 PostgreSQL，创建数据库：
   ```sql
   CREATE DATABASE hplm;
   ```

2. 初始化数据库表结构：
   ```bash
   psql -U postgres -d hplm -f sql/init.sql
   ```

3. （可选）导入演示数据：
   ```bash
   psql -U postgres -d hplm -f sql/seed.sql
   ```

4. 修改 `config/db.ini` 中的数据库连接参数，匹配你的 PostgreSQL 配置。

5. 配置 Qt6 编译环境（以 vcpkg 为例）：
   ```bash
   vcpkg install qt6[widgets,sql,charts,network]:x64-windows
   ```

6. CMake 构建：
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
   cmake --build .
   ```

## 启动方式

```bash
cd build
./HPLM.exe          # Windows
# 或
./HPLM              # Linux
```

程序启动时自动读取 `config/db.ini` 连接数据库，首次运行会弹出数据库配置对话框。

## 项目结构

```
06_src/
├── README.md                   -- 本文件
├── CMakeLists.txt              -- CMake 构建脚本
├── config/
│   └── db.ini                  -- 数据库连接配置（QSettings 格式）
├── sql/
│   ├── init.sql                -- 数据库初始化脚本（18 张表建表）
│   └── seed.sql                -- 演示数据
└── src/
    ├── main.cpp                -- 程序入口
    ├── db/
    │   ├── DatabaseManager.h/cpp   -- 数据库连接单例
    │   └── repository/             -- 数据访问层（18 个 Repository）
    ├── service/                    -- 业务逻辑层（6 个 Service）
    └── ui/                         -- 界面层（MainWindow + 8 个页面）
```
