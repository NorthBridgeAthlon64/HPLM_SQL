# plan1: BOM CSV 文件导入功能

> 基于两个真实嘉立创 EDA 导出的 BOM 示例，验证兼容性

---

## 零、双文件验证结果

| | Server_PCB | Board1 | 结论 |
|------|:---:|:---:|------|
| 总行数 | 60 | 58 | |
| 列数 | 10 | 10 | 一致 |
| 分隔符 | Tab | Tab | 一致 |
| 空 Manufacturer Part | 17 (28%) | 25 (43%) | 通用料特征 |
| 空 Manufacturer | 21 (35%) | 27 (47%) | 同上 |
| 两者都有 | 39 (65%) | 31 (53%) | |
| Designator 含逗号 | ✓ | ✓ | 必须 Tab 分列 |

**结论：两个文件格式完全一致，plan1 的解析代码无需任何修改即可兼容两者。** 嘉立创 EDA 导出的 BOM 使用固定的 10 列 Tab 分隔格式。

---

## 一、真实 CSV 格式

```
列 1: No.           序号
列 2: Quantity      单台用量
列 3: Comment       元器件描述/名称
列 4: Designator    位号（如 C1,C20，逗号分隔多个）
列 5: Footprint     封装（如 C0603, R0603, SOP-16）
列 6: Value         参数值（如 10μF, 100nF, 10K）
列 7: Manufacturer Part  制造商型号
列 8: Manufacturer  制造商名称
列 9: Supplier Part 供应商型号（如 LCSC 编号 Cxxxxx）
列10: Supplier      供应商（如 LCSC）
```

特点：
- 分隔符为 **Tab**（TSV），非逗号
- Comment + Footprint + Value 组合唯一标识一种元器件
- 部分行 Manufacturer Part / Manufacturer 为空（通用料如 100nF 电容）
- Designator 含逗号，不能按逗号 split

---

## 二、数据库字段映射

| CSV 列 | DB 表.字段 | 逻辑 |
|--------|-----------|------|
| Comment | `Component.name` | 元器件名称（直接映射） |
| Footprint + Value | `Component.specification` | 规格，合并为 "C0603 100nF" |
| Manufacturer Part | `Component.component_code` | 编码（为空则自动生成） |
| Manufacturer | `Supplier.name` | 供应商（不存在则自动创建） |
| Supplier | — | 忽略（LCSC 为分销商，不存） |
| Quantity | `ProductBOM.quantity` | 单台用量 |
| Designator | `ProductBOM.position` | PCB 位号（保留原始逗号分隔格式） |
| Supplier Part | — | 可忽略（LCSC 编号，非制造商信息） |

---

## 三、导入流程

```
用户点击"导入BOM" → 选择 CSV 文件
  → 解析 TSV（Tab 分隔），跳过空行
  → 逐行匹配数据库：
      ├─ 用 Comment + Footprint 模糊匹配 Component 表（ILIKE）
      ├─ 🟢 完全匹配：自动关联 component_id
      ├─ 🟡 Comment 匹配但 Footprint/Value 不同：标记"疑似替代"
      └─ 🔴 无匹配：标记"需新建"
  → 生成预览表（QTableWidget + 颜色标记）：
      ├─ 绿色背景：已匹配
      ├─ 黄色背景：待确认（名称匹配规格不同）
      └─ 红色背景：缺失（数据库中无此元器件）
  → 用户选择目标产品 + 版本
  → 一键执行：
      1. 对 🔴 行：自动 INSERT Component + Supplier
      2. 对全部行：INSERT ProductBOM（含 quantity + position）
      3. 对全部行：INSERT Inventory（初始化库存为 0，提示补货）
```

---

## 四、文件变更清单

### 4.1 新增文件（2 个）

| 文件 | 说明 |
|------|------|
| `src/service/BOMImportService.h/.cpp` | CSV 解析 + 元器件匹配 + 批量导入 |
| `src/ui/BOMImportDialog.h/.cpp` | 预览对话框（QTableWidget 颜色标记 + 进度条） |

### 4.2 修改文件（3 个）

| 文件 | 变更 |
|------|------|
| `src/ui/ProductVersionPage.h/.cpp` | 工具栏新增"导入BOM"按钮，调用 BOMImportDialog |
| `CMakeLists.txt` | 新增 2 个源文件 |

---

## 五、BOMImportService 接口

```cpp
struct BOMRow {
    int     rowNo;
    int     quantity;
    QString comment;
    QString designator;
    QString footprint;
    QString value;
    QString mfrPart;
    QString manufacturer;
    int     matchedComponentId = 0;  // 0=未匹配
    enum MatchStatus { Matched, Partial, Missing };
    MatchStatus status = Missing;
    QString statusMessage;
};

class BOMImportService : public QObject {
    Q_OBJECT
public:
    explicit BOMImportService(QSqlDatabase &db);

    /// 解析 CSV 文件（TSV 格式），返回 BOM 行列表
    QList<BOMRow> parseCSV(const QString &filePath, QString &error);

    /// 逐行匹配数据库中的元器件
    void matchComponents(QList<BOMRow> &rows);

    /// 执行导入（在事务内）
    bool executeImport(int versionId, const QList<BOMRow> &rows,
                       QStringList &log);

    /// 自动生成 component_code（基于 Footprint + Value）
    static QString generateComponentCode(const BOMRow &row);
};
```

## 六、匹配算法（已修正）

### 6.1 匹配策略（三级递进）

```
第1级: Manufacturer Part 精确匹配
  SELECT * FROM Component WHERE component_code = :mfrPart
  → 命中即 🟢 Matched（制造商型号是最可靠的匹配凭据）

第2级: Comment 精确匹配
  SELECT * FROM Component WHERE name = :comment
  → 命中即 🟢 Matched

第3级: Comment + Footprint + Value 组合匹配
  SELECT * FROM Component WHERE name = :comment
    AND specification ILIKE '%'||:footprint||'%'
    AND specification ILIKE '%'||:value||'%'
  → 命中即 🟡 Partial（名称一致但规格略有差异）

全部未命中 → 🔴 Missing
```

### 6.2 为什么不用 ILIKE

| BOM 行 | ILIKE '%10K%' | 实际想要 |
|--------|:---:|------|
| Comment = "10K" | 匹配到"10KΩ贴片电阻" ✅、也匹配到"PT100K传感器" ❌ | 假阳性 |
| Comment = "1K" | 匹配到"1KΩ电阻" ✅、也匹配到"10KΩ电阻" ❌、"LM1117-1.2V" ❌ | 多假阳性 |

ILIKE 子串匹配在元器件名称中会产生大量假阳性——电阻值"1K"会成为"10K""100K""1KΩ"的子串。改用精确 `=` 匹配，确保只有"名称完全一致"才判定为已匹配。

### 6.3 修正前后对比

| | 旧方案 (ILIKE) | 新方案 (精确+) |
|------|:---:|:---:|
| Comment 匹配 | `name ILIKE '%comment%'` | `name = :comment` |
| 假阳性风险 | 高（1K 匹配到 10K） | 无 |
| Manufacturer Part | 未使用 | 第1级优先匹配 |
| Footprint+Value | 无 | 第3级辅助验证 |
| 准确率估算 | ~30% | ~70% |

---

## 七、BOMImportDialog 界面

```
┌──────────────────────────────────────┐
│  BOM 文件导入                        │
├──────────────────────────────────────┤
│ 文件: [BOM_Server_PCB_PCB1.csv] [浏览] │
│ 目标版本: [产品A v2.0 ▼]             │
├──────────────────────────────────────┤
│ ┌──────────────────────────────┐     │
│ │ # │状态│名称     │用量│位号    │     │
│ │ 1 │ 🟢 │Y电容    │ 2  │C1,C20 │     │
│ │ 2 │ 🟡 │10μF    │15  │C2..C98│     │
│ │ 3 │ 🔴 │DB307S  │ 1  │D1     │     │
│ └──────────────────────────────┘     │
│ 统计: 🟢 已匹配: 12  🟡 待确认: 5    │
│       🔴 需新建: 43                   │
├──────────────────────────────────────┤
│     [导入全部]  [仅导入已匹配]  [取消]  │
└──────────────────────────────────────┘
```

点击"导入全部"时：
1. 🔴 行：INSERT Component (name=comment, specification=footprint+" "+value, component_code=自动生成)
2. 🔴 行含 Manufacturer：查 Supplier 表，不存在则 INSERT
3. 全部行：INSERT ProductBOM (version_id, component_id, quantity, position)
4. 全部行：INSERT Inventory (初始化 quantity=0，avg_cost=NULL)

---

## 八、CSV 文件校验（导入前置）

导入前必须通过校验，**全部通过才允许进入预览界面**，防止污染数据库。

### 8.1 校验规则

| 检查项 | 条件 | 不通过时的提示 |
|--------|------|---------------|
| 编码检查 | UTF-8 可读（无乱码） | "文件编码异常，请确认导出来源为嘉立创 EDA" |
| 列数检查 | 每行恰好 10 列（Tab 分隔） | "第 N 行列数异常: 期望 10, 实际 X" |
| 表头检查 | 首行必须包含 `No.` `Quantity` `Comment` `Designator` `Footprint` `Value` | "表头格式不匹配嘉立创 BOM 标准，请确认导出格式" |
| 数量检查 | Quantity 列可解析为 int ≥ 1（空默认 1） | "第 N 行数量无效: 'xxx'" |
| Comment 检查 | Comment 列不能为空 | "第 N 行缺少元器件名称" |
| 重复检查 | Comment + Footprint 相同的行去重提示 | "第 3 行与第 8 行 Comment+Footprint 相同，将合并" |
| 文件非空 | 至少 1 行数据（不含表头） | "BOM 文件只有表头，无数据行" |

### 8.2 校验流程

```
用户选择文件 → parseCSV() 同时执行校验
  ├─ 编码错误 → 直接拒绝，弹窗提示
  ├─ 列数/表头错误 → 直接拒绝，列出具体行号
  ├─ 数据不合理（注释为空等）→ 直接拒绝
  └─ 全部通过 → 进入预览界面
```

即使有重复行警告也不阻止导入（自动合并 quantity 和 designator）。

---

## 九、BOM CSV 导出功能

从已配置的 ProductVersion 导出 BOM，格式与嘉立创 BOM 完全对齐。

### 9.1 导出接口

```cpp
class BOMExportService : public QObject {
    Q_OBJECT
public:
    explicit BOMExportService(QSqlDatabase &db);
    /// 导出指定版本的 BOM 为 TSV 文件
    bool exportToCSV(int versionId, const QString &filePath, QString &error);
    /// 生成标准嘉立创 BOM 内容
    QString generateBOMContent(int versionId, QString &error);
};
```

### 9.2 导出映射（逆向）

| 数据库字段 | CSV 列 | 说明 |
|-----------|--------|------|
| ROW_NUMBER() | No. | 自动编号 |
| `ProductBOM.quantity` | Quantity | 单台用量 |
| `Component.name` | Comment | 元器件名称 |
| `ProductBOM.position` | Designator | PCB 位号 |
| `Component.specification` 中提取封装部分 | Footprint | 封装 |
| `Component.specification` 中提取值部分 | Value | 参数值 |
| `Component.component_code` | Manufacturer Part | 编码（无则空） |
| `Supplier.name` | Manufacturer | 制造商（无则空） |
| — | Supplier Part | 留空（非制造商信息） |
| — | Supplier | 留空 |

### 9.3 使用入口

MainWindow 顶部菜单栏："文件" → "导出 BOM" → 弹出产品版本选择对话框 → 选择保存路径 → 生成 TSV 文件。

### 9.4 导出时注意事项

1. position 列保留逗号分隔格式（如 `C1,C20`），与导入一致
2. specification 列需要拆分：Footprint = 前段，Value = 后段（按空格拆）
3. 编码自动生成的行，Manufacturer Part 留空即可
4. 文件编码 UTF-8，行尾 `\n`

---

## 十、MainWindow 菜单栏设计

```
┌────────────────────────────────────────────┐
│ 文件(F)  │ 帮助(H)                          │
├────────────────────────────────────────────┤
│ 导出 BOM  │ 选择版本 → 保存 TSV              │
│ 退出      │                                  │
└────────────────────────────────────────────┘
```

---

## 十一、文件变更清单（更新）

### 新增文件（4 个）

| 文件 | 说明 |
|------|------|
| `src/service/BOMImportService.h/.cpp` | CSV 解析 + 校验 + 元器件匹配 + 批量导入 |
| `src/service/BOMExportService.h/.cpp` | BOM 导出为嘉立创格式 TSV |
| `src/ui/BOMImportDialog.h/.cpp` | 导入预览对话框（QTableWidget 颜色标记） |

### 修改文件（4 个）

| 文件 | 变更 |
|------|------|
| `src/ui/MainWindow.h/.cpp` | 新增菜单栏（文件→导出、退出），信号连接 |
| `src/ui/ProductVersionPage.h/.cpp` | 工具栏新增"导入BOM"按钮 |
| `CMakeLists.txt` | 新增 4 个源文件 |

---

## 十二、生成顺序

| Step | 产出 | 状态 |
|------|------|:---:|
| plan1-1 | `BOMImportService.h/.cpp`（含校验） | pending |
| plan1-2 | `BOMExportService.h/.cpp` | pending |
| plan1-3 | `BOMImportDialog.h/.cpp` | pending |
| plan1-4 | `ProductVersionPage` 增加"导入BOM"按钮 | pending |
| plan1-5 | `MainWindow` 增加菜单栏 + "导出BOM" | pending |
| plan1-6 | `CMakeLists.txt` 更新 | pending |
| plan1-7 | 编译验证 + 双 BOM 文件测试 | pending |

---

## 十三、风险评估与对策

### 13.1 风险矩阵

| # | 风险 | 严重度 | 概率 | 对策 |
|---|------|:---:|:---:|------|
| R1 | ILIKE 子串假阳性（1K→10K） | 高 | 高 | 已修正为精确 `=` 匹配（见六） |
| R2 | 重复导入创建重复 Component | 高 | 中 | generateComponentCode() 前查重，已存在则复用 |
| R3 | 导入后库存为 0，触发 Dashboard 告警 | 中 | 中 | 显式设 `min_stock = 0`，避免误告警 |
| R4 | 仓库 ID 硬编码为 1，仓库不存在时失败 | 中 | 低 | 执行前校验仓库存在，不存在则弹窗提示 |
| R5 | 导入 Dialog 关闭后状态残留 | 低 | 低 | Dialog 析构时自动清理，不持久化临时状态 |
| R6 | comment 列含 Tab 字符 | 低 | 极低 | 嘉立创 EDA 不输出含 Tab 的 Comment，忽略 |
| R7 | 导出 specification 拆分不可逆 | 低 | — | 已知不精确，导出只读不写数据库 |
| R8 | 没有 undo，提交后无法回滚 | 中 | — | 事务原子性保证（全成功/全回滚），导入前二次确认弹窗 |
| R9 | 导入完成后用户无反馈 | 低 | 高 | 弹窗显示汇总：✅ 新建 N 个元器件, 📋 导入 M 行 BOM |

### 13.2 边缘情况处理

#### 空 Manufacturer Part 的元器件
```
嘉立创 BOM 第 2 行: "10μF" "C1206" (无 Manufacturer Part)
→ generateComponentCode() 生成 "AUTO-C1206-10μF"
→ 下一次导入遇到同样的 "10μF" "C1206" 时，name 精确匹配 → 复用
→ 如果 name 不匹配（如 "10μF" vs "10μF 16V"），code 查重可兜底
```

#### 仓库不存在的防御
```cpp
// executeImport() 开头
int warehouseId = 1;
QSqlQuery q(m_db);
q.exec("SELECT COUNT(*) FROM Warehouse WHERE warehouse_id = 1");
if (q.next() && q.value(0).toInt() == 0) {
    log.append("错误: 仓库 WH-A01 不存在，请先创建仓库");
    return false;
}
```

#### 重复 Component 检测
```cpp
QString code = generateComponentCode(row);
QSqlQuery q(m_db);
q.prepare("SELECT component_id FROM Component WHERE component_code = :code");
q.bindValue(":code", code);
if (q.exec() && q.next()) {
    // 已存在，复用
    row.matchedComponentId = q.value(0).toInt();
} else {
    // 新建
}
```

#### 导入后汇总反馈
```
导入完成！
✅ 新建元器件: 43 个
📋 导入 BOM 行: 60 行
📦 供应商自动创建: 12 个
⚠ 库存为 0，请在元器件管理页补货
```

### 13.3 用户体验优化

| 优化 | 实现 |
|------|------|
| 导入前二次确认 | "将新建 43 个元器件，导入 60 行 BOM，确认继续？" |
| 导入完成汇总 | 弹窗显示 ✅ 新建 / 📋 导入 / 📦 供应商 数量 |
| 校验失败详情 | QMessageBox::warning 列出具体行号和原因，而非只显示"失败" |
| BOMImportDialog 非模态 | 使用 exec() 模态对话框，关闭即取消，不留临时状态 |
| 自动 code 可读 | "LCSC-" + footprint + "-" + comment，如 "LCSC-C0603-100nF" |

---

## 十四、注意事项

1. Designator 含逗号（如 C1,C20），**不能**用逗号 split CSV 列 — 必须用 Tab 分隔
2. 部分行 Manufacturer Part 为空，此时自动生成 `component_code = "AUTO-" + footprint + "-" + comment` 简化处理；生成前查重，已存在则复用
3. 数量列可能为空，此时默认 `quantity = 1`
4. Comment 列可能含逗号（如 "Y电容,222M,250V,脚距7.5MM"），说明不能用逗号分列
5. 导入后库存初始为 0，`min_stock` 显式设为 0，用户在元器件管理页手动入库补货
6. 两个测试文件均可通过嘉立创 EDA → 导出 BOM 获得，10 列 Tab 分隔是嘉立创的标准格式
7. ~50% 的通用料（电阻、电容）无 Manufacturer Part/Manufacturer 是正常现象，不影响导入
8. **校验失败整批拒绝**，不写入任何数据，锁死"导入全部"按钮直到校验全部通过
9. 导出格式与嘉立创 EDA 导出的 BOM **完全一致**，可实现"导入→编辑→导出→再导入"闭环
10. 导出时规格列（specification）按空格拆分为 Footprint + Value，若无空格则只填 Footprint
11. 匹配改为精确 `=` 而非 ILIKE，Manufacturer Part 作为第1级优先凭据
12. 二次确认对话框防止误操作，导入后汇总弹窗给用户明确反馈
