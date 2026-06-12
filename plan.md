# HPLM Qt6 上位机 - 代码生成计划

> 基于 01~04 文档的技术选型：Qt6 Widgets + QPSQL + Repository 三层架构 + CMake 3.20+

---

## 一、目录结构

> 根目录：`06_src/`（已存在，不可改名），以下全部文件相对于 `06_src/`

```
06_src/                              ← 根目录（不可改名）
├── README.md                        ← 已存在，需填写
├── CMakeLists.txt                   ← 新建：Qt6 构建脚本
├── config/
│   └── db.ini                       ← 新建：数据库连接配置
├── sql/
│   ├── init.sql                     ← 新建：18张表 DDL（从 02 提取）
│   └── seed.sql                     ← 新建：演示数据
└── src/
    ├── main.cpp                     ← 新建：程序入口
    ├── db/
    │   ├── DatabaseManager.h/.cpp   ← 新建：数据库连接单例
    │   └── repository/
    │       ├── SupplierRepo.h/.cpp
    │       ├── ComponentRepo.h/.cpp
    │       ├── ComponentPriceRepo.h/.cpp
    │       ├── EmployeeRepo.h/.cpp
    │       ├── ProductRepo.h/.cpp
    │       ├── ProductVersionRepo.h/.cpp
    │       ├── VersionCostRepo.h/.cpp
    │       ├── ProductBOMRepo.h/.cpp
    │       ├── ProductionBatchRepo.h/.cpp
    │       ├── BatchMaterialRepo.h/.cpp
    │       ├── TestRecordRepo.h/.cpp
    │       ├── CustomerRepo.h/.cpp
    │       ├── RepairRecordRepo.h/.cpp
    │       ├── RepairMaterialRepo.h/.cpp
    │       ├── WarehouseRepo.h/.cpp
    │       ├── InventoryRepo.h/.cpp
    │       ├── InventoryTransactionRepo.h/.cpp
    │       └── ProductFeedbackRepo.h/.cpp       ← 共 18 个 Repo
    ├── service/
    │   ├── AuthService.h/.cpp
    │   ├── InventoryService.h/.cpp
    │   ├── ProductionService.h/.cpp
    │   ├── RepairService.h/.cpp
    │   ├── TraceService.h/.cpp
    │   └── FeedbackService.h/.cpp               ← 共 6 个 Service
    └── ui/
        ├── MainWindow.h/.cpp
        ├── LoginPage.h/.cpp
        ├── DashboardPage.h/.cpp
        ├── ComponentPage.h/.cpp
        ├── ProductVersionPage.h/.cpp
        ├── ProductionBatchPage.h/.cpp
        ├── RepairPage.h/.cpp
        ├── TracePage.h/.cpp
        └── FeedbackPage.h/.cpp                   ← 共 9 个页面
```

---

## 二、分步生成计划

### Step 0: SQL 脚本 + 配置文件（非代码，数据准备）

- [ ] `sql/init.sql` — 从 02 DDL 提取 18 张表的 `CREATE TABLE IF NOT EXISTS` 语句，合并为一个文件
- [ ] `sql/seed.sql` — 演示数据（至少 2 条可用全集：员工、供应商、元器件、BOM、批次、维修、评价）
- [ ] `config/db.ini` — QSettings 格式，存 host/port/dbName/user/password
- [ ] `README.md` — 填写运行环境、安装步骤、启动方式、项目结构说明

**验证标准**：init.sql 能在 pgAdmin/psql 中直接执行建表成功

---

### Step 1: CMakeLists.txt + main.cpp + config 读取

- [ ] `CMakeLists.txt` — Qt6 Widgets / Sql / Charts / Network，C++17，`find_package` + `qt_add_executable`
- [ ] `src/main.cpp` — 从 `config/db.ini` 读取数据库连接参数，DatabaseManager::connect()，MainWindow 显示，登录拦截逻辑

**验证标准**：编译通过，窗口空白显示

---

### Step 2: 数据库层 — DatabaseManager

- [ ] `src/db/DatabaseManager.h / .cpp`
  - 单例模式
  - `bool connect(host, port, dbName, user, password)` — QPSQL 驱动
  - `QSqlDatabase& db()` — 获取连接
  - `bool execSqlFile(filePath)` — 读取并执行 `sql/init.sql` 或 `sql/seed.sql`
  - 不再内嵌建表 SQL，而是读取 sql/ 目录下的 .sql 文件执行

**验证标准**：C++ 端调用 `execSqlFile("sql/init.sql")` 建表成功，pgAdmin 可见 18 张表

---

### Step 3: Repository 层 — 18 个 Repo（按依赖顺序分批）

每个 Repo 接口统一如下（以 ComponentRepo 为例）：

```cpp
class ComponentRepo {
public:
    explicit ComponentRepo(QSqlDatabase& db);
    QList<Component> findAll();
    Component findById(int id);
    QList<Component> findByFilter(const QString& code, const QString& name);
    bool insert(const Component& c);
    bool update(const Component& c);
    bool remove(int id);
};
```

#### 3.1 基础实体（无外键依赖）

- [ ] `SupplierRepo`
- [ ] `EmployeeRepo` — 含 `findByEmployeeNo(no)` 用于登录
- [ ] `WarehouseRepo`
- [ ] `CustomerRepo`

#### 3.2 元器件相关

- [ ] `ComponentRepo` — 含 `findBySupplier(supplierId)`，`searchByKeyword(kw)`
- [ ] `ComponentPriceRepo`
- [ ] `InventoryRepo` — 含 `findByComponentAndWarehouse(cId, wId)` 联合主键查询
- [ ] `InventoryTransactionRepo`

#### 3.3 产品相关

- [ ] `ProductRepo`
- [ ] `ProductVersionRepo` — 含 `findByProduct(productId)`
- [ ] `VersionCostRepo`
- [ ] `ProductBOMRepo` — 含 `findByVersion(versionId)` JOIN Component

#### 3.4 生产相关

- [ ] `ProductionBatchRepo`
- [ ] `BatchMaterialRepo`
- [ ] `TestRecordRepo`

#### 3.5 维修+评价

- [ ] `RepairRecordRepo`
- [ ] `RepairMaterialRepo`
- [ ] `ProductFeedbackRepo`

**数据层验证**：每个 Repo 编译通过，提供的接口签名与 03 中 35 条 SQL 对应

---

### Step 4: Service 层 — 6 个 Service

#### 4.1 AuthService

- [ ] `bool login(employeeNo, password)` → 查 Employee 表，校验密码，返回登录用户信息
- [ ] `bool registerEmployee(Employee)` → 插入新员工
- 持有 `EmployeeRepo`

#### 4.2 InventoryService

- [ ] `bool inbound(componentId, warehouseId, quantity, unitPrice, operatorId)` — 事务内：
  1. 检查仓库容量（预留）
  2. 加行级锁（`SELECT ... FOR UPDATE`）
  3. 插入 InventoryTransaction
  4. 更新 Component.current_price
  5. 更新 Inventory.quantity 和 avg_cost（移动平均法）
  6. 库存为负回滚
- 持有 `InventoryRepo`, `InventoryTransactionRepo`, `ComponentRepo`

#### 4.3 ProductionService

- [ ] `bool createBatch(batchNo, versionId, quantity, testerId)` — 事务内：
  1. 从 ProductBOM 生成投料清单
  2. 逐项检查库存充足性（任一不足则回滚并返回缺料清单）
  3. 加所有相关库存行锁
  4. 逐项扣减库存 + 插入 BatchMaterial
  5. 计算批次物料成本，更新 ProductionBatch
- 持有 `ProductionBatchRepo`, `BatchMaterialRepo`, `ProductBOMRepo`, `InventoryRepo`, `InventoryTransactionRepo`

#### 4.4 RepairService

- [ ] `bool createRepair(...)` — 创建维修工单
- [ ] `bool addRepairMaterial(repairId, componentId, quantity, unitPrice)` — 事务内：
  1. 检查库存
  2. 扣减库存
  3. 插入 RepairMaterial
  4. 累加 repair_cost 到 RepairRecord
- [ ] `bool completeRepair(repairId)` — 更新状态为 completed
- 持有 `RepairRecordRepo`, `RepairMaterialRepo`, `InventoryRepo`, `InventoryTransactionRepo`

#### 4.5 TraceService

- [ ] `TraceResult traceByRepairNo(repairNo)` — 6 表 JOIN（对应 03 P-07 SQL）：
  ```sql
  RepairRecord → ProductVersion → ProductionBatch
               → BatchMaterial → Component → Supplier
               → TestRecord (同批次其他测试)
  ```
  返回结构体包含完整溯源链条
- `TraceResult traceByBatchNo(batchNo)` — 按批次溯源
- 持有多个 Repo（只读，无事务）

#### 4.6 FeedbackService

- [ ] `bool submitFeedback(versionId, customerId, rating, comment)` — 事务内：
  1. 校验评分 1-5
  2. 检查重复评价（同客户+同版本）
  3. 加行级锁
  4. 插入 ProductFeedback
  5. 更新 ProductVersion 平均评分（可选）
- 持有 `ProductFeedbackRepo`, `ProductVersionRepo`

**Service 层验证**：每个方法能在事务中正确完成多表原子操作，模拟并发场景不丢数据

---

### Step 5: MainWindow + 8 个页面

#### 5.1 MainWindow

- [ ] `MainWindow` — QTabWidget 容器，8 个 Tab
  - 构造函数接收 `Employee`（当前登录用户）
  - 根据 `position` 控制 Tab 可见性（admin 全可见，普通员工限制）
  - QStatusBar 显示当前用户 + 数据库连接状态
  - 未登录时只显示登录页（QStackedWidget 切换）

#### 5.2 P-01 登录/注册页

- [ ] `LoginPage`
  - QLineEdit 工号 + QLineEdit 密码（EchoMode::Password）
  - 登录按钮 → AuthService::login()
  - 注册按钮 → 弹出 QDialog 表单（姓名、职位、电话、邮箱、密码）
  - 登录成功后 emit `loginSucceeded(Employee)` 信号

#### 5.3 P-02 仪表盘

- [ ] `DashboardPage`
  - 低库存预警卡片 → `ComponentRepo` + `InventoryRepo`（quantity < min_stock）
  - 待处理维修工单数量 → `RepairRecordRepo`（status IN received/in_progress）
  - 近期批次质量统计 → `ProductionBatchRepo`（30 天内 GROUP BY quality_status）
  - 可选：QtCharts 饼图展示质量分布

#### 5.4 P-03 元器件管理页

- [ ] `ComponentPage`
  - 顶部搜索栏：编码 + 名称 + 供应商下拉
  - QTableView 展示：编码、名称、规格、单位、当前库存、当前价格、供应商、最低库存
  - 新增/编辑 → QDialog 表单
  - 双击某行 → 弹出库存流水明细（InventoryTransaction）
  - 对应 03 映射表 SQL：P03-1~P03-5

#### 5.5 P-04 产品版本与BOM配置页

- [ ] `ProductVersionPage`
  - QSplitter 布局
  - 左：QTreeView 显示产品 → 版本树（Product → ProductVersion）
  - 右：QTableView 显示选中版本的 BOM 清单（Component + 用量 + PCB位置）
  - 工具栏：新建版本、添加元器件到BOM、修改用量、删除BOM行
  - 底部：预估单台成本 + 建议售价（只读，BOM 变更后自动刷新）
  - 对应 03 SQL：P04-1~P04-7

#### 5.6 P-05 生产批次管理与测试页

- [ ] `ProductionBatchPage`
  - 批次列表 QTableView（批次号、版本号、产品名、投产数量、质量状态、生产日期）
  - 创建批次 → QDialog：选产品版本 + 批次号 + 投产数量
    - 调用 `ProductionService::createBatch()`，失败则弹窗列出缺料明细
  - 选中批次 → 下方显示投料明细 QTableView（BatchMaterial JOIN Component）
  - 选中批次 → 测试记录 QTableView
  - 添加测试 → QDialog：测试项目 + 结果 + 数据
  - 对应 03 SQL：P05-1~P05-5

#### 5.7 P-06 维修工单管理页

- [ ] `RepairPage`
  - 维修工单列表 QTableView（单号、客户、版本、接修日期、状态、费用）
  - 创建工单 → QDialog：选客户 + 版本 + 批次（可选）+ 故障描述
  - 选中工单 → 下方维修用料明细 QTableView
  - 添加换料 → QDialog：选元器件 + 数量 + 单价
    - 调用 `RepairService::addRepairMaterial()`，库存不足弹窗提示
  - 完成维修按钮 → `RepairService::completeRepair()`
  - 对应 03 SQL：P06-1~P06-5

#### 5.8 P-07 故障溯源分析页（核心）

- [ ] `TracePage`
  - 顶部：QLineEdit 输入维修单号或批次号 + 搜索按钮
  - 主体：QTextBrowser 展示完整溯源链路（富文本，缩进表示层级）
    ```
    ┌ 维修单号: R20240001
    ├─ 产品版本: 产品A v2.0
    ├─ 生产批次: B20240001 (投产 100 台, 质量: qualified)
    │   ├─ 元器件: RES-001 10KΩ电阻 × 50pcs @ ¥0.05
    │   │   └─ 供应商: 深圳华强北电子 (评级: 4★)
    │   ├─ 元器件: CAP-002 100μF电容 × 200pcs @ ¥0.30
    │   │   └─ 供应商: 上海元件商城 (评级: 5★)
    │   └─ ...
    └─ 同批次其他产品测试记录:
        ├─ 测试1: 电源测试 → pass
        └─ 测试2: 信号完整性 → pass
    ```
  - 调用 `TraceService::traceByRepairNo()` 或 `traceByBatchNo()`
  - 导出报告按钮 → 生成纯文本文件
  - 对应 03 SQL：P07-1~P07-3

#### 5.9 P-08 客户评价与反馈页

- [ ] `FeedbackPage`
  - 两种模式（QTabWidget 内部切换）：客户视角 / 管理视角
  - 客户视角：
    - 选择已购版本（从 RepairRecord 中取 completed 的）
    - 评分 QSlider/星标 + 评论文本
    - 提交 → `FeedbackService::submitFeedback()`
  - 管理视角：
    - 版本评分统计表（版本号、平均分、评价数）
    - 低评分筛选（<3 星）QTableView
    - 审核评价（pending → approved/rejected）
  - 对应 03 SQL：P08-1~P08-6

**UI 层验证**：每个页面能独立运行，通过 Service 读写数据库，与 03 映射表逐项吻合

---

### Step 6: 演示数据 + 集成联调

- [ ] `sql/seed.sql` — 插入至少 2 条可用的全套演示数据：
  - 2 个员工（admin + 普通员工）
  - 3 个供应商
  - 10+ 个元器件
  - 2 个产品 × 每个 2 个版本
  - 每版本 3-5 行 BOM
  - 2 个生产批次（含投料 + 测试记录）
  - 1 个维修工单（含换料）
  - 2 条客户评价
- [ ] 程序启动时自动执行 `execSqlFile("sql/seed.sql")`（幂等，INSERT ... ON CONFLICT DO NOTHING）
- [ ] 验证 03 全部 35 条 SQL 均可通过 UI 触发执行

---

## 三、生成顺序总结

| 优先级 | Step | 产出 |
|--------|------|------|
| 0 | SQL脚本 + 配置 | `sql/init.sql` `sql/seed.sql` `config/db.ini` `README.md` |
| 1 | CMake + main | 编译骨架 + 读取 db.ini |
| 2 | DatabaseManager | 数据库连接 + 通过 execSqlFile 建表 |
| 3.1 | 4 个基础 Repo | 登录和基础查询 |
| 3.2~3.5 | 剩余 14 个 Repo | 完整数据访问层 |
| 4 | 6 个 Service | 事务 + 业务逻辑 |
| 5 | 9 个页面 | 完整 GUI |
| 6 | SQL种子数据 + 联调 | 可录屏状态 |

---

## 四、关键设计约束

1. **不直接使用 QSqlTableModel 绑 UI**，所有数据访问经 Repository → Service，原因见 04 第 15 次交互
2. **事务保护**：ProductionService / RepairService 涉及多表写操作，必须 BEGIN→操作→COMMIT/ROLLBACK
3. **乐观锁**：Inventory 表加 version 列，并发修改时检测冲突（在 01 流程图中已标注）
4. **密码**：DDL 注释写 SHA-256，课程作业级可用明文对比（01/04 讨论过）
5. **代码注释用中文**（匹配项目文档语言），变量/类名用英文

---

## 五、后续扩展预留

- `AiQueryEngine` 类（testSQL）：QNetworkAccessManager → LLM API，自然语言 → SQL → QTableView
- 端侧加密：Qt Network SSL 绑定
- 这些在 Step 5 之后单独追加，不影响核心流程
