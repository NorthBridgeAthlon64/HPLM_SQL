-- ============================================
-- HPLM 演示数据脚本（幂等，可重复执行）
-- ============================================

-- ============================================
-- 员工数据（2人）
-- ============================================
INSERT INTO Employee (employee_no, name, password, position, phone, email) VALUES
('EMP001', '赵晟闻', 'admin123', 'BOSS', '13800000001', 'zsw@hplm.com'),
('EMP002', '王工',     'user123',  '研发者', '13800000002', 'wang@hplm.com')
ON CONFLICT (employee_no) DO NOTHING;

-- ============================================
-- 供应商数据（3家）
-- ============================================
INSERT INTO Supplier (supplier_code, name, contact_person, phone, address, rating) VALUES
('SUP001', '深圳华强北电子', '李经理', '0755-11111111', '深圳市福田区华强北路1001号', 4),
('SUP002', '上海元件商城',   '张经理', '021-22222222',  '上海市浦东新区张江路200号',   5),
('SUP003', '北京中发电器',   '陈经理', '010-33333333',  '北京市海淀区中关村大街88号',  3)
ON CONFLICT (supplier_code) DO NOTHING;

-- ============================================
-- 仓库数据（2个）
-- ============================================
INSERT INTO Warehouse (warehouse_code, name, location) VALUES
('WH-A01', '主仓库A区', '1号厂房1层A区'),
('WH-B01', '备用仓库B区', '2号厂房1层B区')
ON CONFLICT (warehouse_code) DO NOTHING;

-- ============================================
-- 元器件数据（12种）
-- ============================================
INSERT INTO Component (component_code, name, specification, unit, min_stock, current_price, supplier_id) VALUES
('C-RES-10K',    '10KΩ贴片电阻',   '0603 ±1% 1/10W',  '个', 500,  0.05,  1),
('C-CAP-100U',   '100μF电解电容',  'Φ8×12mm 16V',    '个', 200,  0.30,  2),
('C-MCU-STM32',  'STM32F407VGT6', 'LQFP-100 ARM M4',  '个', 10,  45.00, 3),
('C-FPGA-Z7',    'XC7Z020-1CLG400', 'BGA-400 Zynq-7000', '个', 5, 320.00, 2),
('C-LED-RED',    '红色LED',        '0805 波长620nm',   '个', 300,  0.10,  1),
('C-CON-USB',    'USB Type-C 母座', 'SMD 16P 沉板式',   '个', 50,   2.50,  1),
('C-REG-1117',   'AMS1117-3.3V',  'SOT-223 LDO稳压器', '个', 100,  0.80,  2),
('C-XTAL-8M',    '8MHz无源晶振',   'SMD 3225 20ppm',   '个', 80,   1.20,  3),
('C-FLASH-W25Q', 'W25Q128JVSIQ',  'SOIC-8 128Mbit NOR Flash', '个', 30, 6.50, 3),
('C-DDR-256M',   'DDR3L 256M×16', 'FBGA-96 1866Mbps',  '个', 10,  28.00, 2),
('C-PCB-4LAYER', '4层PCB板',       'FR4 1.6mm 沉金',   '片', 20,  35.00, 1),
('C-HEATSINK',   '散热铝片',       '40×40×10mm 黑色阳极氧化', '个', 50, 3.00, 1)
ON CONFLICT (component_code) DO NOTHING;

-- ============================================
-- 初始化库存（每种元器件在主仓库A区有货）
-- ============================================
INSERT INTO Inventory (component_id, warehouse_id, quantity, avg_cost)
SELECT c.component_id, w.warehouse_id, c.min_stock * 2, c.current_price
FROM Component c, Warehouse w
WHERE w.warehouse_code = 'WH-A01'
ON CONFLICT (component_id, warehouse_id) DO NOTHING;

-- ============================================
-- 客户数据（3个）
-- ============================================
INSERT INTO Customer (customer_code, name, phone, email, address, tier) VALUES
('CUST001', '深圳创新科技公司', '0755-88888888', 'cx@innovation.cn', '深圳市南山区科技园路99号', 4),
('CUST002', '上海智能制造有限公司', '021-99999999', 'zm@smartmfg.cn', '上海市松江区工业园路66号', 5),
('CUST003', '北京电子研究所', '010-77777777', 'bj@electronics.ac.cn', '北京市海淀区学院路55号', 3)
ON CONFLICT (customer_code) DO NOTHING;

-- ============================================
-- 产品数据（2个产品）
-- ============================================
INSERT INTO Product (product_code, name, description, product_manager_id, designer_id)
SELECT 'PROD-001', '嵌入式AI视觉主板', '基于Zynq-7000的AI边缘计算平台，支持4K视频处理和神经网络推理', e1.employee_id, e2.employee_id
FROM Employee e1, Employee e2
WHERE e1.employee_no = 'EMP001' AND e2.employee_no = 'EMP002'
ON CONFLICT (product_code) DO NOTHING;

INSERT INTO Product (product_code, name, description, product_manager_id, designer_id)
SELECT 'PROD-002', '智能传感器采集模块', '多通道模拟信号采集模块，支持16路传感器输入和RS485通信', e1.employee_id, e2.employee_id
FROM Employee e1, Employee e2
WHERE e1.employee_no = 'EMP001' AND e2.employee_no = 'EMP002'
ON CONFLICT (product_code) DO NOTHING;

-- ============================================
-- 产品版本数据（每个产品2个版本）
-- EMP001=BOSS赵晟闻作为发布工程师
-- ============================================
DO $$
DECLARE
    prod1_id INT;
    prod2_id INT;
    eng_id INT;
BEGIN
    SELECT product_id INTO prod1_id FROM Product WHERE product_code = 'PROD-001';
    SELECT product_id INTO prod2_id FROM Product WHERE product_code = 'PROD-002';
    SELECT employee_id INTO eng_id FROM Employee WHERE employee_no = 'EMP001';

    -- PROD-001 v1.0
    INSERT INTO ProductVersion (product_id, version_number, version_title, version_note, release_date, status, release_engineer_id, total_development_cost, estimated_unit_cost, recommended_price)
    VALUES (prod1_id, 'v1.0', '初始原型版本', '首次打样版本，使用4层PCB，基础功能验证通过', '2026-03-15', 'released', eng_id, 85000.00, 520.00, 899.00)
    ON CONFLICT (product_id, version_number) DO NOTHING;

    -- PROD-001 v2.0
    INSERT INTO ProductVersion (product_id, version_number, version_title, version_note, release_date, status, release_engineer_id, total_development_cost, estimated_unit_cost, recommended_price)
    VALUES (prod1_id, 'v2.0', '量产优化版', '优化散热设计，更换更低功耗DDR3L，BOM成本降低15%', '2026-05-20', 'released', eng_id, 120000.00, 445.00, 799.00)
    ON CONFLICT (product_id, version_number) DO NOTHING;

    -- PROD-002 v1.0
    INSERT INTO ProductVersion (product_id, version_number, version_title, version_note, release_date, status, release_engineer_id, total_development_cost, estimated_unit_cost, recommended_price)
    VALUES (prod2_id, 'v1.0', '初版设计', '16通道采集，采样率1kSPS', '2026-04-01', 'released', eng_id, 35000.00, 180.00, 350.00)
    ON CONFLICT (product_id, version_number) DO NOTHING;

    -- PROD-002 v1.1
    INSERT INTO ProductVersion (product_id, version_number, version_title, version_note, release_date, status, release_engineer_id, total_development_cost, estimated_unit_cost, recommended_price)
    VALUES (prod2_id, 'v1.1', '精度增强版', '升级ADC为24bit，增加隔离电源模块', '2026-06-01', 'released', eng_id, 48000.00, 210.00, 399.00)
    ON CONFLICT (product_id, version_number) DO NOTHING;
END $$;

-- ============================================
-- BOM数据（每个版本3-5行）
-- ============================================
DO $$
DECLARE
    v_prod1v1 INT; v_prod1v2 INT; v_prod2v1 INT; v_prod2v2 INT;
    c_res INT; c_cap INT; c_mcu INT; c_fpga INT; c_led INT; c_con INT;
    c_reg INT; c_xtal INT; c_flash INT; c_ddr INT; c_pcb INT; c_heat INT;
BEGIN
    SELECT version_id INTO v_prod1v1 FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-001' AND pv.version_number='v1.0';
    SELECT version_id INTO v_prod1v2 FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-001' AND pv.version_number='v2.0';
    SELECT version_id INTO v_prod2v1 FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-002' AND pv.version_number='v1.0';
    SELECT version_id INTO v_prod2v2 FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-002' AND pv.version_number='v1.1';

    SELECT component_id INTO c_res  FROM Component WHERE component_code='C-RES-10K';
    SELECT component_id INTO c_cap  FROM Component WHERE component_code='C-CAP-100U';
    SELECT component_id INTO c_mcu  FROM Component WHERE component_code='C-MCU-STM32';
    SELECT component_id INTO c_fpga FROM Component WHERE component_code='C-FPGA-Z7';
    SELECT component_id INTO c_led  FROM Component WHERE component_code='C-LED-RED';
    SELECT component_id INTO c_con  FROM Component WHERE component_code='C-CON-USB';
    SELECT component_id INTO c_reg  FROM Component WHERE component_code='C-REG-1117';
    SELECT component_id INTO c_xtal FROM Component WHERE component_code='C-XTAL-8M';
    SELECT component_id INTO c_flash FROM Component WHERE component_code='C-FLASH-W25Q';
    SELECT component_id INTO c_ddr  FROM Component WHERE component_code='C-DDR-256M';
    SELECT component_id INTO c_pcb  FROM Component WHERE component_code='C-PCB-4LAYER';
    SELECT component_id INTO c_heat FROM Component WHERE component_code='C-HEATSINK';

    -- PROD-001 v1.0 BOM（5行：FPGA + DDR + Flash + 电容 + PCB）
    INSERT INTO ProductBOM (version_id, component_id, quantity, position) VALUES
    (v_prod1v1, c_fpga,  1,  'U1'),
    (v_prod1v1, c_ddr,   2,  'U2,U3'),
    (v_prod1v1, c_flash, 1,  'U4'),
    (v_prod1v1, c_cap,  15,  'C1-C15'),
    (v_prod1v1, c_pcb,   1,  'PCB')
    ON CONFLICT (version_id, component_id) DO NOTHING;

    -- PROD-001 v2.0 BOM（5行：FPGA + DDR(换成更低功耗) + Flash + 电容 + PCB + 散热片）
    INSERT INTO ProductBOM (version_id, component_id, quantity, position) VALUES
    (v_prod1v2, c_fpga,  1,  'U1'),
    (v_prod1v2, c_ddr,   2,  'U2,U3'),
    (v_prod1v2, c_flash, 1,  'U4'),
    (v_prod1v2, c_cap,  12,  'C1-C12'),
    (v_prod1v2, c_pcb,   1,  'PCB'),
    (v_prod1v2, c_heat,  1,  'HS1')
    ON CONFLICT (version_id, component_id) DO NOTHING;

    -- PROD-002 v1.0 BOM（4行：MCU + 电阻 + LED + PCB）
    INSERT INTO ProductBOM (version_id, component_id, quantity, position) VALUES
    (v_prod2v1, c_mcu,  1,  'U1'),
    (v_prod2v1, c_res,  20, 'R1-R20'),
    (v_prod2v1, c_led,   4, 'LED1-LED4'),
    (v_prod2v1, c_pcb,   1, 'PCB')
    ON CONFLICT (version_id, component_id) DO NOTHING;

    -- PROD-002 v1.1 BOM（5行：MCU + 电阻 + LED + USB + 稳压器 + PCB）
    INSERT INTO ProductBOM (version_id, component_id, quantity, position) VALUES
    (v_prod2v2, c_mcu,  1,  'U1'),
    (v_prod2v2, c_res,  18, 'R1-R18'),
    (v_prod2v2, c_led,   6, 'LED1-LED6'),
    (v_prod2v2, c_con,   1, 'J1'),
    (v_prod2v2, c_reg,   2, 'U2,U3'),
    (v_prod2v2, c_pcb,   1, 'PCB')
    ON CONFLICT (version_id, component_id) DO NOTHING;
END $$;

-- ============================================
-- 生产批次（2个批次，基于 PROD-001 v2.0 和 PROD-002 v1.1）
-- ============================================
DO $$
DECLARE
    v_prod1v2 INT; v_prod2v2 INT;
    tester_id INT; emp2_id INT;
    batch1_id INT; batch2_id INT;
    c_fpga INT; c_ddr INT; c_flash INT; c_cap INT; c_pcb INT; c_heat INT;
    c_mcu INT; c_res INT; c_led INT; c_con INT; c_reg INT;
BEGIN
    SELECT version_id INTO v_prod1v2 FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-001' AND pv.version_number='v2.0';
    SELECT version_id INTO v_prod2v2 FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-002' AND pv.version_number='v1.1';
    SELECT employee_id INTO tester_id FROM Employee WHERE employee_no = 'EMP001';
    SELECT employee_id INTO emp2_id FROM Employee WHERE employee_no = 'EMP002';

    -- 批次1：PROD-001 v2.0 投产100台
    INSERT INTO ProductionBatch (batch_no, version_id, quantity, production_date, tester_id, quality_status, batch_material_cost, batch_labor_cost, batch_total_cost)
    VALUES ('B20240001', v_prod1v2, 100, '2026-05-25', tester_id, 'qualified', 38000.00, 5000.00, 43000.00)
    ON CONFLICT (batch_no) DO NOTHING;
    SELECT batch_id INTO batch1_id FROM ProductionBatch WHERE batch_no = 'B20240001';

    -- 批次2：PROD-002 v1.1 投产50台
    INSERT INTO ProductionBatch (batch_no, version_id, quantity, production_date, tester_id, quality_status, batch_material_cost, batch_labor_cost, batch_total_cost)
    VALUES ('B20240002', v_prod2v2, 50, '2026-06-03', emp2_id, 'qualified', 10500.00, 2000.00, 12500.00)
    ON CONFLICT (batch_no) DO NOTHING;
    SELECT batch_id INTO batch2_id FROM ProductionBatch WHERE batch_no = 'B20240002';

    -- 元器件ID
    SELECT component_id INTO c_fpga FROM Component WHERE component_code='C-FPGA-Z7';
    SELECT component_id INTO c_ddr  FROM Component WHERE component_code='C-DDR-256M';
    SELECT component_id INTO c_flash FROM Component WHERE component_code='C-FLASH-W25Q';
    SELECT component_id INTO c_cap  FROM Component WHERE component_code='C-CAP-100U';
    SELECT component_id INTO c_pcb  FROM Component WHERE component_code='C-PCB-4LAYER';
    SELECT component_id INTO c_heat FROM Component WHERE component_code='C-HEATSINK';
    SELECT component_id INTO c_mcu  FROM Component WHERE component_code='C-MCU-STM32';
    SELECT component_id INTO c_res  FROM Component WHERE component_code='C-RES-10K';
    SELECT component_id INTO c_led  FROM Component WHERE component_code='C-LED-RED';
    SELECT component_id INTO c_con  FROM Component WHERE component_code='C-CON-USB';
    SELECT component_id INTO c_reg  FROM Component WHERE component_code='C-REG-1117';

    -- 批次1投料
    INSERT INTO BatchMaterial (batch_id, component_id, used_quantity, unit_cost_at_time) VALUES
    (batch1_id, c_fpga,  100, 320.00),
    (batch1_id, c_ddr,   200,  28.00),
    (batch1_id, c_flash, 100,   6.50),
    (batch1_id, c_cap,  1200,   0.30),
    (batch1_id, c_pcb,   100,  35.00),
    (batch1_id, c_heat,  100,   3.00)
    ON CONFLICT (batch_id, component_id) DO NOTHING;

    -- 批次2投料
    INSERT INTO BatchMaterial (batch_id, component_id, used_quantity, unit_cost_at_time) VALUES
    (batch2_id, c_mcu,   50, 45.00),
    (batch2_id, c_res,  900,  0.05),
    (batch2_id, c_led,  300,  0.10),
    (batch2_id, c_con,   50,  2.50),
    (batch2_id, c_reg,  100,  0.80),
    (batch2_id, c_pcb,   50, 35.00)
    ON CONFLICT (batch_id, component_id) DO NOTHING;

    -- 测试记录
    INSERT INTO TestRecord (batch_id, tester_id, test_date, test_item, test_result, test_data) VALUES
    (batch1_id, tester_id, '2026-05-26', '电源上电测试',    'pass', '{"voltage_3v3":3.31,"voltage_1v8":1.79,"current_idle":0.42}'),
    (batch1_id, tester_id, '2026-05-26', 'FPGA JTAG连通性',  'pass', '{"device_detected":"XC7Z020","ir_length":6}'),
    (batch1_id, tester_id, '2026-05-27', 'DDR3内存测试',     'pass', '{"freq":1866,"errors":0,"latency_cl":13}'),
    (batch2_id, emp2_id,  '2026-06-04', 'ADC精度校准',     'pass', '{"channel_count":16,"max_error_mv":2.3}'),
    (batch2_id, emp2_id,  '2026-06-04', 'RS485通信测试',     'pass', '{"baud_rate":115200,"error_rate":0}')
    ON CONFLICT DO NOTHING;
END $$;

-- ============================================
-- 维修工单（1个，关联批次1 + 客户1）
-- ============================================
DO $$
DECLARE
    cust1_id INT; batch1_id INT; ver1v2_id INT; repair1_id INT;
    emp2_id INT;
    c_fpga INT; c_cap INT;
BEGIN
    SELECT customer_id INTO cust1_id FROM Customer WHERE customer_code = 'CUST001';
    SELECT batch_id INTO batch1_id FROM ProductionBatch WHERE batch_no = 'B20240001';
    SELECT version_id INTO ver1v2_id FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-001' AND pv.version_number='v2.0';
    SELECT employee_id INTO emp2_id FROM Employee WHERE employee_no = 'EMP002';

    INSERT INTO RepairRecord (repair_no, version_id, batch_id, customer_id, receive_date, fault_description, repair_status, repairman_id, repair_cost)
    VALUES ('REP20260001', ver1v2_id, batch1_id, cust1_id, '2026-06-10', '设备上电后FPGA指示灯不亮，3.3V电压输出异常，疑似LDO损坏导致FPGA供电不足', 'completed', emp2_id, 0)
    ON CONFLICT (repair_no) DO NOTHING;
    SELECT repair_id INTO repair1_id FROM RepairRecord WHERE repair_no = 'REP20260001';

    -- 换料
    SELECT component_id INTO c_fpga FROM Component WHERE component_code='C-FPGA-Z7';
    SELECT component_id INTO c_cap  FROM Component WHERE component_code='C-CAP-100U';

    INSERT INTO RepairMaterial (repair_id, component_id, quantity, unit_cost_at_time) VALUES
    (repair1_id, c_fpga, 1, 320.00),
    (repair1_id, c_cap,  5,   0.30)
    ON CONFLICT DO NOTHING;

    -- 更新维修总费用
    UPDATE RepairRecord SET repair_cost = 321.50 WHERE repair_id = repair1_id;
END $$;

-- ============================================
-- 客户评价（2条）
-- ============================================
DO $$
DECLARE
    cust1_id INT; cust2_id INT;
    ver1v2_id INT; ver2v2_id INT;
BEGIN
    SELECT customer_id INTO cust1_id FROM Customer WHERE customer_code = 'CUST001';
    SELECT customer_id INTO cust2_id FROM Customer WHERE customer_code = 'CUST002';
    SELECT version_id INTO ver1v2_id FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-001' AND pv.version_number='v2.0';
    SELECT version_id INTO ver2v2_id FROM ProductVersion pv JOIN Product p ON pv.product_id=p.product_id WHERE p.product_code='PROD-002' AND pv.version_number='v1.1';

    INSERT INTO ProductFeedback (version_id, customer_id, rating, comment, feedback_date, status) VALUES
    (ver1v2_id, cust1_id, 4, 'AI推理性能不错，但散热片尺寸偏大导致安装不便，建议下一版优化结构', '2026-06-08', 'approved'),
    (ver2v2_id, cust2_id, 5, '采集精度很好，USB供电即插即用很方便，用于我们产线环境非常稳定', '2026-06-11', 'approved')
    ON CONFLICT (version_id, customer_id) DO NOTHING;
END $$;
