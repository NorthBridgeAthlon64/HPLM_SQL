-- 元器件管理
SELECT * FROM component ORDER BY component_id DESC ;

-- BOM
SELECT * FROM productbom ORDER BY bom_id DESC ;

-- 生产批次
SELECT * FROM productionbatch ORDER BY batch_id DESC ;

-- 投料
SELECT * FROM batchmaterial WHERE batch_id = (SELECT MAX(batch_id) FROM productionbatch);

-- 维修
SELECT * FROM repairrecord ORDER BY repair_id DESC ;

-- 溯源（多表 JOIN）
SELECT r.repair_no, pb.batch_no, c.name AS component_name, s.name AS supplier
FROM repairrecord r
LEFT JOIN productionbatch pb ON r.batch_id = pb.batch_id
LEFT JOIN batchmaterial bm ON pb.batch_id = bm.batch_id
LEFT JOIN component c ON bm.component_id = c.component_id
LEFT JOIN supplier s ON c.supplier_id = s.supplier_id
WHERE r.repair_no = 'REP20260001';

-- 评价
SELECT * FROM productfeedback ORDER BY feedback_id DESC ;

-- ==========================================
-- BOM 导入：验证自动创建的元器件和供应商
-- ==========================================

-- BOM导入自动创建的元器件（component_code 以 LCSC- 开头）
SELECT component_id, component_code, name, specification, min_stock, supplier_id
FROM component
WHERE component_code LIKE 'LCSC-%'
ORDER BY component_id DESC ;

-- BOM导入自动创建的供应商（supplier_code 以 AUTO- 开头）
SELECT supplier_id, supplier_code, name, rating
FROM supplier
WHERE supplier_code LIKE 'AUTO-%'
ORDER BY supplier_id DESC ;

-- 按版本查看完整 BOM（含元器件详情和供应商）
SELECT pb.version_id, pv.version_number, p.name AS product_name,
       c.component_code, c.name AS component_name, c.specification,
       pb.quantity, pb.position, s.name AS supplier
FROM productbom pb
JOIN productversion pv ON pb.version_id = pv.version_id
JOIN product p ON pv.product_id = p.product_id
JOIN component c ON pb.component_id = c.component_id
LEFT JOIN supplier s ON c.supplier_id = s.supplier_id
WHERE pb.version_id = (SELECT MAX(version_id) FROM productversion)
ORDER BY pb.bom_id;

-- BOM导入统计（按版本汇总元器件数量和总物料成本）
SELECT pv.version_id, pv.version_number, p.name AS product_name,
       COUNT(pb.bom_id) AS component_count,
       SUM(pb.quantity * COALESCE(c.current_price, 0)) AS total_material_cost
FROM productbom pb
JOIN productversion pv ON pb.version_id = pv.version_id
JOIN product p ON pv.product_id = p.product_id
JOIN component c ON pb.component_id = c.component_id
GROUP BY pv.version_id, pv.version_number, p.name
ORDER BY pv.version_id DESC;