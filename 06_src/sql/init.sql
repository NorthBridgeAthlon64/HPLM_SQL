-- ============================================
-- 硬件产品全生命周期管理系统（HPLM）数据库初始化脚本
-- 目标数据库：PostgreSQL 14+
-- 创建时间：2026年6月
-- 说明：幂等建表（IF NOT EXISTS），可重复执行
-- ============================================

-- 1. 供应商表
CREATE TABLE IF NOT EXISTS Supplier (
    supplier_id     SERIAL PRIMARY KEY,
    supplier_code   VARCHAR(50) NOT NULL UNIQUE,
    name            VARCHAR(100) NOT NULL,
    contact_person  VARCHAR(50),
    phone           VARCHAR(20),
    address         VARCHAR(200),
    rating          INT NOT NULL DEFAULT 3
                    CHECK (rating BETWEEN 1 AND 5)
);

-- 2. 员工表
CREATE TABLE IF NOT EXISTS Employee (
    employee_id     SERIAL PRIMARY KEY,
    employee_no     VARCHAR(50) NOT NULL UNIQUE,
    name            VARCHAR(50) NOT NULL,
    password        VARCHAR(64) NOT NULL,
    position        VARCHAR(50) NOT NULL,
    phone           VARCHAR(20),
    email           VARCHAR(100) UNIQUE
);

-- 3. 客户表
CREATE TABLE IF NOT EXISTS Customer (
    customer_id     SERIAL PRIMARY KEY,
    customer_code   VARCHAR(50) NOT NULL UNIQUE,
    name            VARCHAR(100) NOT NULL,
    phone           VARCHAR(20),
    email           VARCHAR(100),
    address         VARCHAR(200),
    register_date   DATE NOT NULL DEFAULT CURRENT_DATE,
    tier            INT NOT NULL DEFAULT 1
                    CHECK (tier BETWEEN 1 AND 5)
);

-- 4. 产品表
CREATE TABLE IF NOT EXISTS Product (
    product_id          SERIAL PRIMARY KEY,
    product_code        VARCHAR(50) NOT NULL UNIQUE,
    name                VARCHAR(100) NOT NULL,
    description         TEXT,
    product_manager_id  INT REFERENCES Employee(employee_id) ON DELETE SET NULL,
    designer_id         INT REFERENCES Employee(employee_id) ON DELETE SET NULL
);

-- 5. 产品版本表
CREATE TABLE IF NOT EXISTS ProductVersion (
    version_id              SERIAL PRIMARY KEY,
    product_id              INT NOT NULL REFERENCES Product(product_id) ON DELETE CASCADE,
    version_number          VARCHAR(20) NOT NULL,
    version_title           VARCHAR(100),
    version_note            TEXT,
    release_date            DATE,
    status                  VARCHAR(20) NOT NULL DEFAULT 'draft'
                            CHECK (status IN ('draft', 'released', 'deprecated')),
    release_engineer_id     INT REFERENCES Employee(employee_id) ON DELETE SET NULL,
    total_development_cost  DECIMAL(12,2) NOT NULL DEFAULT 0 CHECK (total_development_cost >= 0),
    estimated_unit_cost     DECIMAL(10,2),
    recommended_price       DECIMAL(10,2),
    UNIQUE(product_id, version_number)
);

-- 6. 版本研发成本明细表
CREATE TABLE IF NOT EXISTS VersionCost (
    cost_id             SERIAL PRIMARY KEY,
    version_id          INT NOT NULL REFERENCES ProductVersion(version_id) ON DELETE CASCADE,
    record_employee_id  INT NOT NULL REFERENCES Employee(employee_id),
    cost_type           VARCHAR(50) NOT NULL,
    amount              DECIMAL(12,2) NOT NULL CHECK (amount >= 0),
    cost_date           DATE NOT NULL DEFAULT CURRENT_DATE,
    description         VARCHAR(200)
);

-- 7. 元器件表
CREATE TABLE IF NOT EXISTS Component (
    component_id    SERIAL PRIMARY KEY,
    component_code  VARCHAR(50) NOT NULL UNIQUE,
    name            VARCHAR(100) NOT NULL,
    specification   VARCHAR(200),
    unit            VARCHAR(20) NOT NULL DEFAULT '个',
    min_stock       INT NOT NULL DEFAULT 0 CHECK (min_stock >= 0),
    current_price   DECIMAL(10,2),
    supplier_id     INT REFERENCES Supplier(supplier_id) ON DELETE SET NULL
);

-- 8. 元器件历史价格表
CREATE TABLE IF NOT EXISTS ComponentPrice (
    price_id        SERIAL PRIMARY KEY,
    component_id    INT NOT NULL REFERENCES Component(component_id) ON DELETE CASCADE,
    unit_price      DECIMAL(10,2) NOT NULL,
    effective_date  DATE NOT NULL DEFAULT CURRENT_DATE,
    remark          VARCHAR(200)
);

-- 9. 仓库表
CREATE TABLE IF NOT EXISTS Warehouse (
    warehouse_id    SERIAL PRIMARY KEY,
    warehouse_code  VARCHAR(50) NOT NULL UNIQUE,
    name            VARCHAR(100) NOT NULL,
    location        VARCHAR(200)
);

-- 10. 库存表
CREATE TABLE IF NOT EXISTS Inventory (
    inventory_id        SERIAL PRIMARY KEY,
    component_id        INT NOT NULL REFERENCES Component(component_id) ON DELETE CASCADE,
    warehouse_id        INT NOT NULL REFERENCES Warehouse(warehouse_id) ON DELETE CASCADE,
    quantity            INT NOT NULL DEFAULT 0 CHECK (quantity >= 0),
    reserved_quantity   INT NOT NULL DEFAULT 0 CHECK (reserved_quantity >= 0),
    avg_cost            DECIMAL(10,2),
    UNIQUE(component_id, warehouse_id)
);

-- 11. 库存流水表
CREATE TABLE IF NOT EXISTS InventoryTransaction (
    transaction_id     SERIAL PRIMARY KEY,
    transaction_no     VARCHAR(50) NOT NULL UNIQUE,
    component_id       INT NOT NULL REFERENCES Component(component_id),
    warehouse_id       INT NOT NULL REFERENCES Warehouse(warehouse_id),
    transaction_type   VARCHAR(20) NOT NULL
                       CHECK (transaction_type IN ('in', 'out', 'return', 'transfer')),
    quantity           INT NOT NULL CHECK (quantity <> 0),
    unit_price         DECIMAL(10,2),
    total_price        DECIMAL(12,2),
    transaction_date   DATE NOT NULL DEFAULT CURRENT_DATE,
    reference_no       VARCHAR(50),
    operator_id        INT NOT NULL REFERENCES Employee(employee_id)
);

-- 12. 产品BOM表
CREATE TABLE IF NOT EXISTS ProductBOM (
    bom_id          SERIAL PRIMARY KEY,
    version_id      INT NOT NULL REFERENCES ProductVersion(version_id) ON DELETE CASCADE,
    component_id    INT NOT NULL REFERENCES Component(component_id) ON DELETE RESTRICT,
    quantity        INT NOT NULL DEFAULT 1 CHECK (quantity > 0),
    position        VARCHAR(500),
    UNIQUE(version_id, component_id)
);

-- 13. 生产批次表
CREATE TABLE IF NOT EXISTS ProductionBatch (
    batch_id            SERIAL PRIMARY KEY,
    batch_no            VARCHAR(50) NOT NULL UNIQUE,
    version_id          INT NOT NULL REFERENCES ProductVersion(version_id),
    quantity            INT NOT NULL CHECK (quantity > 0),
    production_date     DATE NOT NULL DEFAULT CURRENT_DATE,
    tester_id           INT REFERENCES Employee(employee_id) ON DELETE SET NULL,
    quality_status      VARCHAR(20) NOT NULL DEFAULT 'pending'
                        CHECK (quality_status IN ('pending', 'qualified', 'defective', 'rework')),
    batch_material_cost DECIMAL(12,2),
    batch_labor_cost    DECIMAL(12,2) NOT NULL DEFAULT 0 CHECK (batch_labor_cost >= 0),
    batch_total_cost    DECIMAL(12,2)
);

-- 14. 批次投料表
CREATE TABLE IF NOT EXISTS BatchMaterial (
    batch_material_id   SERIAL PRIMARY KEY,
    batch_id            INT NOT NULL REFERENCES ProductionBatch(batch_id) ON DELETE CASCADE,
    component_id        INT NOT NULL REFERENCES Component(component_id) ON DELETE RESTRICT,
    used_quantity       INT NOT NULL CHECK (used_quantity > 0),
    unit_cost_at_time   DECIMAL(10,2) NOT NULL,
    total_cost          DECIMAL(12,2) GENERATED ALWAYS AS (used_quantity * unit_cost_at_time) STORED,
    UNIQUE(batch_id, component_id)
);

-- 15. 测试记录表
CREATE TABLE IF NOT EXISTS TestRecord (
    test_id         SERIAL PRIMARY KEY,
    batch_id        INT NOT NULL REFERENCES ProductionBatch(batch_id) ON DELETE CASCADE,
    tester_id       INT NOT NULL REFERENCES Employee(employee_id),
    test_date       DATE NOT NULL DEFAULT CURRENT_DATE,
    test_item       VARCHAR(100) NOT NULL,
    test_result     VARCHAR(20) NOT NULL
                    CHECK (test_result IN ('pass', 'fail', 'retest')),
    test_data       TEXT
);

-- 16. 维修记录表
CREATE TABLE IF NOT EXISTS RepairRecord (
    repair_id           SERIAL PRIMARY KEY,
    repair_no           VARCHAR(50) NOT NULL UNIQUE,
    version_id          INT NOT NULL REFERENCES ProductVersion(version_id),
    batch_id            INT REFERENCES ProductionBatch(batch_id) ON DELETE SET NULL,
    customer_id         INT NOT NULL REFERENCES Customer(customer_id),
    receive_date        DATE NOT NULL DEFAULT CURRENT_DATE,
    complete_date       DATE,
    fault_description   TEXT NOT NULL,
    repair_status       VARCHAR(20) NOT NULL DEFAULT 'received'
                        CHECK (repair_status IN ('received', 'in_progress', 'completed', 'closed')),
    repairman_id        INT REFERENCES Employee(employee_id) ON DELETE SET NULL,
    repair_cost         DECIMAL(10,2) NOT NULL DEFAULT 0 CHECK (repair_cost >= 0)
);

-- 17. 维修用料表
CREATE TABLE IF NOT EXISTS RepairMaterial (
    repair_material_id  SERIAL PRIMARY KEY,
    repair_id           INT NOT NULL REFERENCES RepairRecord(repair_id) ON DELETE CASCADE,
    component_id        INT NOT NULL REFERENCES Component(component_id) ON DELETE RESTRICT,
    quantity            INT NOT NULL CHECK (quantity > 0),
    unit_cost_at_time   DECIMAL(10,2) NOT NULL,
    total_cost          DECIMAL(12,2) GENERATED ALWAYS AS (quantity * unit_cost_at_time) STORED
);

-- 18. 产品反馈评价表
CREATE TABLE IF NOT EXISTS ProductFeedback (
    feedback_id     SERIAL PRIMARY KEY,
    version_id      INT NOT NULL REFERENCES ProductVersion(version_id) ON DELETE CASCADE,
    customer_id     INT NOT NULL REFERENCES Customer(customer_id) ON DELETE CASCADE,
    rating          INT NOT NULL CHECK (rating BETWEEN 1 AND 5),
    comment         TEXT,
    feedback_date   DATE NOT NULL DEFAULT CURRENT_DATE,
    status          VARCHAR(20) NOT NULL DEFAULT 'pending'
                    CHECK (status IN ('pending', 'approved', 'rejected')),
    UNIQUE(version_id, customer_id)
);
